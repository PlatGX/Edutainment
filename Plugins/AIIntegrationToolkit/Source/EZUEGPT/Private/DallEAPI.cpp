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



// =================== DallEAPI.cpp ===================

#include "DallEAPI.h"
#include "Runtime/ImageWrapper/Public/IImageWrapper.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "ImageUtils.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "Runtime/Engine/Classes/Kismet/KismetRenderingLibrary.h"
#include "Runtime/Core/Public/Misc/Base64.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "AIITKDeveloperSettings.h"
#include "Engine/Engine.h"
#include "OpenAIFunctionLibrary.h"

// ==== INIT ====

void UDallEAPI::BeginDestroy()
{
    if (CurrentDalleRequest.IsValid() && CurrentDalleRequest->GetStatus() == EHttpRequestStatus::Processing)
    {
        CurrentDalleRequest->CancelRequest();
    }

    Super::BeginDestroy();
}


// ==== REQUEST TYPE SELECTION ====

UDallEAPI* UDallEAPI::SendDallEImageRequest_BP(const FDallEAPIParams& Params)
{
    UDallEAPI* DallEAPIInstance = NewObject<UDallEAPI>();

    switch (Params.RequestType)
    {
    case EGenerationType::Image:
        DallEAPIInstance->SendRequest(Params);
        break;
    case EGenerationType::Variation:
        DallEAPIInstance->SendVariationRequest(Params);
        break;
    case EGenerationType::Edit:
        DallEAPIInstance->SendEditRequest(Params);
        break;
    default:
        return nullptr;
    }

    return DallEAPIInstance;
}


// ==== REQUEST/RESPONSE ====

void UDallEAPI::SendRequest(const FDallEAPIParams& Params)
{
    if (this->bRequestInProgress)
    {
        UE_LOG(AIITKLog, Error, TEXT("Request already in progress"));
        return;
    }

    this->bRequestInProgress = true;

    FString ImageSizeString = UOpenAIFunctionLibrary::ImageSizeToString(Params.ImageSize);

    // Fetch decrypted API key
    FString DecryptedAPIKey = UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Params.Endpoint);
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetHeader("Authorization", "Bearer " + DecryptedAPIKey);
    HttpRequest->SetTimeout(Params.Timeout);

    FString EncodingString;
    switch (Params.Encoding)
    {
    case EImageEncoding::Base64:
        EncodingString = "b64_json";
        break;
    case EImageEncoding::URL:
    default:
        EncodingString = "url";
        break;
    }

    // Prepare the request payload
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField("model", Params.Model);
    JsonObject->SetStringField("prompt", Params.Prompt);
    JsonObject->SetStringField("quality", Params.Quality);
    JsonObject->SetNumberField("n", Params.NumberOfImages);
    JsonObject->SetStringField("size", ImageSizeString);
    JsonObject->SetStringField("response_format", EncodingString);

    FString RequestPayload;
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&RequestPayload);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

    HttpRequest->SetContentAsString(RequestPayload);

    // Log the raw request
    UE_LOG(AIITKLog, Log, TEXT("Sending Request: %s"), *RequestPayload);

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
    HttpRequest->OnRequestProgress64().BindLambda([this](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
#else
    HttpRequest->OnRequestProgress().BindLambda([this](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
#endif
    {
        UE_LOG(AIITKLog, Log, TEXT("Request Progress: Bytes Sent %d, Bytes Received %d"), BytesSent, BytesReceived);
    });

    // Process images on request complete
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDallEAPI::ProcessDallEResponse);

    // Process the request
    HttpRequest->ProcessRequest();
    CurrentDalleRequest = HttpRequest;
}

void UDallEAPI::SendVariationRequest(const FDallEAPIParams& Params)
{
    if (this->bRequestInProgress)
    {
        return;
    }

    this->bRequestInProgress = true;

    TArray<uint8> ImageData;
    if (!FFileHelper::LoadFileToArray(ImageData, *Params.ImageToSend))
    {
        return;
    }

    FString ImageSizeString;
    switch (Params.ImageSize)
    {
    case EImageSize::S256x256:
        ImageSizeString = "256x256";
        break;
    case EImageSize::S512x512:
        ImageSizeString = "512x512";
        break;
    case EImageSize::S1024x1024:
        ImageSizeString = "1024x1024";
        break;
    case EImageSize::S1792x1024:
        ImageSizeString = "1792x1024";
        break;
    case EImageSize::S1024x1792:
        ImageSizeString = "1024x1792";
        break;
    default:
        ImageSizeString = "256x256";
        break;
    }

    FString EncodingString;
    switch (Params.Encoding)
    {
    case EImageEncoding::Base64:
        EncodingString = "b64_json";
        break;
    case EImageEncoding::URL:
    default:
        EncodingString = "url";
        break;
    }

    // Fetch decrypted API key
    FString DecryptedAPIKey = UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Params.Endpoint);
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Authorization", "Bearer " + DecryptedAPIKey);
    HttpRequest->SetTimeout(Params.Timeout);

    // Prepare form data
    FString Boundary = "-----------------------------" + FGuid::NewGuid().ToString();
    HttpRequest->SetHeader("Content-Type", "multipart/form-data; boundary=" + Boundary);

    FString BeginBoundary = "--" + Boundary + "\r\n";
    FString EndBoundary = "\r\n--" + Boundary + "--\r\n";

    TArray<uint8> FormData;
    uint8 NewLine[2] = { '\r', '\n' };

    // Add other parts (n, size, response_format)
    FString NPart = BeginBoundary + "Content-Disposition: form-data; name=\"n\"\r\n\r\n" + FString::Printf(TEXT("%d"), Params.NumberOfImages) + "\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*NPart), NPart.Len());

    FString SizePart = BeginBoundary + "Content-Disposition: form-data; name=\"size\"\r\n\r\n" + ImageSizeString + "\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*SizePart), SizePart.Len());

    FString ResponseFormatPart = BeginBoundary + "Content-Disposition: form-data; name=\"response_format\"\r\n\r\n" + EncodingString + "\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*ResponseFormatPart), ResponseFormatPart.Len());

    // Add the image file part
    FString FilePartHeader = BeginBoundary + "Content-Disposition: form-data; name=\"image\"; filename=\"image.png\"\r\n";
    FilePartHeader += "Content-Type: image/png\r\n\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*FilePartHeader), FilePartHeader.Len());

    // Append the raw image data to the FormData
    FormData.Append(ImageData);

    // Add a new line after the image data
    FormData.Append(NewLine, 2);

    // Append the EndBoundary to the FormData
    FormData.Append((uint8*)TCHAR_TO_UTF8(*EndBoundary), EndBoundary.Len());

    // Log the form data as a string
    FString FormDataString;
    FormDataString.Append(FilePartHeader);
    FormDataString.Append("<binary image data>\r\n");  // You can't convert the binary image data to a string, well, you can, but it would be garbage characters
    FormDataString.Append(NPart);
    FormDataString.Append(SizePart);
    FormDataString.Append(ResponseFormatPart);
    FormDataString.Append(EndBoundary);
    UE_LOG(AIITKLog, Warning, TEXT("Form Data: %s"), *FormDataString);

    HttpRequest->SetContent(FormData);

    // Process images on request complete
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDallEAPI::ProcessDallEResponse);

    HttpRequest->ProcessRequest();
    CurrentDalleRequest = HttpRequest;
}

void UDallEAPI::SendEditRequest(const FDallEAPIParams& Params)
{
    if (this->bRequestInProgress)
    {
        return;
    }

    this->bRequestInProgress = true;

    TArray<uint8> ImageData;
    if (!FFileHelper::LoadFileToArray(ImageData, *Params.ImageToSend))
    {
        return;
    }

    TArray<uint8> MaskImageData;
    if (!FFileHelper::LoadFileToArray(MaskImageData, *Params.MaskToSend))
    {
        return;
    }

    FString ImageSizeString;
    switch (Params.ImageSize)
    {
    case EImageSize::S256x256:
        ImageSizeString = "256x256";
        break;
    case EImageSize::S512x512:
        ImageSizeString = "512x512";
        break;
    case EImageSize::S1024x1024:
        ImageSizeString = "1024x1024";
        break;
    case EImageSize::S1792x1024:
        ImageSizeString = "1792x1024";
        break;
    case EImageSize::S1024x1792:
        ImageSizeString = "1024x1792";
        break;
    default:
        ImageSizeString = "256x256";
        break;
    }

    FString EncodingString;
    switch (Params.Encoding)
    {
    case EImageEncoding::Base64:
        EncodingString = "b64_json";
        break;
    case EImageEncoding::URL:
    default:
        EncodingString = "url";
        break;
    }

    // Fetch decrypted API key
    FString DecryptedAPIKey = UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Params.Endpoint);
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Authorization", "Bearer " + DecryptedAPIKey);
    HttpRequest->SetTimeout(Params.Timeout);

    // Prepare form data
    FString Boundary = "-----------------------------" + FGuid::NewGuid().ToString();
    HttpRequest->SetHeader("Content-Type", "multipart/form-data; boundary=" + Boundary);

    FString BeginBoundary = "--" + Boundary + "\r\n";
    FString EndBoundary = "\r\n--" + Boundary + "--\r\n";

    TArray<uint8> FormData;
    uint8 NewLine[2] = { '\r', '\n' };

    // Add other parts (n, size, response_format)
    FString NPart = BeginBoundary + "Content-Disposition: form-data; name=\"n\"\r\n\r\n" + FString::Printf(TEXT("%d"), Params.NumberOfImages) + "\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*NPart), NPart.Len());

    FString SizePart = BeginBoundary + "Content-Disposition: form-data; name=\"size\"\r\n\r\n" + ImageSizeString + "\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*SizePart), SizePart.Len());

    FString ResponseFormatPart = BeginBoundary + "Content-Disposition: form-data; name=\"response_format\"\r\n\r\n" + EncodingString + "\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*ResponseFormatPart), ResponseFormatPart.Len());

    // Add the prompt part
    FString PromptPart = BeginBoundary + "Content-Disposition: form-data; name=\"prompt\"\r\n\r\n" + Params.Prompt + "\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*PromptPart), PromptPart.Len());

    // Add the image file part
    FString ImageFilePartHeader = BeginBoundary + "Content-Disposition: form-data; name=\"image\"; filename=\"image.png\"\r\n";
    ImageFilePartHeader += "Content-Type: image/png\r\n\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*ImageFilePartHeader), ImageFilePartHeader.Len());

    // Append the raw image data to the FormData
    FormData.Append(ImageData);

    // Add a new line after the image data
    FormData.Append(NewLine, 2);

    // Add the mask image file part
    FString MaskFilePartHeader = BeginBoundary + "Content-Disposition: form-data; name=\"mask\"; filename=\"mask.png\"\r\n";
    MaskFilePartHeader += "Content-Type: image/png\r\n\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*MaskFilePartHeader), MaskFilePartHeader.Len());

    // Append the raw mask image data to the FormData
    FormData.Append(MaskImageData);

    // Add a new line after the mask image data
    FormData.Append(NewLine, 2);

    // Append the EndBoundary to the FormData
    FormData.Append((uint8*)TCHAR_TO_UTF8(*EndBoundary), EndBoundary.Len());

    // Log the form data as a string
    FString FormDataString;
    FormDataString.Append(ImageFilePartHeader);
    FormDataString.Append("<binary image data>\r\n");
    FormDataString.Append(MaskFilePartHeader);
    FormDataString.Append("<binary mask image data>\r\n");
    FormDataString.Append(NPart);
    FormDataString.Append(SizePart);
    FormDataString.Append(ResponseFormatPart);
    FormDataString.Append(PromptPart);
    FormDataString.Append(EndBoundary);
    UE_LOG(AIITKLog, Warning, TEXT("Form Data: %s"), *FormDataString);

    HttpRequest->SetContent(FormData);


    // Process images on request complete
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDallEAPI::ProcessDallEResponse);

    HttpRequest->ProcessRequest();
    CurrentDalleRequest = HttpRequest;
}

// ===== Processing =====

void UDallEAPI::ProcessDallEResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
    this->bRequestInProgress = false;

    if (!bSucceeded || !HttpResponse.IsValid())
    {
        UE_LOG(AIITKLog, Error, TEXT("Request failed or response invalid"));
        this->OnError.Broadcast(TEXT("Request failed"));
        return;
    }

    UE_LOG(AIITKLog, Log, TEXT("Received Response: %s"), *HttpResponse->GetContentAsString().LeftChop(128));

    if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
    {
        TArray<FString> ImageURLs;
        TArray<FString> RevisedPrompts;

        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
        if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
        {
            TArray<TSharedPtr<FJsonValue>> DataArray = JsonObject->GetArrayField(TEXT("data"));
            for (TSharedPtr<FJsonValue> Value : DataArray)
            {
                TSharedPtr<FJsonObject> DataObject = Value->AsObject();
                if (DataObject->HasField(TEXT("b64_json")))
                {
                    FString URL = DataObject->GetStringField(TEXT("b64_json"));
                    ImageURLs.Add(URL);
                }
                else if (DataObject->HasField(TEXT("url")))
                {
                    FString URL = DataObject->GetStringField(TEXT("url"));
                    ImageURLs.Add(URL);
                }

                if (DataObject->HasField(TEXT("revised_prompt")))
                {
                    FString RevisedPrompt = DataObject->GetStringField(TEXT("revised_prompt"));
                    RevisedPrompts.Add(RevisedPrompt);
                }
            }
        }

        this->DallEOnCompleted.Broadcast(ImageURLs, RevisedPrompts);
    }
    else
    {
        FString ErrorMessage = FString::Printf(TEXT("Request failed with response code: %d"), HttpResponse->GetResponseCode());
        this->OnError.Broadcast(ErrorMessage);
    }
}
