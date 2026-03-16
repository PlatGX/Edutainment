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



 // =================== GPTChatAPI.h ===================

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Http.h"
#include "JsonObjectConverter.h"
#include "JsonObjectWrapper.h"
#include "GPTChatAPIStructs.h"
#include "OpenAIFunctionLibrary.h"
#include "Async/Async.h"
#include "Misc/CoreDelegates.h"
#include "string"
#include "deque"
#include "algorithm"
#include "vector"
#include "sstream"
#include "queue"
#include "Templates/Tuple.h"
#include "GPTChatAPI.generated.h"

// Delegates for asynchronous events in the GPTChat API
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChunkReceived, FGPTChatCompletion, Completion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGPTChatAPICompleted, FGPTChatCompletion, Completion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGPTChatAPIFailed, FGPTChatCompletion, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGPTChatAPIModelsRetrieved, const TArray<FString>&, ModelIds);

// UGPTChatAPI: Class for handling GPT Chat functions
UCLASS(BlueprintType)
class EZUEGPT_API UGPTChatAPI : public UObject
{
    GENERATED_BODY()

public:
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintCallable, Category = "*AIITK|ChatGPT")
    static UGPTChatAPI* SendGPTChatPrompt(const FGPTChatPromptParams& Params);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|ChatGPT")
    static UGPTChatAPI* FetchModelIds();

    // BlueprintAssignable properties for chat events

    // Triggered when a chunk of data is received
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|ChatGPT")
    FChunkReceived OnChunkReceived;

    // Triggered when a chat API call is completed
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|ChatGPT")
    FOnGPTChatAPICompleted OnCompleted;

    // Triggered when a chat API call fails
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|ChatGPT")
    FOnGPTChatAPIFailed OnFailed;

    // Triggered when model IDs are retrieved
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|ChatGPT")
    FOnGPTChatAPIModelsRetrieved OnModelsRetrieved;


    // Properties for managing chat requests and responses

    // Message to use when a request is canceled
    UPROPERTY(BlueprintReadWrite, Category = "*AIITK|ChatGPT")
    FString CancelMessage = "Request Cancelled";

    // Cancels an ongoing chat request
    UFUNCTION(BlueprintCallable, Category = "*AIITK|ChatGPT|ManageChat")
    void CancelRequest();

    // Utility function for processing chat responses
    static TTuple<FString, bool> GetLastMatch(const FString& Pattern, const FString& SearchString, const int CaptureGroup); // Gets the last match of a regex pattern

    // Regex extraction related properties *Moved to BP for abstraction
    //FString LastFullWord = ""; // Stores the last full word matched
    //int32 WordIndex = 0; // Index of the last word
    //FString LocalAccumulatedChunks = ""; // Locally accumulated chunks of data
    //int32 LastProcessedLine = 0; // Last processed line number
    //int32 LastWordPosition = -1; // Position of the last word

    // Current HTTP request for the chat
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentHttpRequest;

    // Variables to handle early termination and request progress

    UPROPERTY(BlueprintReadOnly, Category = "*AIITK|ChatGPT")
    bool bRequestInProgress = false;

    bool bIsShuttingDown = false;

    void SendRequest(const FGPTChatPromptParams& Params);

private:

    int32 ProcessedLines = 0; // Keep track of the number of processed lines.

    // Log probability parsing functions
    static FTopLogProbs ParseSingleTopLogProb(TSharedPtr<FJsonObject> TopLogProbObject);
    static TArray<FTopLogProbs> ParseTopLogProbsArray(TArray<TSharedPtr<FJsonValue>> TopLogProbsArray);
    static FLogProbs ParseLogProbs(TSharedPtr<FJsonObject> LogProbsObject);

    void ParseResponse(const FString& ResponseStr);
    void ParseStreamedResponse(const FString& ResponseStr);

    // HTTP request handling functions
    void HandleError(FHttpResponsePtr Response);

    // Chat request configuration functions

    void ConfigureHttpRequest(TSharedPtr<FJsonObject> JsonObject, const FGPTChatPromptParams& Params);
    void SetAdvancedParams(TSharedPtr<FJsonObject> JsonObject, const FGPTChatAdvancedParams& AdvancedParams);
    void SetFunctionParams(TSharedPtr<FJsonObject> JsonObject, const FGPTChatPromptParams& Params);

    // Utility functions
    static FString GetFunctionCallOptions(const FGPTChatFunctionParams& Options);
    TArray<TSharedPtr<FJsonValue>> SerializeMessages(const TArray<FGPTChatMessage>& Messages);
    TSharedPtr<FJsonObject> SerializeMessage(const FGPTChatMessage& Message);

    static EGPTRole GetRoleFromString(const FString& RoleStr);
    void GetModelIds();
    FString CurrentFunctionName = "";

protected:

    // Properties for storing chat responses and state
    FGPTChatCompletion FullStreamResponse;
    bool CurrentResponseIsFunctionCall = false;

    FGPTChatCompletion CurrentChatResponse;

};
