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


// ========== AIITK_NTK_WetSocketManager.h ==========

#pragma once

#include "Misc/EngineVersion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IWebSocket.h"
#include "AIITK_NTK_JsonHttpRequestConfig.h"
#include "AIITK_NTK_WebSocketDirectConfig.h"

#include "AIITK_NTK_WetSocketManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(NTKWetSockManLog, Log, All);

// Delegate for handling received messages.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNTKWetSocketMessageReceived, const FString&, Message);

// Delegates for WebSocket connection events.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNTKWetSocketConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNTKWetSocketConnectionError);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNTKWetSocketDisconnected);

/**
 * UAIITK_NTK_WetSocketManager: Handles WebSocket connections.
 * It can either perform an HTTP handshake (using FAIITK_NTK_JsonHttpRequestConfig plus overrides)
 * or directly open a WebSocket connection using FAIITK_NTK_WebSocketDirectConfig.
 */
UCLASS(BlueprintType)
class AIITK_NTK_NETWORKINGTOOLKIT_API UAIITK_NTK_WetSocketManager : public UObject
{
    GENERATED_BODY()

public:
    UAIITK_NTK_WetSocketManager();
    virtual ~UAIITK_NTK_WetSocketManager() {}

    // Override BeginDestroy to add cleanup logic.
    virtual void BeginDestroy() override;

    /**
     * Initiates the connection by using both an HTTP handshake configuration (FAIITK_NTK_JsonHttpRequestConfig)
     * and additional WebSocket-specific settings (FAIITK_NTK_WebSocketDirectConfig). This method is useful
     * when you want to perform the handshake, but also supply or override WebSocket-specific connection parameters.
     * @param HttpConfig - The HTTP handshake configuration.
     * @param WSDirectConfig - The WebSocket-specific configuration.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|WetSocket")
    void Connect(const FAIITK_NTK_JsonHttpRequestConfig& HttpConfig, const FAIITK_NTK_WebSocketDirectConfig& WSDirectConfig);

    /**
     * Initiates a direct WebSocket connection without any preliminary HTTP handshake.
     * Uses the provided FAIITK_NTK_WebSocketDirectConfig.
     * @param DirectConfig - The connection configuration for a direct WebSocket connection.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|WetSocket")
    void ConnectDirect(const FAIITK_NTK_WebSocketDirectConfig& DirectConfig);

    /**
     * Sends a message to the connected WebSocket server.
     * @param Message - The message to send.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|WetSocket")
    void SendMessage(const FString& Message);

    /**
     * Closes the WebSocket connection.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|WetSocket")
    void CloseConnection();

    // Events triggered by the manager.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|NetworkingToolkit|WetSocket|Events")
    FOnNTKWetSocketMessageReceived OnMessageReceived;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|NetworkingToolkit|WetSocket|Events")
    FOnNTKWetSocketConnected OnConnected;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|NetworkingToolkit|WetSocket|Events")
    FOnNTKWetSocketConnectionError OnConnectionError;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|NetworkingToolkit|WetSocket|Events")
    FOnNTKWetSocketDisconnected OnDisconnected;

protected:
    /**
     * Helper method to append query parameters to a URL.
     */
    FString AppendParametersToUrl(const FString& Url, const TMap<FString, FString>& QueryParams);

    /**
     * Callback when the WebSocket connection is established.
     * Marked virtual so subclasses can override.
     */
    virtual void OnWebSocketConnected();

    /**
     * Callback when a message is received from the WebSocket server.
     * Marked virtual so subclasses can extend its behavior.
     */
    virtual void OnWebSocketMessageReceived(const FString& Message);

private:
    /** The underlying WebSocket instance. */
    TSharedPtr<IWebSocket> WebSocket;

    /** Stored HTTP configuration used during the handshake. */
    FAIITK_NTK_JsonHttpRequestConfig PendingRequestConfig;

    /** Stored WebSocket-specific configuration for direct connection or overrides. */
    FAIITK_NTK_WebSocketDirectConfig PendingDirectConfig;

    /** Sets up WebSocket event callbacks. */
    void InitializeWebSocketEvents();

    UFUNCTION()
    void OnWebSocketConnectionError(const FString& Error);

    void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);

    // HTTP handshake delegate handlers:
    UFUNCTION()
    void HandleHttpRequestComplete(const FString& ResponseContent);

    UFUNCTION()
    void HandleHttpRequestError(FString ErrorString);
};
