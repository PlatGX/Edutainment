// JsonHttpFunctionLibrary.h

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JsonHttpToolkit.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "JsonDomBuilder.h"
#include "JsonObjectConverter.h"
#include "JsonObjectWrapper.h"
#include "Misc/Paths.h"
#include "JsonHttpFunctionLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogJsonHttp, Log, All);

/**
 * Blueprint function library for JSON serialization and deserialization.
 */
UCLASS()
class EZUEGPT_API UJsonHttpFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Serializes a UObject to a JSON string.
     * @param InObject - The UObject to serialize.
     * @param OutJsonString - The resulting JSON string.
     * @param bSuccess - Whether the serialization was successful.
     */
    UFUNCTION(BlueprintPure, Category = "*AIITK|JsonHttp|Utility", meta = (DisplayName = "UObjectToJson"))
    static void SerializeObjectToJsonString(const UObject* InObject, FString& OutJsonString, bool& bSuccess);

    /**
     * Deserializes a JSON string to a UObject.
     * @param JsonString - The JSON string to deserialize.
     * @param OutObject - The UObject to populate with data.
     * @param bSuccess - Whether the deserialization was successful.
     */
    UFUNCTION(BlueprintPure, Category = "*AIITK|JsonHttp|Utility", meta = (DisplayName = "JsonToUObject"))
    static void DeserializeJsonStringToObject(const FString& JsonString, UObject* OutObject, bool& bSuccess);

    /**
     * Converts any struct to a JSON string.
     * @param AnyStruct - The struct to convert.
     * @return The JSON string representation of the struct.
     */
    UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "AnyStruct", DisplayName = "StructToJson"), Category = "*AIITK|JsonHttp|Utility")
    static FString ConvertStructToJsonString(const int32& AnyStruct);

    /** Custom thunk implementation for converting a struct to a JSON string. */
    DECLARE_FUNCTION(execConvertStructToJsonString);

    /**
     * Converts a JSON string to any UStruct.
     * @param JsonString - The JSON string to convert.
     * @param OutStruct - The struct to populate with the JSON data.
     * @param bSuccess - Whether the deserialization was successful.
     */
    UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "OutStruct", DisplayName = "JsonToStruct"), Category = "*AIITK|JsonHttp|Utility")
    static void ConvertJsonStringToStruct(const FString& JsonString, int32& OutStruct, bool& bSuccess);

    /** Custom thunk implementation for converting a JSON string to a UStruct. */
    DECLARE_FUNCTION(execConvertJsonStringToStruct);

    /**
     * Retrieves a field from a JSON string, supporting dot notation for nested fields.
     * @param JsonString - The JSON string to search in.
     * @param FieldName - The name of the field to search for, supporting dot notation (e.g., "user.address.city").
     * @param OutValue - The value of the field if found.
     * @param bSearchAll - Whether to search within nested JSON objects and arrays.
     * @param bSuccess - Whether the field retrieval was successful.
     */
    UFUNCTION(BlueprintPure, Category = "*AIITK|JsonHttp|Utility", meta = (DisplayName = "GetFieldFromJsonString"))
    static void GetFieldFromJsonString(const FString& JsonString, const FString& FieldName, FString& OutValue, bool bSearchAll, bool& bSuccess);

    /**
     * Sends an HTTP request using JsonHttpToolkit and returns the instance.
     * @param URL - The URL to send the request to.
     * @param JsonData - The JSON data to include in the request body.
     * @param ApiKey - The API key for authentication.
     * @param OnError - Delegate called on error.
     * @param OnProgress - Delegate called on progress.
     * @param OnHeaderReceived - Delegate called when headers are received.
     * @param OnComplete - Delegate called when the request is complete.
     * @return An instance of UJsonHttpToolkit handling the request.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|JsonHttp|Utility", meta = (AutoCreateRefTerm = "OnError, OnProgress, OnHeaderReceived, OnComplete", AdvancedDisplay = "OnError, OnProgress, OnHeaderReceived, OnComplete"))
    static UJsonHttpToolkit* SendJsonHttpRequest(const FString& URL, const FString& JsonData, const FString& ApiKey,
        const FOnHttpRequestError& OnError,
        const FOnHttpRequestProgress& OnProgress,
        const FOnHttpRequestHeaderReceived& OnHeaderReceived,
        const FOnHttpRequestComplete& OnComplete);

    /**
     * Converts a struct to a JSON string with the ability to exclude specific fields.
     * @param AnyStruct - The struct to convert to JSON.
     * @param FieldsToExclude - An array of field names to exclude from the resulting JSON.
     * @return The JSON string representation of the struct, excluding specified fields.
     */
    UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "AnyStruct", DisplayName = "StructToJsonWithExclusions"), Category = "*AIITK|JsonHttp|Utility")
    static FString ConvertStructToJsonStringWithExclusions(const int32& AnyStruct, const TArray<FString>& FieldsToExclude);

    /** Custom thunk implementation for converting a struct to JSON string with exclusions. */
    DECLARE_FUNCTION(execConvertStructToJsonStringWithExclusions);

    /**
    * Converts a JSON array string to an array of any type.
    * @param JsonString - The JSON string representing the array.
    * @param OutArray - The output array of the specified type.
    * @param bSuccess - Whether the conversion was successful.
    */
    UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "OutArray", DisplayName = "JsonArrayStringToArray"), Category = "*AIITK|JsonHttp|Utility")
    static void ConvertJsonArrayStringToArray(const FString& JsonString, TArray<int32>& OutArray, bool& bSuccess);

    /** Custom thunk implementation for converting a JSON array string to an array */
    DECLARE_FUNCTION(execConvertJsonArrayStringToArray);

private:
    /**
     * Finds a field in a JSON object, supporting dot notation for nested fields.
     * @param JsonObject - The JSON object to search in.
     * @param FieldName - The name of the field to search for.
     * @param OutValue - The value of the field if found.
     * @param bSearchAll - Whether to search within nested JSON objects and arrays.
     * @return True if the field was found, false otherwise.
     */
    static bool FindFieldInJsonObject(TSharedPtr<FJsonObject> JsonObject, const FString& FieldName, FString& OutValue, bool bSearchAll);

    /**
     * Recursive helper function to find a field in a JSON object based on field parts.
     * @param JsonObject - The JSON object to search in.
     * @param FieldParts - An array of field names representing the path.
     * @param PartIndex - The current index in the field parts array.
     * @param OutValue - The value of the field if found.
     * @param bSearchAll - Whether to search within nested JSON objects and arrays.
     * @return True if the field was found, false otherwise.
     */
    static bool FindFieldInJsonObjectRecursive(TSharedPtr<FJsonObject> JsonObject, const TArray<FString>& FieldParts, int32 PartIndex, FString& OutValue, bool bSearchAll);

    /**
     * Removes nested fields from a JSON object based on specified field names.
     * @param JsonObject - The JSON object to modify.
     * @param FieldNames - An array of field names to remove.
     */
    static void RemoveNestedField(TSharedPtr<FJsonObject> JsonObject, const TArray<FString>& FieldNames);
};
