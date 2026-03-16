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


// ========== AIITK_NTK_JsonHttpTools.cpp ==========

#include "AIITK_NTK_JsonHttpTools.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/FileHelper.h"

void UAIITK_NTK_JsonHttpTools::SendHttpRequest(
    const FString& URL,
    const TMap<FString, FString>& Headers,
    const FString& Method,
    const FString& Body,
    const FOnJsonHttpRequestError& OnError,
    const FOnJsonHttpRequestProgress& OnProgress,
    const FOnJsonHttpRequestHeaderReceived& OnHeaderReceived,
    const FOnJsonHttpRequestComplete& OnComplete)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

    // Bind delegates
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UAIITK_NTK_JsonHttpTools::OnResponseReceived, OnError, OnComplete);
    HttpRequest->OnHeaderReceived().BindUObject(this, &UAIITK_NTK_JsonHttpTools::OnRequestHeaderReceived, OnHeaderReceived);

#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4)
    HttpRequest->OnRequestProgress64().BindUObject(this, &UAIITK_NTK_JsonHttpTools::OnRequestProgress64, OnProgress);
#else
    HttpRequest->OnRequestProgress().BindUObject(this, &UAIITK_NTK_JsonHttpTools::OnRequestProgress, OnProgress);
#endif

    // Configure request
    HttpRequest->SetURL(URL);
    HttpRequest->SetVerb(Method);
    for (const auto& Header : Headers)
    {
        HttpRequest->SetHeader(Header.Key, Header.Value);
    }
    HttpRequest->SetContentAsString(Body);

    // Send the request
    if (!HttpRequest->ProcessRequest())
    {
        OnError.ExecuteIfBound(TEXT("Failed to process HTTP request"));
    }
}



void UAIITK_NTK_JsonHttpTools::SendMultipartFormDataRequest(
    const FString& URL,
    const TMap<FString, FString>& FormData,
    const FString& FilePath,
    const FString& FileFieldName,
    const FString& ApiKey,
    const FOnJsonHttpRequestError& OnError,
    const FOnJsonHttpRequestProgress& OnProgress,
    const FOnJsonHttpRequestHeaderReceived& OnHeaderReceived,
    const FOnJsonHttpRequestComplete& OnComplete)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UAIITK_NTK_JsonHttpTools::OnResponseReceived, OnError, OnComplete);

#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4)
    HttpRequest->OnRequestProgress64().BindUObject(this, &UAIITK_NTK_JsonHttpTools::OnRequestProgress64, OnProgress);
#else
    HttpRequest->OnRequestProgress().BindUObject(this, &UAIITK_NTK_JsonHttpTools::OnRequestProgress, OnProgress);
#endif

    HttpRequest->OnHeaderReceived().BindUObject(this, &UAIITK_NTK_JsonHttpTools::OnRequestHeaderReceived, OnHeaderReceived);

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

void UAIITK_NTK_JsonHttpTools::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnJsonHttpRequestError OnError, FOnJsonHttpRequestComplete OnComplete)
{
    if (bWasSuccessful && Response.IsValid())
    {
        FString ResponseString = Response->GetContentAsString();
        int32 ResponseCode = Response->GetResponseCode();
        if (ResponseCode == 200) // Success
        {
            OnComplete.ExecuteIfBound(ResponseString);
        }
        else
        {
            // Print only the response content for errors
            OnError.ExecuteIfBound(ResponseString);
        }
    }
    else
    {
        OnError.ExecuteIfBound(TEXT("Failed to receive a valid response"));
    }
}



#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4)
void UAIITK_NTK_JsonHttpTools::OnRequestProgress64(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived, FOnJsonHttpRequestProgress OnProgress)
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
void UAIITK_NTK_JsonHttpTools::OnRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived, FOnJsonHttpRequestProgress OnProgress)
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

void UAIITK_NTK_JsonHttpTools::OnRequestHeaderReceived(FHttpRequestPtr Request, const FString& HeaderKey, const FString& HeaderValue, FOnJsonHttpRequestHeaderReceived OnHeaderReceived)
{
    FString HeaderInfo = FString::Printf(TEXT("%s: %s"), *HeaderKey, *HeaderValue);
    OnHeaderReceived.ExecuteIfBound(HeaderInfo);
}
