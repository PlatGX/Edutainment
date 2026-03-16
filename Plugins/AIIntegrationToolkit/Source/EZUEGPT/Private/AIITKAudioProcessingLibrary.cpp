/********************************************************************
 * Copyright (C) 2024 Kaleb Knoettgen
 * All Rights Reserved.
 *
 * Unauthorized reproduction, modification, or distribution of this
 * software is prohibited. Provided "AS IS" without warranty; Kaleb
 * Knoettgen is not liable for any damages arising from its use.
 * All rights not expressly granted are reserved.
 *
 * For more information, visit: www.kalebknoettgen.com
 * Contact: kalebknoettgen@gmail.com
 ********************************************************************/


// =================== AIITKAudioProcessingLibrary.cpp ===================

#include "AIITKAudioProcessingLibrary.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "string"


// Third party open source audio conversion library used for converting mp3 data to wav

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"


FCondensedAlignment UAIITKAudioProcessingLibrary::CondenseTextToDigraphs(
    const TArray<FString>& TextInput,
    const TArray<float>& StartTimes)
{
    FCondensedAlignment CondensedData;

    // Ensure input arrays are non-empty and consistent in size
    if (TextInput.Num() == 0 || StartTimes.Num() != TextInput.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("CondenseTextToDigraphs: Input arrays are inconsistent or empty."));
        return CondensedData;  // Return empty data
    }

    // Digraph map
    TMap<FString, TArray<FString>> DigraphMap = {
        {"th", {"t", "h"}},
        {"ch", {"c", "h"}},
        {"sh", {"s", "h"}},
        {"ph", {"p", "h"}},
        {"ee", {"e", "e"}},
        {"oo", {"o", "o"}},
        {"wh", {"w", "h"}},
        {"gh", {"g", "h"}},
        {"ng", {"n", "g"}},
        {"qu", {"q", "u"}},
        {"ai", {"a", "i"}},
        {"ou", {"o", "u"}}
    };

    // Iterate through the input data
    for (int32 i = 0; i < TextInput.Num(); i++)
    {
        FString CurrentText = TextInput[i];
        float WordStartTime = StartTimes[i];

        // Determine the word's end time from the next word's start time or estimate it
        float WordEndTime;
        if (i + 1 < StartTimes.Num())
        {
            WordEndTime = StartTimes[i + 1];
        }
        else
        {
            // Estimate the duration for the last word
            float EstimatedWordDuration = 0.5f; // Default duration
            if (i > 0)
            {
                // Use the average duration of previous words
                float TotalDuration = StartTimes[i] - StartTimes[0];
                float AverageDuration = TotalDuration / i;
                EstimatedWordDuration = AverageDuration > 0 ? AverageDuration : EstimatedWordDuration;
            }
            WordEndTime = WordStartTime + EstimatedWordDuration;
        }

        // Break each word into individual characters
        TArray<FString> CharactersInWord;
        for (int32 CharIndex = 0; CharIndex < CurrentText.Len(); CharIndex++)
        {
            CharactersInWord.Add(CurrentText.Mid(CharIndex, 1).ToLower());
        }

        // Estimate start times for each character within the word
        TArray<float> CharStartTimes;
        int32 NumChars = CharactersInWord.Num();
        for (int32 CharIndex = 0; CharIndex < NumChars; CharIndex++)
        {
            float CharFraction = (float)CharIndex / NumChars;
            float CharStartTime = WordStartTime + CharFraction * (WordEndTime - WordStartTime);
            CharStartTimes.Add(CharStartTime);
        }

        // Apply digraph condensing logic
        int32 j = 0;
        while (j < CharactersInWord.Num())
        {
            FString CurrentChar = CharactersInWord[j];
            float CharStartTime = CharStartTimes[j];
            bool MatchedDigraph = false;

            // Try to match the current char and the next char as a digraph
            if (j + 1 < CharactersInWord.Num())
            {
                FString NextChar = CharactersInWord[j + 1];

                for (const auto& Digraph : DigraphMap)
                {
                    if (Digraph.Value[0] == CurrentChar && Digraph.Value[1] == NextChar)
                    {
                        // Found a digraph
                        FString CondensedChar = Digraph.Key;

                        // Add the digraph and its start time
                        CondensedData.Characters.Add(CondensedChar);
                        CondensedData.StartTimes.Add(CharStartTime);

                        UE_LOG(LogTemp, Log, TEXT("Condensed Digraph: %s, StartTime: %.2f"), *CondensedChar, CharStartTime);

                        // Skip the next character
                        j += 2;
                        MatchedDigraph = true;
                        break;
                    }
                }
            }

            // If no digraph matched, add the single character
            if (!MatchedDigraph)
            {
                CondensedData.Characters.Add(CurrentChar);
                CondensedData.StartTimes.Add(CharStartTime);

                UE_LOG(LogTemp, Log, TEXT("Single Character: %s, StartTime: %.2f"), *CurrentChar, CharStartTime);

                j++;
            }
        }
    }

    // Log the condensed output
    UE_LOG(LogTemp, Log, TEXT("CondenseTextToDigraphs: Output Condensed Data:"));
    for (int32 Index = 0; Index < CondensedData.Characters.Num(); Index++)
    {
        UE_LOG(LogTemp, Log, TEXT("Character[%d]: %s, StartTime: %.2f"), Index, *CondensedData.Characters[Index], CondensedData.StartTimes[Index]);
    }

    return CondensedData;
}


FCharacterAlignment UAIITKAudioProcessingLibrary::CondensePhonemesToDigraphs(const FCharacterAlignment& AlignmentData)
{
    // Updated list of digraphs you want to condense to
    TMap<FString, TArray<FString>> DigraphMap = {
        {"th", {"t", "h"}},
        {"ch", {"c", "h"}},
        {"sh", {"s", "h"}},
        {"ph", {"p", "h"}},
        {"ee", {"e", "e"}},
        {"oo", {"o", "o"}},
        {"wh", {"w", "h"}},
        {"gh", {"g", "h"}},
        {"ng", {"n", "g"}},
        {"qu", {"q", "u"}},
        {"ai", {"a", "i"}},
        {"ou", {"o", "u"}}
    };

    FCharacterAlignment CondensedData;

    // Check if the input arrays are non-empty and consistent in size
    if (AlignmentData.Characters.Num() == 0 ||
        AlignmentData.StartTimes.Num() != AlignmentData.Characters.Num() ||
        AlignmentData.EndTimes.Num() != AlignmentData.Characters.Num())
    {
        // Log an error or warning for debugging
        UE_LOG(LogTemp, Error, TEXT("CondensePhonemesToDigraphs: AlignmentData arrays are inconsistent or empty."));
        return CondensedData;  // Return empty data
    }

    // First, calculate the total duration from start to end
    float TotalDuration = AlignmentData.EndTimes.Last() - AlignmentData.StartTimes[0];

    if (TotalDuration <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("CondensePhonemesToDigraphs: Total duration is zero or negative."));
        return CondensedData;  // Invalid duration, return empty data
    }

    // Variables to store condensed chars and normalized timings
    FString CondensedChar;
    float StartTime, EndTime;

    // Iterate through the alignment data
    int32 i = 0;
    while (i < AlignmentData.Characters.Num())
    {
        bool MatchedDigraph = false;
        FString CurrentChar = AlignmentData.Characters[i].ToLower();

        // Try to match the current char and the next char as a digraph
        if (i + 1 < AlignmentData.Characters.Num())
        {
            FString NextChar = AlignmentData.Characters[i + 1].ToLower();

            for (const auto& Digraph : DigraphMap)
            {
                if (Digraph.Value[0] == CurrentChar && Digraph.Value[1] == NextChar)
                {
                    // Found a digraph, condense it
                    CondensedChar = Digraph.Key;

                    // Start time is the start time of the first char in the digraph
                    StartTime = AlignmentData.StartTimes[i];
                    // End time is the end time of the second char in the digraph
                    EndTime = AlignmentData.EndTimes[i + 1];

                    // Normalize the start and end times
                    float NormalizedStartTime = (StartTime - AlignmentData.StartTimes[0]) / TotalDuration;
                    float NormalizedEndTime = (EndTime - AlignmentData.StartTimes[0]) / TotalDuration;

                    // Add the digraph and normalized timings to the output struct
                    CondensedData.Characters.Add(CondensedChar);
                    CondensedData.StartTimes.Add(StartTime);
                    CondensedData.EndTimes.Add(EndTime);
                    CondensedData.NormalizedStartTimes.Add(NormalizedStartTime);
                    CondensedData.NormalizedEndTimes.Add(NormalizedEndTime);

                    // Skip the next character since it's part of the digraph
                    i += 2;
                    MatchedDigraph = true;
                    break;
                }
            }
        }

        // If no digraph was matched, treat the character as a single phoneme
        if (!MatchedDigraph)
        {
            CondensedChar = CurrentChar;
            StartTime = AlignmentData.StartTimes[i];
            EndTime = AlignmentData.EndTimes[i];

            // Normalize the start and end times
            float NormalizedStartTime = (StartTime - AlignmentData.StartTimes[0]) / TotalDuration;
            float NormalizedEndTime = (EndTime - AlignmentData.StartTimes[0]) / TotalDuration;

            // Add the single character and normalized timings to the output struct
            CondensedData.Characters.Add(CondensedChar);
            CondensedData.StartTimes.Add(StartTime);
            CondensedData.EndTimes.Add(EndTime);
            CondensedData.NormalizedStartTimes.Add(NormalizedStartTime);
            CondensedData.NormalizedEndTimes.Add(NormalizedEndTime);

            // Move to the next character
            i++;
        }
    }

    return CondensedData;
}



int32 UAIITKAudioProcessingLibrary::GetClosestAlignmentEntry(const FCharacterAlignment& AlignmentData, float TimeInSeconds)
{
    if (AlignmentData.StartTimes.Num() == 0 || AlignmentData.EndTimes.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("GetClosestAlignmentEntry: AlignmentData arrays are empty."));
        return -1;  // Return invalid index if there's no data
    }

    // Initialize variables for tracking the closest index and the smallest difference
    int32 ClosestIndex = 0;
    float SmallestDifference = FMath::Abs(AlignmentData.StartTimes[0] - TimeInSeconds);

    // Iterate over the alignment data to find the closest entry considering both start and end times
    for (int32 i = 1; i < AlignmentData.StartTimes.Num(); i++)
    {
        float StartDifference = FMath::Abs(AlignmentData.StartTimes[i] - TimeInSeconds);
        float EndDifference = FMath::Abs(AlignmentData.EndTimes[i] - TimeInSeconds);

        float ClosestDifference = FMath::Min(StartDifference, EndDifference);

        if (ClosestDifference < SmallestDifference)
        {
            SmallestDifference = ClosestDifference;
            ClosestIndex = i;
        }
    }

    // Return the index of the closest entry
    return ClosestIndex;
}




TArray<class UAnimMetaData*> UAIITKAudioProcessingLibrary::GetAnimMetaData(UAnimationAsset* Animation)
{
    TArray<UAnimMetaData*> MetaData;

    if (Animation)
    {
        // Retrieve metadata from the animation asset
        return Animation->GetMetaData();
    }

    // Return an empty array if no animation or metadata is available
    return MetaData;
}


void UAIITKAudioProcessingLibrary::QueuePCMDataToProcSoundWave(USoundWaveProcedural* SoundWave, const TArray<uint8>& PCMData, int32 SampleRate, int32 Channels)
{
    if (!IsValid(SoundWave) || PCMData.Num() == 0)
    {
        UE_LOG(AIITKLog, Error, TEXT("Invalid SoundWave or empty PCM data."));
        return;
    }

    // Set sample rate and channel number based on PCM data
    SoundWave->SetSampleRate(SampleRate);
    SoundWave->NumChannels = Channels; // Should be mono (1) for this context but included for alternate uses

    int32 SampleByteSize = 2; // For 16 bit PCM
    int32 AlignedSize = PCMData.Num() - (PCMData.Num() % SampleByteSize);

    // Queue the PCM data to the procedural sound wave
    if (AlignedSize > 0)
    {
        SoundWave->QueueAudio(PCMData.GetData(), AlignedSize);
    }

    UE_LOG(AIITKLog, Log, TEXT("Queued PCM data to procedural sound wave."));
}

void UAIITKAudioProcessingLibrary::QueueMP3DataToProcSoundWave(USoundWaveProcedural* SoundWave, const TArray<uint8>& MP3Data)
{
    if (!IsValid(SoundWave) || MP3Data.Num() == 0)
    {
        UE_LOG(AIITKLog, Error, TEXT("Invalid SoundWave or empty MP3 data."));
        return;
    }

    drmp3 mp3;
    if (!drmp3_init_memory(&mp3, MP3Data.GetData(), MP3Data.Num(), nullptr))
    {
        UE_LOG(AIITKLog, Error, TEXT("Failed to initialize MP3 from memory."));
        return;
    }

    drmp3_uint64 numSamples = drmp3_get_pcm_frame_count(&mp3);
    drmp3_int16* pSampleData = static_cast<drmp3_int16*>(malloc(numSamples * mp3.channels * sizeof(drmp3_int16)));
    drmp3_uint64 numSamplesRead = drmp3_read_pcm_frames_s16(&mp3, numSamples, pSampleData);

    if (numSamplesRead > 0)
    {
        // Set sample rate and channel number based on MP3 data
        SoundWave->SetSampleRate(mp3.sampleRate);
        SoundWave->NumChannels = mp3.channels;

        // Assuming 16 bit PCM, calculate the byte size of the sample data
        int32 SampleByteSize = 2; // For 16 bit PCM
        int32 DataSize = numSamplesRead * mp3.channels * SampleByteSize;

        // Queue the decoded MP3 data (now in PCM format) to the procedural sound wave
        SoundWave->QueueAudio(reinterpret_cast<const uint8*>(pSampleData), DataSize);

        UE_LOG(AIITKLog, Log, TEXT("Queued MP3 data to procedural sound wave."));
    }
    else
    {
        UE_LOG(AIITKLog, Error, TEXT("No samples read from MP3 data."));
    }

    // Clean up resources
    drmp3_uninit(&mp3);
    free(pSampleData);
}

void UAIITKAudioProcessingLibrary::QueueAudioDataToProcSoundWave(USoundWaveProcedural* SoundWave, const TArray<uint8>& AudioData, EAudioFileFormat InputFormat, int32 SampleRate, int32 Channels)
{
    switch (InputFormat)
    {
    case EAudioFileFormat::PCM:
        QueuePCMDataToProcSoundWave(SoundWave, AudioData, SampleRate, Channels);
        break;
    case EAudioFileFormat::MP3:
        QueueMP3DataToProcSoundWave(SoundWave, AudioData);
        break;
    default:
        UE_LOG(AIITKLog, Error, TEXT("Unsupported audio file format."));
        break;
    }
}


void UAIITKAudioProcessingLibrary::ClearAudioDataFromProcSoundWave(USoundWaveProcedural* SoundWave)
{
    if (!IsValid(SoundWave))
    {
        UE_LOG(AIITKLog, Error, TEXT("Invalid SoundWave."));
        return;
    }

    // Clear the queued audio data
    SoundWave->ResetAudio();

    UE_LOG(AIITKLog, Log, TEXT("Cleared audio data from procedural sound wave."));
}



bool UAIITKAudioProcessingLibrary::SaveRawAudioToDisk(const FString& FilePath, const TArray<uint8>& RawAudioData, EAudioFileFormat InputFormat, int32 InputSampleRate, int32 DesiredOutputSampleRate)
{
    if (RawAudioData.Num() == 0)
    {
        UE_LOG(AIITKLog, Error, TEXT("Audio data is empty."));
        return false;
    }

    // Check the format and save accordingly
    if (InputFormat == EAudioFileFormat::MP3)
    {
        // Directly save the MP3 data to disk
        if (!FFileHelper::SaveArrayToFile(RawAudioData, *FilePath))
        {
            UE_LOG(AIITKLog, Error, TEXT("Failed to save MP3 file: %s"), *FilePath);
            return false;
        }
        else
        {
            UE_LOG(AIITKLog, Warning, TEXT("Warning: MP3 cannot be resampled due to encoding liscensing, PCM recommended"))
        }
        return true;
    }
    else if (InputFormat == EAudioFileFormat::PCM)
    {
        // Convert the uint8 array to an int16 array for processing
        TArray<int16> PCMDataInt16;
        PCMDataInt16.SetNumUninitialized(RawAudioData.Num() / sizeof(int16));
        FMemory::Memcpy(PCMDataInt16.GetData(), RawAudioData.GetData(), RawAudioData.Num());

        // Resample the audio data if the desired output sample rate is different from the input sample rate
        TArray<int16> ResampledData;
        if (InputSampleRate != DesiredOutputSampleRate)
        {
            ResampledData = ResampleAudioData(PCMDataInt16, InputSampleRate, DesiredOutputSampleRate);
        }
        else
        {
            ResampledData = PCMDataInt16;
        }

        // Assuming mono channel for simplicity
        int32 NumChannels = 1;

        // Save the (possibly resampled) audio data to a WAV file
        return SaveToWavFileUsingDrWav(FilePath, ResampledData, DesiredOutputSampleRate, NumChannels);
    }
    else
        return false;
}

TArray<int16> UAIITKAudioProcessingLibrary::ResampleAudioData(const TArray<int16>& InData, int32 InSampleRate, int32 OutSampleRate)
{
    if (InData.Num() == 0 || InSampleRate == OutSampleRate) {
        return InData;
    }

    // Calculate the number of samples after resampling
    uint64 inputSize = InData.Num();
    double ratio = (double)OutSampleRate / (double)InSampleRate;
    uint64 outputSize = (uint64)(inputSize * ratio);

    TArray<int16> OutData;
    OutData.SetNum(outputSize);

    // Resampling logic
    double step = (double)InSampleRate / (double)OutSampleRate;
    double index = 0.0;

    for (uint64 i = 0; i < outputSize; ++i) {
        int16 sample = InData[(int)index];
        OutData[i] = sample;
        index += step;
    }

    return OutData;
}

TArray<uint8> UAIITKAudioProcessingLibrary::ResamplePCMData_BP(const TArray<uint8>& InData, int32 InSampleRate, int32 OutSampleRate, int32 InNumChannels, int32 OutNumChannels)
{
    if (InData.Num() == 0 || (InSampleRate == OutSampleRate && InNumChannels == OutNumChannels)) {
        return InData;
    }

    // Convert uint8 array to int16 array for processing
    TArray<int16> Int16Data;
    int32 NumSamples = InData.Num() / 2;
    Int16Data.Reserve(NumSamples);
    for (int32 i = 0; i < InData.Num(); i += 2) {
        int16 value = (int16)((InData[i + 1] << 8) | InData[i]);
        Int16Data.Add(value);
    }

    // Adjust the number of channels
    TArray<int16> ChannelAdjustedData;
    int32 NumFrames = NumSamples / InNumChannels;
    ChannelAdjustedData.Reserve(NumFrames * OutNumChannels);

    if (InNumChannels == 1 && OutNumChannels == 2) {
        // Mono to Stereo: Duplicate the mono channel
        for (int32 i = 0; i < NumSamples; ++i) {
            int16 sample = Int16Data[i];
            ChannelAdjustedData.Add(sample); // Left channel
            ChannelAdjustedData.Add(sample); // Right channel
        }
    }
    else if (InNumChannels == 2 && OutNumChannels == 1) {
        // Stereo to Mono: Average the two channels
        for (int32 i = 0; i < NumSamples; i += 2) {
            int16 leftSample = Int16Data[i];
            int16 rightSample = Int16Data[i + 1];
            int16 monoSample = (int16)((leftSample + rightSample) / 2);
            ChannelAdjustedData.Add(monoSample);
        }
    }
    else if (InNumChannels == OutNumChannels) {
        // No change in channels
        ChannelAdjustedData = Int16Data;
    }
    else {
        // Unsupported channel conversion
        UE_LOG(AIITKLog, Error, TEXT("Unsupported channel conversion from %d to %d channels"), InNumChannels, OutNumChannels);
        return TArray<uint8>();
    }

    // Resampling
    TArray<int16> ResampledData;

    if (InSampleRate == OutSampleRate) {
        ResampledData = ChannelAdjustedData;
    }
    else {
        // Calculate the number of output samples
        int32 InputFrames = ChannelAdjustedData.Num() / OutNumChannels;
        double ratio = (double)OutSampleRate / (double)InSampleRate;
        int32 OutputFrames = (int32)(InputFrames * ratio);

        ResampledData.SetNum(OutputFrames * OutNumChannels);

        // Resampling logic using linear interpolation
        double step = (double)InputFrames / (double)OutputFrames;
        for (int32 frame = 0; frame < OutputFrames; ++frame) {
            double index = frame * step;
            int32 idx = (int32)index;
            double frac = index - idx;

            for (int32 channel = 0; channel < OutNumChannels; ++channel) {
                int32 idx1 = idx * OutNumChannels + channel;
                int32 idx2 = FMath::Min(idx1 + OutNumChannels, ChannelAdjustedData.Num() - 1);

                int16 sample1 = ChannelAdjustedData[idx1];
                int16 sample2 = ChannelAdjustedData[idx2];

                int16 resampledSample = (int16)(sample1 + frac * (sample2 - sample1));
                ResampledData[frame * OutNumChannels + channel] = resampledSample;
            }
        }
    }

    // Convert processed int16 array back to uint8 array
    TArray<uint8> OutDataUint8;
    OutDataUint8.Reserve(ResampledData.Num() * 2);
    for (int16 Sample : ResampledData) {
        OutDataUint8.Add(Sample & 0xFF);
        OutDataUint8.Add((Sample >> 8) & 0xFF);
    }

    return OutDataUint8;
}

bool UAIITKAudioProcessingLibrary::SaveToWavFileUsingDrWav(const FString& FilePath, const TArray<int16>& Data, int32 SampleRate, int32 NumChannels)
{
    std::string FilePathStd = TCHAR_TO_UTF8(*FilePath);
    drwav_data_format format = {};
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_PCM;
    format.channels = static_cast<uint16>(NumChannels);
    format.sampleRate = static_cast<uint32>(SampleRate);
    format.bitsPerSample = 16;

    drwav wav;
    if (!drwav_init_file_write(&wav, FilePathStd.c_str(), &format, nullptr)) {
        UE_LOG(AIITKLog, Error, TEXT("Failed to initialize WAV file for writing."));
        return false;
    }

    size_t samplesToWrite = Data.Num();
    size_t samplesWritten = drwav_write_pcm_frames(&wav, samplesToWrite, Data.GetData());

    drwav_uninit(&wav);

    return samplesWritten == samplesToWrite;
}

void UAIITKAudioProcessingLibrary::ConvertMP3ToWAV(const FString& MP3Filename, const FString& WAVFilename)
{
    drmp3 mp3;
    if (!drmp3_init_file(&mp3, TCHAR_TO_ANSI(*MP3Filename), nullptr))
    {
        UE_LOG(AIITKLog, Error, TEXT("Failed to open MP3 file: %s"), *MP3Filename);
        return;
    }


    drmp3_uint64 numSamples = drmp3_get_pcm_frame_count(&mp3);
    drmp3_int16* pSampleData = static_cast<drmp3_int16*>(malloc(numSamples * mp3.channels * sizeof(drmp3_int16)));
    drmp3_read_pcm_frames_s16(&mp3, numSamples, pSampleData);

    drwav_data_format format;
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_PCM;
    format.channels = mp3.channels;
    format.sampleRate = mp3.sampleRate;
    format.bitsPerSample = 16;

    drwav wav;
    if (!drwav_init_file_write(&wav, TCHAR_TO_ANSI(*WAVFilename), &format, nullptr))
    {
        UE_LOG(AIITKLog, Error, TEXT("Failed to open WAV file for writing: %s"), *WAVFilename);
        return;
    }

    drwav_uint64 numSamplesWritten = drwav_write_pcm_frames(&wav, numSamples, pSampleData);
    UE_LOG(AIITKLog, Log, TEXT("Converted %s to %s: %llu samples written"), *MP3Filename, *WAVFilename, numSamplesWritten);

    // Clean up resources
    drwav_uninit(&wav);
    drmp3_uninit(&mp3);
    free(pSampleData);
}

USoundWave* UAIITKAudioProcessingLibrary::LoadWAVFileAsSoundWave(const FString& WAVFilename)
{
    TArray<uint8> WAVData;
    if (!FFileHelper::LoadFileToArray(WAVData, *WAVFilename))
    {
        UE_LOG(AIITKLog, Error, TEXT("Failed to load WAV file: %s"), *WAVFilename);
        return nullptr;
    }

    FWaveModInfo WaveInfo;
    if (!WaveInfo.ReadWaveInfo(WAVData.GetData(), WAVData.Num()))
    {
        UE_LOG(AIITKLog, Error, TEXT("Failed to read WAV file information"));
        return nullptr;
    }

    USoundWave* SoundWave = NewObject<USoundWave>(USoundWave::StaticClass());

    if (!SoundWave) return nullptr;

    // Calculate the duration
    int32 SampleDataSize = WaveInfo.SampleDataSize;
    int32 BytesPerSample = *WaveInfo.pBitsPerSample / 8;
    int32 NumChannels = *WaveInfo.pChannels;
    int32 SampleRate = *WaveInfo.pSamplesPerSec;
    float Duration = static_cast<float>(SampleDataSize) / (BytesPerSample * NumChannels * SampleRate);

    //SoundWave->InvalidateCompressedData();
    SoundWave->RawPCMDataSize = WaveInfo.SampleDataSize;
    SoundWave->Duration = Duration;
    SoundWave->SetSampleRate(SampleRate);
    SoundWave->NumChannels = NumChannels;
    SoundWave->RawPCMData = (uint8*)FMemory::Malloc(WaveInfo.SampleDataSize);
    FMemory::Memcpy(SoundWave->RawPCMData, WaveInfo.SampleDataStart, WaveInfo.SampleDataSize);

    SoundWave->SoundGroup = ESoundGroup::SOUNDGROUP_Default;

    // Set the compression quality
    //SoundWave->CompressionQuality = 40;

    return SoundWave;
}

TArray<uint8> UAIITKAudioProcessingLibrary::LoadWAVFileAsRawPCMData(const FString& WAVFilename)
{
    TArray<uint8> WAVData;
    if (!FFileHelper::LoadFileToArray(WAVData, *WAVFilename))
    {
        UE_LOG(AIITKLog, Error, TEXT("Failed to load WAV file: %s"), *WAVFilename);
        return TArray<uint8>(); // Return an empty array on failure
    }

    FWaveModInfo WaveInfo;
    if (!WaveInfo.ReadWaveInfo(WAVData.GetData(), WAVData.Num()))
    {
        UE_LOG(AIITKLog, Error, TEXT("Failed to read WAV file information"));
        return TArray<uint8>(); // Return an empty array on failure
    }

    TArray<uint8> RawPCMData;
    int32 SampleDataSize = WaveInfo.SampleDataSize;
    const uint8* SampleDataStart = WaveInfo.SampleDataStart;

    // Append the sample data to the RawPCMData array
    RawPCMData.Append(SampleDataStart, SampleDataSize);

    return RawPCMData;
}

FString UAIITKAudioProcessingLibrary::EncodeBase64Audio(const TArray<uint8>& AudioData)
{
    if (AudioData.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("EncodeBase64Audio: Audio data is empty."));
        return FString();
    }

    // Encode the audio data to a Base64 string
    FString Base64EncodedString = FBase64::Encode(AudioData);
    return Base64EncodedString;
}

TArray<uint8> UAIITKAudioProcessingLibrary::DecodeBase64Audio(const FString& Base64AudioData)
{
    TArray<uint8> DecodedData;

    if (Base64AudioData.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("DecodeBase64Audio: Input Base64 string is empty."));
        return DecodedData;
    }

    // Decode the Base64 string back to audio data
    if (!FBase64::Decode(Base64AudioData, DecodedData))
    {
        UE_LOG(LogTemp, Error, TEXT("DecodeBase64Audio: Failed to decode Base64 string."));
    }

    return DecodedData;
}