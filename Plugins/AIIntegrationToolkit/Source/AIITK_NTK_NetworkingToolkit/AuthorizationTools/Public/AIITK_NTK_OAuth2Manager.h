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


 // ========== AIITK_NTK_OAuth2Manager.h ==========

#pragma once

#include "Misc/EngineVersion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "CoreMinimal.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"

#include "AIITK_NTK_OAuth2Config.h"
#include "AIITK_NTK_OAuth2Manager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOAuthTokenReceived, const FString&, AccessToken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOAuthError, const FString&, ErrorMessage);

/**
 * UAIITK_NTK_OAuth2Manager handles the OAuth 2.0 authentication flow.
 * This class is fully configurable via FAIITK_NTK_OAuth2Config and supports customization
 * for different providers (e.g., Twitch, YouTube).
 */
UCLASS(Blueprintable, BlueprintType)
class AIITK_NTK_NETWORKINGTOOLKIT_API UAIITK_NTK_OAuth2Manager : public UObject
{
    GENERATED_BODY()

public:
    UAIITK_NTK_OAuth2Manager();

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|NetworkingToolkit|OAuth2")
    FOnOAuthTokenReceived OnTokenReceived;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|NetworkingToolkit|OAuth2")
    FOnOAuthError OnAuthError;

    /**
     * Initializes the OAuth flow using the provided configuration.
     * @param Config - Struct containing client ID, client secret, redirect URI, token and auth URLs, scopes, and flow settings.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|OAuth2")
    void InitializeOAuthFlow(const FAIITK_NTK_OAuth2Config& Config);

    /**
     * Starts the OAuth authentication flow by launching the browser to the authorization URL
     * and starting the local HTTP listener for the redirect.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|OAuth2")
    void StartAuthFlow();

    /**
     * Stops the ongoing OAuth authentication flow and shuts down any active HTTP listeners.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|OAuth2")
    void StopAuthFlow();

    /**
     * Destroys this OAuth2 manager instance, cleaning up internal resources and handlers.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|OAuth2")
    void DestroyManager();

protected:
    virtual void BeginDestroy() override;

private:
    // Configuration parameters
    FString ClientId;
    FString ClientSecret;
    FString RedirectUri;
    FString AuthUrl;
    FString TokenUrl;
    TArray<FString> Scopes;
    int32 Port;
    EAIITK_NTK_OAuth2Flow FlowType;

    // Additional query parameters for the authorization URL.
    TMap<FString, FString> AuthorizationQueryParams;

    // HTTP router and route handle for the local server.
    TSharedPtr<IHttpRouter> HttpRouter;
    FHttpRouteHandle AuthRouteHandle;

    // Pending HTTP request for token exchange.
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> PendingHttpRequest;
    bool bExchangeInProgress = false;

    // Prevent double destruction.
    bool bIsDestroyed = false;

    // Internal functions.
    void DebugLogState();
    void BindRoutes();
    TUniquePtr<FHttpServerResponse> HandleRedirect(const FHttpServerRequest& Request);
    void ExchangeAuthorizationCodeForToken(const FString& AuthorizationCode);
    void OnTokenRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void ValidateTwitchToken(const FString& AccessToken);
    void HandleSuccessfulToken(const FString& AccessToken);
};
