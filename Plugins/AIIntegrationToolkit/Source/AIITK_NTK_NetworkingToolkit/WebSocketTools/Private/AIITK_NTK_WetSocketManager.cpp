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


// ========== AIITK_NTK_WetSocketManager.cpp ==========

#include "AIITK_NTK_WetSocketManager.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "AIITK_NTK_JsonHttpFunctionLibrary.h"  // For SendHttpRequestWithConfig

DEFINE_LOG_CATEGORY(NTKWetSockManLog);

UAIITK_NTK_WetSocketManager::UAIITK_NTK_WetSocketManager()
{
    WebSocket = nullptr;
}

// Doesn't work in PIE, call CloseConnection from EndPlay in parent Actor
void UAIITK_NTK_WetSocketManager::BeginDestroy()
{
    // Close any active WebSocket connection when the object is about to be destroyed.
    if (WebSocket.IsValid())
    {
        CloseConnection();
    }

    RemoveFromRoot();  // Allow garbage collection.

    // Call the base class implementation.
    Super::BeginDestroy();
}


FString UAIITK_NTK_WetSocketManager::AppendParametersToUrl(const FString& Url, const TMap<FString, FString>& QueryParams)
{
    FString ModifiedUrl = Url;
    bool bHasQuery = Url.Contains(TEXT("?"));

    if (QueryParams.Num() > 0)
    {
        ModifiedUrl += bHasQuery ? "&" : "?";

        TArray<FString> QueryStringArray;
        for (const auto& Pair : QueryParams)
        {
            QueryStringArray.Add(FString::Printf(TEXT("%s=%s"), *Pair.Key, *Pair.Value));
        }
        ModifiedUrl += FString::Join(QueryStringArray, TEXT("&"));
    }
    return ModifiedUrl;
}

//////////////////////////////////////////////////////////////////////////
// Connection Methods

// Connect with both HTTP handshake config and WebSocket-specific overrides.
void UAIITK_NTK_WetSocketManager::Connect(const FAIITK_NTK_JsonHttpRequestConfig& HttpConfig, const FAIITK_NTK_WebSocketDirectConfig& WSDirectConfig)
{
    // Store both configurations.
    PendingRequestConfig = HttpConfig;
    PendingDirectConfig = WSDirectConfig;

    // For this combined connection, we initiate the HTTP handshake.
    // Build the handshake URL from the HTTP configuration.
    FString HandshakeUrl = AppendParametersToUrl(HttpConfig.URL, HttpConfig.QueryParameters);

    // Prepare a handshake configuration: force GET and clear the body.
    FAIITK_NTK_JsonHttpRequestConfig HandshakeConfig = HttpConfig;
    HandshakeConfig.URL = HandshakeUrl;
    HandshakeConfig.Method = TEXT("GET");
    HandshakeConfig.Body = TEXT("");

    // Create dynamic delegates for error and complete callbacks.
    FOnJsonHttpRequestError ErrorDelegate;
    ErrorDelegate.BindDynamic(this, &UAIITK_NTK_WetSocketManager::HandleHttpRequestError);

    FOnJsonHttpRequestComplete CompleteDelegate;
    CompleteDelegate.BindDynamic(this, &UAIITK_NTK_WetSocketManager::HandleHttpRequestComplete);

    // Initiate the HTTP handshake using the JSON HTTP function library.
    UAIITK_NTK_JsonHttpTools* HttpTool = UAIITK_NTK_JsonHttpFunctionLibrary::SendHttpRequestWithConfig(
        HandshakeConfig,
        ErrorDelegate,
        FOnJsonHttpRequestProgress(),         // No progress handler.
        FOnJsonHttpRequestHeaderReceived(),     // No header handler.
        CompleteDelegate
    );

    if (!HttpTool)
    {
        UE_LOG(NTKWetSockManLog, Error, TEXT("WetSocket: Failed to initiate HTTP handshake."));
        if (OnConnectionError.IsBound())
        {
            OnConnectionError.Broadcast();
        }
    }
    else
    {
        UE_LOG(NTKWetSockManLog, Log, TEXT("WetSocket: HTTP handshake initiated for %s"), *HandshakeConfig.URL);
    }
}

// Direct connection using FAIITK_NTK_WebSocketDirectConfig only.
void UAIITK_NTK_WetSocketManager::ConnectDirect(const FAIITK_NTK_WebSocketDirectConfig& DirectConfig)
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        UE_LOG(NTKWetSockManLog, Warning, TEXT("WetSocket: Already connected."));
        if (OnConnectionError.IsBound())
        {
            OnConnectionError.Broadcast();
        }
        return;
    }

    // Build final URL using the direct configuration.
    FString FinalUrl = AppendParametersToUrl(DirectConfig.URL, DirectConfig.QueryParameters);

    // Create the WebSocket instance with the provided headers.
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(FinalUrl, TArray<FString>(), DirectConfig.Headers);

    if (!WebSocket.IsValid())
    {
        UE_LOG(NTKWetSockManLog, Error, TEXT("WetSocket: Failed to create WebSocket for direct connection to %s"), *FinalUrl);
        if (OnConnectionError.IsBound())
        {
            OnConnectionError.Broadcast();
        }
        return;
    }

    InitializeWebSocketEvents();
    AddToRoot();  // Prevent garbage collection while connected.

    WebSocket->Connect();
    UE_LOG(NTKWetSockManLog, Log, TEXT("WetSocket: Directly connecting to %s"), *FinalUrl);
}

//////////////////////////////////////////////////////////////////////////
// HTTP Handshake Delegate Handlers

void UAIITK_NTK_WetSocketManager::HandleHttpRequestComplete(const FString& ResponseContent)
{
    UE_LOG(NTKWetSockManLog, Log, TEXT("WetSocket: HTTP handshake completed successfully. Response: %s"), *ResponseContent);

    // Determine final URL and headers.
    FString FinalUrl;
    TMap<FString, FString> FinalHeaders = PendingRequestConfig.Headers;
    TMap<FString, FString> FinalQueryParams = PendingRequestConfig.QueryParameters;

    // If a WebSocket-specific override was provided, use its URL and merge its headers.
    if (!PendingDirectConfig.URL.IsEmpty())
    {
        FinalUrl = AppendParametersToUrl(PendingDirectConfig.URL, PendingDirectConfig.QueryParameters);
        for (const TPair<FString, FString>& Pair : PendingDirectConfig.Headers)
        {
            FinalHeaders.Add(Pair.Key, Pair.Value);
        }
    }
    else
    {
        FinalUrl = AppendParametersToUrl(PendingRequestConfig.URL, PendingRequestConfig.QueryParameters);
    }

    // Create the WebSocket instance.
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(FinalUrl, TArray<FString>(), FinalHeaders);
    if (!WebSocket.IsValid())
    {
        UE_LOG(NTKWetSockManLog, Error, TEXT("WetSocket: Failed to create WebSocket after HTTP handshake."));
        if (OnConnectionError.IsBound())
        {
            OnConnectionError.Broadcast();
        }
        return;
    }

    InitializeWebSocketEvents();
    AddToRoot();  // Prevent garbage collection while connected.

    WebSocket->Connect();
    UE_LOG(NTKWetSockManLog, Log, TEXT("WetSocket: Attempting to connect to %s after HTTP handshake."), *FinalUrl);
}

void UAIITK_NTK_WetSocketManager::HandleHttpRequestError(FString ErrorString)
{
    UE_LOG(NTKWetSockManLog, Error, TEXT("WetSocket: HTTP handshake error: %s"), *ErrorString);
    if (OnConnectionError.IsBound())
    {
        OnConnectionError.Broadcast();
    }
}

//////////////////////////////////////////////////////////////////////////
// WebSocket Event Setup

void UAIITK_NTK_WetSocketManager::InitializeWebSocketEvents()
{
    if (WebSocket.IsValid())
    {
        WebSocket->OnConnected().AddUObject(this, &UAIITK_NTK_WetSocketManager::OnWebSocketConnected);
        WebSocket->OnMessage().AddUObject(this, &UAIITK_NTK_WetSocketManager::OnWebSocketMessageReceived);
        WebSocket->OnConnectionError().AddUObject(this, &UAIITK_NTK_WetSocketManager::OnWebSocketConnectionError);
        WebSocket->OnClosed().AddUObject(this, &UAIITK_NTK_WetSocketManager::OnWebSocketClosed);
    }
}

//////////////////////////////////////////////////////////////////////////
// WebSocket Event Callbacks

void UAIITK_NTK_WetSocketManager::OnWebSocketConnected()
{
    UE_LOG(NTKWetSockManLog, Log, TEXT("WetSocket: WebSocket connected."));
    if (OnConnected.IsBound())
    {
        OnConnected.Broadcast();
    }
}

void UAIITK_NTK_WetSocketManager::OnWebSocketMessageReceived(const FString& Message)
{
    if (IsValid(this) && OnMessageReceived.IsBound())
    {
        UE_LOG(NTKWetSockManLog, Log, TEXT("WetSocket: Received message: %s"), *Message);
        OnMessageReceived.Broadcast(Message);
    }
}

void UAIITK_NTK_WetSocketManager::OnWebSocketConnectionError(const FString& Error)
{
    UE_LOG(NTKWetSockManLog, Error, TEXT("WetSocket: Connection error: %s"), *Error);
    if (OnConnectionError.IsBound())
    {
        OnConnectionError.Broadcast();
    }
    RemoveFromRoot();  // Allow garbage collection.
}

void UAIITK_NTK_WetSocketManager::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(NTKWetSockManLog, Log, TEXT("WetSocket: WebSocket closed. Status: %d, Reason: %s, Clean: %s"),
        StatusCode, *Reason, bWasClean ? TEXT("true") : TEXT("false"));
    if (OnDisconnected.IsBound())
    {
        OnDisconnected.Broadcast();
    }
    WebSocket = nullptr;
    RemoveFromRoot();  // Allow garbage collection.
}

//////////////////////////////////////////////////////////////////////////
// Message Sending and Connection Closing

void UAIITK_NTK_WetSocketManager::SendMessage(const FString& Message)
{
    if (!WebSocket.IsValid() || !WebSocket->IsConnected())
    {
        UE_LOG(NTKWetSockManLog, Warning, TEXT("WetSocket: WebSocket is not connected, cannot send message."));
        return;
    }
    WebSocket->Send(Message);
    UE_LOG(NTKWetSockManLog, Log, TEXT("WetSocket: Sent message"));
}

void UAIITK_NTK_WetSocketManager::CloseConnection()
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        WebSocket->Close();
        UE_LOG(NTKWetSockManLog, Log, TEXT("WetSocket: WebSocket connection closed."));
    }
    else
    {
        UE_LOG(NTKWetSockManLog, Log, TEXT("WetSocket: WebSocket is not connected."));
    }
    WebSocket = nullptr;
    RemoveFromRoot();  // Allow garbage collection.
}
