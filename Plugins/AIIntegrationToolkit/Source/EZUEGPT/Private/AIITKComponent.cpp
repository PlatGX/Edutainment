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



// =================== AIITKComponent.cpp ===================

#include "AIITKComponent.h"



UAIITKComponent::UAIITKComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Initialize API instances to nullptr
    GPTAPIInstance = nullptr;
    TextToSpeechAPIInstance = nullptr;
    DallEAPIInstance = nullptr;
    WhisperAPIInstance = nullptr;
}


UGPTChatAPI* UAIITKComponent::SendChatGPTRequest(FGPTChatPromptParams Params)
{
    GPTAPIInstance = UGPTChatAPI::SendGPTChatPrompt(Params);
    GPTAPIInstance->OnChunkReceived.AddDynamic(this, &UAIITKComponent::OnChunkReceivedHandler);
    GPTAPIInstance->OnCompleted.AddDynamic(this, &UAIITKComponent::OnGPTCompletedHandler);
    GPTAPIInstance->OnFailed.AddDynamic(this, &UAIITKComponent::OnGPTFailedHandler);
    return GPTAPIInstance;
}

UGPTChatAPI* UAIITKComponent::FetchGPTModelIds()
{
    GPTAPIInstance = UGPTChatAPI::FetchModelIds();
    if (GPTAPIInstance)
    {
        GPTAPIInstance->OnModelsRetrieved.AddDynamic(this, &UAIITKComponent::OnGPTModelsRetrievedHandler);
    }
    return GPTAPIInstance;
}


UDallEAPI* UAIITKComponent::SendDallEImageRequest(FDallEAPIParams Params)
{
    DallEAPIInstance = UDallEAPI::SendDallEImageRequest_BP(Params);
    DallEAPIInstance->DallEOnCompleted.AddDynamic(this, &UAIITKComponent::OnDallECompletedHandler);
    DallEAPIInstance->OnError.AddDynamic(this, &UAIITKComponent::OnDallEErrorHandler);
    return DallEAPIInstance;
}

UWhisperAPI* UAIITKComponent::SendWhisperTranscriptRequest(FWhisperAPIParams Params)
{
    WhisperAPIInstance = UWhisperAPI::RequestWhisperTranscript(Params);
    WhisperAPIInstance->OnWhisperAPIResponse.AddDynamic(this, &UAIITKComponent::OnWhisperTranscriptGeneratedHandler);
    WhisperAPIInstance->OnWhisperAPIFailed.AddDynamic(this, &UAIITKComponent::OnWhisperErrorHandler);
    return WhisperAPIInstance;
}


UTextToSpeechAPI* UAIITKComponent::SendTextTo11LabsRequest(F11LabsRequestParams Params)
{
    TextToSpeechAPIInstance = UTextToSpeechAPI::SendTextTo11Labs_BP(Params);
    TextToSpeechAPIInstance->OnAudioInitialized.AddDynamic(this, &UAIITKComponent::OnAudioInitializedHandler);
    TextToSpeechAPIInstance->OnAudioGenerated.AddDynamic(this, &UAIITKComponent::OnAudioGeneratedHandler);
    TextToSpeechAPIInstance->OnProceduralAudioGenerated.AddDynamic(this, &UAIITKComponent::OnProcAudioGeneratedHandler);
    TextToSpeechAPIInstance->OnProceduralAudioFinished.AddDynamic(this, &UAIITKComponent::OnAudioFinishedHandler);
    TextToSpeechAPIInstance->OnAudioFailed.AddDynamic(this, &UAIITKComponent::OnAudioFailedHandler);
    return TextToSpeechAPIInstance;
}

UTextToSpeechAPI* UAIITKComponent::SendTextToOpenAITTSRequest(FOpenAITTSRequestParams Params)
{
    TextToSpeechAPIInstance = UTextToSpeechAPI::SendTextToOpenAITTS_BP(Params);
    TextToSpeechAPIInstance->OnAudioInitialized.AddDynamic(this, &UAIITKComponent::OnAudioInitializedHandler);
    TextToSpeechAPIInstance->OnAudioGenerated.AddDynamic(this, &UAIITKComponent::OnAudioGeneratedHandler);
    TextToSpeechAPIInstance->OnProceduralAudioGenerated.AddDynamic(this, &UAIITKComponent::OnProcAudioGeneratedHandler);
    TextToSpeechAPIInstance->OnProceduralAudioFinished.AddDynamic(this, &UAIITKComponent::OnAudioFinishedHandler);
    TextToSpeechAPIInstance->OnAudioFailed.AddDynamic(this, &UAIITKComponent::OnAudioFailedHandler);
    return TextToSpeechAPIInstance;
}

//UTextToSpeechAPI* UAIITKComponent::GetVoicesFrom11Labs()
//{
//    TextToSpeechAPIInstance = NewObject<UTextToSpeechAPI>();  // Initialize it accordingly
//    TextToSpeechAPIInstance->GetVoices_BP();
//
//    // Bind the OnVoicesReceived delegate
//    TextToSpeechAPIInstance->OnVoicesReceived.AddDynamic(this, &UAIITKComponent::OnVoicesReceivedHandler);
//
//    return TextToSpeechAPIInstance;
//}
//
//UTextToSpeechAPI* UAIITKComponent::GetModelsFrom11Labs()
//{
//    TextToSpeechAPIInstance = NewObject<UTextToSpeechAPI>();  // Initialize it accordingly
//    TextToSpeechAPIInstance->GetModels_BP();
//
//    // Bind the OnModelsReceived delegate
//    TextToSpeechAPIInstance->OnModelsReceived.AddDynamic(this, &UAIITKComponent::OnModelsReceivedHandler);
//
//    return TextToSpeechAPIInstance;
//}


void UAIITKComponent::OnChunkReceivedHandler_Implementation(FGPTChatCompletion Completion)
{
    OnGPTChunkReceived.Broadcast(Completion);
}

void UAIITKComponent::OnGPTCompletedHandler_Implementation(FGPTChatCompletion Completion)
{
    OnGPTCompleted.Broadcast(Completion);
}

void UAIITKComponent::OnGPTFailedHandler_Implementation(FGPTChatCompletion ErrorMessage)
{
    OnGPTFailed.Broadcast(ErrorMessage);
}

void UAIITKComponent::OnGPTModelsRetrievedHandler_Implementation(const TArray<FString>& ModelIds)
{
    OnGPTModelsRetrieved.Broadcast(ModelIds);
}


void UAIITKComponent::OnAudioInitializedHandler_Implementation(USoundWaveProcedural* NewSoundWave)
{
    OnAudioInitialized.Broadcast(NewSoundWave);
}

void UAIITKComponent::OnAudioGeneratedHandler_Implementation(USoundWave* GeneratedAudio, const TArray<uint8>& RawAudioData)
{
    OnAudioGenerated.Broadcast(GeneratedAudio, RawAudioData);
}

void UAIITKComponent::OnProcAudioGeneratedHandler_Implementation(USoundWaveProcedural* GeneratedAudio, const TArray<uint8>& RawAudioData)
{
    OnProcAudioGenerated.Broadcast(GeneratedAudio, RawAudioData);
}

void UAIITKComponent::OnAudioFinishedHandler_Implementation()
{
    OnAudioFinished.Broadcast();
}

void UAIITKComponent::OnAudioFailedHandler_Implementation(const FString& ErrorMessage)
{
    OnError.Broadcast(ErrorMessage);
}

//void UAIITKComponent::OnVoicesReceivedHandler_Implementation(const TArray<FVoiceInfo>& Voices)
//{
//    OnVoicesReceived.Broadcast(Voices);
//}
//
//void UAIITKComponent::OnModelsReceivedHandler_Implementation(const TArray<FModelInfo>& Models)
//{
//    OnModelsReceived.Broadcast(Models);
//}


void UAIITKComponent::OnDallECompletedHandler_Implementation(const TArray<FString>& ImageURLs, const TArray<FString>& RevisedPrompts)
{
    OnDallECompleted.Broadcast(ImageURLs, RevisedPrompts);
}

void UAIITKComponent::OnDallEErrorHandler_Implementation(const FString& ErrorMessage)
{
    OnDallEError.Broadcast(ErrorMessage);
}


void UAIITKComponent::OnWhisperTranscriptGeneratedHandler_Implementation(const FString& Transcription)
{
    OnWhisperTranscriptGenerated.Broadcast(Transcription);
}

void UAIITKComponent::OnWhisperErrorHandler_Implementation(const FString& ErrorMessage)
{
    OnWhisperError.Broadcast(ErrorMessage);
}


