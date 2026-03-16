#include "GPTRunsAPI.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

// Submit tool outputs
UGPTRunsAPI* UGPTRunsAPI::SubmitToolOutputs(const FGPTSubmitToolOutputsParams& Params)
{
    UGPTRunsAPI* APIInstance = NewObject<UGPTRunsAPI>();
    APIInstance->SendSubmitToolOutputsRequest(Params);
    return APIInstance;
}

void UGPTRunsAPI::SendSubmitToolOutputsRequest(const FGPTSubmitToolOutputsParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendSubmitToolOutputsRequest called with Params."));

    CurrentRequestType = "submit_tool_outputs";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/threads/%s/runs/%s/submit_tool_outputs"), *Params.ThreadId, *Params.RunId));
    CurrentHttpRequest->SetVerb(TEXT("POST"));
    CurrentHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    // Prepare the tool outputs as JSON
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    TArray<TSharedPtr<FJsonValue>> ToolOutputsArray;
    for (const FString& Output : Params.ToolOutputs)
    {
        TSharedPtr<FJsonObject> ToolOutputObj = MakeShareable(new FJsonObject);
        ToolOutputObj->SetStringField(TEXT("output"), Output);
        ToolOutputsArray.Add(MakeShareable(new FJsonValueObject(ToolOutputObj)));
    }
    JsonObject->SetArrayField(TEXT("tool_outputs"), ToolOutputsArray);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    CurrentHttpRequest->SetContentAsString(RequestBody);

    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTRunsAPI::HandleModifiedRunResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Submit tool outputs request sent."));
}

// Cancel run
UGPTRunsAPI* UGPTRunsAPI::CancelRun(const FGPTCancelRunParams& Params)
{
    UGPTRunsAPI* APIInstance = NewObject<UGPTRunsAPI>();
    APIInstance->SendCancelRunRequest(Params);
    return APIInstance;
}

void UGPTRunsAPI::SendCancelRunRequest(const FGPTCancelRunParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendCancelRunRequest called with Params."));

    CurrentRequestType = "cancel_run";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/threads/%s/runs/%s/cancel"), *Params.ThreadId, *Params.RunId));
    CurrentHttpRequest->SetVerb(TEXT("POST"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTRunsAPI::HandleModifiedRunResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Cancel run request sent."));
}

// Create run
UGPTRunsAPI* UGPTRunsAPI::CreateRun(const FGPTCreateRunParams& Params)
{
    UGPTRunsAPI* APIInstance = NewObject<UGPTRunsAPI>();
    APIInstance->SendCreateRunRequest(Params);
    return APIInstance;
}

void UGPTRunsAPI::SendCreateRunRequest(const FGPTCreateRunParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendCreateRunRequest called with Params."));

    CurrentRequestType = "create_run";

    // Ensure the URL includes the Thread ID
    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/threads/%s/runs"), *Params.ThreadId));
    CurrentHttpRequest->SetVerb(TEXT("POST"));
    CurrentHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));

    // Prepare the JSON body
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("assistant_id"), Params.AssistantId);

    // Optional fields
    if (!Params.Instructions.IsEmpty())
    {
        JsonObject->SetStringField(TEXT("instructions"), Params.Instructions);
    }

    if (!Params.Model.IsEmpty())
    {
        JsonObject->SetStringField(TEXT("model"), Params.Model);
    }

    JsonObject->SetNumberField(TEXT("temperature"), Params.Temperature);
    JsonObject->SetNumberField(TEXT("top_p"), Params.TopP);
    JsonObject->SetBoolField(TEXT("stream"), Params.Stream);

    // Serialize the JSON object
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    CurrentHttpRequest->SetContentAsString(RequestBody);

    // Set up the response handling
    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTRunsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Create run request sent."));
}


// Retrieve run
UGPTRunsAPI* UGPTRunsAPI::RetrieveRun(const FGPTRetrieveRunParams& Params)
{
    UGPTRunsAPI* APIInstance = NewObject<UGPTRunsAPI>();
    APIInstance->SendRetrieveRunRequest(Params);
    return APIInstance;
}

void UGPTRunsAPI::SendRetrieveRunRequest(const FGPTRetrieveRunParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendRetrieveRunRequest called with Params."));

    CurrentRequestType = "retrieve_run";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/threads/%s/runs/%s"), *Params.ThreadId, *Params.RunId));
    CurrentHttpRequest->SetVerb(TEXT("GET"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTRunsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Retrieve run request sent."));
}

// List runs
UGPTRunsAPI* UGPTRunsAPI::ListRuns(const FGPTRunListParams& Params)
{
    UGPTRunsAPI* APIInstance = NewObject<UGPTRunsAPI>();
    APIInstance->SendListRunsRequest(Params);
    return APIInstance;
}

void UGPTRunsAPI::SendListRunsRequest(const FGPTRunListParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendListRunsRequest called with Params."));

    CurrentRequestType = "list_runs";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    FString URL = FString::Printf(TEXT("https://api.openai.com/v1/threads/%s/runs?limit=%d&order=%s"), *Params.ThreadId, Params.Limit, *Params.Order);
    CurrentHttpRequest->SetURL(URL);
    CurrentHttpRequest->SetVerb(TEXT("GET"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTRunsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("List runs request sent."));
}

// Modify run
UGPTRunsAPI* UGPTRunsAPI::ModifyRun(const FGPTModifyRunParams& Params)
{
    UGPTRunsAPI* APIInstance = NewObject<UGPTRunsAPI>();
    APIInstance->SendModifyRunRequest(Params);
    return APIInstance;
}

void UGPTRunsAPI::SendModifyRunRequest(const FGPTModifyRunParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendModifyRunRequest called with Params."));

    CurrentRequestType = "modify_run";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/threads/%s/runs/%s"), *Params.ThreadId, *Params.RunId));
    CurrentHttpRequest->SetVerb(TEXT("POST"));
    CurrentHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    // Attach metadata if available
    for (const auto& Elem : Params.Metadata)
    {
        JsonObject->SetStringField(Elem.Key, Elem.Value);
    }

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    CurrentHttpRequest->SetContentAsString(RequestBody);

    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTRunsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Modify run request sent."));
}

// Handle responses
void UGPTRunsAPI::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response->GetResponseCode() == 200)
    {
        FString ResponseStr = Response->GetContentAsString();
        UE_LOG(LogTemp, Warning, TEXT("HandleResponse called for request type: %s. Response: %s"), *CurrentRequestType, *ResponseStr);

        FGPTRunObject RunObject;
        RunObject.RawResponse = ResponseStr; // Store the raw response

        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            JsonObject->TryGetStringField(TEXT("id"), RunObject.Id);
            JsonObject->TryGetStringField(TEXT("object"), RunObject.Object);
            JsonObject->TryGetNumberField(TEXT("created_at"), RunObject.CreatedAt);
            JsonObject->TryGetStringField(TEXT("status"), RunObject.Status);
            JsonObject->TryGetStringField(TEXT("assistant_id"), RunObject.AssistantId);
            JsonObject->TryGetStringField(TEXT("thread_id"), RunObject.ThreadId);

            // Parse tools array if available
            const TArray<TSharedPtr<FJsonValue>>* ToolsArray;
            if (JsonObject->TryGetArrayField(TEXT("tools"), ToolsArray))
            {
                for (const TSharedPtr<FJsonValue>& ToolValue : *ToolsArray)
                {
                    RunObject.Tools.Add(ToolValue->AsString());
                }
            }

            // Broadcast the filled RunObject based on the request type
            if (CurrentRequestType == "create_run")
            {
                OnRunCreated.Broadcast(RunObject);
            }
            else if (CurrentRequestType == "retrieve_run")
            {
                OnRunRetrieved.Broadcast(RunObject);
            }
            else if (CurrentRequestType == "list_runs")
            {
                TArray<FGPTRunObject> RunObjects;
                RunObjects.Add(RunObject); // Append run to the list
                OnRunsListed.Broadcast(RunObjects);
            }
        }
    }
    else
    {
        FString ErrorMessage = Response.IsValid() ? Response->GetContentAsString() : TEXT("Unknown error occurred.");
        HandleError(ErrorMessage);
    }
}

void UGPTRunsAPI::HandleError(const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("HandleError called for request type: %s. Error: %s"), *CurrentRequestType, *ErrorMessage);
    OnAPIError.Broadcast(ErrorMessage);
}

// Handle responses for modified runs
void UGPTRunsAPI::HandleModifiedRunResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response->GetResponseCode() == 200)
    {
        FString ResponseStr = Response->GetContentAsString();
        UE_LOG(LogTemp, Warning, TEXT("HandleModifiedRunResponse called for request type: %s. Response: %s"), *CurrentRequestType, *ResponseStr);

        FModifiedRunObject ModifiedRunObject;
        ModifiedRunObject.RawResponse = ResponseStr; // Store the raw response

        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            JsonObject->TryGetStringField(TEXT("id"), ModifiedRunObject.Id);
            JsonObject->TryGetStringField(TEXT("status"), ModifiedRunObject.Status);
            JsonObject->TryGetStringField(TEXT("assistant_id"), ModifiedRunObject.AssistantId);
            JsonObject->TryGetStringField(TEXT("thread_id"), ModifiedRunObject.ThreadId);
            JsonObject->TryGetStringField(TEXT("model"), ModifiedRunObject.Model);
            JsonObject->TryGetStringField(TEXT("response_format"), ModifiedRunObject.ResponseFormat);

            // Broadcast based on the request type
            if (CurrentRequestType == "submit_tool_outputs")
            {
                OnToolOutputsSubmitted.Broadcast(ModifiedRunObject);
            }
            else if (CurrentRequestType == "cancel_run")
            {
                OnRunCancelled.Broadcast(ModifiedRunObject);
            }
        }
    }
    else
    {
        FString ErrorMessage = Response.IsValid() ? Response->GetContentAsString() : TEXT("Unknown error occurred.");
        HandleModifiedRunError(ErrorMessage);
    }
}

void UGPTRunsAPI::HandleModifiedRunError(const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("HandleModifiedRunError called for request type: %s. Error: %s"), *CurrentRequestType, *ErrorMessage);
    // You can broadcast specific error delegates here if needed.
}
