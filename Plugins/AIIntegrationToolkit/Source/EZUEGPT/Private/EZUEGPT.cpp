#include "EZUEGPT.h"

#define LOCTEXT_NAMESPACE "FEZUEGPTModule"

void FEZUEGPTModule::StartupModule()
{
    // Initialization logic
}

void FEZUEGPTModule::ShutdownModule()
{
    // Cleanup logic
    OngoingRequests.Empty();
}

UGPTChatAPI* FEZUEGPTModule::CreateGPTChatAPIInstance()
{
    UGPTChatAPI* GPTAPIInstance = NewObject<UGPTChatAPI>();
    TStrongObjectPtr<UGPTChatAPI> StrongGPTAPIInstance = TStrongObjectPtr<UGPTChatAPI>(GPTAPIInstance);
    OngoingRequests.Add(StrongGPTAPIInstance);
    return GPTAPIInstance;
}

UTextToSpeechAPI* FEZUEGPTModule::CreateTextToSpeechAPIInstance()
{
    UTextToSpeechAPI* TTSAPIInstance = NewObject<UTextToSpeechAPI>();
    TStrongObjectPtr<UTextToSpeechAPI> StrongTTSAPIInstance = TStrongObjectPtr<UTextToSpeechAPI>(TTSAPIInstance);
    OngoingRequests.Add(StrongTTSAPIInstance);
    return TTSAPIInstance;
}

void FEZUEGPTModule::AddRequest(TStrongObjectPtr<UObject> Request)
{
    OngoingRequests.Add(Request);
}

void FEZUEGPTModule::RemoveRequest(TStrongObjectPtr<UObject> Request)
{
    OngoingRequests.Remove(Request);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEZUEGPTModule, EZUEGPT)
