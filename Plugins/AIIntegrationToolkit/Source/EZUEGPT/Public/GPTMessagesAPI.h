// =================== GPTMessagesAPI.h ===================
#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Http.h"
#include "JsonObjectConverter.h"
#include "JsonObjectWrapper.h"
#include "JsonObjectConverter.h"
#include "GPTChatAPIStructs.h"
#include "OpenAIFunctionLibrary.h"
#include "AIITKDeveloperSettings.h"
#include "GPTMessagesAPI.generated.h"

// =================== MESSAGES ===================

// Struct for the response object of a message
USTRUCT(BlueprintType, Category = "MessagesAPI")
struct FGPTMessageObject
{
    GENERATED_BODY()

    // Unique ID of the message
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString Id = "";

    // Type of the object, typically "thread.message"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString Object = "";

    // Timestamp of message creation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    int64 CreatedAt = 0;

    // Role of the entity that sent the message, either "user" or "assistant"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString Role = "";

    // Thread ID the message belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString ThreadID = "";

    // Content of the message, can be text or array of content
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    TArray<FString> Content;

    // Metadata attached to the message
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    TMap<FString, FString> Metadata;

    // Stores the raw JSON response from the API
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString RawResponse = "";
};

// Struct for creating a message
USTRUCT(BlueprintType, Category = "MessagesAPI")
struct FGPTMessageCreateParams
{
    GENERATED_BODY()

    // The thread ID to which this message belongs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString ThreadID = "";

    // Role of the entity that sends the message (user or assistant)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString Role = "";

    // The content of the message
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString Content = "";

    // Optional metadata key-value pairs attached to the message
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    TMap<FString, FString> Metadata;
};

// Struct for retrieving a message
USTRUCT(BlueprintType, Category = "MessagesAPI")
struct FGPTMessageRetrieveParams
{
    GENERATED_BODY()

    // The thread ID the message belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString ThreadID = "";

    // The ID of the message to retrieve
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString MessageID = "";
};

// Struct for listing messages in a thread
USTRUCT(BlueprintType, Category = "MessagesAPI")
struct FGPTMessageListParams
{
    GENERATED_BODY()

    // The thread ID to list messages for
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString ThreadID = "";

    // Limit on the number of messages returned
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    int32 Limit = 20;

    // Sort order by created_at timestamp, "asc" or "desc"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString Order = "desc";

    // Cursor for pagination, start from this message ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString After = "";

    // Cursor for pagination, end at this message ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString Before = "";
};

// Struct for modifying a message
USTRUCT(BlueprintType, Category = "MessagesAPI")
struct FGPTMessageModifyParams
{
    GENERATED_BODY()

    // The thread ID the message belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString ThreadID = "";

    // The ID of the message to modify
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString MessageID = "";

    // Optional metadata key-value pairs attached to the message
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    TMap<FString, FString> Metadata;
};

// Struct for deleting a message
USTRUCT(BlueprintType, Category = "MessagesAPI")
struct FGPTMessageDeleteParams
{
    GENERATED_BODY()

    // The thread ID the message belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString ThreadID = "";

    // The ID of the message to delete
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessagesAPI")
    FString MessageID = "";
};

// Delegates for asynchronous events in the Messages API
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageCreated, const FGPTMessageObject&, MessageObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageRetrieved, const FGPTMessageObject&, MessageObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessagesListed, const TArray<FGPTMessageObject>&, Messages);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageModified, const FGPTMessageObject&, MessageObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageDeleted, const FGPTMessageObject&, MessageObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageAPIError, const FString&, ErrorMessage);

// UGPTMessagesAPI: Class for handling Messages API functions
UCLASS(BlueprintType)
class EZUEGPT_API UGPTMessagesAPI : public UObject
{
    GENERATED_BODY()

public:
    // Creates a new message in a thread using the specified parameters.
    // @param Params - The parameters required to create the message.
    // @return A pointer to the UGPTMessagesAPI instance handling the request.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|MessagesAPI")
    static UGPTMessagesAPI* CreateMessage(const FGPTMessageCreateParams& Params);

    // Retrieves an existing message by its ID.
    // @param Params - The parameters containing the message ID and thread ID.
    // @return A pointer to the UGPTMessagesAPI instance handling the request.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|MessagesAPI")
    static UGPTMessagesAPI* RetrieveMessage(const FGPTMessageRetrieveParams& Params);

    // Lists messages in a thread using the provided parameters.
    // @param Params - The parameters controlling the listing behavior.
    // @return A pointer to the UGPTMessagesAPI instance handling the request.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|MessagesAPI")
    static UGPTMessagesAPI* ListMessages(const FGPTMessageListParams& Params);

    // Modifies an existing message with new metadata.
    // @param Params - The parameters required to modify the message.
    // @return A pointer to the UGPTMessagesAPI instance handling the request.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|MessagesAPI")
    static UGPTMessagesAPI* ModifyMessage(const FGPTMessageModifyParams& Params);

    // Deletes a message specified by its ID.
    // @param Params - The parameters containing the message ID and thread ID.
    // @return A pointer to the UGPTMessagesAPI instance handling the request.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|MessagesAPI")
    static UGPTMessagesAPI* DeleteMessage(const FGPTMessageDeleteParams& Params);

    // Delegate called when a message is successfully created.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|MessagesAPI")
    FOnMessageCreated OnMessageCreated;

    // Delegate called when a message is successfully retrieved.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|MessagesAPI")
    FOnMessageRetrieved OnMessageRetrieved;

    // Delegate called when messages are successfully listed (array of messages).
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|MessagesAPI")
    FOnMessagesListed OnMessagesListed;

    // Delegate called when a message is successfully modified.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|MessagesAPI")
    FOnMessageModified OnMessageModified;

    // Delegate called when a message is successfully deleted.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|MessagesAPI")
    FOnMessageDeleted OnMessageDeleted;

    // Delegate called when an API error occurs.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|MessagesAPI")
    FOnMessageAPIError OnAPIError;

private:
    // Current HTTP request being processed.
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentHttpRequest;

    // Stores the type of request currently being processed (e.g., create, retrieve).
    FString CurrentRequestType;

    // Sends a create request to the API with the provided parameters.
    void SendCreateRequest(const FGPTMessageCreateParams& Params);

    // Sends a retrieve request to the API for the specified message ID.
    void SendRetrieveRequest(const FGPTMessageRetrieveParams& Params);

    // Sends a list request to retrieve a list of messages based on parameters.
    void SendListRequest(const FGPTMessageListParams& Params);

    // Sends a modify request to update a message with new metadata.
    void SendModifyRequest(const FGPTMessageModifyParams& Params);

    // Sends a delete request to remove the specified message.
    void SendDeleteRequest(const FGPTMessageDeleteParams& Params);

    // Handles the HTTP response based on the type of request made.
    void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    // Handles errors returned by the API and broadcasts error messages.
    void HandleError(const FString& ErrorMessage);
};
