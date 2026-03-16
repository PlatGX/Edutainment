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



 // =================== Meshy.cpp ===================

#include "MeshyAPI.h"
#include "AIITKDeveloperSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "AIITKModelLoader.h"
#include "JsonObjectConverter.h"

#define MESHY_API_BASE_URL TEXT("https://api.meshy.ai")

void UMeshyAPI::BeginDestroy()
{
    UE_LOG(AIITKLog, Log, TEXT("Meshy BeginDestroy called, do not be alarmed this is for information only."));

    if (CurrentMeshyRequest.IsValid() && CurrentMeshyRequest->GetStatus() == EHttpRequestStatus::Processing)
    {
        UE_LOG(AIITKLog, Log, TEXT("Cancelling ongoing request"));
        CurrentMeshyRequest->CancelRequest();
    }

    Super::BeginDestroy();
}

// ==== AUTHENTICATION ====

void UMeshyAPI::SetAuthorizationHeader(TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& HttpRequest)
{
    FString APIKey = UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::Meshy);
    HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *APIKey));
}

// ==== PREVIEW TASK CREATION ====

UMeshyAPI* UMeshyAPI::CreatePreviewTask(const FMeshyPreviewTaskParams& Params)
{
    UE_LOG(AIITKLog, Log, TEXT("CreatePreviewTask called"));

    UMeshyAPI* MeshyAPIInstance = NewObject<UMeshyAPI>();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(MESHY_API_BASE_URL TEXT("/v2/text-to-3d"));
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    MeshyAPIInstance->SetAuthorizationHeader(HttpRequest);

    FString RequestPayload;
    FJsonObjectConverter::UStructToJsonObjectString(Params, RequestPayload);
    HttpRequest->SetContentAsString(RequestPayload);

    HttpRequest->OnProcessRequestComplete().BindUObject(MeshyAPIInstance, &UMeshyAPI::ProcessCreatePreviewTaskResponse);

    UE_LOG(AIITKLog, Log, TEXT("Processing CreatePreviewTask request with payload: %s"), *RequestPayload);
    HttpRequest->ProcessRequest();
    MeshyAPIInstance->CurrentMeshyRequest = HttpRequest;
    return MeshyAPIInstance;
}

void UMeshyAPI::ProcessCreatePreviewTaskResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
    UE_LOG(AIITKLog, Log, TEXT("ProcessCreatePreviewTaskResponse called with success: %s"), bSucceeded ? TEXT("true") : TEXT("false"));

    if (!bSucceeded || !HttpResponse.IsValid())
    {
        FString ErrorMessage = "Failed to receive a valid response.";
        UE_LOG(AIITKLog, Error, TEXT("%s"), *ErrorMessage);
        OnError.Broadcast(ErrorMessage);
        return;
    }

    FString ResponseContent = HttpResponse->GetContentAsString();
    UE_LOG(AIITKLog, Log, TEXT("Response content: %s"), *ResponseContent);

    FString TaskID;
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        TaskID = JsonObject->GetStringField(TEXT("result"));
    }

    if (!TaskID.IsEmpty())
    {
        UE_LOG(AIITKLog, Log, TEXT("Preview Task Created: %s"), *TaskID);
        OnTaskCreated.Broadcast(TaskID);
    }
    else
    {
        FString ErrorMessage = "Failed to parse task ID.";
        UE_LOG(AIITKLog, Error, TEXT("%s"), *ErrorMessage);
        OnError.Broadcast(ErrorMessage);
    }
}

// ==== REFINE TASK CREATION ====

UMeshyAPI* UMeshyAPI::CreateRefineTask(const FMeshyRefineTaskParams& Params)
{
    UE_LOG(AIITKLog, Log, TEXT("CreateRefineTask called"));

    UMeshyAPI* MeshyAPIInstance = NewObject<UMeshyAPI>();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(MESHY_API_BASE_URL TEXT("/v2/text-to-3d"));
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    MeshyAPIInstance->SetAuthorizationHeader(HttpRequest);

    FString RequestPayload;
    FJsonObjectConverter::UStructToJsonObjectString(Params, RequestPayload);
    HttpRequest->SetContentAsString(RequestPayload);

    HttpRequest->OnProcessRequestComplete().BindUObject(MeshyAPIInstance, &UMeshyAPI::ProcessCreateRefineTaskResponse);

    UE_LOG(AIITKLog, Log, TEXT("Processing CreateRefineTask request with payload: %s"), *RequestPayload);
    HttpRequest->ProcessRequest();
    MeshyAPIInstance->CurrentMeshyRequest = HttpRequest;
    return MeshyAPIInstance;
}

void UMeshyAPI::ProcessCreateRefineTaskResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
    UE_LOG(AIITKLog, Log, TEXT("ProcessCreateRefineTaskResponse called with success: %s"), bSucceeded ? TEXT("true") : TEXT("false"));

    if (!bSucceeded || !HttpResponse.IsValid())
    {
        FString ErrorMessage = "Failed to receive a valid response.";
        UE_LOG(AIITKLog, Error, TEXT("%s"), *ErrorMessage);
        OnError.Broadcast(ErrorMessage);
        return;
    }

    FString ResponseContent = HttpResponse->GetContentAsString();
    UE_LOG(AIITKLog, Log, TEXT("Response content: %s"), *ResponseContent);

    FString TaskID;
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        TaskID = JsonObject->GetStringField(TEXT("result"));
    }

    if (!TaskID.IsEmpty())
    {
        UE_LOG(AIITKLog, Log, TEXT("Refine Task Created: %s"), *TaskID);
        OnTaskCreated.Broadcast(TaskID);
    }
    else
    {
        FString ErrorMessage = "Failed to parse task ID.";
        UE_LOG(AIITKLog, Error, TEXT("%s"), *ErrorMessage);
        OnError.Broadcast(ErrorMessage);
    }
}

// ==== TEXTURE TASK CREATION ====

UMeshyAPI* UMeshyAPI::CreateTextToTextureTask(const FMeshyTextToTextureTaskParams& Params)
{
    UE_LOG(AIITKLog, Log, TEXT("CreateTextToTextureTask called"));

    UMeshyAPI* MeshyAPIInstance = NewObject<UMeshyAPI>();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(MESHY_API_BASE_URL TEXT("/v1/text-to-texture"));
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    MeshyAPIInstance->SetAuthorizationHeader(HttpRequest);

    FString RequestPayload;
    FJsonObjectConverter::UStructToJsonObjectString(Params, RequestPayload);
    HttpRequest->SetContentAsString(RequestPayload);

    HttpRequest->OnProcessRequestComplete().BindUObject(MeshyAPIInstance, &UMeshyAPI::ProcessCreateTextToTextureTaskResponse);

    UE_LOG(AIITKLog, Log, TEXT("Processing CreateTextToTextureTask request with payload: %s"), *RequestPayload);
    HttpRequest->ProcessRequest();
    MeshyAPIInstance->CurrentMeshyRequest = HttpRequest;
    return MeshyAPIInstance;
}

void UMeshyAPI::ProcessCreateTextToTextureTaskResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
    UE_LOG(AIITKLog, Log, TEXT("ProcessCreateTextToTextureTaskResponse called with success: %s"), bSucceeded ? TEXT("true") : TEXT("false"));

    if (!bSucceeded || !HttpResponse.IsValid())
    {
        FString ErrorMessage = "Failed to receive a valid response.";
        UE_LOG(AIITKLog, Error, TEXT("%s"), *ErrorMessage);
        OnError.Broadcast(ErrorMessage);
        return;
    }

    FString ResponseContent = HttpResponse->GetContentAsString();
    UE_LOG(AIITKLog, Log, TEXT("Response content: %s"), *ResponseContent);

    FString TaskID;
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        TaskID = JsonObject->GetStringField(TEXT("result"));
    }

    if (!TaskID.IsEmpty())
    {
        UE_LOG(AIITKLog, Log, TEXT("Text to Texture Task Created: %s"), *TaskID);
        OnTaskCreated.Broadcast(TaskID);
    }
    else
    {
        FString ErrorMessage = "Failed to parse task ID.";
        UE_LOG(AIITKLog, Error, TEXT("%s"), *ErrorMessage);
        OnError.Broadcast(ErrorMessage);
    }
}

// ==== RETRIEVE TASK ====

UMeshyAPI* UMeshyAPI::RetrieveTask(const FString& TaskID, EMeshyTaskType TaskType)
{
    UE_LOG(AIITKLog, Log, TEXT("RetrieveTask called with TaskID: %s"), *TaskID);

    UMeshyAPI* MeshyAPIInstance = NewObject<UMeshyAPI>();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    FString URL;

    if (TaskType == EMeshyTaskType::TextTo3D)
    {
        URL = FString::Printf(TEXT("%s/v2/text-to-3d/%s"), MESHY_API_BASE_URL, *TaskID);
    }
    else if (TaskType == EMeshyTaskType::TextToTexture)
    {
        URL = FString::Printf(TEXT("%s/v1/text-to-texture/%s"), MESHY_API_BASE_URL, *TaskID);
    }

    HttpRequest->SetURL(URL);
    HttpRequest->SetVerb(TEXT("GET"));
    MeshyAPIInstance->SetAuthorizationHeader(HttpRequest);

    HttpRequest->OnProcessRequestComplete().BindUObject(MeshyAPIInstance, &UMeshyAPI::ProcessRetrieveTaskResponse);

    UE_LOG(AIITKLog, Log, TEXT("Processing RetrieveTask request to URL: %s"), *URL);
    HttpRequest->ProcessRequest();
    MeshyAPIInstance->CurrentMeshyRequest = HttpRequest;
    return MeshyAPIInstance;
}

void UMeshyAPI::ProcessRetrieveTaskResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
    UE_LOG(AIITKLog, Log, TEXT("ProcessRetrieveTaskResponse called with success: %s"), bSucceeded ? TEXT("true") : TEXT("false"));

    if (!bSucceeded || !HttpResponse.IsValid())
    {
        FString ErrorMessage = "Failed to receive a valid response.";
        UE_LOG(AIITKLog, Error, TEXT("%s"), *ErrorMessage);
        OnError.Broadcast(ErrorMessage);
        return;
    }

    FString ResponseContent = HttpResponse->GetContentAsString();
    UE_LOG(AIITKLog, Log, TEXT("Response content: %s"), *ResponseContent);

    // Attempt to parse as FTextTo3DTask (used for both Text to 3D and Text to Texture tasks)
    FTextTo3DTask Task;
    if (FJsonObjectConverter::JsonObjectStringToUStruct(ResponseContent, &Task, 0, 0))
    {
        if (Task.status == "SUCCEEDED" || Task.status == "IN_PROGRESS" || Task.status == "PENDING")
        {
            OnMeshCompleted.Broadcast(Task);
            return;
        }
        else if (Task.status == "FAILED" || Task.status == "EXPIRED")
        {
            FString ErrorMessage = FString::Printf(TEXT("Task failed with status: %s"), *Task.status);
            if (!Task.task_error.message.IsEmpty())
            {
                ErrorMessage.Append(FString::Printf(TEXT(" Error: %s"), *Task.task_error.message));
            }
            UE_LOG(AIITKLog, Error, TEXT("%s"), *ErrorMessage);
            OnError.Broadcast(ErrorMessage);
            return;
        }
    }

    // If parsing fails
    FString ErrorMessage = "Failed to parse response JSON.";
    UE_LOG(AIITKLog, Error, TEXT("%s"), *ErrorMessage);
    OnError.Broadcast(ErrorMessage);
}


// ==== DOWNLOAD MODEL ====

UMeshyAPI* UMeshyAPI::DownloadModel(const FString& ModelURL, const FString& ModelFilePath)
{
    UE_LOG(AIITKLog, Log, TEXT("DownloadModel called with URL: %s and FilePath: %s"), *ModelURL, *ModelFilePath);

    UMeshyAPI* MeshyAPIInstance = NewObject<UMeshyAPI>();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(ModelURL);
    HttpRequest->SetVerb(TEXT("GET"));
    MeshyAPIInstance->SetAuthorizationHeader(HttpRequest);
    FString Path = ModelFilePath;
    HttpRequest->OnProcessRequestComplete().BindLambda([MeshyAPIInstance, Path](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        UE_LOG(AIITKLog, Log, TEXT("ProcessDownloadModelResponse called with success: %s"), bWasSuccessful ? TEXT("true") : TEXT("false"));

        if (!bWasSuccessful || !Response.IsValid())
        {
            FString ErrorMessage = "Failed to receive a valid response.";
            UE_LOG(AIITKLog, Error, TEXT("%s"), *ErrorMessage);
            MeshyAPIInstance->OnError.Broadcast(ErrorMessage);
            return;
        }

        TArray<uint8> FileData = Response->GetContent();
        if (FFileHelper::SaveArrayToFile(FileData, *Path))
        {
            UE_LOG(AIITKLog, Log, TEXT("Model downloaded and saved to: %s"), *Path);
            MeshyAPIInstance->OnDownloadModelCompleted.Broadcast(Path);
        }
        else
        {
            FString ErrorMessage = "Failed to save model file.";
            UE_LOG(AIITKLog, Error, TEXT("%s"), *ErrorMessage);
            MeshyAPIInstance->OnError.Broadcast(ErrorMessage);
        }
    });

    UE_LOG(AIITKLog, Log, TEXT("Processing DownloadModel request to URL: %s"), *ModelURL);
    HttpRequest->ProcessRequest();
    MeshyAPIInstance->CurrentMeshyRequest = HttpRequest;
    return MeshyAPIInstance;
}