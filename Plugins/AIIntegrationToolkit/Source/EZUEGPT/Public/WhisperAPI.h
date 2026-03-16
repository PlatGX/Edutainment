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



// =================== WhisperAPI.h ===================

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GPTChatAPIStructs.h"
#include "OnlineSubsystemUtils.h"
#include "Voice.h"
#include "Sound/SoundWaveProcedural.h"
#include "UObject/NoExportTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Http.h"
#include "HttpManager.h"
#include "AudioCaptureComponent.h"
#include "WhisperAPI.generated.h"

//Would be cool to add a function that checks for silence and "auto sends" the current audio clip

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWhisperAPIResponse, const FString&, Transcription);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWhisperAPIFailed, const FString&, ErrorMessage);


UCLASS(BlueprintType)
class EZUEGPT_API UWhisperAPI : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Whisper")
        static UWhisperAPI* RequestWhisperTranscript(const FWhisperAPIParams& Params);

    UPROPERTY(BlueprintAssignable)
        FOnWhisperAPIResponse OnWhisperAPIResponse;

    UPROPERTY(BlueprintAssignable)
        FOnWhisperAPIFailed OnWhisperAPIFailed;

    FString ResponseFormatToString(EWhisperResponseFormat Format)
    {
        switch (Format)
        {
        case EWhisperResponseFormat::json:
            return "json";
        case EWhisperResponseFormat::text:
            return "text";
        case EWhisperResponseFormat::srt:
            return "srt";
        case EWhisperResponseFormat::verbose_json:
            return "verbose_json";
        case EWhisperResponseFormat::vtt:
            return "vtt";
        default:
            return "json";
        }
    }


protected:

    void SendRequest(const FWhisperAPIParams& Params);

    UPROPERTY()
        bool bRequestInProgress = false;
};