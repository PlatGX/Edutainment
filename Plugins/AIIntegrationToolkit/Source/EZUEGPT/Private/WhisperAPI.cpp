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


// =================== WhisperAPI.cpp ===================

#include "WhisperAPI.h"
#include "TimerManager.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Sound/AudioSettings.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "Misc/Guid.h"
#include "Misc/Base64.h"
#include "AIITKDeveloperSettings.h"
#include "OpenAIFunctionLibrary.h"


UWhisperAPI* UWhisperAPI::RequestWhisperTranscript(const FWhisperAPIParams& Params)
{
    UWhisperAPI* WhisperAPIInstance = NewObject<UWhisperAPI>();
    if (WhisperAPIInstance)
    {
        WhisperAPIInstance->SendRequest(Params);
    }
    return WhisperAPIInstance;
}

void UWhisperAPI::SendRequest(const FWhisperAPIParams& Params)
{
    if (bRequestInProgress)
    {
        OnWhisperAPIFailed.Broadcast(TEXT("Request already in progress."));
        return;
    }

    bRequestInProgress = true;

    TArray<uint8> AudioData;
    if (!FFileHelper::LoadFileToArray(AudioData, *Params.FilePath))
    {
        OnWhisperAPIFailed.Broadcast(TEXT("Failed to load file."));
        return;
    }

    // Fetch decrypted API key
    FString DecryptedAPIKey = UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType::OpenAI);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    FString EndpointURL;
    switch (Params.Endpoint)
    {
    case EWhisperEndpoint::Transcription:
        EndpointURL = "https://api.openai.com/v1/audio/transcriptions";
        break;
    case EWhisperEndpoint::Translation:
        EndpointURL = "https://api.openai.com/v1/audio/translations";
        break;
    default:
        OnWhisperAPIFailed.Broadcast(TEXT("Invalid endpoint."));
        return;
    }
    Request->SetURL(EndpointURL);
    Request->SetVerb("POST");
    Request->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *DecryptedAPIKey));

    // Your multipart form construction logic
    FString Boundary = "-----------------------------" + FGuid::NewGuid().ToString();
    Request->SetHeader("Content-Type", "multipart/form-data; boundary=" + Boundary);

    FString BeginBoundary = "--" + Boundary + "\r\n";
    FString EndBoundary = "\r\n--" + Boundary + "--\r\n";

    TArray<uint8> FormData;

    // Add the model part
    FString ModelPartHeader = "Content-Disposition: form-data; name=\"model\"\r\n\r\n";
    FString ModelPart = BeginBoundary + ModelPartHeader + Params.Model + "\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*ModelPart), ModelPart.Len());

    // Add the optional prompt part
    if (!Params.Prompt.IsEmpty())
    {
        FString PromptPartHeader = "Content-Disposition: form-data; name=\"prompt\"\r\n\r\n";
        FString PromptPart = BeginBoundary + PromptPartHeader + Params.Prompt + "\r\n";
        FormData.Append((uint8*)TCHAR_TO_UTF8(*PromptPart), PromptPart.Len());
    }

    // Add the optional temperature part
    if (Params.Temperature != 0.0f)
    {
        FString TemperaturePartHeader = "Content-Disposition: form-data; name=\"temperature\"\r\n\r\n";
        FString TemperaturePart = BeginBoundary + TemperaturePartHeader + FString::Printf(TEXT("%.2f"), Params.Temperature) + "\r\n";
        FormData.Append((uint8*)TCHAR_TO_UTF8(*TemperaturePart), TemperaturePart.Len());
    }

    // Add the optional response_format part
    FString FormatPartHeader = "Content-Disposition: form-data; name=\"response_format\"\r\n\r\n";
    FString FormatEnumAsString = ResponseFormatToString(Params.ResponseFormat);
    FString FormatPart = BeginBoundary + FormatPartHeader + FormatEnumAsString + "\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*FormatPart), FormatPart.Len());


    // Add the audio file part
    FString FilePartHeader = BeginBoundary + "Content-Disposition: form-data; name=\"file\"; filename=\"audio.wav\"\r\n";
    FilePartHeader += "Content-Type: audio/wav\r\n\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*FilePartHeader), FilePartHeader.Len());

    // Append the raw audio data (not base64-encoded) to the FormData
    FormData.Append(AudioData);

    // Add a new line after the audio data
    FString NewLine = "\r\n";
    FormData.Append((uint8*)TCHAR_TO_UTF8(*NewLine), NewLine.Len());

    // Append the EndBoundary to the FormData
    FormData.Append((uint8*)TCHAR_TO_UTF8(*EndBoundary), EndBoundary.Len());

    Request->SetContent(FormData);

    // Bind callback
    Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            bRequestInProgress = false;

            if (!bWasSuccessful || !Response.IsValid())
            {
                OnWhisperAPIFailed.Broadcast(TEXT("Request failed."));
                return;
            }

            if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
            {
                // Parse JSON and broadcast through delegate
                FString ResponseString = Response->GetContentAsString();
                UE_LOG(AIITKLog, Warning, TEXT("%s"), *ResponseString);
                TSharedPtr<FJsonValue> JsonParsed;
                TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(ResponseString);

                if (FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed->Type == EJson::Object)
                {
                    TSharedPtr<FJsonObject> JsonObject = JsonParsed->AsObject();

                    FString text;
                    if (JsonObject->TryGetStringField(TEXT("text"), text))
                    {
                        OnWhisperAPIResponse.Broadcast(text);
                    }
                    else
                    {
                        OnWhisperAPIFailed.Broadcast(TEXT("Couldn't get text field from json, try a different response format."));
                    }
                }
                else
                {
                    OnWhisperAPIResponse.Broadcast(ResponseString);
                }
            }
            else
            {
                OnWhisperAPIFailed.Broadcast(TEXT("Bad response code."));
            }
        });

    Request->ProcessRequest();
}

