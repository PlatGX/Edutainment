// =================== GPTAssistantsAPI.cpp ===================
#include "GPTAssistantsAPI.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

UGPTAssistantsAPI* UGPTAssistantsAPI::CreateAssistant(const FGPTAssistantCreateParams& Params)
{
    UGPTAssistantsAPI* APIInstance = NewObject<UGPTAssistantsAPI>();
    APIInstance->SendCreateRequest(Params);
    return APIInstance;
}

void UGPTAssistantsAPI::SendCreateRequest(const FGPTAssistantCreateParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendCreateRequest called with Params."));

    // Set the current request type
    CurrentRequestType = "create";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(TEXT("https://api.openai.com/v1/assistants"));
    CurrentHttpRequest->SetVerb(TEXT("POST"));
    CurrentHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("model"), Params.Model);
    JsonObject->SetStringField(TEXT("name"), Params.Name);
    JsonObject->SetStringField(TEXT("description"), Params.Description);
    JsonObject->SetStringField(TEXT("instructions"), Params.Instructions);

    TArray<TSharedPtr<FJsonValue>> ToolsArray;
    for (const FString& Tool : Params.Tools)
    {
        TSharedPtr<FJsonObject> ToolObject = MakeShareable(new FJsonObject);
        ToolObject->SetStringField(TEXT("type"), Tool);
        ToolsArray.Add(MakeShareable(new FJsonValueObject(ToolObject)));
    }
    JsonObject->SetArrayField(TEXT("tools"), ToolsArray);
    JsonObject->SetNumberField(TEXT("temperature"), Params.Temperature);
    JsonObject->SetNumberField(TEXT("top_p"), Params.TopP);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    CurrentHttpRequest->SetContentAsString(RequestBody);

    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTAssistantsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Create request sent."));
}

UGPTAssistantsAPI* UGPTAssistantsAPI::RetrieveAssistant(const FGPTAssistantRetrieveParams& Params)
{
    UGPTAssistantsAPI* APIInstance = NewObject<UGPTAssistantsAPI>();
    APIInstance->SendRetrieveRequest(Params);
    return APIInstance;
}

void UGPTAssistantsAPI::SendRetrieveRequest(const FGPTAssistantRetrieveParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendRetrieveRequest called with Params."));

    // Set the current request type
    CurrentRequestType = "retrieve";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/assistants/%s"), *Params.AssistantID));
    CurrentHttpRequest->SetVerb(TEXT("GET"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTAssistantsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Retrieve request sent."));
}

UGPTAssistantsAPI* UGPTAssistantsAPI::ModifyAssistant(const FGPTAssistantModifyParams& Params)
{
    UGPTAssistantsAPI* APIInstance = NewObject<UGPTAssistantsAPI>();
    APIInstance->SendModifyRequest(Params);
    return APIInstance;
}

void UGPTAssistantsAPI::SendModifyRequest(const FGPTAssistantModifyParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendModifyRequest called with Params."));

    // Set the current request type
    CurrentRequestType = "modify";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/assistants/%s"), *Params.AssistantID));
    CurrentHttpRequest->SetVerb(TEXT("POST"));
    CurrentHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("model"), Params.Model);
    JsonObject->SetStringField(TEXT("name"), Params.Name);
    JsonObject->SetStringField(TEXT("description"), Params.Description);
    JsonObject->SetStringField(TEXT("instructions"), Params.Instructions);

    TArray<TSharedPtr<FJsonValue>> ToolsArray;
    for (const FString& Tool : Params.Tools)
    {
        TSharedPtr<FJsonObject> ToolObject = MakeShareable(new FJsonObject);
        ToolObject->SetStringField(TEXT("type"), Tool);
        ToolsArray.Add(MakeShareable(new FJsonValueObject(ToolObject)));
    }
    JsonObject->SetArrayField(TEXT("tools"), ToolsArray);
    JsonObject->SetNumberField(TEXT("temperature"), Params.Temperature);
    JsonObject->SetNumberField(TEXT("top_p"), Params.TopP);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    CurrentHttpRequest->SetContentAsString(RequestBody);

    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTAssistantsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Modify request sent."));
}

UGPTAssistantsAPI* UGPTAssistantsAPI::DeleteAssistant(const FGPTAssistantRetrieveParams& Params)
{
    UGPTAssistantsAPI* APIInstance = NewObject<UGPTAssistantsAPI>();
    APIInstance->SendDeleteRequest(Params);
    return APIInstance;
}

void UGPTAssistantsAPI::SendDeleteRequest(const FGPTAssistantRetrieveParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendDeleteRequest called with Params."));

    // Set the current request type
    CurrentRequestType = "delete";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();
    CurrentHttpRequest->SetURL(FString::Printf(TEXT("https://api.openai.com/v1/assistants/%s"), *Params.AssistantID));
    CurrentHttpRequest->SetVerb(TEXT("DELETE"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));


    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTAssistantsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("Delete request sent."));
}

UGPTAssistantsAPI* UGPTAssistantsAPI::ListAssistants(const FGPTAssistantListParams& Params)
{
    UGPTAssistantsAPI* APIInstance = NewObject<UGPTAssistantsAPI>();
    APIInstance->SendListRequest(Params);
    return APIInstance;
}

void UGPTAssistantsAPI::SendListRequest(const FGPTAssistantListParams& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("SendListRequest called with Limit: %d, Order: %s, After: %s, Before: %s"), Params.Limit, *Params.Order, *Params.After, *Params.Before);

    // Set the current request type
    CurrentRequestType = "list";

    CurrentHttpRequest = FHttpModule::Get().CreateRequest();

    // Construct the URL with query parameters
    FString URL = FString::Printf(TEXT("https://api.openai.com/v1/assistants?limit=%d&order=%s"), Params.Limit, *Params.Order);
    if (!Params.After.IsEmpty())
    {
        URL += FString::Printf(TEXT("&after=%s"), *Params.After);
    }
    if (!Params.Before.IsEmpty())
    {
        URL += FString::Printf(TEXT("&before=%s"), *Params.Before);
    }

    CurrentHttpRequest->SetURL(URL);
    CurrentHttpRequest->SetVerb(TEXT("GET"));
    CurrentHttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI)));
    CurrentHttpRequest->SetHeader(TEXT("OpenAI-Beta"), TEXT("assistants=v2"));

    CurrentHttpRequest->OnProcessRequestComplete().BindUObject(this, &UGPTAssistantsAPI::HandleResponse);
    CurrentHttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("List request sent."));
}


void UGPTAssistantsAPI::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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
                    TArray<FGPTAssistantObject> AssistantObjects;

                    for (const TSharedPtr<FJsonValue>& Value : *DataArray)
                    {
                        TSharedPtr<FJsonObject> AssistantData = Value->AsObject();
                        FGPTAssistantObject AssistantObject;

                        // Parse each assistant object within the list
                        AssistantData->TryGetStringField(TEXT("id"), AssistantObject.Id);
                        AssistantData->TryGetStringField(TEXT("object"), AssistantObject.Object);
                        AssistantData->TryGetNumberField(TEXT("created_at"), AssistantObject.CreatedAt);
                        AssistantData->TryGetStringField(TEXT("name"), AssistantObject.Name);
                        AssistantData->TryGetStringField(TEXT("description"), AssistantObject.Description);
                        AssistantData->TryGetStringField(TEXT("model"), AssistantObject.Model);
                        AssistantData->TryGetStringField(TEXT("instructions"), AssistantObject.Instructions);

                        // Parse tools array
                        const TArray<TSharedPtr<FJsonValue>>* ToolsArray;
                        if (AssistantData->TryGetArrayField(TEXT("tools"), ToolsArray))
                        {
                            for (const TSharedPtr<FJsonValue>& ToolValue : *ToolsArray)
                            {
                                TSharedPtr<FJsonObject> ToolObject = ToolValue->AsObject();
                                FString ToolType;
                                if (ToolObject->TryGetStringField(TEXT("type"), ToolType))
                                {
                                    AssistantObject.Tools.Add(ToolType);
                                }
                            }
                        }

                        // Parse Metadata
                        const TSharedPtr<FJsonObject>* MetadataObject;
                        if (AssistantData->TryGetObjectField(TEXT("metadata"), MetadataObject))
                        {
                            for (const auto& Pair : (*MetadataObject)->Values)
                            {
                                AssistantObject.Metadata.Add(Pair.Key, Pair.Value->AsString());
                            }
                        }

                        // Add the parsed assistant object to the array
                        AssistantObjects.Add(AssistantObject);
                    }

                    // Broadcast the list of assistants via the delegate
                    OnAssistantsListed.Broadcast(AssistantObjects);
                }
            }
            else
            {
                // Handle other types of response (create, retrieve, modify, delete)
                FGPTAssistantObject AssistantObject;
                AssistantObject.RawResponse = ResponseStr; // Store the raw response

                // Parse the common assistant fields
                JsonObject->TryGetStringField(TEXT("id"), AssistantObject.Id);
                JsonObject->TryGetStringField(TEXT("object"), AssistantObject.Object);
                JsonObject->TryGetNumberField(TEXT("created_at"), AssistantObject.CreatedAt);
                JsonObject->TryGetStringField(TEXT("name"), AssistantObject.Name);
                JsonObject->TryGetStringField(TEXT("description"), AssistantObject.Description);
                JsonObject->TryGetStringField(TEXT("model"), AssistantObject.Model);
                JsonObject->TryGetStringField(TEXT("instructions"), AssistantObject.Instructions);

                // Parse tools array
                const TArray<TSharedPtr<FJsonValue>>* ToolsArray;
                if (JsonObject->TryGetArrayField(TEXT("tools"), ToolsArray))
                {
                    for (const TSharedPtr<FJsonValue>& ToolValue : *ToolsArray)
                    {
                        TSharedPtr<FJsonObject> ToolObject = ToolValue->AsObject();
                        FString ToolType;
                        if (ToolObject->TryGetStringField(TEXT("type"), ToolType))
                        {
                            AssistantObject.Tools.Add(ToolType);
                        }
                    }
                }

                // Parse Metadata
                const TSharedPtr<FJsonObject>* MetadataObject;
                if (JsonObject->TryGetObjectField(TEXT("metadata"), MetadataObject))
                {
                    for (const auto& Pair : (*MetadataObject)->Values)
                    {
                        AssistantObject.Metadata.Add(Pair.Key, Pair.Value->AsString());
                    }
                }

                // Broadcast the filled AssistantObject based on the request type
                if (CurrentRequestType == "create")
                {
                    OnAssistantCreated.Broadcast(AssistantObject);
                }
                else if (CurrentRequestType == "retrieve")
                {
                    OnAssistantRetrieved.Broadcast(AssistantObject);
                }
                else if (CurrentRequestType == "modify")
                {
                    OnAssistantModified.Broadcast(AssistantObject);
                }
                else if (CurrentRequestType == "delete")
                {
                    OnAssistantDeleted.Broadcast(AssistantObject);
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



void UGPTAssistantsAPI::HandleError(const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("HandleError called for request type: %s. Error: %s"), *CurrentRequestType, *ErrorMessage);
    OnAPIError.Broadcast(ErrorMessage);
}
