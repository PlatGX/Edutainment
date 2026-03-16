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


// ========== AIITK_NTK_OAuth2Manager.cpp ==========

#include "AIITK_NTK_OAuth2Manager.h"
#include "HttpModule.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "AIITK_NTK_OAuth2FunctionLibrary.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Async/Async.h"
#include "HAL/PlatformProcess.h"

UAIITK_NTK_OAuth2Manager::UAIITK_NTK_OAuth2Manager()
{
    // Keep this object alive until explicitly destroyed.
    AddToRoot();
    UE_LOG(LogTemp, Log, TEXT("AIITK_NTK_OAuth2Manager initialized with default values."));
}

void UAIITK_NTK_OAuth2Manager::BeginDestroy()
{
    UE_LOG(LogTemp, Log, TEXT("BeginDestroy called for UAIITK_NTK_OAuth2Manager."));
    if (HttpRouter.IsValid())
    {
        StopAuthFlow();
    }
    Super::BeginDestroy();
}

void UAIITK_NTK_OAuth2Manager::InitializeOAuthFlow(const FAIITK_NTK_OAuth2Config& Config)
{
    ClientId = Config.ClientId;
    ClientSecret = Config.ClientSecret;
    RedirectUri = Config.RedirectUri;
    Scopes = Config.Scopes;
    AuthUrl = Config.AuthUrl;
    TokenUrl = Config.TokenUrl;
    Port = Config.Port;
    FlowType = Config.FlowType;
    AuthorizationQueryParams = Config.AuthQueryParams;

    UE_LOG(LogTemp, Log, TEXT("InitializeOAuthFlow called. ClientSecret: %s"), *ClientSecret);
    DebugLogState();

    auto& HttpServerModule = FHttpServerModule::Get();
    HttpRouter = HttpServerModule.GetHttpRouter(Port);
    if (!HttpRouter.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize HTTP Router on port %d"), Port);
        OnAuthError.Broadcast(TEXT("HTTP Router not initialized."));
        return;
    }
    BindRoutes();
}

void UAIITK_NTK_OAuth2Manager::StartAuthFlow()
{
    if (!HttpRouter.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot start auth flow: HTTP Router invalid."));
        OnAuthError.Broadcast(TEXT("HTTP Router not initialized."));
        return;
    }

    FString QueryString;
    if (AuthorizationQueryParams.Num() > 0)
    {
        QueryString = UAIITK_NTK_OAuth2FunctionLibrary::BuildQueryString(AuthorizationQueryParams);
    }
    else
    {
        FString ScopeString = FString::Join(Scopes, TEXT(" "));
        if (FlowType == EAIITK_NTK_OAuth2Flow::AuthorizationCode)
        {
            QueryString = FString::Printf(TEXT("response_type=code&client_id=%s&redirect_uri=%s&scope=%s"),
                *ClientId,
                *FGenericPlatformHttp::UrlEncode(RedirectUri),
                *FGenericPlatformHttp::UrlEncode(ScopeString));
        }
        else if (FlowType == EAIITK_NTK_OAuth2Flow::Implicit)
        {
            QueryString = FString::Printf(TEXT("response_type=token&client_id=%s&redirect_uri=%s&scope=%s"),
                *ClientId,
                *FGenericPlatformHttp::UrlEncode(RedirectUri),
                *FGenericPlatformHttp::UrlEncode(ScopeString));
        }
        else if (FlowType == EAIITK_NTK_OAuth2Flow::ClientCredentials)
        {
            QueryString = FString::Printf(TEXT("client_id=%s&client_secret=%s&grant_type=client_credentials"),
                *ClientId,
                *ClientSecret);
        }
    }

    FString AuthorizationUrlComplete = FString::Printf(TEXT("%s?%s"), *AuthUrl, *QueryString);
    UE_LOG(LogTemp, Log, TEXT("Starting auth flow. Authorization URL: %s"), *AuthorizationUrlComplete);
    FPlatformProcess::LaunchURL(*AuthorizationUrlComplete, nullptr, nullptr);

    FHttpServerModule::Get().StartAllListeners();
    UE_LOG(LogTemp, Log, TEXT("HTTP listeners started."));
}

void UAIITK_NTK_OAuth2Manager::StopAuthFlow()
{
    if (bExchangeInProgress && PendingHttpRequest.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("Cancelling in-flight token request..."));
        PendingHttpRequest->OnProcessRequestComplete().Unbind();
        PendingHttpRequest->CancelRequest();
        PendingHttpRequest.Reset();
        bExchangeInProgress = false;
    }

    if (HttpRouter.IsValid() && AuthRouteHandle.IsValid())
    {
        HttpRouter->UnbindRoute(AuthRouteHandle);
        AuthRouteHandle.Reset();
    }

    if (HttpRouter.IsValid())
    {
        HttpRouter.Reset();
        FHttpServerModule::Get().StopAllListeners();
    }
    UE_LOG(LogTemp, Log, TEXT("Auth flow stopped. HTTP listeners shut down."));
}

void UAIITK_NTK_OAuth2Manager::DestroyManager()
{
    if (bIsDestroyed)
    {
        UE_LOG(LogTemp, Warning, TEXT("DestroyManager() called more than once! Ignoring."));
        return;
    }
    bIsDestroyed = true;
    StopAuthFlow();
    RemoveFromRoot();
    UE_LOG(LogTemp, Log, TEXT("AIITK_NTK_OAuth2Manager removed from root and can now be GC'd."));
}

void UAIITK_NTK_OAuth2Manager::DebugLogState()
{
    UE_LOG(LogTemp, Log, TEXT("AIITK_NTK_OAuth2Manager State:"));
    UE_LOG(LogTemp, Log, TEXT("ClientId: %s"), *ClientId);
    UE_LOG(LogTemp, Log, TEXT("ClientSecret: %s"), ClientSecret.IsEmpty() ? TEXT("[Not Set]") : TEXT("[Set]"));
    UE_LOG(LogTemp, Log, TEXT("RedirectUri: %s"), *RedirectUri);
    UE_LOG(LogTemp, Log, TEXT("AuthUrl: %s"), *AuthUrl);
    UE_LOG(LogTemp, Log, TEXT("TokenUrl: %s"), *TokenUrl);
    UE_LOG(LogTemp, Log, TEXT("Scopes: %s"), *FString::Join(Scopes, TEXT(", ")));
    UE_LOG(LogTemp, Log, TEXT("Port: %d"), Port);
    UE_LOG(LogTemp, Log, TEXT("FlowType: %d"), static_cast<int32>(FlowType));
}

void UAIITK_NTK_OAuth2Manager::BindRoutes()
{
    if (!HttpRouter.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot bind routes: HTTP Router invalid."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Binding route /auth on port %d"), Port);

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 4)
    // UE 5.3 and earlier: FHttpRequestHandler is a TFunction.
    FHttpRequestHandler AuthHandler = [this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) -> bool
    {
        TUniquePtr<FHttpServerResponse> Response = HandleRedirect(Request);
        if (Response)
        {
            OnComplete(MoveTemp(Response));
            return true;
        }
        return false;
    };
#else
    // UE 5.5 and later: FHttpRequestHandler is a TDelegate; use CreateLambda.
    FHttpRequestHandler AuthHandler = TDelegate<bool(const FHttpServerRequest&, const FHttpResultCallback&)>::CreateLambda(
        [this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) -> bool
    {
        TUniquePtr<FHttpServerResponse> Response = HandleRedirect(Request);
        if (Response)
        {
            OnComplete(MoveTemp(Response));
            return true;
        }
        return false;
    }
    );
#endif

    AuthRouteHandle = HttpRouter->BindRoute(
        FHttpPath(TEXT("/auth")),
        EHttpServerRequestVerbs::VERB_GET,
        AuthHandler
    );
}


TUniquePtr<FHttpServerResponse> UAIITK_NTK_OAuth2Manager::HandleRedirect(const FHttpServerRequest& Request)
{
    UE_LOG(LogTemp, Log, TEXT("Received redirect request. Checking for authorization code."));
    if (!Request.QueryParams.Contains(TEXT("code")))
    {
        UE_LOG(LogTemp, Error, TEXT("Authorization code not found in redirect."));
        OnAuthError.Broadcast(TEXT("Authorization code not found in redirect."));
        return FHttpServerResponse::Create(FString(TEXT("Authorization code not found.")), TEXT("text/plain"));
    }

    FString AuthorizationCode = *Request.QueryParams.Find(TEXT("code"));
    UE_LOG(LogTemp, Log, TEXT("Received authorization code: %s"), *AuthorizationCode);

    AsyncTask(ENamedThreads::GameThread, [this, AuthorizationCode]()
    {
        ExchangeAuthorizationCodeForToken(AuthorizationCode);
    });

    FString PageResponse = TEXT("<html><body><h1>Authorization Complete</h1><p>You can close this window.</p></body></html>");
    return FHttpServerResponse::Create(FString(PageResponse), TEXT("text/html"));
}

void UAIITK_NTK_OAuth2Manager::ExchangeAuthorizationCodeForToken(const FString& AuthorizationCode)
{
    if (TokenUrl.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Token URL is not set or invalid."));
        OnAuthError.Broadcast(TEXT("Token URL is invalid."));
        return;
    }
    if (FlowType == EAIITK_NTK_OAuth2Flow::ClientCredentials)
    {
        UE_LOG(LogTemp, Error, TEXT("Client Credentials flow does not use an authorization code."));
        OnAuthError.Broadcast(TEXT("Invalid flow for authorization code exchange."));
        return;
    }
    if (ClientSecret.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Client secret not provided. The token exchange may fail."));
    }

    bExchangeInProgress = true;
    PendingHttpRequest = FHttpModule::Get().CreateRequest();
    PendingHttpRequest->SetURL(TokenUrl);
    PendingHttpRequest->SetVerb(TEXT("POST"));
    PendingHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

    // Send the raw redirect URI (without extra URL encoding) to avoid mismatches.
    FString PostData = FString::Printf(
        TEXT("code=%s&client_id=%s&client_secret=%s&redirect_uri=%s&grant_type=authorization_code"),
        *AuthorizationCode,
        *ClientId,
        *ClientSecret,
        *RedirectUri);
    UE_LOG(LogTemp, Log, TEXT("Exchanging authorization code for token. PostData: %s"), *PostData);
    PendingHttpRequest->SetContentAsString(PostData);
    PendingHttpRequest->OnProcessRequestComplete().BindUObject(this, &UAIITK_NTK_OAuth2Manager::OnTokenRequestComplete);
    PendingHttpRequest->ProcessRequest();
}

void UAIITK_NTK_OAuth2Manager::OnTokenRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("Token exchange request completed. Success: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));
    bExchangeInProgress = false;

    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("HTTP request failed or response invalid."));
        OnAuthError.Broadcast(TEXT("HTTP request failed."));
        PendingHttpRequest.Reset();
        return;
    }

    FString ResponseContent = Response->GetContentAsString();
    UE_LOG(LogTemp, Log, TEXT("Response Code: %d, Content: %s"), Response->GetResponseCode(), *ResponseContent);

    if (Response->GetResponseCode() == 200)
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            FString AccessToken;
            if (JsonObject->TryGetStringField(TEXT("access_token"), AccessToken))
            {
                // For Twitch, perform an additional validation step.
                if (TokenUrl.Contains(TEXT("twitch.tv")))
                {
                    ValidateTwitchToken(AccessToken);
                }
                else
                {
                    HandleSuccessfulToken(AccessToken);
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Missing 'access_token' field in token response."));
                OnAuthError.Broadcast(TEXT("Missing 'access_token' in token response."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to parse token response JSON."));
            OnAuthError.Broadcast(TEXT("Failed to parse token response."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Token exchange failed. HTTP Code: %d"), Response->GetResponseCode());
        OnAuthError.Broadcast(TEXT("Failed to exchange authorization code for token."));
    }
    PendingHttpRequest.Reset();
}

void UAIITK_NTK_OAuth2Manager::ValidateTwitchToken(const FString& AccessToken)
{
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> ValidateRequest = FHttpModule::Get().CreateRequest();
    ValidateRequest->SetURL(TEXT("https://id.twitch.tv/oauth2/validate"));
    ValidateRequest->SetVerb(TEXT("GET"));
    ValidateRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AccessToken));

    ValidateRequest->OnProcessRequestComplete().BindLambda([this, AccessToken](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
        {
            UE_LOG(LogTemp, Error, TEXT("Token validation failed."));
            OnAuthError.Broadcast(TEXT("Token validation failed."));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Token validation succeeded."));
            HandleSuccessfulToken(AccessToken);
        }
    });
    ValidateRequest->ProcessRequest();
}

void UAIITK_NTK_OAuth2Manager::HandleSuccessfulToken(const FString& AccessToken)
{
    UE_LOG(LogTemp, Log, TEXT("Access token retrieved: %s"), *AccessToken);
    AsyncTask(ENamedThreads::GameThread, [this, AccessToken]()
    {
        OnTokenReceived.Broadcast(AccessToken);
        StopAuthFlow();
        DestroyManager();
    });
}
