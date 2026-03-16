// =================== GPTAssistantsAPI.h ===================
#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Http.h"
#include "JsonObjectConverter.h"
#include "JsonObjectWrapper.h"
#include "GPTChatAPIStructs.h"
#include "OpenAIFunctionLibrary.h"
#include "AIITKDeveloperSettings.h"
#include "GPTAssistantsAPI.generated.h"

// =================== ASSISTANTS ===================

// Struct for the response object of an assistant creation
USTRUCT(BlueprintType, Category = "AssistantsAPI")
struct FGPTAssistantObject
{
    GENERATED_BODY()

    // Unique ID of the assistant
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Id = "";

    // Type of the object, typically "assistant" or "assistant.deleted"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Object = "";

    // Timestamp of assistant creation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    int64 CreatedAt = 0;

    // Name of the assistant
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Name = "";

    // Description of the assistant
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Description = "";

    // Model used by the assistant
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Model = "";

    // Instructions provided to the assistant
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Instructions = "";

    // Tools associated with the assistant
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    TArray<FString> Tools;

    // Metadata attached to the assistant
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    TMap<FString, FString> Metadata;

    // Nucleus sampling parameter
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    float TopP = 1.0;

    // Sampling temperature
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    float Temperature = 1.0;

    // Format of the response
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString ResponseFormat = "";

    // Indicates if the assistant was deleted, relevant for delete responses
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    bool Deleted = false;

    // Stores the raw JSON response from the API
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString RawResponse = "";
};

// Struct for defining function tools
USTRUCT(BlueprintType, Category = "AssistantsAPI")
struct FGPTAssistantFunctionParams
{
    GENERATED_BODY()

    // Name of the function to be called, max length 64 characters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Name = "";

    // Description of what the function does
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Description = "";

    // Parameters accepted by the function as a JSON Schema string
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Parameters = "";

    // Whether to enforce strict schema adherence when generating function calls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    bool Strict = false;
};

// Struct for defining an assistant creation request
USTRUCT(BlueprintType, Category = "AssistantsAPI")
struct FGPTAssistantCreateParams
{
    GENERATED_BODY()

    // ID of the model to use for the assistant
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Model = "";

    // Name of the assistant, max length 256 characters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Name = "";

    // Description of the assistant, max length 512 characters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Description = "";

    // System instructions that the assistant uses, max length 256,000 characters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Instructions = "";

    // List of tools enabled on the assistant (e.g., "code_interpreter", "file_search", "function")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    TArray<FString> Tools;

    // Sampling temperature to use, between 0 and 2
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    float Temperature = 1.0f;

    // Nucleus sampling parameter, top_p probability mass consideration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    float TopP = 1.0f;

    // Format of the response, can be "auto", "text", "json_object", or "json_schema"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString ResponseFormat = "auto";

    // Optional metadata key-value pairs attached to the assistant
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    TMap<FString, FString> Metadata;

    // Tool-specific resources: List of file IDs available to the code interpreter tool
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    TArray<FString> CodeInterpreterFileIDs;

    // Tool-specific resources: Vector store IDs for the file search tool
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    TArray<FString> FileSearchVectorStoreIDs;
};

// Struct for modifying an assistant
USTRUCT(BlueprintType, Category = "AssistantsAPI")
struct FGPTAssistantModifyParams
{
    GENERATED_BODY()

    // ID of the assistant to modify
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString AssistantID = "";

    // ID of the model to use for the assistant
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Model = "";

    // Name of the assistant, max length 256 characters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Name = "";

    // Description of the assistant, max length 512 characters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Description = "";

    // System instructions that the assistant uses, max length 256,000 characters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Instructions = "";

    // List of tools enabled on the assistant (e.g., "code_interpreter", "file_search", "function")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    TArray<FString> Tools;

    // Sampling temperature to use, between 0 and 2
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    float Temperature = 1.0f;

    // Nucleus sampling parameter, top_p probability mass consideration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    float TopP = 1.0f;

    // Format of the response, can be "auto", "text", "json_object", or "json_schema"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString ResponseFormat = "auto";

    // Optional metadata key-value pairs attached to the assistant
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    TMap<FString, FString> Metadata;

    // Tool-specific resources: Overrides for file IDs in code interpreter tool
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    TArray<FString> CodeInterpreterFileIDs;

    // Tool-specific resources: Overrides for vector store IDs in file search tool
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    TArray<FString> FileSearchVectorStoreIDs;
};

// Struct for assistant retrieval parameters
USTRUCT(BlueprintType, Category = "AssistantsAPI")
struct FGPTAssistantRetrieveParams
{
    GENERATED_BODY()

    // ID of the assistant to retrieve
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString AssistantID = "";
};

// Struct for listing assistants with pagination and sorting options
USTRUCT(BlueprintType, Category = "AssistantsAPI")
struct FGPTAssistantListParams
{
    GENERATED_BODY()

    // Limit on the number of objects to be returned, between 1 and 100
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    int32 Limit = 20;

    // Sort order by the created_at timestamp of the objects, "asc" or "desc"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Order = "desc";

    // Cursor for pagination, defining the starting point in the list
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString After = "";

    // Cursor for pagination, defining the ending point in the list
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AssistantsAPI")
    FString Before = "";
};

// Delegates for asynchronous events in the Assistants API
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssistantCreated, const FGPTAssistantObject&, AssistantObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssistantRetrieved, const FGPTAssistantObject&, AssistantObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssistantsListed, const TArray<FGPTAssistantObject>&, Assistants);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssistantModified, const FGPTAssistantObject&, AssistantObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssistantDeleted, const FGPTAssistantObject&, AssistantObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssistantAPIError, const FString&, ErrorMessage);

// UGPTAssistantsAPI: Class for handling Assistants API functions
UCLASS(BlueprintType)
class EZUEGPT_API UGPTAssistantsAPI : public UObject
{
    GENERATED_BODY()

public:
    // Creates a new assistant using the specified parameters.
    // @param Params - The parameters required to create the assistant.
    // @return A pointer to the UGPTAssistantsAPI instance handling the request.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|AssistantsAPI")
    static UGPTAssistantsAPI* CreateAssistant(const FGPTAssistantCreateParams& Params);

    // Retrieves an existing assistant by its ID.
    // @param Params - The parameters containing the assistant ID to retrieve.
    // @return A pointer to the UGPTAssistantsAPI instance handling the request.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|AssistantsAPI")
    static UGPTAssistantsAPI* RetrieveAssistant(const FGPTAssistantRetrieveParams& Params);

    // Modifies an existing assistant with new parameters.
    // @param Params - The parameters required to modify the assistant.
    // @return A pointer to the UGPTAssistantsAPI instance handling the request.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|AssistantsAPI")
    static UGPTAssistantsAPI* ModifyAssistant(const FGPTAssistantModifyParams& Params);

    // Deletes an assistant specified by its ID.
    // @param Params - The parameters containing the assistant ID to delete.
    // @return A pointer to the UGPTAssistantsAPI instance handling the request.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|AssistantsAPI")
    static UGPTAssistantsAPI* DeleteAssistant(const FGPTAssistantRetrieveParams& Params);

    // Lists all assistants using the provided parameters for pagination and sorting.
    // @param Params - The parameters controlling the listing behavior, including limits and order.
    // @return A pointer to the UGPTAssistantsAPI instance handling the request.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|AssistantsAPI")
    static UGPTAssistantsAPI* ListAssistants(const FGPTAssistantListParams& Params);

    // Delegate called when an assistant is successfully created.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|AssistantsAPI")
    FOnAssistantCreated OnAssistantCreated;

    // Delegate called when an assistant is successfully retrieved or listed.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|AssistantsAPI")
    FOnAssistantRetrieved OnAssistantRetrieved;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|AssistantsAPI")
    FOnAssistantsListed OnAssistantsListed;

    // Delegate called when an assistant is successfully modified.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|AssistantsAPI")
    FOnAssistantModified OnAssistantModified;

    // Delegate called when an assistant is successfully deleted.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|AssistantsAPI")
    FOnAssistantDeleted OnAssistantDeleted;

    // Delegate called when an API error occurs.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|AssistantsAPI")
    FOnAssistantAPIError OnAPIError;

private:
    // Current HTTP request being processed.
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentHttpRequest;

    // Stores the type of request currently being processed (e.g., create, retrieve).
    FString CurrentRequestType;

    // Sends a create request to the API with the provided parameters.
    void SendCreateRequest(const FGPTAssistantCreateParams& Params);

    // Sends a retrieve request to the API for the specified assistant ID.
    void SendRetrieveRequest(const FGPTAssistantRetrieveParams& Params);

    // Sends a modify request to update an assistant with new parameters.
    void SendModifyRequest(const FGPTAssistantModifyParams& Params);

    // Sends a delete request to remove the specified assistant.
    void SendDeleteRequest(const FGPTAssistantRetrieveParams& Params);

    // Sends a list request to retrieve a list of assistants based on parameters.
    void SendListRequest(const FGPTAssistantListParams& Params);

    // Handles the HTTP response based on the type of request made.
    void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    // Handles errors returned by the API and broadcasts error messages.
    void HandleError(const FString& ErrorMessage);
};