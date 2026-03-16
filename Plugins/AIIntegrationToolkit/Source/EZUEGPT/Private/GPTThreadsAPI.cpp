// =================== GPTThreadsAPI.cpp ===================

#include "GPTThreadsAPI.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

UGPTThreadsAPI* UGPTThreadsAPI::CreateThread(const FGPTThreadCreateParams& Params)
{
    UGPTThreadsAPI* APIInstance = NewObject<UGPTThreadsAPI>();
    APIInstance->SendCreateRequest(Params);
    return APIInstance;
}

void UGPTThreadsAPI::SendCreateRequest(const FGPTThreadCreateParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendCreateRequest called"));

    CurrentRequestType = "create";
    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(TEXT("https://api.openai.com/v1/threads"));
    CurrentHttpRequest->SetVerb(TEXT("POST"));
    CurrentHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    TArray<TSharedPtr<FJsonValue>> MessagesArray;
    for (const FGPTMessage& Message : Params.Messages)
    {
        TSharedPtr<FJsonObject> MessageObject = MakeShareable(new FJsonObject);
        MessageObject->SetStringField(TEXT("role"), Message.Role);
        MessageObject->SetStringField(TEXT("content"), Message.Content);
        MessagesArray.Add(MakeShareable(new FJsonValueObject(MessageObject)));
    }
    JsonObject->SetArrayField(TEXT("messages"), MessagesArray);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    CurrentHttpRequest->SetContentAsString(RequestBody);

    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTThreadsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Create request sent."));
}

UGPTThreadsAPI* UGPTThreadsAPI::RetrieveThread(const FGPTThreadRetrieveParams& Params)
{
    UGPTThreadsAPI* APIInstance = NewObject<UGPTThreadsAPI>();
    APIInstance->SendRetrieveRequest(Params);
    return APIInstance;
}

void UGPTThreadsAPI::SendRetrieveRequest(const FGPTThreadRetrieveParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendRetrieveRequest called"));

    CurrentRequestType = "retrieve";
    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/threads/%s"), *Params.ThreadID));
    CurrentHttpRequest->SetVerb(TEXT("GET"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTThreadsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Retrieve request sent."));
}

UGPTThreadsAPI* UGPTThreadsAPI::ModifyThread(const FGPTThreadModifyParams& Params)
{
    UGPTThreadsAPI* APIInstance = NewObject<UGPTThreadsAPI>();
    APIInstance->SendModifyRequest(Params);
    return APIInstance;
}

void UGPTThreadsAPI::SendModifyRequest(const FGPTThreadModifyParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendModifyRequest called"));

    CurrentRequestType = "modify";
    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/threads/%s"), *Params.ThreadID));
    CurrentHttpRequest->SetVerb(TEXT("POST"));
    CurrentHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    for (const auto& Pair : Params.Metadata)
    {
        JsonObject->SetStringField(Pair.Key, Pair.Value);
    }

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    CurrentHttpRequest->SetContentAsString(RequestBody);

    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTThreadsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Modify request sent."));
}

UGPTThreadsAPI* UGPTThreadsAPI::DeleteThread(const FGPTThreadRetrieveParams& Params)
{
    UGPTThreadsAPI* APIInstance = NewObject<UGPTThreadsAPI>();
    APIInstance->SendDeleteRequest(Params);
    return APIInstance;
}

void UGPTThreadsAPI::SendDeleteRequest(const FGPTThreadRetrieveParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendDeleteRequest called"));

    CurrentRequestType = "delete";
    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/threads/%s"), *Params.ThreadID));
    CurrentHttpRequest->SetVerb(TEXT("DELETE"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTThreadsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Delete request sent."));
}

void UGPTThreadsAPI::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response->GetResponseCode() == 200)
    {
        FString ResponseStr = Response->GetContentAsString();
        UE_LOG(LogTemp, Warning, TEXT("HandleResponse called: %s"), *ResponseStr);

        FGPTThreadObject ThreadObject;
        ThreadObject.RawResponse = ResponseStr;

        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            FString ObjectType;
            JsonObject->TryGetStringField(TEXT("object"), ObjectType);

            if (ObjectType == "thread" || ObjectType == "thread.deleted")
            {
                JsonObject->TryGetStringField(TEXT("id"), ThreadObject.Id);
                ThreadObject.Object = ObjectType;
                JsonObject->TryGetNumberField(TEXT("created_at"), ThreadObject.CreatedAt);

                const TSharedPtr<FJsonObject>* MetadataObject;
                if (JsonObject->TryGetObjectField(TEXT("metadata"), MetadataObject))
                {
                    for (const auto& Pair : (*MetadataObject)->Values)
                    {
                        ThreadObject.Metadata.Add(Pair.Key, Pair.Value->AsString());
                    }
                }

                if (CurrentRequestType == "create")
                {
                    OnThreadCreated.Broadcast(ThreadObject);
                }
                else if (CurrentRequestType == "retrieve")
                {
                    OnThreadRetrieved.Broadcast(ThreadObject);
                }
                else if (CurrentRequestType == "modify")
                {
                    OnThreadModified.Broadcast(ThreadObject);
                }
                else if (CurrentRequestType == "delete")
                {
                    OnThreadDeleted.Broadcast(ThreadObject);
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

void UGPTThreadsAPI::HandleError(const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("HandleError called: %s"), *ErrorMessage);
    OnAPIError.Broadcast(ErrorMessage);
}
