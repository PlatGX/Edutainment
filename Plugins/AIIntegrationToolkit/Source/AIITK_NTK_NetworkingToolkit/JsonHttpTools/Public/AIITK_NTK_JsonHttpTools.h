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


// ========== AIITK_NTK_JsonHttpTools.h ==========

#pragma once

#include "Misc/EngineVersion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "CoreMinimal.h"
#include "Http.h"

#include "AIITK_NTK_JsonHttpTools.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnJsonHttpRequestError, FString, ErrorMessage);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnJsonHttpRequestProgress, FString, PartialData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnJsonHttpRequestHeaderReceived, const FString&, HeaderInfo);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnJsonHttpRequestComplete, const FString&, ResponseData);

/**
 * UAIITK_NTK_JsonHttpTools provides functionality to send HTTP requests with JSON data and multipart form data.
 */
UCLASS(Blueprintable, BlueprintType)
class AIITK_NTK_NETWORKINGTOOLKIT_API UAIITK_NTK_JsonHttpTools : public UObject 
{
    GENERATED_BODY()

public:
    /**
     * Sends an HTTP POST request with the given JSON string, query parameters, and the specified URL.
     * @param URL - The base endpoint URL to send the request to.
     * @param QueryParameters - A map of query parameters to append to the URL.
     * @param JsonData - The JSON-formatted string to be sent in the request body.
     * @param ApiKey - The API key used for authorization.
     * @param OnError - Delegate called when an error occurs during the request.
     * @param OnProgress - Delegate called to provide progress updates.
     * @param OnHeaderReceived - Delegate called when a header is received.
     * @param OnComplete - Delegate called when the request completes successfully.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|HTTP")
    void SendHttpRequest(
        const FString& URL,
        const TMap<FString, FString>& QueryParameters, // Added query parameters
        const FString& JsonData,
        const FString& ApiKey,
        const FOnJsonHttpRequestError& OnError,
        const FOnJsonHttpRequestProgress& OnProgress,
        const FOnJsonHttpRequestHeaderReceived& OnHeaderReceived,
        const FOnJsonHttpRequestComplete& OnComplete);

    /**
     * Sends an HTTP POST request with multipart form data.
     * @param URL - The endpoint URL to send the request to.
     * @param FormData - Key-value pairs representing form fields.
     * @param FilePath - The path to the file to be uploaded.
     * @param FileFieldName - The name of the form field for the file upload.
     * @param ApiKey - The API key used for authorization.
     * @param OnError - Delegate called when an error occurs during the request.
     * @param OnProgress - Delegate called to provide progress updates.
     * @param OnHeaderReceived - Delegate called when a header is received.
     * @param OnComplete - Delegate called when the request completes successfully.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|HTTP")
    void SendMultipartFormDataRequest(
        const FString& URL,
        const TMap<FString, FString>& FormData,
        const FString& FilePath,
        const FString& FileFieldName,
        const FString& ApiKey,
        const FOnJsonHttpRequestError& OnError,
        const FOnJsonHttpRequestProgress& OnProgress,
        const FOnJsonHttpRequestHeaderReceived& OnHeaderReceived,
        const FOnJsonHttpRequestComplete& OnComplete);


private:

    /**
     * Handles the response for the request.
     * @param Request - The HTTP request that was processed.
     * @param Response - The HTTP response received.
     * @param bWasSuccessful - Whether the request was successful.
     * @param OnError - Delegate to be called on error.
     * @param OnComplete - Delegate to be called on completion.
     */
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnJsonHttpRequestError OnError, FOnJsonHttpRequestComplete OnComplete);

    /**
     * Handles HTTP progress updates.
     * @param Request - The HTTP request being processed.
     * @param BytesSent - The number of bytes sent.
     * @param BytesReceived - The number of bytes received.
     * @param OnProgress - Delegate to be called with progress updates.
     */
#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4)
    void OnRequestProgress64(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived, FOnJsonHttpRequestProgress OnProgress);
#else
    void OnRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived, FOnJsonHttpRequestProgress OnProgress);
#endif

    /**
     * Handles headers received during the HTTP request.
     * @param Request - The HTTP request being processed.
     * @param HeaderKey - The key of the header received.
     * @param HeaderValue - The value of the header received.
     * @param OnHeaderReceived - Delegate to be called when a header is received.
     */
    void OnRequestHeaderReceived(FHttpRequestPtr Request, const FString& HeaderKey, const FString& HeaderValue, FOnJsonHttpRequestHeaderReceived OnHeaderReceived);
};
