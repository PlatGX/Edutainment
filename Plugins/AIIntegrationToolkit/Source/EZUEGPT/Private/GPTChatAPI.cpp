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



// =================== GPTChatAPI.cpp ===================

#include "GPTChatAPI.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "UObject/TextProperty.h"
#include "AIITKDeveloperSettings.h"
#include "EZUEGPT.h"
#include "Engine/UserDefinedStruct.h"


void UGPTChatAPI::BeginDestroy()
{
    bIsShuttingDown = true;
    CancelRequest();

    // Remove from root to allow garbage collection
    RemoveFromRoot();

    Super::BeginDestroy();
}


UGPTChatAPI* UGPTChatAPI::SendGPTChatPrompt(const FGPTChatPromptParams& Params)
{
    UGPTChatAPI* GPTAPIInstance = NewObject<UGPTChatAPI>();
    GPTAPIInstance->AddToRoot();  // Prevent garbage collection
    GPTAPIInstance->SendRequest(Params);
    return GPTAPIInstance;
}


void UGPTChatAPI::SendRequest(const FGPTChatPromptParams& Params)
{
    UE_LOG(AIITKLog, Warning, TEXT("SendRequest called with Params."));

    if (bRequestInProgress)
    {
        UE_LOG(AIITKLog, Error, TEXT("Request already in progress"));
        return;
    }

    bRequestInProgress = true;
    ProcessedLines = 0;
    CurrentChatResponse = FGPTChatCompletion(); // Reset the member variable at the start of a new request
    UE_LOG(AIITKLog, Warning, TEXT("Request processing initiated."));

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();

    UE_LOG(AIITKLog, Warning, TEXT("HTTP request created."));

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    ConfigureHttpRequest(JsonObject, Params);
    UE_LOG(AIITKLog, Warning, TEXT("HTTP request configured."));

    SetFunctionParams(JsonObject, Params);
    UE_LOG(AIITKLog, Warning, TEXT("Function params set."));

    SetAdvancedParams(JsonObject, Params.AdvancedParams);
    UE_LOG(AIITKLog, Warning, TEXT("Advanced params set."));

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: Raw Request: %s"), *RequestBody);

    CurrentHttpRequest->SetContentAsString(RequestBody);
    UE_LOG(AIITKLog, Warning, TEXT("Request content set."));

    if (Params.RequiredParams.StreamResponse)
    {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
        CurrentHttpRequest->OnRequestProgress64().BindLambda([this](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
#else
        CurrentHttpRequest->OnRequestProgress().BindLambda([this](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
#endif
        {
            FHttpResponsePtr Response = Request->GetResponse();
            if (Response)
            {
                FString ResponseStr = Response->GetContentAsString();
                UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: Raw Streamed Response: %s"), *ResponseStr); // Log the raw streamed response

                if (!ResponseStr.IsEmpty())
                {
                    ParseStreamedResponse(ResponseStr);
                    UE_LOG(AIITKLog, Warning, TEXT("Streamed response received and parsed."));

                    AsyncTask(ENamedThreads::GameThread, [this]()
                    {
                        this->OnChunkReceived.Broadcast(CurrentChatResponse);
                    });
                    UE_LOG(AIITKLog, Warning, TEXT("ChunkReceived event broadcasted."));
                }
            }
        });
    }

    CurrentHttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        if (this->bIsShuttingDown)
        {
            UE_LOG(AIITKLog, Warning, TEXT("Request halted due to shutdown."));
            return;
        }

        if (Response)
        {
            FString ResponseStr = Response->GetContentAsString();
            UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: Raw Response: %s"), *ResponseStr); // Log the raw response

            if (bWasSuccessful && Response->GetResponseCode() == 200)
            {
                ParseResponse(ResponseStr);
                UE_LOG(AIITKLog, Warning, TEXT("Response received and parsed."));

                AsyncTask(ENamedThreads::GameThread, [this]()
                {
                    this->OnCompleted.Broadcast(CurrentChatResponse);
                });
                UE_LOG(AIITKLog, Warning, TEXT("Completed event broadcasted."));
            }
            else
            {
                HandleError(Response);
                UE_LOG(AIITKLog, Error, TEXT("Error handling triggered."));
            }
        }
        else
        {
            UE_LOG(AIITKLog, Error, TEXT("Received null response."));
        }

        if (Request->GetStatus() == EHttpRequestStatus::Failed)
        {
            HandleError(Response);
            UE_LOG(AIITKLog, Error, TEXT("Request failed, error handling triggered."));
        }
        this->bRequestInProgress = false;
        UE_LOG(AIITKLog, Warning, TEXT("Request processing completed."));
        // Remove from root to allow garbage collection
        RemoveFromRoot();
    });

    CurrentHttpRequest->ProcessRequest();
    UE_LOG(AIITKLog, Warning, TEXT("HTTP request processing started."));
}





// ============ CONFIGURATION ============

void UGPTChatAPI::ConfigureHttpRequest(TSharedPtr<FJsonObject> JsonObject, const FGPTChatPromptParams& Params)
{
    if (Params.RequiredParams.StreamResponse)
    {
        JsonObject->SetBoolField(TEXT("stream"), true);
        CurrentHttpRequest->SetHeader(TEXT("Accept"), TEXT("text/event-stream"));
    }

    // Construct the request body
    JsonObject->SetStringField(TEXT("model"), Params.RequiredParams.Model);
    JsonObject->SetArrayField(TEXT("messages"), SerializeMessages(Params.Messages));

    // Set default header for JSON content
    CurrentHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // Fetch decrypted API key
    FString DecryptedAPIKey = UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI);

    // Set the decrypted API key in the Authorization header
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *DecryptedAPIKey));
    CurrentHttpRequest->SetHeader(TEXT("Connection"), TEXT("keep-alive"));

    // Remaining configurations
    CurrentHttpRequest->SetURL(Params.RequiredParams.Endpoint);
    CurrentHttpRequest->SetVerb(TEXT("POST"));
    CurrentHttpRequest->SetTimeout(Params.AdvancedParams.ResponseTimeout);
}

void UGPTChatAPI::SetAdvancedParams(TSharedPtr<FJsonObject> JsonObject, const FGPTChatAdvancedParams& AdvancedParams)
{
    if (AdvancedParams.FrequencyPenalty != 0.0f)
    {
        JsonObject->SetNumberField(TEXT("frequency_penalty"), AdvancedParams.FrequencyPenalty);
    }
    if (AdvancedParams.LogitBias.Num() > 0)
    {
        JsonObject->SetObjectField(TEXT("logit_bias"), UOpenAIFunctionLibrary::CreateLogitBiasObject(AdvancedParams.LogitBias));
    }
    if (AdvancedParams.LogProbs)
    {
        JsonObject->SetBoolField(TEXT("logprobs"), AdvancedParams.LogProbs);
    }
    if (AdvancedParams.TopLogProbs > 0)
    {
        JsonObject->SetNumberField(TEXT("top_logprobs"), AdvancedParams.TopLogProbs);
    }
    if (AdvancedParams.MaxTokens > 0)
    {
        JsonObject->SetNumberField(TEXT("max_tokens"), AdvancedParams.MaxTokens);
    }
    if (AdvancedParams.N != 1)
    {
        JsonObject->SetNumberField(TEXT("n"), AdvancedParams.N);
    }
    if (AdvancedParams.PresencePenalty != 0.0f)
    {
        JsonObject->SetNumberField(TEXT("presence_penalty"), AdvancedParams.PresencePenalty);
    }
    if (AdvancedParams.ResponseFormat == EGPTResponseFormat::JsonObject)
    {
        TSharedPtr<FJsonObject> ResponseFormatObject = MakeShareable(new FJsonObject());
        ResponseFormatObject->SetStringField(TEXT("type"), TEXT("json_object"));
        JsonObject->SetObjectField(TEXT("response_format"), ResponseFormatObject);
    }
    else if (AdvancedParams.ResponseFormat == EGPTResponseFormat::Text)
    {
        TSharedPtr<FJsonObject> ResponseFormatObject = MakeShareable(new FJsonObject());
        ResponseFormatObject->SetStringField(TEXT("type"), TEXT("text"));
        JsonObject->SetObjectField(TEXT("response_format"), ResponseFormatObject);
    }
    if (AdvancedParams.Seed != -1)
    {
        JsonObject->SetNumberField(TEXT("seed"), AdvancedParams.Seed);
    }
    if (!AdvancedParams.Stop.IsEmpty())
    {
        JsonObject->SetStringField(TEXT("stop"), AdvancedParams.Stop);
    }
    if (AdvancedParams.Temperature != 1.0f)
    {
        JsonObject->SetNumberField(TEXT("temperature"), AdvancedParams.Temperature);
    }
    if (AdvancedParams.TopP != 1.0f)
    {
        JsonObject->SetNumberField(TEXT("top_p"), AdvancedParams.TopP);
    }
    if (!AdvancedParams.User.IsEmpty())
    {
        JsonObject->SetStringField(TEXT("user"), AdvancedParams.User);
    }
}

void UGPTChatAPI::SetFunctionParams(TSharedPtr<FJsonObject> JsonObject, const FGPTChatPromptParams& Params)
{
    // If tools are present, add them to the request
    if (Params.ToolParams.Tools.Num() > 0)
    {
        JsonObject->SetArrayField(TEXT("tools"), UOpenAIFunctionLibrary::CreateFunctionsArrayForAPI(Params.ToolParams.Tools));
    }

    // Get tool choice option
    FString ToolChoiceOption = GetFunctionCallOptions(Params.ToolParams.ToolChoice);

    // Handle different function call modes
    switch (Params.ToolParams.ToolChoice.FunctionCallMode)
    {
    case EFunctionCallMode::SpecificFunction:
    {
        TSharedPtr<FJsonObject> ToolChoiceObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ToolChoiceOption);
        if (FJsonSerializer::Deserialize(Reader, ToolChoiceObject) && ToolChoiceObject.IsValid())
        {
            JsonObject->SetObjectField(TEXT("tool_choice"), ToolChoiceObject);
        }
    }
    break;

    case EFunctionCallMode::Auto:
        JsonObject->SetStringField(TEXT("tool_choice"), ToolChoiceOption);
        break;

    case EFunctionCallMode::None:
        JsonObject->SetStringField(TEXT("tool_choice"), ToolChoiceOption);
        break;

    case EFunctionCallMode::Null:
        break;

    default:
        break;
    }
}

FString UGPTChatAPI::GetFunctionCallOptions(const FGPTChatFunctionParams& Options)
{
    FString result;

    switch (Options.FunctionCallMode)
    {
    case EFunctionCallMode::Auto:
        result = TEXT("auto");
        break;

    case EFunctionCallMode::None:
        result = TEXT("none");
        break;

    case EFunctionCallMode::SpecificFunction:
        result = FString::Printf(TEXT("{\"type\": \"function\", \"function\": {\"name\": \"%s\"}}"), *Options.SpecificFunctionName);
        break;

    case EFunctionCallMode::Null:
        result = TEXT("");
        break;

    default:
        break;
    }

    return result;
}

TArray<TSharedPtr<FJsonValue>> UGPTChatAPI::SerializeMessages(const TArray<FGPTChatMessage>& Messages)
{
    TArray<TSharedPtr<FJsonValue>> JsonMessagesArray;

    for (const FGPTChatMessage& Message : Messages)
    {
        TSharedPtr<FJsonObject> JsonMessageObject = SerializeMessage(Message);
        JsonMessagesArray.Add(MakeShareable(new FJsonValueObject(JsonMessageObject)));
    }

    return JsonMessagesArray;
}

TSharedPtr<FJsonObject> UGPTChatAPI::SerializeMessage(const FGPTChatMessage& Message)
{
    TSharedPtr<FJsonObject> JsonMessageObject = MakeShareable(new FJsonObject);

    // Use GetRoleAsString for role conversion
    FString RoleString = UOpenAIFunctionLibrary::GetRoleAsString(Message.Role);
    JsonMessageObject->SetStringField(TEXT("role"), RoleString);

    // Include the name field if it is not empty
    if (!Message.Name.IsEmpty())
    {
        JsonMessageObject->SetStringField(TEXT("name"), Message.Name);
    }

    // Serialize message content based on the role
    switch (Message.Role)
    {
    case EGPTRole::system:
        JsonMessageObject->SetStringField(TEXT("content"), Message.SystemMessage.Content);
        break;

    case EGPTRole::user:
    {
        TArray<TSharedPtr<FJsonValue>> ContentArray;
        for (const FContentPart& Part : Message.UserMessage.Content)
        {
            TSharedPtr<FJsonObject> PartObject = MakeShareable(new FJsonObject);
            PartObject->SetStringField(TEXT("type"), Part.Type == EContentType::Text ? TEXT("text") : TEXT("image_url"));

            // Check if the content type is an image URL or base64 image data
            if (Part.Type == EContentType::Image)
            {
                FString ContentValue = Part.Content;

                // Regex patterns for URL and base64 specifier
                FRegexPattern UrlPattern(TEXT("^http[s]?://.*"));
                FRegexPattern Base64Pattern(TEXT("^(data:image\\/(jpeg|png|gif|bmp);base64,)"));
                FRegexMatcher UrlMatcher(UrlPattern, ContentValue);
                FRegexMatcher Base64Matcher(Base64Pattern, ContentValue);

                // Check if content is not a URL or base64 data
                if (!UrlMatcher.FindNext() && !Base64Matcher.FindNext())
                {
                    // Handle garbage data: prepend base64 specifier
                    ContentValue = FString::Printf(TEXT("data:image/png;base64,%s"), *ContentValue);
                }

                TSharedPtr<FJsonObject> ImageObject = MakeShareable(new FJsonObject);
                ImageObject->SetStringField(TEXT("url"), ContentValue);
                PartObject->SetObjectField(TEXT("image_url"), ImageObject);
            }
            else
            {
                PartObject->SetStringField(TEXT("text"), Part.Content);
            }

            if (!Part.Detail.IsEmpty())
            {
                PartObject->SetStringField(TEXT("detail"), Part.Detail);
            }
            ContentArray.Add(MakeShareable(new FJsonValueObject(PartObject)));
        }
        JsonMessageObject->SetArrayField(TEXT("content"), ContentArray);
    }
    break;

    case EGPTRole::assistant:
        JsonMessageObject->SetStringField(TEXT("content"), Message.AssistantMessage.Content);
        // Serialize tool calls if any
        if (Message.AssistantMessage.ToolCalls.Num() > 0)
        {
            TArray<TSharedPtr<FJsonValue>> ToolCallsArray;
            for (const FToolCall& ToolCall : Message.AssistantMessage.ToolCalls)
            {
                TSharedPtr<FJsonObject> ToolCallObject = MakeShareable(new FJsonObject);
                ToolCallObject->SetStringField(TEXT("id"), ToolCall.ID);
                ToolCallObject->SetStringField(TEXT("type"), ToolCall.Type);
                TSharedPtr<FJsonObject> FunctionObject = MakeShareable(new FJsonObject);
                FunctionObject->SetStringField(TEXT("name"), ToolCall.FunctionName);
                FunctionObject->SetStringField(TEXT("arguments"), ToolCall.FunctionArguments);
                ToolCallObject->SetObjectField(TEXT("function"), FunctionObject);

                ToolCallsArray.Add(MakeShareable(new FJsonValueObject(ToolCallObject)));
            }
            JsonMessageObject->SetArrayField(TEXT("tool_calls"), ToolCallsArray);
        }
        break;

    case EGPTRole::tool:
        JsonMessageObject->SetStringField(TEXT("content"), Message.ToolMessage.Content);
        JsonMessageObject->SetStringField(TEXT("tool_call_id"), Message.ToolMessage.ToolCallID);
        break;

    default:
        break; // Handle other roles if necessary
    }

    return JsonMessageObject;
}



// ============ PARSING/OPERATIONS ============


void UGPTChatAPI::ParseStreamedResponse(const FString& RawStream)
{
    // Declare vars for data we need to retrieve
    TSharedPtr<FJsonObject> MainJsonObject;
    FString CompletionID = "";
    FString CompletionObject = "";
    int32 CompletionCreated = 0;
    FString CompletionModel = "";
    FString CompletionSystemFingerprint = "";

    TArray<TSharedPtr<FJsonValue>> ChoicesArray;
    TSharedPtr<FJsonObject> ChoiceObject;
    int32 ChoiceIndex = 0;
    FString ChoiceFinishReason = "";

    TSharedPtr<FJsonObject> ChoiceDeltaObject;
    EGPTRole DeltaRole = EGPTRole::assistant;
    FString TempRole;
    FString DeltaContent = "";
    FLogProbs DeltaLogProbs;

    TArray<TSharedPtr<FJsonValue>> DeltaToolArray;
    int32 ToolIndex = 0;
    FString ToolID = "";
    FString ToolType = "";
    bool HasToolField = false;

    TSharedPtr<FJsonObject> ToolFunctionObject;
    FString FunctionName = "";
    FString FunctionArguments = "";

    // Split the raw stream into lines
    TArray<FString> Lines;
    RawStream.ParseIntoArray(Lines, TEXT("\n"), true);

    // Start processing from the next unprocessed line
    for (int32 LineIndex = ProcessedLines; LineIndex < Lines.Num(); LineIndex++)
    {
        // Update ProcessedLines to indicate this line is being processed
        ProcessedLines++;

        // Remove the "data: " prefix
        FString CurrentLine = Lines[LineIndex].RightChop(6);
        UE_LOG(AIITKLog, Warning, TEXT("Current chunk being processed: %s"), *CurrentLine);

        // Create a JSON Reader
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(CurrentLine);

        // Parse the line into the JSON object
        if (!FJsonSerializer::Deserialize(Reader, MainJsonObject) || !MainJsonObject.IsValid())
        {
            UE_LOG(AIITKLog, Warning, TEXT("Invalid JSON: %s"), *CurrentLine);
            return; // Skip processing if JSON is invalid
        }
        else
        {
            //////////////////////
            // Root JSON Object //
            //////////////////////
            //
            MainJsonObject->TryGetStringField(TEXT("id"), CompletionID);
            MainJsonObject->TryGetStringField(TEXT("object"), CompletionObject);
            MainJsonObject->TryGetNumberField(TEXT("created"), CompletionCreated);
            MainJsonObject->TryGetStringField(TEXT("model"), CompletionModel);
            MainJsonObject->TryGetStringField(TEXT("system_fingerprint"), CompletionSystemFingerprint);

            //////////////
            // Choices  //
            //////////////
            //
            if (MainJsonObject->HasField(TEXT("choices")))
            {
                ChoicesArray = MainJsonObject->GetArrayField(TEXT("choices"));
                for (int32 CIndex = 0; CIndex < ChoicesArray.Num(); CIndex++)
                {
                    ChoiceObject = ChoicesArray[CIndex]->AsObject();
                    if (ChoiceObject.IsValid())
                    {
                        ChoiceObject->TryGetNumberField(TEXT("index"), ChoiceIndex);
                        ChoiceObject->TryGetStringField(TEXT("finish_reason"), ChoiceFinishReason);


                        //////////////
                        // LogProbs //
                        //////////////
                        //
                        const TSharedPtr<FJsonObject>* LogProbsObjectPtr = nullptr;
                        if (ChoiceObject->TryGetObjectField(TEXT("logprobs"), LogProbsObjectPtr) && LogProbsObjectPtr != nullptr)
                        {
                            TSharedPtr<FJsonObject> LogProbsObject = *LogProbsObjectPtr;
                            if (LogProbsObject->HasField(TEXT("content")))
                            {
                                TArray<TSharedPtr<FJsonValue>> ContentArray = LogProbsObject->GetArrayField(TEXT("content"));
                                for (const TSharedPtr<FJsonValue>& ContentValue : ContentArray)
                                {
                                    if (ContentValue->Type == EJson::Object)
                                    {
                                        TSharedPtr<FJsonObject> ContentObject = ContentValue->AsObject();
                                        FLogProbContent LogProbContent;
                                        if (ContentObject->HasField(TEXT("token")))
                                            LogProbContent.Token = ContentObject->GetStringField(TEXT("token"));
                                        if (ContentObject->HasField(TEXT("logprob")))
                                            LogProbContent.LogProb = static_cast<float>(ContentObject->GetNumberField(TEXT("logprob")));
                                        if (ContentObject->HasField(TEXT("bytes")))
                                        {
                                            TArray<TSharedPtr<FJsonValue>> BytesArray = ContentObject->GetArrayField(TEXT("bytes"));
                                            for (const TSharedPtr<FJsonValue>& ByteValue : BytesArray)
                                            {
                                                LogProbContent.Bytes.Add(ByteValue->AsNumber());
                                            }
                                        }
                                        if (ContentObject->HasField(TEXT("top_logprobs")))
                                        {
                                            TArray<TSharedPtr<FJsonValue>> TopLogProbsArray = ContentObject->GetArrayField(TEXT("top_logprobs"));
                                            LogProbContent.TopLogProbs = ParseTopLogProbsArray(TopLogProbsArray);
                                        }
                                        DeltaLogProbs.Content.Add(LogProbContent);
                                    }
                                }
                            }
                        }

                        ////////////
                        // Delta  //
                        ////////////
                        //
                        const TSharedPtr<FJsonObject>* ChoiceDeltaObjectPtr = nullptr;
                        if (ChoiceObject->TryGetObjectField(TEXT("delta"), ChoiceDeltaObjectPtr) && ChoiceDeltaObjectPtr != nullptr)
                        {
                            ChoiceDeltaObject = *ChoiceDeltaObjectPtr; // Dereference to get the actual object
                            if (ChoiceDeltaObject.IsValid())
                            {
                                DeltaRole = ChoiceDeltaObject->TryGetStringField(TEXT("role"), TempRole) ? GetRoleFromString(TempRole) : DeltaRole;
                                ChoiceDeltaObject->TryGetStringField(TEXT("content"), DeltaContent);


                                ////////////////
                                // ToolCalls  //
                                ////////////////
                                //
                                if (ChoiceDeltaObject->HasField(TEXT("tool_calls")))
                                {
                                    HasToolField = true;
                                    DeltaToolArray = ChoiceDeltaObject->GetArrayField(TEXT("tool_calls"));
                                    for (int32 TIndex = 0; TIndex < DeltaToolArray.Num(); TIndex++)
                                    {
                                        TSharedPtr<FJsonObject> ToolObject = DeltaToolArray[TIndex]->AsObject();
                                        if (ToolObject.IsValid())
                                        {
                                            ToolObject->TryGetNumberField(TEXT("index"), ToolIndex);
                                            ToolObject->TryGetStringField(TEXT("id"), ToolID);
                                            ToolObject->TryGetStringField(TEXT("type"), ToolType);

                                            //////////////
                                            // Function //
                                            //////////////
                                            //
                                            if (ToolObject->HasField(TEXT("function")))
                                            {
                                                ToolFunctionObject = ToolObject->GetObjectField(TEXT("function"));
                                                if (ToolFunctionObject.IsValid())
                                                {
                                                    ToolFunctionObject->TryGetStringField(TEXT("name"), FunctionName);
                                                    ToolFunctionObject->TryGetStringField(TEXT("arguments"), FunctionArguments);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        //////////////////////////
        // Store Extracted Data //
        //////////////////////////

        // CompletionInfo
        CurrentChatResponse.CompletionInfo.ID = !CompletionID.IsEmpty() ? CompletionID : CurrentChatResponse.CompletionInfo.ID;
        CurrentChatResponse.CompletionInfo.Object = !CompletionObject.IsEmpty() ? CompletionObject : CurrentChatResponse.CompletionInfo.Object;
        CurrentChatResponse.CompletionInfo.Created = CompletionCreated != 0 ? CompletionCreated : CurrentChatResponse.CompletionInfo.Created;
        CurrentChatResponse.CompletionInfo.Model = !CompletionModel.IsEmpty() ? CompletionModel : CurrentChatResponse.CompletionInfo.Model;
        CurrentChatResponse.CompletionInfo.SystemFingerprint = !CompletionSystemFingerprint.IsEmpty() ? CompletionSystemFingerprint : CurrentChatResponse.CompletionInfo.SystemFingerprint;
        CurrentChatResponse.CompletionInfo.Usage.CompletionTokens = Lines.Num();

        // Ensure Choices array is large enough
        if (CurrentChatResponse.Choices.Num() <= ChoiceIndex)
        {
            CurrentChatResponse.Choices.SetNum(ChoiceIndex + 1); // Resizes the array
        }

        // Now, set the choice data
        CurrentChatResponse.Choices[ChoiceIndex].FinishReason = !ChoiceFinishReason.IsEmpty() ? ChoiceFinishReason : CurrentChatResponse.Choices[ChoiceIndex].FinishReason;
        CurrentChatResponse.Choices[ChoiceIndex].Index = ChoiceIndex;
        CurrentChatResponse.Choices[ChoiceIndex].Message.Role = DeltaRole;

        FString SerializedDelta;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedDelta);
        FJsonSerializer::Serialize(ChoiceDeltaObject.ToSharedRef(), Writer);
        CurrentChatResponse.Choices[ChoiceIndex].Message.RawMessage = SerializedDelta;

        // Handling different message types which may not even be needed
        int32 ToolArraySize = CurrentChatResponse.Choices[ChoiceIndex].Message.AssistantMessage.ToolCalls.Num();

        switch (DeltaRole)
        {
        case EGPTRole::system:
            CurrentChatResponse.Choices[ChoiceIndex].Message.SystemMessage.Content += !DeltaContent.IsEmpty() ? DeltaContent : "";
            break;

        case EGPTRole::tool: // I don't think system or tool messages are returned from API but are included for redundancy
            CurrentChatResponse.Choices[ChoiceIndex].Message.ToolMessage.Content = !DeltaContent.IsEmpty() ? DeltaContent : "";
            CurrentChatResponse.Choices[ChoiceIndex].Message.ToolMessage.ToolCallID = ToolID; // Assuming ToolCallID holds the relevant ID
            break;

        case EGPTRole::assistant:
            CurrentChatResponse.Choices[ChoiceIndex].Message.AssistantMessage.Content += !DeltaContent.IsEmpty() ? DeltaContent : "";

            // Handle AssistantMessage ToolCalls
            if (HasToolField)
            {
                // Ensure ToolCalls array is large enough
                if (CurrentChatResponse.Choices[ChoiceIndex].Message.AssistantMessage.ToolCalls.Num() <= ToolIndex)
                {
                    CurrentChatResponse.Choices[ChoiceIndex].Message.AssistantMessage.ToolCalls.SetNum(ToolIndex + 1);

                }

                // Now, set the tool call data
                CurrentChatResponse.Choices[ChoiceIndex].Message.AssistantMessage.ToolCalls[ToolIndex].ToolIndex = ToolIndex;
                CurrentChatResponse.Choices[ChoiceIndex].Message.AssistantMessage.ToolCalls[ToolIndex].ID = !ToolID.IsEmpty() ? ToolID : CurrentChatResponse.Choices[ChoiceIndex].Message.AssistantMessage.ToolCalls[ToolIndex].ID;
                CurrentChatResponse.Choices[ChoiceIndex].Message.AssistantMessage.ToolCalls[ToolIndex].Type = !ToolType.IsEmpty() ? ToolType : CurrentChatResponse.Choices[ChoiceIndex].Message.AssistantMessage.ToolCalls[ToolIndex].Type;
                CurrentChatResponse.Choices[ChoiceIndex].Message.AssistantMessage.ToolCalls[ToolIndex].FunctionName = !FunctionName.IsEmpty() ? FunctionName : CurrentChatResponse.Choices[ChoiceIndex].Message.AssistantMessage.ToolCalls[ToolIndex].FunctionName;
                CurrentChatResponse.Choices[ChoiceIndex].Message.AssistantMessage.ToolCalls[ToolIndex].FunctionArguments += !FunctionArguments.IsEmpty() ? FunctionArguments : "";
            }
            break;
        }

        CurrentChatResponse.Choices[ChoiceIndex].LogProbs = DeltaLogProbs;

        // Clear vars for next lines
        CompletionID = "";
        CompletionObject = "";
        CompletionCreated = 0;
        CompletionModel = "";
        CompletionSystemFingerprint = "";
        ChoiceIndex = 0;
        ChoiceFinishReason = "";
        DeltaRole = EGPTRole::assistant;
        DeltaContent = "";
        ToolIndex = 0;
        ToolID = "";
        ToolType = "";
        HasToolField = false;
        FunctionName = "";
        FunctionArguments = "";
    }
}

// Parser function for non-streamed ChatGPT API responses
void UGPTChatAPI::ParseResponse(const FString& ResponseStr)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        // Log the raw response
        //UE_LOG(AIITKLog, Warning, TEXT("Raw Response: %s"), *ResponseStr);

        FGPTChatCompletion& ChatCompletion = CurrentChatResponse;

        // Common parsing
        ChatCompletion.CompletionInfo.ID = JsonObject->GetStringField(TEXT("id"));
        ChatCompletion.CompletionInfo.Object = JsonObject->GetStringField(TEXT("object"));
        ChatCompletion.CompletionInfo.Created = JsonObject->GetIntegerField(TEXT("created"));
        ChatCompletion.CompletionInfo.Model = JsonObject->GetStringField(TEXT("model"));
        ChatCompletion.CompletionInfo.SystemFingerprint = JsonObject->GetStringField(TEXT("system_fingerprint"));

        const TArray<TSharedPtr<FJsonValue>>* ChoicesArray;
        if (JsonObject->TryGetArrayField(TEXT("choices"), ChoicesArray))
        {
            for (int32 i = 0; i < ChoicesArray->Num(); ++i)
            {
                const TSharedPtr<FJsonObject>* ChoiceObject;
                if ((*ChoicesArray)[i]->TryGetObject(ChoiceObject))
                {
                    FGPTChatChoice Choice;
                    Choice.Index = i;
                    Choice.FinishReason = (*ChoiceObject)->GetStringField(TEXT("finish_reason"));

                    const TSharedPtr<FJsonObject>* MessageObject;
                    if ((*ChoiceObject)->TryGetObjectField(TEXT("message"), MessageObject))
                    {
                        // Determine the role of the message
                        FString RoleString = (*MessageObject)->GetStringField(TEXT("role"));
                        if (RoleString == TEXT("system"))
                        {
                            Choice.Message.Role = EGPTRole::system;
                            Choice.Message.SystemMessage.Content = (*MessageObject)->GetStringField(TEXT("content"));
                        }
                        else if (RoleString == TEXT("assistant"))
                        {
                            Choice.Message.Role = EGPTRole::assistant;
                            Choice.Message.AssistantMessage.Content = (*MessageObject)->GetStringField(TEXT("content"));

                            // Parsing tool_calls
                            const TArray<TSharedPtr<FJsonValue>>* ToolCallsArray;
                            if ((*MessageObject)->TryGetArrayField(TEXT("tool_calls"), ToolCallsArray))
                            {
                                for (int32 j = 0; j < ToolCallsArray->Num(); ++j)
                                {
                                    const TSharedPtr<FJsonObject>* ToolCallObject;
                                    if ((*ToolCallsArray)[j]->TryGetObject(ToolCallObject))
                                    {
                                        FToolCall ToolCall;
                                        ToolCall.ToolIndex = j;
                                        ToolCall.ID = (*ToolCallObject)->GetStringField(TEXT("id"));
                                        ToolCall.Type = (*ToolCallObject)->GetStringField(TEXT("type"));
                                        ToolCall.FunctionName = (*ToolCallObject)->GetObjectField(TEXT("function"))->GetStringField(TEXT("name"));
                                        ToolCall.FunctionArguments = (*ToolCallObject)->GetObjectField(TEXT("function"))->GetStringField(TEXT("arguments"));

                                        Choice.Message.AssistantMessage.ToolCalls.Add(ToolCall);
                                    }
                                }
                            }
                        }
                        else if (RoleString == TEXT("tool"))
                        {
                            Choice.Message.Role = EGPTRole::tool;
                            Choice.Message.ToolMessage.Content = (*MessageObject)->GetStringField(TEXT("content"));
                        }
                        // Future additional roles go here
                    }

                    // Check and parse 'logprobs' if present
                    const TSharedPtr<FJsonObject>* LogProbsObject;
                    if ((*ChoiceObject)->TryGetObjectField(TEXT("logprobs"), LogProbsObject) && LogProbsObject->IsValid())
                    {
                        Choice.LogProbs = ParseLogProbs(*LogProbsObject); // Assuming ParseLogProbs is a function to handle log probabilities
                    }

                    ChatCompletion.Choices.Add(Choice);
                }
            }
        }


        // Parsing 'usage' field
        ChatCompletion.CompletionInfo.Usage.PromptTokens = JsonObject->GetObjectField(TEXT("usage"))->GetIntegerField(TEXT("prompt_tokens"));
        ChatCompletion.CompletionInfo.Usage.CompletionTokens = JsonObject->GetObjectField(TEXT("usage"))->GetIntegerField(TEXT("completion_tokens"));
        ChatCompletion.CompletionInfo.Usage.TotalTokens = JsonObject->GetObjectField(TEXT("usage"))->GetIntegerField(TEXT("total_tokens"));
    }
    else
    {
        UE_LOG(AIITKLog, Warning, TEXT("Failed to parse the response as single JSON, trying stream response parsing method"));

        // Process all lines on completion to correct for missing chunks
        CurrentChatResponse = FGPTChatCompletion();
        ProcessedLines = 0;
        ParseStreamedResponse(ResponseStr);
    }
}


// Helpers


FTopLogProbs UGPTChatAPI::ParseSingleTopLogProb(TSharedPtr<FJsonObject> TopLogProbObject)
{
    FTopLogProbs TopLogProb;
    if (TopLogProbObject->HasField(TEXT("token")))
        TopLogProb.Token = TopLogProbObject->GetStringField(TEXT("token"));
    if (TopLogProbObject->HasField(TEXT("logprob")))
        TopLogProb.LogProb = static_cast<float>(TopLogProbObject->GetNumberField(TEXT("logprob")));
    if (TopLogProbObject->HasField(TEXT("bytes")))
    {
        TArray<TSharedPtr<FJsonValue>> BytesArray = TopLogProbObject->GetArrayField(TEXT("bytes"));
        for (const TSharedPtr<FJsonValue>& ByteValue : BytesArray)
        {
            TopLogProb.Bytes.Add(ByteValue->AsNumber());
        }
    }
    return TopLogProb;
}

TArray<FTopLogProbs> UGPTChatAPI::ParseTopLogProbsArray(TArray<TSharedPtr<FJsonValue>> TopLogProbsArray)
{
    TArray<FTopLogProbs> TopLogProbsList;
    for (const TSharedPtr<FJsonValue>& TopLogProbValue : TopLogProbsArray)
    {
        if (TopLogProbValue->Type == EJson::Object)
        {
            FTopLogProbs TopLogProb = ParseSingleTopLogProb(TopLogProbValue->AsObject());
            TopLogProbsList.Add(TopLogProb);
        }
    }
    return TopLogProbsList;
}

FLogProbs UGPTChatAPI::ParseLogProbs(TSharedPtr<FJsonObject> LogProbsObject)
{
    FLogProbs LogProbs;
    if (LogProbsObject->HasField(TEXT("content")))
    {
        TArray<TSharedPtr<FJsonValue>> ContentArray = LogProbsObject->GetArrayField(TEXT("content"));
        for (const TSharedPtr<FJsonValue>& ContentValue : ContentArray)
        {
            if (ContentValue->Type == EJson::Object)
            {
                TSharedPtr<FJsonObject> ContentObject = ContentValue->AsObject();
                FLogProbContent LogProbContent;
                if (ContentObject->HasField(TEXT("token")))
                    LogProbContent.Token = ContentObject->GetStringField(TEXT("token"));
                if (ContentObject->HasField(TEXT("logprob")))
                    LogProbContent.LogProb = static_cast<float>(ContentObject->GetNumberField(TEXT("logprob")));
                if (ContentObject->HasField(TEXT("bytes")))
                {
                    TArray<TSharedPtr<FJsonValue>> BytesArray = ContentObject->GetArrayField(TEXT("bytes"));
                    for (const TSharedPtr<FJsonValue>& ByteValue : BytesArray)
                    {
                        LogProbContent.Bytes.Add(ByteValue->AsNumber());
                    }
                }
                if (ContentObject->HasField(TEXT("top_logprobs")))
                {
                    TArray<TSharedPtr<FJsonValue>> TopLogProbsArray = ContentObject->GetArrayField(TEXT("top_logprobs"));
                    LogProbContent.TopLogProbs = ParseTopLogProbsArray(TopLogProbsArray);
                }
                LogProbs.Content.Add(LogProbContent);
            }
        }
    }
    return LogProbs;
}

EGPTRole UGPTChatAPI::GetRoleFromString(const FString& RoleStr)
{
    if (RoleStr == "system")
    {
        return EGPTRole::system;
    }
    else if (RoleStr == "user")
    {
        return EGPTRole::user;
    }
    else if (RoleStr == "assistant")
    {
        return EGPTRole::assistant;
    }
    else
    {
        return EGPTRole::assistant;
    }
}


// ============ ERROR ============

void UGPTChatAPI::CancelRequest()
{
    if (CurrentHttpRequest.IsValid() && CurrentHttpRequest->GetStatus() == EHttpRequestStatus::Processing)
    {
        CurrentHttpRequest->CancelRequest();
    }
}

void UGPTChatAPI::HandleError(FHttpResponsePtr Response)
{
    FGPTChatCompletion FailedResponse;
    FGPTChatChoice FailedChoice;
    FailedChoice.Message.Role = EGPTRole::system; // Assuming EGPTRole::system indicates an error
    FailedChoice.Message.Name = TEXT("Error Handler"); // Optionally set a name for the error handler

    if (Response.IsValid())
    {
        // Log the raw response content
        UE_LOG(AIITKLog, Error, TEXT("Raw Error Response: %s"), *Response->GetContentAsString());
        FailedChoice.Message.SystemMessage.Content = Response->GetContentAsString();
    }
    else
    {
        // Handle the case where the response is not valid
        FailedChoice.Message.SystemMessage.Content = TEXT("A.I.I.T.K: Request failed with no status code.");
    }

    // Check if request was cancelled due to timeout or other reasons
    if (Response.IsValid() && Response->GetResponseCode() == 0)
    {
        FailedChoice.Message.SystemMessage.Content = TEXT("A.I.I.T.K: Request was cancelled or timed out.");
    }
    else if (!Response.IsValid())
    {
        FailedChoice.Message.SystemMessage.Content = FString::Printf(TEXT("A.I.I.T.K: Request failed with status code: %d"), Response.IsValid() ? Response->GetResponseCode() : -1);
    }

    // Add the failed choice to the FailedResponse and broadcast it
    FailedResponse.Choices.Add(FailedChoice);
    this->OnFailed.Broadcast(FailedResponse);
}




// ============ UTILITY ============

TTuple<FString, bool> UGPTChatAPI::GetLastMatch(const FString& Pattern, const FString& SearchString, const int CaptureGroup)
{
    FRegexPattern MyPattern(Pattern);
    FRegexMatcher MyMatcher(MyPattern, SearchString);

    FString LastMatch;
    bool bMatchFound = false;

    while (MyMatcher.FindNext())
    {
        LastMatch = MyMatcher.GetCaptureGroup(CaptureGroup);  // Group 0 is the entire match
        bMatchFound = true;
    }

    return MakeTuple(LastMatch, bMatchFound);
}

UGPTChatAPI* UGPTChatAPI::FetchModelIds()
{
    UGPTChatAPI* GPTAPIInstance = NewObject<UGPTChatAPI>();
    GPTAPIInstance->GetModelIds();
    return GPTAPIInstance;
}

void UGPTChatAPI::GetModelIds()
{
    // Return if there is already an ongoing request
    if (bRequestInProgress)
    {
        return;
    }

    // Set request in progress flag to true
    bRequestInProgress = true;

    // Create an HTTP GET request for the models endpoint
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest = HttpRequest;

    // Set up the HTTP request for the model list
    if (CurrentHttpRequest.IsValid())
    {
        CurrentHttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));

        // Fetch decrypted API key
        FString DecryptedAPIKey = UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI);
        CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *DecryptedAPIKey));

        CurrentHttpRequest->SetHeader(TEXT("Connection"), TEXT("keep-alive"));
        CurrentHttpRequest->SetURL(TEXT("https://api.openai.com/v1/models"));
        CurrentHttpRequest->SetVerb(TEXT("GET"));
    }

    CurrentHttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (this->bIsShuttingDown)
            {
                return;
            }

            TArray<FString> ModelIds;

            if (bWasSuccessful && Response->GetResponseCode() == 200)
            {
                FString ResponseStr = Response->GetContentAsString();
                TSharedPtr<FJsonObject> JsonObject;
                TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(ResponseStr);

                if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject->HasField(TEXT("data")))
                {
                    TArray<TSharedPtr<FJsonValue>> ModelsArray = JsonObject->GetArrayField(TEXT("data"));
                    for (const auto& ModelValue : ModelsArray)
                    {
                        if (ModelValue->AsObject()->HasField(TEXT("id")))
                        {
                            ModelIds.Add(ModelValue->AsObject()->GetStringField(TEXT("id")));
                        }
                    }

                    // Alphabetize the ModelIds list
                    Algo::Sort(ModelIds);

                    // Broadcast the retrieved model IDs through the delegate
                    OnModelsRetrieved.Broadcast(ModelIds);
                }
            }
            else
            {
                HandleError(Response);
            }

            this->bRequestInProgress = false;
            // Remove from root to allow garbage collection
            RemoveFromRoot();
        });

    // Send the HttpRequest
    CurrentHttpRequest->ProcessRequest();
}










