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


// ========== AIITK_NTK_OAuth2FunctionLibrary.h ==========

#pragma once


#include "Misc/EngineVersion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AIITK_NTK_OAuth2Manager.h"
#include "AIITK_NTK_OAuth2Config.h" // Include the configuration struct
#include "AIITK_NTK_OAuth2FunctionLibrary.generated.h"

/**
 * Blueprint library providing helper functions for OAuth2 flows.
 */
UCLASS()
class AIITK_NTK_NETWORKINGTOOLKIT_API UAIITK_NTK_OAuth2FunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Creates and starts an OAuth2 authentication flow using the provided configuration.
     * @param Config - A configuration struct containing all necessary OAuth parameters.
     * @return An instance of UAIITK_NTK_OAuth2Manager managing the flow.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|OAuth2")
    static UAIITK_NTK_OAuth2Manager* CreateAndStartOAuthFlow(const FAIITK_NTK_OAuth2Config& Config);

    /**
     * Helper to build a query string from a map of parameters.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|OAuth2")
    static FString BuildQueryString(const TMap<FString, FString>& Params);
};
