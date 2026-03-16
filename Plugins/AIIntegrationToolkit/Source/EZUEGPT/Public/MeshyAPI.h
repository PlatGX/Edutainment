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



 // =================== Meshy.h ===================

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Http.h"
#include "HttpManager.h"
#include "GPTChatAPIStructs.h"
#include "ProceduralMeshComponent.h"
#include "OpenAIFunctionLibrary.h"
#include "MeshyAPI.generated.h"

// Delegates for Meshy API events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeshyTaskCreated, const FString&, TaskID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeshyAPICompleted, const FTextTo3DTask&, TaskData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeshyAPIError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDownloadModelCompleted, const FString&, ModelFilePath);

// UMeshyAPI: Class for handling Meshy model requests
UCLASS(BlueprintType)
class EZUEGPT_API UMeshyAPI : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    virtual void BeginDestroy() override;

    // Function exposed to Blueprints for creating a preview task
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Meshy", meta = (DisplayName = "Create Preview Task", ToolTip = "Creates a new Text to 3D Preview Task"))
    static UMeshyAPI* CreatePreviewTask(const FMeshyPreviewTaskParams& Params);

    // Function exposed to Blueprints for creating a refine task
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Meshy", meta = (DisplayName = "Create Refine Task", ToolTip = "Creates a new Text to 3D Refine Task"))
    static UMeshyAPI* CreateRefineTask(const FMeshyRefineTaskParams& Params);

    // Function exposed to Blueprints for creating a text to texture task
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Meshy", meta = (DisplayName = "Create Text to Texture Task", ToolTip = "Creates a new Text to Texture Task"))
    static UMeshyAPI* CreateTextToTextureTask(const FMeshyTextToTextureTaskParams& Params);

    // Function exposed to Blueprints for retrieving a task
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Meshy", meta = (DisplayName = "Retrieve Task", ToolTip = "Retrieves a Text to 3D or Text to Texture Task"))
    static UMeshyAPI* RetrieveTask(const FString& TaskID, EMeshyTaskType TaskType);

    // Function exposed to Blueprints for downloading a model
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Meshy", meta = (DisplayName = "Download Model", ToolTip = "Downloads a model from the specified URL"))
    static UMeshyAPI* DownloadModel(const FString& ModelURL, const FString& ModelFilePath);

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Meshy")
    FOnMeshyTaskCreated OnTaskCreated;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Meshy")
    FOnMeshyAPICompleted OnMeshCompleted;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Meshy")
    FOnMeshyAPIError OnError;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Meshy")
    FOnDownloadModelCompleted OnDownloadModelCompleted;

    // Property to keep track of whether a Meshy request is ongoing
    UPROPERTY(BlueprintReadOnly, Category = "*AIITK|Meshy")
    bool bRequestInProgress;

    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentMeshyRequest;

private:
    // Utility function to set the authorization header
    static void SetAuthorizationHeader(TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& HttpRequest);

    // Internal function for processing the preview task creation response
    void ProcessCreatePreviewTaskResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

    // Internal function for processing the refine task creation response
    void ProcessCreateRefineTaskResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

    // Internal function for processing the text to texture task creation response
    void ProcessCreateTextToTextureTaskResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

    // Internal function for processing the retrieve task response
    void ProcessRetrieveTaskResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
};
