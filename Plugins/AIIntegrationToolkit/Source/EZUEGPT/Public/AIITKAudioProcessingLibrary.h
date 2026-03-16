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

 // =================== AIITKAudioProcessingLibrary.h ===================

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Sound/SoundWave.h"
#include "OpenAIFunctionLibrary.h"
#include "Animation/AnimationAsset.h"
#include "AIITKAudioProcessingLibrary.generated.h"

/**
 * Class for audio processing tasks, including resampling and saving to WAV format.
 */
UCLASS()
class EZUEGPT_API UAIITKAudioProcessingLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    static void QueuePCMDataToProcSoundWave(USoundWaveProcedural* SoundWave, const TArray<uint8>& RawAudioData, int32 SampleRate, int32 Channels = 1);

    static void QueueMP3DataToProcSoundWave(USoundWaveProcedural* SoundWave, const TArray<uint8>& MP3Data);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|AudioProcessing")
    static void QueueAudioDataToProcSoundWave(USoundWaveProcedural* SoundWave, const TArray<uint8>& AudioData, EAudioFileFormat InputFormat, int32 SampleRate = 24000, int32 Channels = 1);

    // Saves uint8 array of audio data to the disk. Warning: MP3 cannot be resampled due to licensing with encoding, PCM recommended.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|AudioProcessing")
    static bool SaveRawAudioToDisk(const FString& FilePath, const TArray<uint8>& RawAudioData, EAudioFileFormat InputFormat, int32 InputSampleRate = 24000, int32 DesiredOutputSampleRate = 24000);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|AudioProcessing", meta = (ToolTip = "Convert an MP3 file to a WAV file"))
    static void ConvertMP3ToWAV(const FString& MP3Filename, const FString& WAVFilename);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|AudioProcessing", meta = (DisplayName = "Load WAV File As Sound Wave", ToolTip = "Load a WAV file from disk and create a SoundWave object"))
    static USoundWave* LoadWAVFileAsSoundWave(const FString& WAVFilename);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|AudioProcessing", meta = (DisplayName = "Load WAV File As Raw PCM Data", ToolTip = "Load a WAV file from disk and return the raw PCM data bytes"))
    static TArray<uint8> LoadWAVFileAsRawPCMData(const FString& WAVFilename);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|AudioProcessing", meta = (ToolTip = "Resample and change the number of channels of PCM audio data in Blueprints."))
    static TArray<uint8> ResamplePCMData_BP(const TArray<uint8>& InData, int32 InSampleRate, int32 OutSampleRate, int32 InNumChannels, int32 OutNumChannels);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|AudioProcessing", meta = (ToolTip = "Condense any text/timing pairs into digraphs"))
    static FCondensedAlignment CondenseTextToDigraphs(const TArray<FString>& TextInput, const TArray<float>& StartTimes);

    // Condenses phonemes into digraphs and outputs both normalized and unnormalized data
    UFUNCTION(BlueprintCallable, Category = "*AIITK|AudioProcessing", meta = (ToolTip = "Condense single-letter phonemes to digraphs, output both normalized and unnormalized alignment times"))
    static FCharacterAlignment CondensePhonemesToDigraphs(const FCharacterAlignment& AlignmentData);



    // Finds the closest character alignment entry based on time in seconds (considering both StartTimes and EndTimes)
    UFUNCTION(BlueprintCallable, Category = "*AIITK|AudioProcessing", meta = (ToolTip = "Find the closest character alignment entry based on time in seconds"))
    static int32 GetClosestAlignmentEntry(const FCharacterAlignment& AlignmentData, float TimeInSeconds);

    UFUNCTION(BlueprintPure, Category = "*AIITK|Animation")
    static TArray<class UAnimMetaData*> GetAnimMetaData(UAnimationAsset* Animation);

    /** Encode PCM16 audio data to a Base64 string */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|AudioProcessing", meta = (ToolTip = "Encode PCM16 audio data to a Base64 string"))
    static FString EncodeBase64Audio(const TArray<uint8>& AudioData);

    /** Decode a Base64 string to PCM16 audio data */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|AudioProcessing", meta = (ToolTip = "Decode a Base64 string to PCM16 audio data"))
    static TArray<uint8> DecodeBase64Audio(const FString& Base64AudioData);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|AudioProcessing")
    static void ClearAudioDataFromProcSoundWave(USoundWaveProcedural* SoundWave);


private:
    static TArray<int16> ResampleAudioData(const TArray<int16>& InData, int32 InSampleRate, int32 OutSampleRate);
    static bool SaveToWavFileUsingDrWav(const FString& FilePath, const TArray<int16>& Data, int32 SampleRate, int32 NumChannels);
};
