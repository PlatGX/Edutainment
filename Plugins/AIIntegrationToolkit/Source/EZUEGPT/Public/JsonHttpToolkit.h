#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "Http.h"
#include "Misc/Paths.h"
#include "Runtime/Launch/Resources/Version.h" // Include the version header
#include "JsonHttpToolkit.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHttpRequestError, FString, ErrorMessage);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHttpRequestProgress, FString, PartialData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHttpRequestHeaderReceived, const FString&, HeaderInfo);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHttpRequestComplete, const FString&, ResponseData);

/**
 * UJsonHttpToolkit provides functionality to send HTTP requests with JSON data and multipart form data.
 */
UCLASS(Blueprintable, BlueprintType)
class EZUEGPT_API UJsonHttpToolkit : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Sends an HTTP POST request with the given JSON string and the specified URL.
     * @param URL - The endpoint URL to send the request to.
     * @param JsonData - The JSON-formatted string to be sent in the request body.
     * @param ApiKey - The API key used for authorization.
     * @param OnError - Delegate called when an error occurs during the request.
     * @param OnProgress - Delegate called to provide progress updates.
     * @param OnHeaderReceived - Delegate called when a header is received.
     * @param OnComplete - Delegate called when the request completes successfully.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|HTTP")
    void SendHttpRequest(
        const FString& URL,
        const FString& JsonData,
        const FString& ApiKey, // Added API Key parameter
        const FOnHttpRequestError& OnError,
        const FOnHttpRequestProgress& OnProgress,
        const FOnHttpRequestHeaderReceived& OnHeaderReceived,
        const FOnHttpRequestComplete& OnComplete);

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
    UFUNCTION(BlueprintCallable, Category = "*AIITK|HTTP")
    void SendMultipartFormDataRequest(
        const FString& URL,
        const TMap<FString, FString>& FormData,
        const FString& FilePath,
        const FString& FileFieldName,
        const FString& ApiKey,
        const FOnHttpRequestError& OnError,
        const FOnHttpRequestProgress& OnProgress,
        const FOnHttpRequestHeaderReceived& OnHeaderReceived,
        const FOnHttpRequestComplete& OnComplete);

private:
    /**
     * Handles the response for the request.
     * @param Request - The HTTP request that was processed.
     * @param Response - The HTTP response received.
     * @param bWasSuccessful - Whether the request was successful.
     * @param OnError - Delegate to be called on error.
     * @param OnComplete - Delegate to be called on completion.
     */
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnHttpRequestError OnError, FOnHttpRequestComplete OnComplete);

    /**
     * Handles HTTP progress updates.
     * @param Request - The HTTP request being processed.
     * @param BytesSent - The number of bytes sent.
     * @param BytesReceived - The number of bytes received.
     * @param OnProgress - Delegate to be called with progress updates.
     */
#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4)
    void OnRequestProgress64(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived, FOnHttpRequestProgress OnProgress);
#else
    void OnRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived, FOnHttpRequestProgress OnProgress);
#endif

    /**
     * Handles headers received during the HTTP request.
     * @param Request - The HTTP request being processed.
     * @param HeaderKey - The key of the header received.
     * @param HeaderValue - The value of the header received.
     * @param OnHeaderReceived - Delegate to be called when a header is received.
     */
    void OnRequestHeaderReceived(FHttpRequestPtr Request, const FString& HeaderKey, const FString& HeaderValue, FOnHttpRequestHeaderReceived OnHeaderReceived);
};
