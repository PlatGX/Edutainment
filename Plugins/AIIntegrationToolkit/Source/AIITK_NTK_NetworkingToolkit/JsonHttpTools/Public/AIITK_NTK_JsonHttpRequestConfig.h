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


// ========== AIITK_NTK_JsonHttpRequestConfig.h ==========

#pragma once


#include "Misc/EngineVersion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "CoreMinimal.h"
#include "AIITK_NTK_JsonHttpRequestConfig.generated.h"

/**
 * Struct for configuring HTTP requests.
 */
USTRUCT(BlueprintType)
struct AIITK_NTK_NETWORKINGTOOLKIT_API FAIITK_NTK_JsonHttpRequestConfig
{
    GENERATED_BODY()

public:
    /** The base URL for the HTTP request. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|NetworkingToolkit|JsonHttp|Config")
    FString URL;

    /** The HTTP method (e.g., GET, POST, PUT, DELETE). */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|NetworkingToolkit|JsonHttp|Config")
    FString Method;

    /** The query parameters to append to the URL. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|NetworkingToolkit|JsonHttp|Config")
    TMap<FString, FString> QueryParameters;

    /** The headers to include in the request. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|NetworkingToolkit|JsonHttp|Config")
    TMap<FString, FString> Headers;

    /** The body content for the request (if applicable). */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|NetworkingToolkit|JsonHttp|Config")
    FString Body;

    /** Constructor to initialize defaults. */
    FAIITK_NTK_JsonHttpRequestConfig()
    {
        Method = TEXT("GET");
    }
};
