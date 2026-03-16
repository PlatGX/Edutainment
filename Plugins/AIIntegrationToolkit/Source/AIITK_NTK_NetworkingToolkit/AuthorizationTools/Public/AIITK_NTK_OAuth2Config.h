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


// ========== AIITK_NTK_OAuth2Config.h ==========

#pragma once

#include "Misc/EngineVersion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "CoreMinimal.h"
#include "AIITK_NTK_OAuth2Config.generated.h"

UENUM(BlueprintType)
enum class EAIITK_NTK_OAuth2Flow : uint8 {
    AuthorizationCode UMETA(DisplayName = "Authorization Code Grant"),
    ClientCredentials UMETA(DisplayName = "Client Credentials Grant"),
    Implicit          UMETA(DisplayName = "Implicit Grant")
};

USTRUCT(BlueprintType)
struct FAIITK_NTK_OAuth2Config
{
    GENERATED_BODY()

    // The client ID for your OAuth application.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "*AIITK|NetworkingToolkit|OAuth2")
    FString ClientId;

    // The client secret for your OAuth application.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "*AIITK|NetworkingToolkit|OAuth2")
    FString ClientSecret;

    // The URI to which the user will be redirected after authentication.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "*AIITK|NetworkingToolkit|OAuth2")
    FString RedirectUri;

    // A list of OAuth scopes.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "*AIITK|NetworkingToolkit|OAuth2")
    TArray<FString> Scopes;

    // The base URL for the OAuth authorization endpoint.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "*AIITK|NetworkingToolkit|OAuth2")
    FString AuthUrl;

    // The URL for the token endpoint.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "*AIITK|NetworkingToolkit|OAuth2")
    FString TokenUrl;

    // The local port to start the HTTP listener on.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "*AIITK|NetworkingToolkit|OAuth2")
    int32 Port = 8080;

    // Optional: Custom query parameters for the authorization URL.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "*AIITK|NetworkingToolkit|OAuth2")
    TMap<FString, FString> AuthQueryParams;

    // Specify which OAuth2 flow to use.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "*AIITK|NetworkingToolkit|OAuth2")
    EAIITK_NTK_OAuth2Flow FlowType = EAIITK_NTK_OAuth2Flow::AuthorizationCode;
};