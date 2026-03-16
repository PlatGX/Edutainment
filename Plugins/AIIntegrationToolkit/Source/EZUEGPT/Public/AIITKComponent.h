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

// =================== AIITKComponent.h ===================

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GPTChatAPI.h"
#include "TextToSpeechAPI.h"
#include "DallEAPI.h" 
#include "GPTChatAPIStructs.h"
#include "WhisperAPI.h"
#include "OpenAIFunctionLibrary.h"
#include "AIITKComponent.generated.h"

UCLASS(ClassGroup = (AIITK), meta = (BlueprintSpawnableComponent), Blueprintable, Category = "AIITK")
class EZUEGPT_API UAIITKComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAIITKComponent();

    // ============================ Internal|Components|Requests ============================

    UFUNCTION(BlueprintCallable, Category = "Internal|Components|Requests")
    virtual UGPTChatAPI* SendChatGPTRequest(FGPTChatPromptParams Params);

    UFUNCTION(BlueprintCallable, Category = "Internal|Components|Requests")
    virtual UGPTChatAPI* FetchGPTModelIds();

    UFUNCTION(BlueprintCallable, Category = "Internal|Components|Requests")
    virtual UTextToSpeechAPI* SendTextTo11LabsRequest(F11LabsRequestParams Params); 

    UFUNCTION(BlueprintCallable, Category = "Internal|Components|Requests")
    virtual UTextToSpeechAPI* SendTextToOpenAITTSRequest(FOpenAITTSRequestParams Params);

    //UFUNCTION(BlueprintCallable, Category = "Internal|Components|Requests")
    //virtual UTextToSpeechAPI* GetVoicesFrom11Labs();

    //UFUNCTION(BlueprintCallable, Category = "Internal|Components|Requests")
    //virtual UTextToSpeechAPI* GetModelsFrom11Labs();

    UFUNCTION(BlueprintCallable, Category = "Internal|Components|Requests")
    virtual UDallEAPI* SendDallEImageRequest(FDallEAPIParams Params);

    UFUNCTION(BlueprintCallable, Category = "Internal|Components|Requests")
    virtual UWhisperAPI* SendWhisperTranscriptRequest(FWhisperAPIParams Params);

protected:

    // ============================ *AIITK|Components|Handlers ============================

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnChunkReceivedHandler(FGPTChatCompletion Completion);

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnGPTCompletedHandler(FGPTChatCompletion Completion);

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnGPTFailedHandler(FGPTChatCompletion ErrorMessage);

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnGPTModelsRetrievedHandler(const TArray<FString>& ModelIds);

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnAudioInitializedHandler(USoundWaveProcedural* NewSoundWave);

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnAudioGeneratedHandler(USoundWave* GeneratedAudio, const TArray<uint8>& RawAudioData);

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnProcAudioGeneratedHandler(USoundWaveProcedural* GeneratedAudio, const TArray<uint8>& RawAudioData);

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnAudioFinishedHandler();

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnAudioFailedHandler(const FString& ErrorMessage);

    //UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    //void OnVoicesReceivedHandler(const TArray<FVoiceInfo>& Voices);

    //UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    //void OnModelsReceivedHandler(const TArray<FModelInfo>& Models);

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnDallECompletedHandler(const TArray<FString>& ImageURLs, const TArray<FString>& RevisedPrompts);

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnDallEErrorHandler(const FString& ErrorMessage);

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnWhisperTranscriptGeneratedHandler(const FString& Transcription);

    UFUNCTION(BlueprintNativeEvent, Category = "*AIITK|Components|Handlers")
    void OnWhisperErrorHandler(const FString& ErrorMessage);


    // ============================ AIITKComponent|Responses|ChatGPT ============================

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|ChatGPT", DisplayName = "On GPT Chunk Received")
    FChunkReceived OnGPTChunkReceived;

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|ChatGPT", DisplayName = "On GPT Completed")
    FOnGPTChatAPICompleted OnGPTCompleted;

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|ChatGPT", DisplayName = "On GPT Error")
    FOnGPTChatAPIFailed OnGPTFailed;

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|ChatGPT", DisplayName = "On GPT Models Retrieved")
    FOnGPTChatAPIModelsRetrieved OnGPTModelsRetrieved;


    // ============================ AIITKComponent|Responses|11Labs ============================

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|11Labs", DisplayName = "On Audio Initialized")
    FOnAudioInitialized OnAudioInitialized;

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|11Labs", DisplayName = "On Audio Generated")
    FOnAudioGenerated OnAudioGenerated;

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|11Labs", DisplayName = "On Audio Chunk Received")
    FOnProceduralAudioGenerated OnProcAudioGenerated;

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|11Labs", DisplayName = "On Audio Finished")
    FOnProceduralAudioFinished OnAudioFinished;

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|11Labs", DisplayName = "On Audio Error")
    FOnAudioFailed OnError;

    //UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|11Labs", DisplayName = "On 11Labs Voices Received")
    //FOnVoicesReceived OnVoicesReceived;

    //UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|11Labs", DisplayName = "On 11Labs Models Received")
    //FOnModelsReceived OnModelsReceived;

    // ============================ AIITKComponent|Responses|Dalle ============================

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|Dalle", DisplayName = "On DallE Completed")
    FOnDallEAPICompleted OnDallECompleted;

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|Dalle", DisplayName = "On DallE Error")
    FOnDallEAPIError OnDallEError;

    // ============================ AIITKComponent|Responses|Whisper ============================

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|Whisper", DisplayName = "On Whisper Completed")
    FOnWhisperAPIResponse OnWhisperTranscriptGenerated;

    UPROPERTY(BlueprintAssignable, Category = "AIITKComponent|Responses|Whisper", DisplayName = "On Whisper Error")
    FOnWhisperAPIFailed OnWhisperError;

    UGPTChatAPI* GPTAPIInstance;
    UTextToSpeechAPI* TextToSpeechAPIInstance;
    UDallEAPI* DallEAPIInstance;
    UWhisperAPI* WhisperAPIInstance;
};