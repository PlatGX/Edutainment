#include "JsonHttpToolkit.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Runtime/Launch/Resources/Version.h" // Include the version header

void UJsonHttpToolkit::SendHttpRequest(
    const FString& URL,
    const FString& JsonData,
    const FString& ApiKey, // Added API Key parameter
    const FOnHttpRequestError& OnError,
    const FOnHttpRequestProgress& OnProgress,
    const FOnHttpRequestHeaderReceived& OnHeaderReceived,
    const FOnHttpRequestComplete& OnComplete)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UJsonHttpToolkit::OnResponseReceived, OnError, OnComplete);

#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4)
    HttpRequest->OnRequestProgress64().BindUObject(this, &UJsonHttpToolkit::OnRequestProgress64, OnProgress);
#else
    HttpRequest->OnRequestProgress().BindUObject(this, &UJsonHttpToolkit::OnRequestProgress, OnProgress);
#endif

    HttpRequest->OnHeaderReceived().BindUObject(this, &UJsonHttpToolkit::OnRequestHeaderReceived, OnHeaderReceived);

    // Set the URL, content-type, and authorization header
    HttpRequest->SetURL(URL);
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *ApiKey)); // Added Authorization header
    HttpRequest->SetContentAsString(JsonData);

    // Send the request
    if (!HttpRequest->ProcessRequest())
    {
        OnError.ExecuteIfBound(TEXT("Failed to process HTTP request"));
    }
}

void UJsonHttpToolkit::SendMultipartFormDataRequest(
    const FString& URL,
    const TMap<FString, FString>& FormData,
    const FString& FilePath,
    const FString& FileFieldName,
    const FString& ApiKey,
    const FOnHttpRequestError& OnError,
    const FOnHttpRequestProgress& OnProgress,
    const FOnHttpRequestHeaderReceived& OnHeaderReceived,
    const FOnHttpRequestComplete& OnComplete)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UJsonHttpToolkit::OnResponseReceived, OnError, OnComplete);

#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4)
    HttpRequest->OnRequestProgress64().BindUObject(this, &UJsonHttpToolkit::OnRequestProgress64, OnProgress);
#else
    HttpRequest->OnRequestProgress().BindUObject(this, &UJsonHttpToolkit::OnRequestProgress, OnProgress);
#endif

    HttpRequest->OnHeaderReceived().BindUObject(this, &UJsonHttpToolkit::OnRequestHeaderReceived, OnHeaderReceived);

    // Set the URL and headers (including authorization)
    HttpRequest->SetURL(URL);
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *ApiKey));

    // Create the boundary for multi-part form data
    FString Boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    HttpRequest->SetHeader("Content-Type", FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));

    // Build the multi-part body
    FString FormDataBody;

    // Make a copy of FormData to ensure no modifications are made during the loop
    TMap<FString, FString> LocalFormData = FormData;

    // Add form fields
    for (const auto& Entry : LocalFormData)
    {
        FormDataBody += FString::Printf(TEXT("--%s\r\n"), *Boundary);
        FormDataBody += FString::Printf(TEXT("Content-Disposition: form-data; name=\"%s\"\r\n\r\n"), *Entry.Key);
        FormDataBody += Entry.Value + TEXT("\r\n");
    }

    // Add file part
    if (!FilePath.IsEmpty())
    {
        FString FileContent;
        if (FFileHelper::LoadFileToString(FileContent, *FilePath))
        {
            FString FileName = FPaths::GetCleanFilename(FilePath);
            FormDataBody += FString::Printf(TEXT("--%s\r\n"), *Boundary);
            FormDataBody += FString::Printf(TEXT("Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"), *FileFieldName, *FileName);
            FormDataBody += TEXT("Content-Type: application/octet-stream\r\n\r\n");
            FormDataBody += FileContent + TEXT("\r\n");
        }
    }

    // End the multi-part body
    FormDataBody += FString::Printf(TEXT("--%s--\r\n"), *Boundary);

    // Set the request content as the multi-part form data
    HttpRequest->SetContentAsString(FormDataBody);

    // Send the request
    if (!HttpRequest->ProcessRequest())
    {
        OnError.ExecuteIfBound(TEXT("Failed to process multipart HTTP request"));
    }
}

void UJsonHttpToolkit::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnHttpRequestError OnError, FOnHttpRequestComplete OnComplete)
{
    if (bWasSuccessful && Response.IsValid())
    {
        int32 ResponseCode = Response->GetResponseCode();
        if (ResponseCode == 200) // Success
        {
            FString ResponseString = Response->GetContentAsString();
            OnComplete.ExecuteIfBound(ResponseString);
        }
        else
        {
            OnError.ExecuteIfBound(FString::Printf(TEXT("HTTP Error: %d"), ResponseCode));
        }
    }
    else
    {
        OnError.ExecuteIfBound(TEXT("Failed to receive a valid response"));
    }
}

#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4)
void UJsonHttpToolkit::OnRequestProgress64(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived, FOnHttpRequestProgress OnProgress)
{
    if (Request.IsValid())
    {
        uint64 ContentLength = Request->GetContentLength();
        float Progress = (ContentLength > 0) ? (static_cast<float>(BytesSent) / static_cast<float>(ContentLength)) : 0.0f;

        // Safely access response content
        if (Request->GetResponse().IsValid())
        {
            FString PartialData = Request->GetResponse()->GetContentAsString();
            OnProgress.ExecuteIfBound(PartialData); // Return partial data to delegate
        }
        else
        {
            OnProgress.ExecuteIfBound(FString::Printf(TEXT("Progress: %f"), Progress)); // Fallback to progress percentage
        }
    }
}
#else
void UJsonHttpToolkit::OnRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived, FOnHttpRequestProgress OnProgress)
{
    if (Request.IsValid())
    {
        int32 ContentLength = Request->GetContentLength();
        float Progress = (ContentLength > 0) ? (static_cast<float>(BytesSent) / static_cast<float>(ContentLength)) : 0.0f;

        // Safely access response content
        if (Request->GetResponse().IsValid())
        {
            FString PartialData = Request->GetResponse()->GetContentAsString();
            OnProgress.ExecuteIfBound(PartialData); // Return partial data to delegate
        }
        else
        {
            OnProgress.ExecuteIfBound(FString::Printf(TEXT("Progress: %f"), Progress)); // Fallback to progress percentage
        }
    }
}
#endif

void UJsonHttpToolkit::OnRequestHeaderReceived(FHttpRequestPtr Request, const FString& HeaderKey, const FString& HeaderValue, FOnHttpRequestHeaderReceived OnHeaderReceived)
{
    FString HeaderInfo = FString::Printf(TEXT("%s: %s"), *HeaderKey, *HeaderValue);
    OnHeaderReceived.ExecuteIfBound(HeaderInfo);
}
