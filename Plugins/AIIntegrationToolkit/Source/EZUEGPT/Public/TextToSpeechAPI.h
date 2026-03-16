/********************************************************************
 * COPYRIGHT NOTICE                                                 *
 ********************************************************************
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

 // =================== TextToSpeechAPI.h ===================

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GPTChatAPIStructs.h"
#include "Sound/SoundWaveProcedural.h"
#include "OpenAIFunctionLibrary.h"
#include "TimerManager.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "TextToSpeechAPI.generated.h"


// Delegates for various events in the TextToSpeech API
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioInitialized, USoundWaveProcedural*, NewSoundWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioGenerated, USoundWave*, GeneratedAudio, const TArray<uint8>&, RawAudioData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProceduralAudioGenerated, USoundWaveProcedural*, GeneratedAudio, const TArray<uint8>&, RawAudioData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioWithAlignmentGenerated, USoundWave*, GeneratedAudio, const FCharacterAlignment&, AlignmentData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProceduralAudioFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioFailed, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoicesReceived, const TArray<FVoiceInfo>&, Voices);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModelsReceived, const TArray<FModelInfo>&, Models);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioSyncUpdate, float, ElapsedTime);



// UTextToSpeechAPI: Class for handling Text-to-Speech functions
UCLASS(BlueprintType)
class EZUEGPT_API UTextToSpeechAPI : public UObject
{
    GENERATED_BODY()

public:

    virtual void BeginDestroy() override;


    int32 LastProcessedByte = 0;

    // Delegate to broadcast audio sync updates
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Audio")
    FOnAudioSyncUpdate OnAudioSyncUpdate;

    // Static function wrapper for playing audio with synced updates
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Audio", meta = (DisplayName = "Play Audio Synced", ToolTip = "Plays an audio component and tracks elapsed time, broadcasting update ticks."))
    static UTextToSpeechAPI* PlayAudioSynced(UAudioComponent* AudioComponent, float UpdateRate, USoundBase* NewSound = nullptr);

    // Functions exposed to Blueprints for interacting with the Text-to-Speech API
    UFUNCTION(BlueprintCallable, Category = "*AIITK|11Labs", meta = (DisplayName = "Send Text To 11 Labs", ToolTip = "Send text to ElevenLabs Text-to-Speech API and generate audio"))
    static UTextToSpeechAPI* SendTextTo11Labs_BP(const F11LabsRequestParams& Params);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|OpenAI", meta = (DisplayName = "Send Text To OpenAI TTS", ToolTip = "Send text to OpenAI Text-to-Speech API and generate audio"))
    static UTextToSpeechAPI* SendTextToOpenAITTS_BP(const FOpenAITTSRequestParams& Params);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|11Labs", meta = (DisplayName = "Get Voices", ToolTip = "Retrieve available voice models from ElevenLabs"))
    static UTextToSpeechAPI* GetVoices_BP();

    UFUNCTION(BlueprintCallable, Category = "*AIITK|11Labs", meta = (DisplayName = "Get Models", ToolTip = "Retrieve available models from ElevenLabs"))
    static UTextToSpeechAPI* GetModels_BP();

    // BlueprintAssignable properties for various audio events

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|TextToSpeech")
    FOnAudioWithAlignmentGenerated OnAudioWithAlignmentGenerated;

    // Triggers on request completion, provides a normal sound wave
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|TextToSpeech")
    FOnAudioGenerated OnAudioGenerated;

    // Triggers on API initial response after proc sound wave creation
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|TextToSpeech")
    FOnAudioInitialized OnAudioInitialized;

    // Triggers when a streamed request recieves data from the API
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|TextToSpeech")
    FOnProceduralAudioGenerated OnProceduralAudioGenerated;

    // Triggers when the proc sound wave has run out of data to play
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|TextToSpeech")
    FOnProceduralAudioFinished OnProceduralAudioFinished;

    // More error info provided in logs
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|TextToSpeech")
    FOnAudioFailed OnAudioFailed;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|11Labs")
    FOnVoicesReceived OnVoicesReceived;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|11Labs")
    FOnModelsReceived OnModelsReceived;

    // Member variables for audio processing
    UPROPERTY(BlueprintReadWrite, Category = "*AIITK|TextToSpeech")
    USoundWaveProcedural* TTSProcSoundWave;

    UPROPERTY(BlueprintReadOnly, Category = "*AIITK|TextToSpeech")
    bool bRequestInProgress = false;



private:

    // Internal function to handle the audio playing and syncing logic
    void PlayAudioSynced_Internal(UAudioComponent* AudioComponent, float UpdateRate, USoundBase* NewSound);

    // Timer handle for managing update ticks
    FTimerHandle AudioSyncTimerHandle;

    // Internal function to update elapsed time and broadcast through delegate
    void UpdateElapsedTime(UAudioComponent* AudioComponent, float& ElapsedTime, float UpdateRate);

    // Internal functions for handling API requests and audio processing
    void SendTextTo11Labs_Internal(const F11LabsRequestParams& Params);

    void SendTextToOpenAITTS_Internal(const FOpenAITTSRequestParams& Params);

    USoundWaveProcedural* ProcessAudioResponse(const TArray<uint8>& Content, const F11LabsRequestParams& Params);

    USoundWaveProcedural* ProcessOpenAIAudioResponse(const TArray<uint8>& Content, const FOpenAITTSRequestParams& Params);

    void GetVoices();

    void GetModels();

    int32 AlreadyProcessedDataSize = 0;

    int32 AudioBufferThreshold = 240000;  // Buffer size in bytes

    TArray<uint8> AudioBuffer;

    bool AudioIsInitialized = false;

    bool AudioIsFinished = false;


    // Handler for procedural underflow event
    void OnProceduralUnderflowHandler(USoundWaveProcedural* InProceduralSoundWave, int32 SamplesRequired);
};
