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


// ========== AIITK_NTK_JsonHttpFunctionLibrary.cpp ==========

// AIITK_NTK_JsonHttpFunctionLibrary.cpp

#include "AIITK_NTK_JsonHttpFunctionLibrary.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Policies/JsonPrintPolicy.h"


// Define the logging category
DEFINE_LOG_CATEGORY(NTKLogJsonHttp);

// Custom JSON Print Policy with Rounding and Standard Formatting
template <typename CharType>
class TRoundingJsonPrintPolicy : public TJsonPrintPolicy<CharType>
{
public:
    // Helper function to remove trailing zeros and decimal point
    static void RemoveTrailingZerosAndDot(FString& Str)
    {
        while (Str.Len() > 0 && Str.EndsWith("0"))
        {
            Str = Str.LeftChop(1);
        }
        if (Str.Len() > 0 && Str.EndsWith("."))
        {
            Str = Str.LeftChop(1);
        }
    }

    // Override for writing floating-point numbers with rounding
    static void WriteFloat(FArchive* Stream, float Value)
    {
        float RoundedValue = FMath::RoundHalfToEven(Value * 1000.0f) / 1000.0f;
        FString ValueString = FString::Printf(TEXT("%.3f"), RoundedValue);
        RemoveTrailingZerosAndDot(ValueString);
        TJsonPrintPolicy<CharType>::WriteString(Stream, ValueString);
    }

    // Override for writing double numbers with rounding
    static void WriteDouble(FArchive* Stream, double Value)
    {
        double RoundedValue = FMath::RoundHalfToEven(Value * 1000.0) / 1000.0;
        FString ValueString = FString::Printf(TEXT("%.3f"), RoundedValue);
        RemoveTrailingZerosAndDot(ValueString);
        TJsonPrintPolicy<CharType>::WriteString(Stream, ValueString);
    }

    // Override to add line terminators (standard formatting)
    static void WriteLineTerminator(FArchive* Stream)
    {
        CharType Terminator = '\n';
        Stream->Serialize(&Terminator, sizeof(CharType));
    }

    // Override to add space characters (standard formatting)
    static void WriteSpace(FArchive* Stream)
    {
        CharType Space = ' ';
        Stream->Serialize(&Space, sizeof(CharType));
    }

    // Override to add tabs for indentation (standard formatting)
    static void WriteTabs(FArchive* Stream, int32 Count)
    {
        CharType Tab = '\t';
        for (int32 i = 0; i < Count; ++i)
        {
            Stream->Serialize(&Tab, sizeof(CharType));
        }
    }
};


UAIITK_NTK_JsonHttpTools* UAIITK_NTK_JsonHttpFunctionLibrary::SendHttpRequestWithConfig(
    const FAIITK_NTK_JsonHttpRequestConfig& RequestConfig,
    const FOnJsonHttpRequestError& OnError,
    const FOnJsonHttpRequestProgress& OnProgress,
    const FOnJsonHttpRequestHeaderReceived& OnHeaderReceived,
    const FOnJsonHttpRequestComplete& OnComplete)
{
    // Create an instance of the AIITK_NTK_JsonHttpTools
    UAIITK_NTK_JsonHttpTools* ToolkitInstance = NewObject<UAIITK_NTK_JsonHttpTools>();

    // Build the full URL with query parameters
    FString FullURL = RequestConfig.URL;
    if (RequestConfig.QueryParameters.Num() > 0)
    {
        FullURL += TEXT("?");
        for (const auto& Param : RequestConfig.QueryParameters)
        {
            FullURL += FString::Printf(TEXT("%s=%s&"),
                *FGenericPlatformHttp::UrlEncode(Param.Key),
                *FGenericPlatformHttp::UrlEncode(Param.Value));
        }
        FullURL.RemoveFromEnd(TEXT("&"));
    }

    // Configure headers
    TMap<FString, FString> Headers = RequestConfig.Headers;

    // Helper lambda to detect sensitive headers
    auto IsSensitiveHeader = [](const FString& HeaderName)
    {
        return HeaderName.ToLower().Contains(TEXT("authorization"));
    };

    // Debug logging: print out request details before sending
    UE_LOG(NTKLogJsonHttp, Log, TEXT("-------- Request Debug Start --------"));
    UE_LOG(NTKLogJsonHttp, Log, TEXT("Sending HTTP Request:"));
    UE_LOG(NTKLogJsonHttp, Log, TEXT("URL: %s"), *FullURL);
    UE_LOG(NTKLogJsonHttp, Log, TEXT("Method: %s"), *RequestConfig.Method);
    UE_LOG(NTKLogJsonHttp, Log, TEXT("Body: %s"), *RequestConfig.Body);

    // Build a headers string, masking sensitive values
    FString HeadersString;
    for (const auto& Header : Headers)
    {
        if (IsSensitiveHeader(Header.Key))
        {
            HeadersString += FString::Printf(TEXT("%s: [REDACTED]; "), *Header.Key);
        }
        else
        {
            HeadersString += FString::Printf(TEXT("%s: %s; "), *Header.Key, *Header.Value);
        }
    }
    UE_LOG(NTKLogJsonHttp, Log, TEXT("Headers: %s"), *HeadersString);
    UE_LOG(NTKLogJsonHttp, Log, TEXT("-------- Request Debug End --------"));

    // Call SendHttpRequest with the assembled data
    ToolkitInstance->SendHttpRequest(
        FullURL,
        Headers,
        RequestConfig.Method,
        RequestConfig.Body,
        OnError,
        OnProgress,
        OnHeaderReceived,
        OnComplete);

    // Return the toolkit instance
    return ToolkitInstance;
}




void UAIITK_NTK_JsonHttpFunctionLibrary::SerializeObjectToJsonString(const UObject* InObject, FString& OutJsonString, bool& bSuccess)
{
    if (!InObject)
    {
        bSuccess = false;
        return;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    bSuccess = FJsonObjectConverter::UStructToJsonObject(InObject->GetClass(), InObject, JsonObject.ToSharedRef(), 0, 0);

    if (bSuccess)
    {
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJsonString);
        bSuccess = FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    }
}

void UAIITK_NTK_JsonHttpFunctionLibrary::DeserializeJsonStringToObject(const FString& JsonString, UObject* OutObject, bool& bSuccess)
{
    if (!OutObject)
    {
        bSuccess = false;
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        bSuccess = FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), OutObject->GetClass(), OutObject, 0, 0);
    }
    else
    {
        bSuccess = false;
    }
}

// Custom thunk implementation for ConvertJsonStringToStruct
DEFINE_FUNCTION(UAIITK_NTK_JsonHttpFunctionLibrary::execConvertJsonStringToStruct)
{
    P_GET_PROPERTY(FStrProperty, JsonString);
    Stack.MostRecentProperty = nullptr;
    Stack.StepCompiledIn<FStructProperty>(nullptr);
    FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
    void* StructPtr = Stack.MostRecentPropertyAddress;
    P_GET_UBOOL_REF(bSuccess);
    P_FINISH;
    P_NATIVE_BEGIN;

    if (!StructProperty || !StructPtr)
    {
        UE_LOG(NTKLogJsonHttp, Error, TEXT("Invalid struct or struct pointer address."));
        bSuccess = false;
        return;
    }

    if (JsonString.IsEmpty())
    {
        UE_LOG(NTKLogJsonHttp, Warning, TEXT("JSON string is empty."));
        bSuccess = false;
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(NTKLogJsonHttp, Warning, TEXT("Failed to parse JSON string."));
        bSuccess = false;
        return;
    }

    bool bDeserializationSuccess = FJsonObjectConverter::JsonObjectToUStruct(
        JsonObject.ToSharedRef(),
        StructProperty->Struct,
        StructPtr,
        0,
        0,
        false
    );

    if (!bDeserializationSuccess)
    {
        UE_LOG(NTKLogJsonHttp, Error, TEXT("Failed to deserialize JSON into UStruct."));
        bSuccess = false;
        return;
    }

    bSuccess = true;

    P_NATIVE_END;
}

void UAIITK_NTK_JsonHttpFunctionLibrary::GetFieldFromJsonString(const FString& JsonString, const FString& FieldName, FString& OutValue, bool bSearchAll, bool& bSuccess)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        bSuccess = FindFieldInJsonObject(JsonObject, FieldName, OutValue, bSearchAll);
    }
    else
    {
        bSuccess = false;
    }
}

bool UAIITK_NTK_JsonHttpFunctionLibrary::FindFieldInJsonObject(TSharedPtr<FJsonObject> JsonObject, const FString& FieldName, FString& OutValue, bool bSearchAll)
{
    TArray<FString> FieldParts;
    FieldName.ParseIntoArray(FieldParts, TEXT("."), true);

    return FindFieldInJsonObjectRecursive(JsonObject, FieldParts, 0, OutValue, bSearchAll);
}

bool UAIITK_NTK_JsonHttpFunctionLibrary::FindFieldInJsonObjectRecursive(TSharedPtr<FJsonObject> JsonObject,
    const TArray<FString>& FieldParts, int32 PartIndex, FString& OutValue, bool bSearchAll)
{
    if (!JsonObject.IsValid())
    {
        return false;
    }

    FString CurrentField = FieldParts[PartIndex];
    bool bFound = false;

    // If the current object has the key, try to use it.
    if (JsonObject->HasField(CurrentField))
    {
        TSharedPtr<FJsonValue> FieldValue = JsonObject->TryGetField(CurrentField);
        if (FieldValue.IsValid())
        {
            if (PartIndex == FieldParts.Num() - 1)
            {
                // Final field: extract the value based on type.
                if (FieldValue->Type == EJson::String)
                {
                    OutValue = FieldValue->AsString();
                }
                else if (FieldValue->Type == EJson::Object)
                {
                    TSharedPtr<FJsonObject> FieldObject = FieldValue->AsObject();
                    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutValue);
                    FJsonSerializer::Serialize(FieldObject.ToSharedRef(), Writer);
                }
                else if (FieldValue->Type == EJson::Number)
                {
                    double NumberValue = FieldValue->AsNumber();
                    // Check if the number is effectively an integer.
                    if (FMath::IsNearlyEqual(NumberValue, FMath::FloorToDouble(NumberValue)))
                    {
                        OutValue = FString::FromInt(static_cast<int32>(NumberValue));
                    }
                    else
                    {
                        OutValue = FString::SanitizeFloat(NumberValue);
                    }
                }
                else if (FieldValue->Type == EJson::Boolean)
                {
                    OutValue = FieldValue->AsBool() ? TEXT("true") : TEXT("false");
                }
                else if (FieldValue->Type == EJson::Array)
                {
                    TArray<TSharedPtr<FJsonValue>> ArrayValues = FieldValue->AsArray();
                    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutValue);
                    FJsonSerializer::Serialize(ArrayValues, Writer);
                }
                else
                {
                    OutValue = TEXT("Unsupported field type");
                }
                return true;
            }
            else if (FieldValue->Type == EJson::Object)
            {
                // We have a nested object; continue down the chain.
                bFound = FindFieldInJsonObjectRecursive(FieldValue->AsObject(), FieldParts, PartIndex + 1, OutValue, bSearchAll);
                // If found, return immediately.
                if (bFound)
                {
                    return true;
                }
            }
        }
    }

    // If not found at this level or the chain didnt match,
    // search all nested objects and arrays (if search-all is enabled)
    if (bSearchAll)
    {
        for (const auto& Pair : JsonObject->Values)
        {
            TSharedPtr<FJsonValue> Value = Pair.Value;
            if (Value->Type == EJson::Object)
            {
                if (FindFieldInJsonObjectRecursive(Value->AsObject(), FieldParts, PartIndex, OutValue, bSearchAll))
                {
                    return true;
                }
            }
            else if (Value->Type == EJson::Array)
            {
                TArray<TSharedPtr<FJsonValue>> ArrayValues = Value->AsArray();
                for (const TSharedPtr<FJsonValue>& ArrayElement : ArrayValues)
                {
                    if (ArrayElement->Type == EJson::Object)
                    {
                        if (FindFieldInJsonObjectRecursive(ArrayElement->AsObject(), FieldParts, PartIndex, OutValue, bSearchAll))
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}




// Custom thunk implementation for ConvertStructToJsonString
DEFINE_FUNCTION(UAIITK_NTK_JsonHttpFunctionLibrary::execConvertStructToJsonString)
{
    Stack.MostRecentProperty = nullptr;
    Stack.StepCompiledIn<FStructProperty>(nullptr);
    FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
    void* StructPtr = Stack.MostRecentPropertyAddress;
    P_FINISH;

    if (!StructProperty || !StructPtr)
    {
        UE_LOG(NTKLogJsonHttp, Warning, TEXT("Invalid struct or address"));
        *(FString*)RESULT_PARAM = TEXT("{}");
        return;
    }

    TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    bool bSuccess = FJsonObjectConverter::UStructToJsonObject(StructProperty->Struct, StructPtr, JsonObject, 0, 0);

    if (!bSuccess)
    {
        UE_LOG(NTKLogJsonHttp, Warning, TEXT("Failed to serialize struct to JSON"));
        *(FString*)RESULT_PARAM = TEXT("{}");
        return;
    }

    FString OutputString;
    TSharedRef<TJsonWriter<TCHAR, TRoundingJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TRoundingJsonPrintPolicy<TCHAR>>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject, Writer);

    OutputString = OutputString.Replace(TEXT("\\u0000"), TEXT(""));

    *(FString*)RESULT_PARAM = OutputString;
}

// Custom thunk implementation for ConvertStructToJsonStringWithExclusions
DEFINE_FUNCTION(UAIITK_NTK_JsonHttpFunctionLibrary::execConvertStructToJsonStringWithExclusions)
{
    Stack.MostRecentProperty = nullptr;
    Stack.StepCompiledIn<FStructProperty>(nullptr);
    FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
    void* StructPtr = Stack.MostRecentPropertyAddress;
    P_GET_TARRAY(FString, FieldsToExclude);
    P_FINISH;

    if (!StructProperty || !StructPtr)
    {
        UE_LOG(NTKLogJsonHttp, Warning, TEXT("Invalid struct or address"));
        *(FString*)RESULT_PARAM = TEXT("{}");
        return;
    }

    TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    FJsonObjectConverter::UStructToJsonObject(StructProperty->Struct, StructPtr, JsonObject, 0, 0);

    for (const FString& FieldPath : FieldsToExclude)
    {
        TArray<FString> FieldNames;
        FieldPath.ParseIntoArray(FieldNames, TEXT("."), true);

        if (FieldNames.Num() > 0)
        {
            RemoveNestedField(JsonObject, FieldNames);
        }
    }

    FString OutputString;
    TSharedRef<TJsonWriter<TCHAR, TRoundingJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TRoundingJsonPrintPolicy<TCHAR>>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject, Writer);

    OutputString = OutputString.Replace(TEXT("\\u0000"), TEXT(""));

    *(FString*)RESULT_PARAM = OutputString;
}

void UAIITK_NTK_JsonHttpFunctionLibrary::RemoveNestedField(TSharedPtr<FJsonObject> JsonObject, const TArray<FString>& FieldNames)
{
    if (!JsonObject.IsValid() || FieldNames.Num() == 0)
    {
        return;
    }

    TSharedPtr<FJsonObject> CurrentObject = JsonObject;
    for (int32 i = 0; i < FieldNames.Num() - 1; ++i)
    {
        if (CurrentObject->HasField(FieldNames[i]))
        {
            TSharedPtr<FJsonObject> NestedObject = CurrentObject->GetObjectField(FieldNames[i]);
            if (!NestedObject.IsValid())
            {
                return;
            }
            CurrentObject = NestedObject;
        }
        else
        {
            return;
        }
    }

    CurrentObject->RemoveField(FieldNames.Last());
}

DEFINE_FUNCTION(UAIITK_NTK_JsonHttpFunctionLibrary::execConvertJsonArrayStringToArray)
{
    // Retrieve the JSON string parameter
    P_GET_PROPERTY(FStrProperty, JsonString);

    // Retrieve the OutArray parameter (any array type)
    Stack.StepCompiledIn<FArrayProperty>(nullptr);
    FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
    void* OutArrayPtr = Stack.MostRecentPropertyAddress;

    // Retrieve the bSuccess parameter
    P_GET_UBOOL_REF(bSuccess);

    P_FINISH;

    // Initialize bSuccess to false
    bSuccess = false;

    // Validate the array property and pointer
    if (!ArrayProperty || !OutArrayPtr)
    {
        UE_LOG(NTKLogJsonHttp, Error, TEXT("Invalid array parameter."));
        return;
    }

    // Parse the JSON string into a JSON array
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    TArray<TSharedPtr<FJsonValue>> JsonArray;
    if (!FJsonSerializer::Deserialize(Reader, JsonArray) || JsonArray.Num() == 0)
    {
        UE_LOG(NTKLogJsonHttp, Error, TEXT("Failed to parse JSON array string."));
        return;
    }

    // Get the inner property of the array (element type)
    FProperty* InnerProperty = ArrayProperty->Inner;

    // Helper to manipulate the array
    FScriptArrayHelper ArrayHelper(ArrayProperty, OutArrayPtr);

    // Clear the existing array
    ArrayHelper.EmptyValues();

    // Iterate over the JSON array and convert each element
    for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
    {
        void* ValuePtr = FMemory::Malloc(InnerProperty->GetSize(), InnerProperty->GetMinAlignment());
        InnerProperty->InitializeValue(ValuePtr);

        bool bElementSuccess = false;

        // Handle different property types
        if (FStrProperty* StringProperty = CastField<FStrProperty>(InnerProperty))
        {
            FString StringValue;
            if (JsonValue->TryGetString(StringValue))
            {
                StringProperty->SetPropertyValue(ValuePtr, StringValue);
                bElementSuccess = true;
            }
        }
        else if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(InnerProperty))
        {
            double NumberValue;
            if (JsonValue->TryGetNumber(NumberValue))
            {
                FloatProperty->SetFloatingPointPropertyValue(ValuePtr, NumberValue);
                bElementSuccess = true;
            }
        }
        else if (FIntProperty* IntProperty = CastField<FIntProperty>(InnerProperty))
        {
            double NumberValue;
            if (JsonValue->TryGetNumber(NumberValue))
            {
                IntProperty->SetPropertyValue(ValuePtr, static_cast<int32>(NumberValue));
                bElementSuccess = true;
            }
        }
        else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(InnerProperty))
        {
            bool BoolValue;
            if (JsonValue->TryGetBool(BoolValue))
            {
                BoolProperty->SetPropertyValue(ValuePtr, BoolValue);
                bElementSuccess = true;
            }
        }
        else if (FStructProperty* StructProperty = CastField<FStructProperty>(InnerProperty))
        {
            if (JsonValue->Type == EJson::Object)
            {
                TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
                if (JsonObject.IsValid())
                {
                    // Convert JSON object to struct
                    if (FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), StructProperty->Struct, ValuePtr, 0, 0))
                    {
                        bElementSuccess = true;
                    }
                }
            }
        }
        else
        {
            // Unsupported property type
            UE_LOG(NTKLogJsonHttp, Warning, TEXT("Unsupported property type in array conversion."));
        }

        if (bElementSuccess)
        {
            // Add the value to the array
            int32 NewIndex = ArrayHelper.AddValue();
            void* Dest = ArrayHelper.GetRawPtr(NewIndex);
            InnerProperty->CopyCompleteValue(Dest, ValuePtr);
        }
        else
        {
            UE_LOG(NTKLogJsonHttp, Warning, TEXT("Failed to convert JSON value to property type."));
        }

        // Clean up
        InnerProperty->DestroyValue(ValuePtr);
        FMemory::Free(ValuePtr);
    }

    // Set bSuccess to true if elements were added
    bSuccess = ArrayHelper.Num() > 0;
}