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
#include "GPTRunsAPI.generated.h"

// =================== RUNS ===================

// Struct for the response object of a run
USTRUCT(BlueprintType, Category = "RunsAPI")
struct FGPTRunObject
{
    GENERATED_BODY()

    // Unique ID of the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString Id = "";

    // Type of the object, typically "thread.run"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString Object = "";

    // Timestamp of run creation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    int64 CreatedAt = 0;

    // Status of the run (e.g., queued, in_progress, completed)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString Status = "";

    // ID of the assistant used for the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString AssistantId = "";

    // ID of the thread associated with the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString ThreadId = "";

    // Timestamp of when the run was started
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    int64 StartedAt = 0;

    // Timestamp of when the run was completed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    int64 CompletedAt = 0;

    // List of tools used by the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    TArray<FString> Tools;

    // Metadata attached to the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    TMap<FString, FString> Metadata;

    // Sampling temperature
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    float Temperature = 1.0;

    // Nucleus sampling parameter
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    float TopP = 1.0;

    // Format of the response
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString ResponseFormat = "";

    // Stores the raw JSON response from the API
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString RawResponse = "";
};

// Struct for creating a run
USTRUCT(BlueprintType, Category = "RunsAPI")
struct FGPTCreateRunParams
{
    GENERATED_BODY()

    // ID of the assistant to use for the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString AssistantId = "";

    // ID of the thread to use for the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString ThreadId = "";

    // Optional: Model to use for the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString Model = "";

    // Optional: Instructions for the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString Instructions = "";

    // Optional: Additional instructions to append
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString AdditionalInstructions = "";

    // Optional: List of additional messages for the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    TArray<FString> AdditionalMessages;

    // Temperature value for sampling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    float Temperature = 1.0f;

    // Top_p value for nucleus sampling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    float TopP = 1.0f;

    // Response format (auto, text, json_object, json_schema)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString ResponseFormat = "auto";

    // Stream the response?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    bool Stream = false;

    // Optional metadata key-value pairs attached to the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    TMap<FString, FString> Metadata;
};

// Struct for submitting tool outputs to a run
USTRUCT(BlueprintType, Category = "RunsAPI")
struct FGPTSubmitToolOutputsParams
{
    GENERATED_BODY()

    // ID of the thread associated with the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString ThreadId = "";

    // ID of the run to submit tool outputs for
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString RunId = "";

    // Tool outputs to submit
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    TArray<FString> ToolOutputs;
};

// Struct for canceling a run
USTRUCT(BlueprintType, Category = "RunsAPI")
struct FGPTCancelRunParams
{
    GENERATED_BODY()

    // ID of the thread associated with the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString ThreadId = "";

    // ID of the run to cancel
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString RunId = "";
};

// Struct for retrieving a run
USTRUCT(BlueprintType, Category = "RunsAPI")
struct FGPTRetrieveRunParams
{
    GENERATED_BODY()

    // ID of the run to retrieve
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString RunId = "";

    // ID of the thread associated with the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString ThreadId = "";
};

// Struct for listing runs in a thread
USTRUCT(BlueprintType, Category = "RunsAPI")
struct FGPTRunListParams
{
    GENERATED_BODY()

    // ID of the thread whose runs should be listed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString ThreadId = "";

    // Optional limit of objects returned
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    int32 Limit = 20;

    // Sort order of the list, can be 'asc' or 'desc'
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString Order = "desc";
};

// Struct for modifying a run
USTRUCT(BlueprintType, Category = "RunsAPI")
struct FGPTModifyRunParams
{
    GENERATED_BODY()

    // ID of the run to modify
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString RunId = "";

    // ID of the thread associated with the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString ThreadId = "";

    // Optional metadata key-value pairs to modify
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    TMap<FString, FString> Metadata;
};

// Struct for the modified run response
USTRUCT(BlueprintType, Category = "RunsAPI")
struct FModifiedRunObject
{
    GENERATED_BODY()

    // Unique ID of the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString Id = "";

    // Status of the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString Status = "";

    // ID of the assistant used for the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString AssistantId = "";

    // ID of the thread associated with the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString ThreadId = "";

    // Model used for the run
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString Model = "";

    // Format of the response
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString ResponseFormat = "";

    // Stores the raw JSON response from the API
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RunsAPI")
    FString RawResponse = "";
};

// Delegates for asynchronous events in the Runs API
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunCreated, const FGPTRunObject&, RunObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunRetrieved, const FGPTRunObject&, RunObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunsListed, const TArray<FGPTRunObject>&, RunObjects);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToolOutputsSubmitted, const FModifiedRunObject&, ModifiedRunObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunCancelled, const FModifiedRunObject&, ModifiedRunObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunAPIError, const FString&, ErrorMessage);

// UGPTRunsAPI: Class for handling Runs API functions
UCLASS(BlueprintType)
class EZUEGPT_API UGPTRunsAPI : public UObject
{
    GENERATED_BODY()

public:
    // Creates a new run using the specified parameters.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|RunsAPI")
    static UGPTRunsAPI* CreateRun(const FGPTCreateRunParams& Params);

    // Retrieves an existing run by its ID.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|RunsAPI")
    static UGPTRunsAPI* RetrieveRun(const FGPTRetrieveRunParams& Params);

    // Lists all runs for a given thread.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|RunsAPI")
    static UGPTRunsAPI* ListRuns(const FGPTRunListParams& Params);

    // Submits tool outputs for a specific run.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|RunsAPI")
    static UGPTRunsAPI* SubmitToolOutputs(const FGPTSubmitToolOutputsParams& Params);

    // Cancels a specific run.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|RunsAPI")
    static UGPTRunsAPI* CancelRun(const FGPTCancelRunParams& Params);

    // Modifies an existing run with new parameters.
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|RunsAPI")
    static UGPTRunsAPI* ModifyRun(const FGPTModifyRunParams& Params);

    // Delegate called when a run is successfully created.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|RunsAPI")
    FOnRunCreated OnRunCreated;

    // Delegate called when a run is successfully retrieved.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|RunsAPI")
    FOnRunRetrieved OnRunRetrieved;

    // Delegate called when a list of runs is successfully retrieved.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|RunsAPI")
    FOnRunsListed OnRunsListed;

    // Delegate called when tool outputs are successfully submitted.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|RunsAPI")
    FOnToolOutputsSubmitted OnToolOutputsSubmitted;

    // Delegate called when a run is successfully canceled.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|RunsAPI")
    FOnRunCancelled OnRunCancelled;

    // Delegate called when an API error occurs.
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|RunsAPI")
    FOnRunAPIError OnAPIError;

private:
    // Sends a create request to the API with the provided parameters.
    void SendCreateRunRequest(const FGPTCreateRunParams& Params);

    // Sends a retrieve request to the API for the specified run ID.
    void SendRetrieveRunRequest(const FGPTRetrieveRunParams& Params);

    // Sends a list request for runs in a thread.
    void SendListRunsRequest(const FGPTRunListParams& Params);

    // Sends a submit tool outputs request for a run.
    void SendSubmitToolOutputsRequest(const FGPTSubmitToolOutputsParams& Params);

    // Sends a cancel request for a run.
    void SendCancelRunRequest(const FGPTCancelRunParams& Params);

    // Sends a modify request for a run.
    void SendModifyRunRequest(const FGPTModifyRunParams& Params);

    // Handles the HTTP response based on the type of request made.
    void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    // Handles responses for modified runs (submit, cancel).
    void HandleModifiedRunResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    // Handles errors returned by the API and broadcasts error messages.
    void HandleError(const FString& ErrorMessage);
    void HandleModifiedRunError(const FString& ErrorMessage);

    // Current HTTP request being processed.
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentHttpRequest;

    // Stores the type of request currently being processed (e.g., create, retrieve, submit, cancel).
    FString CurrentRequestType;
};
