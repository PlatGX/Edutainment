// =================== GPTThreadsAPI.h ===================
#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Http.h"
#include "JsonObjectConverter.h"
#include "GPTChatAPIStructs.h"
#include "AIITKDeveloperSettings.h"
#include "GPTThreadsAPI.generated.h"

// Struct for thread object
USTRUCT(BlueprintType, Category = "ThreadsAPI")
struct FGPTThreadObject
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThreadsAPI")
    FString Id = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThreadsAPI")
    FString Object = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThreadsAPI")
    int64 CreatedAt = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThreadsAPI")
    TMap<FString, FString> Metadata;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThreadsAPI")
    FString RawResponse = "";
};

// Struct for message within a thread
USTRUCT(BlueprintType, Category = "ThreadsAPI")
struct FGPTMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThreadsAPI")
    FString Role = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThreadsAPI")
    FString Content = "";
};

// Struct for creating a thread
USTRUCT(BlueprintType, Category = "ThreadsAPI")
struct FGPTThreadCreateParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThreadsAPI")
    TArray<FGPTMessage> Messages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThreadsAPI")
    TMap<FString, FString> Metadata;
};

// Struct for retrieving, modifying, or deleting a thread by ID
USTRUCT(BlueprintType, Category = "ThreadsAPI")
struct FGPTThreadRetrieveParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThreadsAPI")
    FString ThreadID = "";
};

// Struct for modifying a thread
USTRUCT(BlueprintType, Category = "ThreadsAPI")
struct FGPTThreadModifyParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThreadsAPI")
    FString ThreadID = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThreadsAPI")
    TMap<FString, FString> Metadata;
};

// Delegate signatures
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreadCreated, const FGPTThreadObject&, ThreadObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreadRetrieved, const FGPTThreadObject&, ThreadObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreadModified, const FGPTThreadObject&, ThreadObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreadDeleted, const FGPTThreadObject&, ThreadObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreadAPIError, const FString&, ErrorMessage);

UCLASS(BlueprintType)
class EZUEGPT_API UGPTThreadsAPI : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|ThreadsAPI")
    static UGPTThreadsAPI* CreateThread(const FGPTThreadCreateParams& Params);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|ThreadsAPI")
    static UGPTThreadsAPI* RetrieveThread(const FGPTThreadRetrieveParams& Params);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|ThreadsAPI")
    static UGPTThreadsAPI* ModifyThread(const FGPTThreadModifyParams& Params);

    UFUNCTION(BlueprintCallable, Category = "*AIITK|Assistants|ThreadsAPI")
    static UGPTThreadsAPI* DeleteThread(const FGPTThreadRetrieveParams& Params);

    // Delegates for thread operations
    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|ThreadsAPI")
    FOnThreadCreated OnThreadCreated;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|ThreadsAPI")
    FOnThreadRetrieved OnThreadRetrieved;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|ThreadsAPI")
    FOnThreadModified OnThreadModified;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|ThreadsAPI")
    FOnThreadDeleted OnThreadDeleted;

    UPROPERTY(BlueprintAssignable, Category = "*AIITK|Assistants|ThreadsAPI")
    FOnThreadAPIError OnAPIError;

private:
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentHttpRequest;
    FString CurrentRequestType;

    void SendCreateRequest(const FGPTThreadCreateParams& Params);
    void SendRetrieveRequest(const FGPTThreadRetrieveParams& Params);
    void SendModifyRequest(const FGPTThreadModifyParams& Params);
    void SendDeleteRequest(const FGPTThreadRetrieveParams& Params);

    void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void HandleError(const FString& ErrorMessage);
};
