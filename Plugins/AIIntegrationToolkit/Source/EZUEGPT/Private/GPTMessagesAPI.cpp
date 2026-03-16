// =================== GPTMessagesAPI.cpp ===================
#include "GPTMessagesAPI.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

UGPTMessagesAPI* UGPTMessagesAPI::CreateMessage(const FGPTMessageCreateParams& Params)
{
    UGPTMessagesAPI* APIInstance = NewObject<UGPTMessagesAPI>();
    APIInstance->SendCreateRequest(Params);
    return APIInstance;
}

void UGPTMessagesAPI::SendCreateRequest(const FGPTMessageCreateParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendCreateRequest called with Params."));

    // Set the current request type
    CurrentRequestType = "create";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/threads/%s/messages"), *Params.ThreadID));
    CurrentHttpRequest->SetVerb(TEXT("POST"));
    CurrentHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("role"), Params.Role);
    JsonObject->SetStringField(TEXT("content"), Params.Content);

    // Add metadata if available
    if (Params.Metadata.Num() > 0)
    {
        TSharedPtr<FJsonObject> MetadataObject = MakeShareable(new FJsonObject);
        for (const auto& Pair : Params.Metadata)
        {
            MetadataObject->SetStringField(Pair.Key, Pair.Value);
        }
        JsonObject->SetObjectField(TEXT("metadata"), MetadataObject);
    }

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    CurrentHttpRequest->SetContentAsString(RequestBody);

    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTMessagesAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Create request sent."));
}

UGPTMessagesAPI* UGPTMessagesAPI::RetrieveMessage(const FGPTMessageRetrieveParams& Params)
{
    UGPTMessagesAPI* APIInstance = NewObject<UGPTMessagesAPI>();
    APIInstance->SendRetrieveRequest(Params);
    return APIInstance;
}

void UGPTMessagesAPI::SendRetrieveRequest(const FGPTMessageRetrieveParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendRetrieveRequest called with Params."));

    // Set the current request type
    CurrentRequestType = "retrieve";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/threads/%s/messages/%s"), *Params.ThreadID, *Params.MessageID));
    CurrentHttpRequest->SetVerb(TEXT("GET"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTMessagesAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Retrieve request sent."));
}

UGPTMessagesAPI* UGPTMessagesAPI::ListMessages(const FGPTMessageListParams& Params)
{
    UGPTMessagesAPI* APIInstance = NewObject<UGPTMessagesAPI>();
    APIInstance->SendListRequest(Params);
    return APIInstance;
}

void UGPTMessagesAPI::SendListRequest(const FGPTMessageListParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendListRequest called with Params."));

    // Set the current request type
    CurrentRequestType = "list";

    FString URL = FString::Printf(TEXT("https://api.openai.com/v1/threads/%s/messages?limit=%d&order=%s"), *Params.ThreadID, Params.Limit, *Params.Order);
    if (!Params.After.IsEmpty())
    {
        URL += FString::Printf(TEXT("&after=%s"), *Params.After);
    }
    if (!Params.Before.IsEmpty())
    {
        URL += FString::Printf(TEXT("&before=%s"), *Params.Before);
    }

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(URL);
    CurrentHttpRequest->SetVerb(TEXT("GET"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTMessagesAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("List request sent."));
}

UGPTMessagesAPI* UGPTMessagesAPI::ModifyMessage(const FGPTMessageModifyParams& Params)
{
    UGPTMessagesAPI* APIInstance = NewObject<UGPTMessagesAPI>();
    APIInstance->SendModifyRequest(Params);
    return APIInstance;
}

void UGPTMessagesAPI::SendModifyRequest(const FGPTMessageModifyParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendModifyRequest called with Params."));

    // Set the current request type
    CurrentRequestType = "modify";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/threads/%s/messages/%s"), *Params.ThreadID, *Params.MessageID));
    CurrentHttpRequest->SetVerb(TEXT("POST"));
    CurrentHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    // Add metadata if available
    if (Params.Metadata.Num() > 0)
    {
        TSharedPtr<FJsonObject> MetadataObject = MakeShareable(new FJsonObject);
        for (const auto& Pair : Params.Metadata)
        {
            MetadataObject->SetStringField(Pair.Key, Pair.Value);
        }
        JsonObject->SetObjectField(TEXT("metadata"), MetadataObject);
    }

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    CurrentHttpRequest->SetContentAsString(RequestBody);

    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTMessagesAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Modify request sent."));
}

UGPTMessagesAPI* UGPTMessagesAPI::DeleteMessage(const FGPTMessageDeleteParams& Params)
{
    UGPTMessagesAPI* APIInstance = NewObject<UGPTMessagesAPI>();
    APIInstance->SendDeleteRequest(Params);
    return APIInstance;
}

void UGPTMessagesAPI::SendDeleteRequest(const FGPTMessageDeleteParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendDeleteRequest called with Params."));

    // Set the current request type
    CurrentRequestType = "delete";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/threads/%s/messages/%s"), *Params.ThreadID, *Params.MessageID));
    CurrentHttpRequest->SetVerb(TEXT("DELETE"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTMessagesAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Delete request sent."));
}

void UGPTMessagesAPI::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response->GetResponseCode() == 200)
    {
        FString ResponseStr = Response->GetContentAsString();
        UE_LOG(LogTemp, Warning, TEXT("HandleResponse called for request type: %s. Response: %s"), *CurrentRequestType, *ResponseStr);

        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            FString ObjectType;
            JsonObject->TryGetStringField(TEXT("object"), ObjectType);

            // Check if the response is of type "list"
            if (ObjectType == "list")
            {
                const TArray<TSharedPtr<FJsonValue>>* DataArray;
                if (JsonObject->TryGetArrayField(TEXT("data"), DataArray))
                {
                    TArray<FGPTMessageObject> MessageObjects;

                    for (const TSharedPtr<FJsonValue>& Value : *DataArray)
                    {
                        TSharedPtr<FJsonObject> MessageData = Value->AsObject();
                        FGPTMessageObject MessageObject;

                        // Parse each message object within the list
                        MessageData->TryGetStringField(TEXT("id"), MessageObject.Id);
                        MessageData->TryGetStringField(TEXT("object"), MessageObject.Object);
                        MessageData->TryGetNumberField(TEXT("created_at"), MessageObject.CreatedAt);
                        MessageData->TryGetStringField(TEXT("role"), MessageObject.Role);
                        MessageData->TryGetStringField(TEXT("thread_id"), MessageObject.ThreadID);

                        // Parse the content array
                        const TArray<TSharedPtr<FJsonValue>>* ContentArray;
                        if (MessageData->TryGetArrayField(TEXT("content"), ContentArray))
                        {
                            for (const TSharedPtr<FJsonValue>& ContentValue : *ContentArray)
                            {
                                TSharedPtr<FJsonObject> ContentObject = ContentValue->AsObject();
                                FString ContentText;

                                // Check if the content type is "text" and extract the value
                                FString ContentType;
                                if (ContentObject->TryGetStringField(TEXT("type"), ContentType) && ContentType == "text")
                                {
                                    // Declare a shared pointer for the text object
                                    const TSharedPtr<FJsonObject>* TextObjectPtr;

                                    // Pass the pointer to TryGetObjectField
                                    if (ContentObject->TryGetObjectField(TEXT("text"), TextObjectPtr))
                                    {
                                        (*TextObjectPtr)->TryGetStringField(TEXT("value"), ContentText);
                                        MessageObject.Content.Add(ContentText);
                                    }
                                }
                            }
                        }

                        // Add message metadata if available
                        const TSharedPtr<FJsonObject>* MetadataObject;
                        if (MessageData->TryGetObjectField(TEXT("metadata"), MetadataObject))
                        {
                            for (const auto& Pair : (*MetadataObject)->Values)
                            {
                                MessageObject.Metadata.Add(Pair.Key, Pair.Value->AsString());
                            }
                        }

                        // Add the parsed message object to the array
                        MessageObjects.Add(MessageObject);
                    }

                    // Broadcast the list of messages via the new delegate
                    OnMessagesListed.Broadcast(MessageObjects);
                }
            }
            else
            {
                // Handle other types of response (create, retrieve, modify, delete)
                FGPTMessageObject MessageObject;
                MessageObject.RawResponse = ResponseStr; // Store the raw response

                // Parse the common message fields
                JsonObject->TryGetStringField(TEXT("id"), MessageObject.Id);
                JsonObject->TryGetStringField(TEXT("object"), MessageObject.Object);
                JsonObject->TryGetNumberField(TEXT("created_at"), MessageObject.CreatedAt);
                JsonObject->TryGetStringField(TEXT("role"), MessageObject.Role);
                JsonObject->TryGetStringField(TEXT("thread_id"), MessageObject.ThreadID);

                // Parse content array
                const TArray<TSharedPtr<FJsonValue>>* ContentArray;
                if (JsonObject->TryGetArrayField(TEXT("content"), ContentArray))
                {
                    for (const TSharedPtr<FJsonValue>& Value : *ContentArray)
                    {
                        TSharedPtr<FJsonObject> ContentObject = Value->AsObject();
                        FString ContentText;

                        // Check if the content type is "text" and extract the value
                        FString ContentType;
                        if (ContentObject->TryGetStringField(TEXT("type"), ContentType) && ContentType == "text")
                        {
                            // Declare a shared pointer for the text object
                            const TSharedPtr<FJsonObject>* TextObjectPtr;

                            // Pass the pointer to TryGetObjectField
                            if (ContentObject->TryGetObjectField(TEXT("text"), TextObjectPtr))
                            {
                                (*TextObjectPtr)->TryGetStringField(TEXT("value"), ContentText);
                                MessageObject.Content.Add(ContentText);
                            }
                        }
                    }
                }

                // Parse Metadata if available
                const TSharedPtr<FJsonObject>* MetadataObject;
                if (JsonObject->TryGetObjectField(TEXT("metadata"), MetadataObject))
                {
                    for (const auto& Pair : (*MetadataObject)->Values)
                    {
                        MessageObject.Metadata.Add(Pair.Key, Pair.Value->AsString());
                    }
                }

                // Broadcast the filled MessageObject based on the request type
                if (CurrentRequestType == "create")
                {
                    OnMessageCreated.Broadcast(MessageObject);
                }
                else if (CurrentRequestType == "retrieve")
                {
                    OnMessageRetrieved.Broadcast(MessageObject);
                }
                else if (CurrentRequestType == "modify")
                {
                    OnMessageModified.Broadcast(MessageObject);
                }
                else if (CurrentRequestType == "delete")
                {
                    OnMessageDeleted.Broadcast(MessageObject);
                }
            }
        }
    }
    else
    {
        FString ErrorMessage = Response.IsValid() ? Response->GetContentAsString() : TEXT("Unknown error occurred.");
        HandleError(ErrorMessage);
    }
}




void UGPTMessagesAPI::HandleError(const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("HandleError called for request type: %s. Error: %s"), *CurrentRequestType, *ErrorMessage);
    OnAPIError.Broadcast(ErrorMessage);
}
