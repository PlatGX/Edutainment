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


// ========== AIITK_NTK_WebSocketDirectConfig.h ==========

#pragma once


#include "Misc/EngineVersion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "CoreMinimal.h"
#include "AIITK_NTK_WebSocketDirectConfig.generated.h"

/**
 * FAIITK_NTK_WebSocketDirectConfig: Configuration for direct WebSocket connections.
 */
USTRUCT(BlueprintType)
struct AIITK_NTK_NETWORKINGTOOLKIT_API FAIITK_NTK_WebSocketDirectConfig
{
    GENERATED_BODY()

public:
    // The URL for the direct WebSocket connection.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|NetworkingToolkit|WebSocket")
    FString URL;

    // Query parameters that will be appended to the URL.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|NetworkingToolkit|WebSocket")
    TMap<FString, FString> QueryParameters;

    // Headers to be sent with the WebSocket connection.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|NetworkingToolkit|WebSocket")
    TMap<FString, FString> Headers;
};