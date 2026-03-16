#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IWebSocket.h"
#include "WetSocketManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(WetSockManLog, Log, All);


// Delegate for handling received messages
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWetSocketMessageReceived, const FString&, Message);

// Delegate for WebSocket connection events
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWetSocketConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWetSocketConnectionError);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWetSocketDisconnected);

USTRUCT(BlueprintType)
struct FWebSocketConnectionParams
{
    GENERATED_BODY()

    // The URL of the WebSocket server
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|WetSocket")
    FString Url;

    // The array of path extensions to be appended to the URL
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|WetSocket")
    TArray<FString> PathExts;

    // Additional query parameters to be appended to the URL
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|WetSocket")
    TMap<FString, FString> QueryParams;

    // Additional headers to be included in the connection request
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|WetSocket")
    TMap<FString, FString> HeaderParams;

    // The API key to be included in the connection request
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|WetSocket")
    FString ApiKey;

    // The API key to be included in the connection request
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "*AIITK|WetSocket")
    FString Xi_ApiKey;
};

/**
 * UWetSocketManager: Class for handling WebSocket connections using Unreal Engine's WebSocket interface.
 */
UCLASS(BlueprintType)
class EZUEGPT_API UWetSocketManager : public UObject
{
    GENERATED_BODY()

public:
    UWetSocketManager();

    /**
     * Connects to a WebSocket server using the specified URL, path extensions, query parameters, header parameters, and API key.
     * The result of the connection attempt is notified via the WebSocket events: OnConnected, OnConnectionError.
     *
     * @param Url The URL of the WebSocket server.
     * @param PathExts The array of path extensions to be appended to the URL.
     * @param QueryParams Additional query parameters to be appended to the URL.
     * @param HeaderParams Additional headers to be included in the connection request.
     * @param ApiKey The API key to be included in the connection request.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|WetSocket")
    void Connect(const FWebSocketConnectionParams& ConnectionParams);

    /**
     * Sends a message to the connected WebSocket server.
     *
     * @param Message The message to be sent.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|WetSocket")
    void SendMessage(const FString& Message);

    /**
     * Closes the WebSocket connection.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|WetSocket")
    void CloseConnection();

    // Event triggered when a message is received from the WebSocket server.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|WetSocket|Events")
    FOnWetSocketMessageReceived OnMessageReceived;

    // Event triggered when the WebSocket connection is successfully established.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|WetSocket|Events")
    FOnWetSocketConnected OnConnected;

    // Event triggered when an error occurs during WebSocket connection.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|WetSocket|Events")
    FOnWetSocketConnectionError OnConnectionError;

    // Event triggered when the WebSocket connection is closed.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|WetSocket|Events")
    FOnWetSocketDisconnected OnDisconnected;

protected:
    /**
     * Appends additional parameters to the URL.
     *
     * @param Url The original URL.
     * @param QueryParams The additional query parameters to be appended.
     * @return The modified URL with the query parameters.
     */
    FString AppendParametersToUrl(const FString& Url, const TMap<FString, FString>& QueryParams);


private:
    TSharedPtr<IWebSocket> WebSocket;

    /**
     * Initializes WebSocket events.
     */
    void InitializeWebSocketEvents();

    /**
     * Event handler for WebSocket connection.
     */
    void OnWebSocketConnected();

    /**
     * Event handler for WebSocket message received.
     *
     * @param Message The received message.
     */
    void OnWebSocketMessageReceived(const FString& Message);

    /**
     * Event handler for WebSocket connection error.
     *
     * @param Error The error message.
     */
    void OnWebSocketConnectionError(const FString& Error);

    /**
     * Event handler for WebSocket connection closed.
     *
     * @param StatusCode The status code of the connection.
     * @param Reason The reason for the connection closure.
     * @param bWasClean Indicates if the connection was closed cleanly.
     */
    void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);

    /**
     * Parses header strings and returns a map of headers.
     *
     * @param HeaderStrings The header strings to be parsed.
     * @return A map of headers.
     */
    TMap<FString, FString> ParseHeaders(const TArray<FString>& HeaderStrings);
};
