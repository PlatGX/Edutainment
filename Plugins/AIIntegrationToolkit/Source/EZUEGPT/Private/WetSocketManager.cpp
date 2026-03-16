#include "WetSocketManager.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"

DEFINE_LOG_CATEGORY(WetSockManLog);

UWetSocketManager::UWetSocketManager()
{
    WebSocket = nullptr;
}

TMap<FString, FString> UWetSocketManager::ParseHeaders(const TArray<FString>& HeaderStrings)
{
    TMap<FString, FString> HeaderMap;

    for (const FString& HeaderString : HeaderStrings)
    {
        FString Key, Value;
        if (HeaderString.Split(TEXT(":"), &Key, &Value))
        {
            Key = Key.TrimStartAndEnd();
            Value = Value.TrimStartAndEnd();

            if (!Key.IsEmpty() && !Value.IsEmpty())
            {
                HeaderMap.Add(Key, Value);
            }
            else
            {
                UE_LOG(WetSockManLog, Warning, TEXT("WetSocket: Invalid header key or value in '%s'."), *HeaderString);
            }
        }
        else
        {
            UE_LOG(WetSockManLog, Warning, TEXT("WetSocket: Invalid header format: '%s'"), *HeaderString);
        }
    }

    return HeaderMap;
}

FString UWetSocketManager::AppendParametersToUrl(const FString& Url, const TMap<FString, FString>& QueryParams)
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

void UWetSocketManager::Connect(const FWebSocketConnectionParams& ConnectionParams)
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        UE_LOG(WetSockManLog, Warning, TEXT("WetSocket: Already connected."));
        if (OnConnectionError.IsBound())
        {
            OnConnectionError.Broadcast();
        }
        return;
    }

    TMap<FString, FString> UpgradeHeaders = ConnectionParams.HeaderParams;

    if (!ConnectionParams.ApiKey.IsEmpty())
    {
        UpgradeHeaders.Add(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *ConnectionParams.ApiKey));
    }

    if (!ConnectionParams.Xi_ApiKey.IsEmpty())
    {
        UpgradeHeaders.Add(TEXT("xi-api-key"), *ConnectionParams.Xi_ApiKey);
    }

    FString FullPathExt;
    if (ConnectionParams.PathExts.Num() > 0)
    {
        FullPathExt = TEXT("/") + FString::Join(ConnectionParams.PathExts, TEXT("/"));
    }

    FString ModifiedUrl = AppendParametersToUrl(ConnectionParams.Url + FullPathExt, ConnectionParams.QueryParams);
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(ModifiedUrl, TArray<FString>(), UpgradeHeaders);

    if (!WebSocket.IsValid())
    {
        UE_LOG(WetSockManLog, Error, TEXT("WetSocket: Failed to create WebSocket."));

        if (OnConnectionError.IsBound())
        {
            OnConnectionError.Broadcast();
        }
        return;
    }

    InitializeWebSocketEvents();

    AddToRoot();  // Prevent garbage collection

    WebSocket->Connect();
    UE_LOG(WetSockManLog, Log, TEXT("WetSocket: Attempting to connect to %s"), *ModifiedUrl);
}

void UWetSocketManager::InitializeWebSocketEvents()
{
    WebSocket->OnConnected().AddUObject(this, &UWetSocketManager::OnWebSocketConnected);
    WebSocket->OnMessage().AddUObject(this, &UWetSocketManager::OnWebSocketMessageReceived);
    WebSocket->OnConnectionError().AddUObject(this, &UWetSocketManager::OnWebSocketConnectionError);
    WebSocket->OnClosed().AddUObject(this, &UWetSocketManager::OnWebSocketClosed);
}

void UWetSocketManager::OnWebSocketConnected()
{
    UE_LOG(WetSockManLog, Log, TEXT("WetSocket: WebSocket connected."));
    if (OnConnected.IsBound())
    {
        OnConnected.Broadcast();
    }
}

void UWetSocketManager::OnWebSocketMessageReceived(const FString& Message)
{
    if (IsValid(this) && OnMessageReceived.IsBound())
    {
        UE_LOG(WetSockManLog, Log, TEXT("WetSocket: Received message: %s"), *Message);
        OnMessageReceived.Broadcast(Message);
    }
}


void UWetSocketManager::OnWebSocketConnectionError(const FString& Error)
{
    UE_LOG(WetSockManLog, Error, TEXT("WetSocket: Connection error: %s"), *Error);

    if (OnConnectionError.IsBound())
    {
        OnConnectionError.Broadcast();
    }

    RemoveFromRoot();  // Allow garbage collection
}

void UWetSocketManager::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(WetSockManLog, Log, TEXT("WetSocket: WebSocket closed. Status: %d, Reason: %s, Clean: %s"),
        StatusCode, *Reason, bWasClean ? TEXT("true") : TEXT("false"));

    if (OnDisconnected.IsBound())
    {
        OnDisconnected.Broadcast();
    }

    WebSocket = nullptr;
    RemoveFromRoot();  // Allow garbage collection
}

void UWetSocketManager::SendMessage(const FString& Message)
{
    if (!WebSocket.IsValid() || !WebSocket->IsConnected())
    {
        UE_LOG(WetSockManLog, Warning, TEXT("WetSocket: WebSocket is not connected, cannot send message."));
        return;
    }

    WebSocket->Send(Message);
    UE_LOG(WetSockManLog, Log, TEXT("WetSocket: Sent message"));
}

void UWetSocketManager::CloseConnection()
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        WebSocket->Close();
        UE_LOG(WetSockManLog, Log, TEXT("WetSocket: WebSocket connection closed."));
    }
    else
    {
        UE_LOG(WetSockManLog, Log, TEXT("WetSocket: WebSocket is not connected."));
    }

    WebSocket = nullptr;
    RemoveFromRoot();  // Allow garbage collection
}
