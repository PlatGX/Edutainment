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

 // =================== TextToSpeechAPI.cpp ===================

#include "TextToSpeechAPI.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "Sound/SoundWaveProcedural.h"
#include "AudioCompressionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "Runtime/Engine/Classes/Sound/SoundWave.h"
#include "Misc/FileHelper.h"
#include "Audio.h"
#include "AIITKDeveloperSettings.h"
#include "JsonObjectConverter.h"
#include "Async/Async.h"
#include "OpenAIFunctionLibrary.h"
#include "AIITKAudioProcessingLibrary.h"


void UTextToSpeechAPI::BeginDestroy()
{
    // Remove from root to allow garbage collection
    RemoveFromRoot();

    Super::BeginDestroy();
}

UTextToSpeechAPI* UTextToSpeechAPI::PlayAudioSynced(UAudioComponent* AudioComponent, float UpdateRate, USoundBase* NewSound)
{
    if (!AudioComponent)
    {
        UE_LOG(AIITKLog, Error, TEXT("Invalid Audio Component"));
        return nullptr;
    }

    // Create an instance of UTextToSpeechAPI to manage state and delegates
    UTextToSpeechAPI* Instance = NewObject<UTextToSpeechAPI>();
    if (Instance)
    {
        Instance->PlayAudioSynced_Internal(AudioComponent, UpdateRate, NewSound);
    }

    return Instance;
}

void UTextToSpeechAPI::PlayAudioSynced_Internal(UAudioComponent* AudioComponent, float UpdateRate, USoundBase* NewSound)
{
    if (!AudioComponent)
    {
        UE_LOG(AIITKLog, Error, TEXT("Invalid Audio Component"));
        return;
    }

    // Set the new sound if provided
    if (NewSound)
    {
        AudioComponent->SetSound(NewSound);
    }

    // Initialize elapsed time
    float ElapsedTime = 0.0f;

    // Start playing the audio
    AudioComponent->Play();

    // Ensure we have a valid world context
    UWorld* World = AudioComponent->GetWorld();
    if (!World)
    {
        UE_LOG(AIITKLog, Error, TEXT("Failed to get a valid World context from the Audio Component"));
        return;
    }

    // Set up a recurring timer to track elapsed time and broadcast updates
    World->GetTimerManager().SetTimer(
        AudioSyncTimerHandle,
        [this, AudioComponent, UpdateRate, ElapsedTime]() mutable
    {
        // Ensure the instance and world are valid before proceeding
        if (!AudioComponent || !AudioComponent->GetWorld())
        {
            return;
        }

        // Increment the elapsed time
        ElapsedTime += UpdateRate;

        // Broadcast the current elapsed time through the delegate
        OnAudioSyncUpdate.Broadcast(ElapsedTime);

        // Check if the audio is finished playing
        if (!AudioComponent->IsPlaying())
        {
            // Stop the timer when audio is finished
            AudioComponent->GetWorld()->GetTimerManager().ClearTimer(AudioSyncTimerHandle);
        }
    },
        UpdateRate,  // Time between ticks
        true         // Loop the timer
    );
}

void UTextToSpeechAPI::UpdateElapsedTime(UAudioComponent* AudioComponent, float& ElapsedTime, float UpdateRate)
{
    // Increment the elapsed time
    ElapsedTime += UpdateRate;

    // Broadcast the current elapsed time through the delegate
    OnAudioSyncUpdate.Broadcast(ElapsedTime);

    // Check if the audio is finished playing
    if (!AudioComponent->IsPlaying())
    {
        // Stop the timer when audio is finished
        GetWorld()->GetTimerManager().ClearTimer(AudioSyncTimerHandle);
    }
}

// RESPONSE PROCESSING

USoundWaveProcedural* UTextToSpeechAPI::ProcessAudioResponse(const TArray<uint8>& Content, const F11LabsRequestParams& Params)
{

    if (Content.Num() == 0)
    {
        UE_LOG(AIITKLog, Error, TEXT("Received empty content, initial response will always be empty."));
        return nullptr;
    }

    if (!IsValid(this->TTSProcSoundWave))
    {
        UE_LOG(AIITKLog, Warning, TEXT("TTSProcSoundWave is not valid. Creating a new instance."));
        this->TTSProcSoundWave = NewObject<USoundWaveProcedural>();
        if (!IsValid(TTSProcSoundWave))
        {
            UE_LOG(AIITKLog, Error, TEXT("Failed to create a new TTSProcSoundWave instance."));
            return nullptr;
        }
        else
        {
            // Initialize proc audio wave
            int32 CurrentSampleRate = UOpenAIFunctionLibrary::OutputFormatToSampleRate(Params.StreamParams.OutputFormat);
            this->TTSProcSoundWave = NewObject<USoundWaveProcedural>();
            this->TTSProcSoundWave->SetSampleRate(CurrentSampleRate);
            this->TTSProcSoundWave->NumChannels = 1; // Mono audio
            UE_LOG(AIITKLog, Warning, TEXT("Backup sound Wave Initialized! Sample Rate: %d"), CurrentSampleRate);
        }

    }

    FString OutputFormatString = UOpenAIFunctionLibrary::OutputFormatEnumToString(Params.StreamParams.OutputFormat);
    if (OutputFormatString.Contains(TEXT("mp3")))
    {
        FString MP3Filename = "";
        FString WAVFilename = "";
        if (!Params.RequiredParams.FileName.IsEmpty())
        {
            MP3Filename = FPaths::Combine(FPaths::ProjectDir(), TEXT("Sounds/AIITKConvert")) + TEXT(".mp3");
            WAVFilename = FPaths::Combine(FPaths::ProjectDir(), TEXT("Sounds/AIITKConvert")) + TEXT(".wav");
        }
        else
        {
            MP3Filename = Params.RequiredParams.FileName + TEXT(".mp3");
            WAVFilename = Params.RequiredParams.FileName + TEXT(".wav");
        }
        if (!FFileHelper::SaveArrayToFile(Content, *MP3Filename))
        {
            UE_LOG(AIITKLog, Error, TEXT("Failed to save MP3 file: %s"), *MP3Filename);
            return nullptr;
        }

        UAIITKAudioProcessingLibrary::ConvertMP3ToWAV(MP3Filename, WAVFilename);
        TArray<uint8> WAVData;
        if (!FFileHelper::LoadFileToArray(WAVData, *WAVFilename))
        {
            UE_LOG(AIITKLog, Error, TEXT("Failed to load WAV file: %s"), *WAVFilename);
            return nullptr;
        }

        int32 DataToQueue = WAVData.Num() - this->AlreadyProcessedDataSize;
        if (DataToQueue > 0)
        {
            this->TTSProcSoundWave->QueueAudio(WAVData.GetData() + this->AlreadyProcessedDataSize, DataToQueue);
            this->AlreadyProcessedDataSize = WAVData.Num();
        }

    }
    else // Handling PCM or other formats
    {
        // Since the data is 16-bit, ensure correct handling of the byte size
        const int32 SampleByteSize = 2; // 16-bit samples are 2 bytes each
        int32 AlignedSize = Content.Num() - (Content.Num() % SampleByteSize);
        int32 DataToQueue = AlignedSize - this->AlreadyProcessedDataSize;

        if (DataToQueue > 0)
        {
            TArray<uint8> ProcessedContent;
            ProcessedContent.Append(Content.GetData() + this->AlreadyProcessedDataSize, DataToQueue);
            this->TTSProcSoundWave->QueueAudio(ProcessedContent.GetData(), ProcessedContent.Num());
            this->AlreadyProcessedDataSize = AlignedSize;
        }
    }

    UE_LOG(AIITKLog, Warning, TEXT("Queued new audio data to procedural sound wave."));
    return this->TTSProcSoundWave;
}


USoundWaveProcedural* UTextToSpeechAPI::ProcessOpenAIAudioResponse(const TArray<uint8>& Content, const FOpenAITTSRequestParams& Params)
{
    if (Content.Num() == 0)
    {
        UE_LOG(AIITKLog, Error, TEXT("Received empty content from OpenAI, the response is empty."));
        return nullptr;
    }

    if (!IsValid(this->TTSProcSoundWave))
    {
        UE_LOG(AIITKLog, Warning, TEXT("TTSProcSoundWave is not valid. Creating a new instance."));
        this->TTSProcSoundWave = NewObject<USoundWaveProcedural>();
        if (!IsValid(TTSProcSoundWave))
        {
            UE_LOG(AIITKLog, Error, TEXT("Failed to create a new TTSProcSoundWave instance."));
            return nullptr;
        }

        // Initialize proc audio wave
        int32 CurrentSampleRate = Params.ProcAudioSampleRate; // Default sample rate for OpenAI TTS
        this->TTSProcSoundWave = NewObject<USoundWaveProcedural>();
        this->TTSProcSoundWave->SetSampleRate(CurrentSampleRate);
        this->TTSProcSoundWave->NumChannels = 1; // Mono audio

        UE_LOG(AIITKLog, Warning, TEXT("Backup sound Wave Initialized! Sample Rate: %d"), CurrentSampleRate);
    }

    // Determine if the response is in MP3 format
    bool isMP3Format = Params.ResponseFormat == EAudioFileFormat::MP3;

    // Handle MP3 format
    if (isMP3Format)
    {
        FString MP3Filename = "";
        FString WAVFilename = "";
        if (!Params.FileName.IsEmpty())
        {
            MP3Filename = FPaths::Combine(FPaths::ProjectDir(), TEXT("Sounds/AIITKConvert")) + TEXT(".mp3");
            WAVFilename = FPaths::Combine(FPaths::ProjectDir(), TEXT("Sounds/AIITKConvert")) + TEXT(".wav");
        }
        else
        {
            MP3Filename = Params.FileName + TEXT(".mp3");
            WAVFilename = Params.FileName + TEXT(".wav");
        }

        if (!FFileHelper::SaveArrayToFile(Content, *MP3Filename))
        {
            UE_LOG(AIITKLog, Error, TEXT("Failed to save MP3 file from OpenAI: %s"), *MP3Filename);
            return nullptr;
        }

        UAIITKAudioProcessingLibrary::ConvertMP3ToWAV(MP3Filename, WAVFilename);
        TArray<uint8> WAVData;
        if (!FFileHelper::LoadFileToArray(WAVData, *WAVFilename))
        {
            UE_LOG(AIITKLog, Error, TEXT("Failed to load WAV file: %s"), *WAVFilename);
            return nullptr;
        }

        int32 DataToQueue = WAVData.Num() - this->AlreadyProcessedDataSize;
        if (DataToQueue > 0)
        {
            this->TTSProcSoundWave->QueueAudio(WAVData.GetData() + this->AlreadyProcessedDataSize, DataToQueue);
            this->AlreadyProcessedDataSize = WAVData.Num();
        }
    }
    else // Unsupported format
    {
        int32 SampleByteSize = 2; // Adjust based on your audio format
        int32 AlignedSize = Content.Num() - (Content.Num() % SampleByteSize);
        int32 NewDataSize = AlignedSize - this->AlreadyProcessedDataSize;

        if (NewDataSize > 0)
        {
            this->TTSProcSoundWave->QueueAudio(Content.GetData() + this->AlreadyProcessedDataSize, NewDataSize);
            this->AlreadyProcessedDataSize = AlignedSize;
        }
    }

    UE_LOG(AIITKLog, Warning, TEXT("Queued new OpenAI audio data to procedural sound wave."));
    return this->TTSProcSoundWave;
}




// DETECT PROCEDURAL AUDIO END

void UTextToSpeechAPI::OnProceduralUnderflowHandler(USoundWaveProcedural* InProceduralSoundWave, int32 SamplesRequired)
{
    if (InProceduralSoundWave)
    {
        int32 RemainingBytes = InProceduralSoundWave->GetAvailableAudioByteCount();

        if (RemainingBytes <= 0)
        {
            // Ensure the delegate is bound and the object it's bound to is valid and not pending kill
            if (this->OnProceduralAudioFinished.IsBound() && IsValid(this))
            {
                AsyncTask(ENamedThreads::GameThread, [this, InProceduralSoundWave]()
                {
                    this->OnProceduralAudioFinished.Broadcast();
                    UE_LOG(AIITKLog, Warning, TEXT("Audio finished playing, or buffer underrun, you should stop the audio component soon to prevent errors."));
                    // Unbind this handler from the delegate to prevent future calls
                    //InProceduralSoundWave->OnSoundWaveProceduralUnderflow.Unbind();
                });

            }
        }
    }
}


// REQUEST

UTextToSpeechAPI* UTextToSpeechAPI::SendTextTo11Labs_BP(const F11LabsRequestParams& Params)
{
    // Create a new instance of UTextToSpeechAPI
    UTextToSpeechAPI* APIInstance = NewObject<UTextToSpeechAPI>();
	APIInstance->AddToRoot();  // Prevent garbage collection

    // Initialize procedural audio wave
    int32 CurrentSampleRate = UOpenAIFunctionLibrary::OutputFormatToSampleRate(Params.StreamParams.OutputFormat);

    // Create a new procedural sound wave
    APIInstance->TTSProcSoundWave = NewObject<USoundWaveProcedural>();
    APIInstance->TTSProcSoundWave->SetSampleRate(CurrentSampleRate);
    APIInstance->TTSProcSoundWave->NumChannels = 1; // Mono audio

    UE_LOG(AIITKLog, Warning, TEXT("Sound Wave Initialized! Sample Rate: %d"), CurrentSampleRate);

    // Start the internal process to send text to 11 Labs
    APIInstance->SendTextTo11Labs_Internal(Params);
    return APIInstance;
}


void UTextToSpeechAPI::SendTextTo11Labs_Internal(const F11LabsRequestParams& Params)
{
    if (bRequestInProgress)
    {
        UE_LOG(AIITKLog, Error, TEXT("Request already in progress"));
        return;
    }

    bRequestInProgress = true;

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("POST");

    // Determine endpoint based on streaming and alignment data requirement
    FString FinalEndpoint = Params.RequiredParams.Endpoint + Params.RequiredParams.VoiceId +
        (Params.StreamParams.bStreamedResponse ? TEXT("/stream") :
            (Params.RequiredParams.IncludeTimestamps ? TEXT("/with-timestamps") : TEXT(""))) +
        TEXT("?output_format=") + UOpenAIFunctionLibrary::OutputFormatEnumToString(Params.StreamParams.OutputFormat);

    FString DecryptedAPIKey = UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::Labs11);

    HttpRequest->SetURL(FinalEndpoint);
    HttpRequest->SetHeader(TEXT("accept"), TEXT("audio/mpeg"));
    HttpRequest->SetHeader(TEXT("xi-api-key"), DecryptedAPIKey);
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("text"), Params.TextPrompt);
    JsonObject->SetStringField(TEXT("model_id"), Params.RequiredParams.VoiceModel);

    TSharedPtr<FJsonObject> VoiceSettingsObject = MakeShareable(new FJsonObject());
    VoiceSettingsObject->SetNumberField(TEXT("stability"), Params.VoiceSettings.Stability);
    VoiceSettingsObject->SetNumberField(TEXT("similarity_boost"), Params.VoiceSettings.SimilarityBoost);
    VoiceSettingsObject->SetNumberField(TEXT("style"), Params.VoiceSettings.Style);
    VoiceSettingsObject->SetBoolField(TEXT("use_speaker_boost"), Params.VoiceSettings.UseSpeakerBoost);
    JsonObject->SetObjectField(TEXT("voice_settings"), VoiceSettingsObject);

    FString Content;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Content);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    HttpRequest->SetContentAsString(Content);

    FDateTime RequestStartTime = FDateTime::UtcNow();

    HttpRequest->OnHeaderReceived().BindLambda([this](FHttpRequestPtr Request, FString BytesSent, FString BytesReceived)
    {
        if (!Request.IsValid() || !Request->GetResponse().IsValid())
        {
            UE_LOG(AIITKLog, Warning, TEXT("HTTP Request or Response not valid yet."));
            return;
        }

        if (!this->AudioIsInitialized)
        {
            if (TTSProcSoundWave && this->OnAudioInitialized.IsBound())
            {
                this->OnAudioInitialized.Broadcast(TTSProcSoundWave);
            }
            else
            {
                UE_LOG(AIITKLog, Warning, TEXT("OnAudioInitialized delegate is not bound or ProceduralSoundWave is null."));
            }

            this->AudioIsInitialized = true;
        }
    });

    if (Params.StreamParams.bStreamedResponse)
    {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
        HttpRequest->OnRequestProgress64().BindLambda([this, RequestStartTime, Params](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
#else
        HttpRequest->OnRequestProgress().BindLambda([this, RequestStartTime, Params](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
#endif
        {
            if (!Request.IsValid() || !Request->GetResponse().IsValid())
            {
                UE_LOG(AIITKLog, Warning, TEXT("Invalid HTTP Request or Response not valid yet."));
                return;
            }

            FDateTime ProgressUpdateTime = FDateTime::UtcNow();
            FTimespan ResponseTime = ProgressUpdateTime - RequestStartTime;
            float ResponseTimeSeconds = ResponseTime.GetTotalMilliseconds() / 1000.0f;
            UE_LOG(AIITKLog, Warning, TEXT("=========================== Bytes Sent: %d | STREAM REQUEST IN PROGRESS | Bytes Received: %d | Response Time Stamp: %.3f seconds ============================"), BytesSent, BytesReceived, ResponseTimeSeconds);

            TArray<uint8> Content;
            Content.Append(Request->GetResponse()->GetContent().GetData(), BytesReceived);

            if (Content.Num() > 0)
            {
                ProcessAudioResponse(Content, Params);
                if (TTSProcSoundWave && OnProceduralAudioGenerated.IsBound())
                {
                    OnProceduralAudioGenerated.Broadcast(TTSProcSoundWave, Content);
                }
            }
            });
        }

    HttpRequest->OnProcessRequestComplete().BindLambda([this, RequestStartTime, Params](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        FDateTime RequestEndTime = FDateTime::UtcNow();
        FTimespan ResponseTime = RequestEndTime - RequestStartTime;
        float ResponseTimeSeconds = ResponseTime.GetTotalMilliseconds() / 1000.0f;
        UE_LOG(AIITKLog, Warning, TEXT("=========================== 11Labs REQUEST COMPLETE ================================"));
        UE_LOG(AIITKLog, Warning, TEXT("Total Response Time: %.3f seconds"), ResponseTimeSeconds);

        if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() <= 400)
        {

            TTSProcSoundWave->OnSoundWaveProceduralUnderflow.BindUObject(this, &UTextToSpeechAPI::OnProceduralUnderflowHandler);

            TArray<uint8> CompleteContent;
            CompleteContent.Append(Response->GetContent());

            // If alignment data is requested, parse the JSON response for audio and alignment data
            if (Params.RequiredParams.IncludeTimestamps)
            {
                TSharedPtr<FJsonObject> JsonResponse;
                TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

                if (FJsonSerializer::Deserialize(Reader, JsonResponse))
                {
                    // Process Audio Data
                    FString AudioBase64 = JsonResponse->GetStringField(TEXT("audio_base64"));
                    TArray<uint8> AudioData;
                    FBase64::Decode(AudioBase64, AudioData);

                    ProcessAudioResponse(AudioData, Params);

                    // Process Alignment Data
                    TSharedPtr<FJsonObject> AlignmentObject = JsonResponse->GetObjectField(TEXT("alignment"));
                    FCharacterAlignment AlignmentData;

                    const TArray<TSharedPtr<FJsonValue>> CharactersArray = AlignmentObject->GetArrayField(TEXT("characters"));
                    for (const TSharedPtr<FJsonValue>& CharacterValue : CharactersArray)
                    {
                        AlignmentData.Characters.Add(CharacterValue->AsString());
                    }

                    const TArray<TSharedPtr<FJsonValue>> StartTimesArray = AlignmentObject->GetArrayField(TEXT("character_start_times_seconds"));
                    for (const TSharedPtr<FJsonValue>& StartTimeValue : StartTimesArray)
                    {
                        AlignmentData.StartTimes.Add(StartTimeValue->AsNumber());
                    }

                    const TArray<TSharedPtr<FJsonValue>> EndTimesArray = AlignmentObject->GetArrayField(TEXT("character_end_times_seconds"));
                    for (const TSharedPtr<FJsonValue>& EndTimeValue : EndTimesArray)
                    {
                        AlignmentData.EndTimes.Add(EndTimeValue->AsNumber());
                    }

                    if (JsonResponse->HasField(TEXT("normalized_alignment")))
                    {
                        TSharedPtr<FJsonObject> NormalizedAlignmentObject = JsonResponse->GetObjectField(TEXT("normalized_alignment"));

                        const TArray<TSharedPtr<FJsonValue>> NormalizedCharactersArray = NormalizedAlignmentObject->GetArrayField(TEXT("characters"));
                        for (const TSharedPtr<FJsonValue>& NormalizedCharacterValue : NormalizedCharactersArray)
                        {
                            AlignmentData.NormalizedCharacters.Add(NormalizedCharacterValue->AsString());
                        }

                        const TArray<TSharedPtr<FJsonValue>> NormalizedStartTimesArray = NormalizedAlignmentObject->GetArrayField(TEXT("character_start_times_seconds"));
                        for (const TSharedPtr<FJsonValue>& NormalizedStartTimeValue : NormalizedStartTimesArray)
                        {
                            AlignmentData.NormalizedStartTimes.Add(NormalizedStartTimeValue->AsNumber());
                        }

                        const TArray<TSharedPtr<FJsonValue>> NormalizedEndTimesArray = NormalizedAlignmentObject->GetArrayField(TEXT("character_end_times_seconds"));
                        for (const TSharedPtr<FJsonValue>& NormalizedEndTimeValue : NormalizedEndTimesArray)
                        {
                            AlignmentData.NormalizedEndTimes.Add(NormalizedEndTimeValue->AsNumber());
                        }
                    }

                    if (OnAudioWithAlignmentGenerated.IsBound())
                    {
                        OnAudioWithAlignmentGenerated.Broadcast(TTSProcSoundWave, AlignmentData);
                    }
                }
            }
            else
            {
                // Handle raw audio data without alignment parsing
                ProcessAudioResponse(CompleteContent, Params);
                if (TTSProcSoundWave && OnAudioGenerated.IsBound())
                {
                    OnAudioGenerated.Broadcast(TTSProcSoundWave, CompleteContent);
                }
            }
        }
        else
        {
            int32 StatusCode = Response.IsValid() ? Response->GetResponseCode() : -1;
            UE_LOG(AIITKLog, Error, TEXT("HTTP request failed with status code: %d"), StatusCode);

            if (OnAudioFailed.IsBound())
            {
                OnAudioFailed.Broadcast(Response.IsValid() ? FString::Printf(TEXT("HTTP Error: %d"), StatusCode) : TEXT("HTTP Error: Invalid Response"));
            }
        }

        UE_LOG(AIITKLog, Warning, TEXT("Raw Response: %s"), *Response->GetContentAsString());
        UE_LOG(AIITKLog, Warning, TEXT("=========================== END OF 11Labs REQUEST ========================="));
        bRequestInProgress = false;
        // Remove from root to allow garbage collection
        RemoveFromRoot();
    });

    HttpRequest->ProcessRequest();
}




UTextToSpeechAPI* UTextToSpeechAPI::SendTextToOpenAITTS_BP(const FOpenAITTSRequestParams& Params)
{
    UTextToSpeechAPI* APIInstance = NewObject<UTextToSpeechAPI>();
    APIInstance->AddToRoot();  // Prevent garbage collection

    if (!APIInstance) {
        UE_LOG(AIITKLog, Error, TEXT("Failed to create UTextToSpeechAPI instance."));
        return nullptr;
    }

    // Initialize proc audio wave
    int32 CurrentSampleRate = Params.ProcAudioSampleRate; // Default sample rate for OpenAI TTS
    APIInstance->TTSProcSoundWave = NewObject<USoundWaveProcedural>();
    if (!APIInstance->TTSProcSoundWave) {
        UE_LOG(AIITKLog, Error, TEXT("Failed to create USoundWaveProcedural instance."));
        return nullptr;
    }

    APIInstance->TTSProcSoundWave->SetSampleRate(CurrentSampleRate);
    APIInstance->TTSProcSoundWave->NumChannels = 1; // Mono audio

    UE_LOG(AIITKLog, Warning, TEXT("Sound Wave Initialized! Sample Rate: %d"), CurrentSampleRate);

    APIInstance->SendTextToOpenAITTS_Internal(Params);
    return APIInstance;
}

void UTextToSpeechAPI::SendTextToOpenAITTS_Internal(const FOpenAITTSRequestParams& Params)
{
    if (bRequestInProgress)
    {
        UE_LOG(AIITKLog, Error, TEXT("Request already in progress"));
        return;
    }

    bRequestInProgress = true;

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("POST");

    FString FinalEndpoint = "https://api.openai.com/v1/audio/speech";

    // Fetch decrypted API key
    FString DecryptedAPIKey = UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI);

    HttpRequest->SetURL(FinalEndpoint);
    HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *DecryptedAPIKey));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // Prepare JSON payload
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("model"), Params.Model);
    JsonObject->SetStringField(TEXT("input"), Params.TextPrompt);
    JsonObject->SetStringField(TEXT("voice"), Params.Voice);
    JsonObject->SetStringField(TEXT("response_format"), UOpenAIFunctionLibrary::AudioFileFormatEnumToString(Params.ResponseFormat));
    JsonObject->SetNumberField(TEXT("speed"), Params.Speed);

    FString Content;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Content);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    HttpRequest->SetContentAsString(Content);

    // Record the request time
    FDateTime RequestStartTime = FDateTime::UtcNow();

    // Debugging: print the request data
    FString RawRequestData = "POST " + FinalEndpoint + Content;
    UE_LOG(AIITKLog, Log, TEXT("Raw HTTP Request Data:\n%s"), *RawRequestData);

    HttpRequest->OnHeaderReceived().BindLambda([this](FHttpRequestPtr Request, FString BytesSent, FString BytesReceived)
    {
        // Ensure Request and its Response are valid
        if (!Request.IsValid() || !Request->GetResponse().IsValid())
        {
            UE_LOG(AIITKLog, Warning, TEXT("HTTP Request or Response not valid yet."));
            return;
        }

        if (!this->AudioIsInitialized)
        {
            if (TTSProcSoundWave && this->OnAudioInitialized.IsBound())
            {
                this->OnAudioInitialized.Broadcast(TTSProcSoundWave);
            }
            else
            {
                UE_LOG(AIITKLog, Warning, TEXT("OnAudioInitialized delegate is not bound or ProceduralSoundWave is null."));
            }

            this->AudioIsInitialized = true;
        }
    });

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
    HttpRequest->OnRequestProgress64().BindLambda([this, Params, RequestStartTime](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
#else
    HttpRequest->OnRequestProgress().BindLambda([this, Params, RequestStartTime](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
#endif
    {
        // Ensure Request and its Response are valid
        if (!Request.IsValid() || !Request->GetResponse().IsValid())
        {
            UE_LOG(AIITKLog, Warning, TEXT("HTTP Request or Response not valid yet."));
            return;
        }

        FDateTime ProgressUpdateTime = FDateTime::UtcNow();
        FTimespan ResponseTime = ProgressUpdateTime - RequestStartTime;

        // Convert response time to seconds and format it
        float ResponseTimeSeconds = ResponseTime.GetTotalMilliseconds() / 1000.0f;
        UE_LOG(AIITKLog, Warning, TEXT("Bytes Sent: %d | Bytes Received: %d | Response Time: %.3f seconds"), BytesSent, BytesReceived, ResponseTimeSeconds);

        // Direct use of content and bytes received
        TArray<uint8> Content;
        Content.Append(Request->GetResponse()->GetContent().GetData(), BytesReceived);

        // Process the response
        if (Content.Num() > 0)
        {
            ProcessOpenAIAudioResponse(Content, Params);
            if (TTSProcSoundWave && OnProceduralAudioGenerated.IsBound())
            {
                OnProceduralAudioGenerated.Broadcast(TTSProcSoundWave, Content);
            }
            else
            {
                UE_LOG(AIITKLog, Warning, TEXT("ProcessOpenAIAudioResponse returned null or OnProceduralAudioGenerated is not bound."));
            }
        }
    });



    HttpRequest->OnProcessRequestComplete().BindLambda([this, Params, RequestStartTime](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        FDateTime RequestEndTime = FDateTime::UtcNow();
        FTimespan ResponseTime = RequestEndTime - RequestStartTime;
        float ResponseTimeSeconds = ResponseTime.GetTotalMilliseconds() / 1000.0f;
        UE_LOG(AIITKLog, Log, TEXT("HTTP Request Complete. Total Response Time: %.3f seconds"), ResponseTimeSeconds);

        if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
        {
            TArray<uint8> CompleteContent;
            CompleteContent.Append(Response->GetContent());
            ProcessOpenAIAudioResponse(CompleteContent, Params);

            if (TTSProcSoundWave)
            {
                TTSProcSoundWave->OnSoundWaveProceduralUnderflow.BindUObject(this, &UTextToSpeechAPI::OnProceduralUnderflowHandler);
                if (OnAudioGenerated.IsBound())
                {
                    OnAudioGenerated.Broadcast(TTSProcSoundWave, CompleteContent);
                }
                else
                {
                    UE_LOG(AIITKLog, Warning, TEXT("OnAudioGenerated delegate is not bound."));
                }
            }
            else
            {
                UE_LOG(AIITKLog, Error, TEXT("Failed to process OpenAITS response"));
            }
        }
        else
        {
            int32 StatusCode = Response.IsValid() ? Response->GetResponseCode() : -1;
            UE_LOG(AIITKLog, Error, TEXT("HTTP request failed with status code: %d"), StatusCode);

            if (this->OnAudioFailed.IsBound())
            {
                this->OnAudioFailed.Broadcast(Response.IsValid() ? FString::Printf(TEXT("HTTP Error: %d"), StatusCode) : TEXT("HTTP Error: Invalid Response"));
            }
            else
            {
                UE_LOG(AIITKLog, Warning, TEXT("OnAudioFailed delegate is not bound."));
            }
        }

        if (Response.IsValid())
        {
            UE_LOG(AIITKLog, Log, TEXT("Raw HTTP Response Data:\n%s"), *Response->GetContentAsString());
        }

        bRequestInProgress = false;

        // Remove from root to allow garbage collection
        RemoveFromRoot();

    });

    HttpRequest->ProcessRequest();
}



// INFORMATIONAL REQUESTS

UTextToSpeechAPI* UTextToSpeechAPI::GetVoices_BP()
{
    UTextToSpeechAPI* APIInstance = NewObject<UTextToSpeechAPI>();
    APIInstance->GetVoices();

    return APIInstance;
}

void UTextToSpeechAPI::GetVoices()
{
    // Fetch decrypted API key
    FString DecryptedAPIKey = UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::Labs11);

    // Create an HTTP request and set headers
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("GET");
    HttpRequest->SetURL("https://api.elevenlabs.io/v1/voices");
    HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));
    HttpRequest->SetHeader(TEXT("xi-api-key"), DecryptedAPIKey);

    HttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            TArray<FVoiceInfo> VoiceArray;

            if (bWasSuccessful && Response.IsValid())
            {
                FString JsonResponse = Response->GetContentAsString();

                // Print the raw JSON response
                UE_LOG(AIITKLog, Log, TEXT("Raw JSON Response for GetVoices: %s"), *JsonResponse);

                TSharedPtr<FJsonObject> JsonObject;
                TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonResponse);

                if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
                {
                    TArray<TSharedPtr<FJsonValue>> Voices = JsonObject->GetArrayField(TEXT("voices"));

                    // Pre-allocate space in the array to avoid multiple reallocations
                    VoiceArray.Reserve(Voices.Num());

                    for (const TSharedPtr<FJsonValue>& Voice : Voices)
                    {
                        TSharedPtr<FJsonObject> VoiceObject = Voice->AsObject();
                        FVoiceInfo VoiceInfo;
                        VoiceInfo.VoiceID = VoiceObject->GetStringField(TEXT("voice_id"));
                        VoiceInfo.Name = VoiceObject->GetStringField(TEXT("name"));

                        VoiceArray.Add(VoiceInfo);
                    }

                    // Trigger the delegate with the populated array
                    if (OnVoicesReceived.IsBound())
                    {
                        OnVoicesReceived.Broadcast(VoiceArray);
                    }
                }
                else
                {
                    UE_LOG(AIITKLog, Error, TEXT("Failed to parse JSON response"));
                }
            }
            else
            {
                UE_LOG(AIITKLog, Error, TEXT("HTTP request failed"));
            }
        });

    // Send the HTTP request
    HttpRequest->ProcessRequest();
}

UTextToSpeechAPI* UTextToSpeechAPI::GetModels_BP()
{
    UTextToSpeechAPI* APIInstance = NewObject<UTextToSpeechAPI>();
    APIInstance->GetModels();

    return APIInstance;

}

void UTextToSpeechAPI::GetModels()
{
    // Fetch decrypted API key
    FString DecryptedAPIKey = UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::Labs11);

    // Create an HTTP request and set headers
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("GET");
    HttpRequest->SetURL("https://api.elevenlabs.io/v1/models");
    HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));
    HttpRequest->SetHeader(TEXT("xi-api-key"), DecryptedAPIKey);

    HttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            TArray<FModelInfo> ModelArray;

            if (bWasSuccessful && Response.IsValid())
            {
                FString JsonResponse = Response->GetContentAsString();

                // Print the raw JSON response
                UE_LOG(AIITKLog, Log, TEXT("Raw JSON Response for GetModels: %s"), *JsonResponse);

                TSharedPtr<FJsonObject> JsonObject;
                TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonResponse);
                TArray<TSharedPtr<FJsonValue>> Models;

                if (FJsonSerializer::Deserialize(JsonReader, Models))
                {
                    ModelArray.Reserve(Models.Num());

                    for (const TSharedPtr<FJsonValue>& Model : Models)
                    {
                        TSharedPtr<FJsonObject> ModelObject = Model->AsObject();
                        FModelInfo ModelInfo;
                        ModelInfo.ModelID = ModelObject->GetStringField(TEXT("model_id"));
                        ModelInfo.Name = ModelObject->GetStringField(TEXT("name"));

                        ModelArray.Add(ModelInfo);
                    }

                    // Trigger the delegate with the populated array
                    if (OnModelsReceived.IsBound())
                    {
                        OnModelsReceived.Broadcast(ModelArray);
                    }
                }
                else
                {
                    UE_LOG(AIITKLog, Error, TEXT("Failed to parse JSON response"));
                }
            }
            else
            {
                UE_LOG(AIITKLog, Error, TEXT("HTTP request failed"));
            }
        });

    // Send the HTTP request
    HttpRequest->ProcessRequest();
}