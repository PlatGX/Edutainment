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



 // =================== DallEAPI.h ===================

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Http.h"
#include "HttpManager.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "GPTChatAPIStructs.h"
#include "DallEAPI.generated.h"

// Delegates for DallE API events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDallEAPICompleted, const TArray<FString>&, ImageURLs, const TArray<FString>&, RevisedPrompts);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDallEAPIError, const FString&, ErrorMessage);

// UDallEAPI: Class for handling DallE Image requests
UCLASS(BlueprintType)
class EZUEGPT_API UDallEAPI : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    virtual void BeginDestroy() override;

    // Function exposed to Blueprints for sending image requests to DallE API
    UFUNCTION(BlueprintCallable, Category = "*AIITK|DallE", meta = (DisplayName = "Send DallE Image Request", ToolTip = "Master function to send request based on Generation Type"))
    static UDallEAPI* SendDallEImageRequest_BP(const FDallEAPIParams& Params);

    // BlueprintAssignable properties for DallE API events
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|DallE")
    FOnDallEAPICompleted DallEOnCompleted;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|DallE")
    FOnDallEAPIError OnError;

    // Property to keep track of whether a DallE request is ongoing
    UPROPERTY(BlueprintReadOnly, Category = "*AIITK|DallE")
    bool bRequestInProgress = false;

    // HTTP Request object for sending DallE requests
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentDalleRequest;

    void ProcessDallEResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

protected:
    // Internal functions for sending requests to the DallE API
    void SendRequest(const FDallEAPIParams& Params);

    // Functions for different types of DallE requests
    void SendVariationRequest(const FDallEAPIParams& Params);

    void SendEditRequest(const FDallEAPIParams& Params);
};
