// JsonHttpFunctionLibrary.cpp

#include "JsonHttpFunctionLibrary.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Policies/JsonPrintPolicy.h"


// Define the logging category
DEFINE_LOG_CATEGORY(LogJsonHttp);

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

UJsonHttpToolkit* UJsonHttpFunctionLibrary::SendJsonHttpRequest(const FString& URL, const FString& JsonData, const FString& ApiKey,
    const FOnHttpRequestError& OnError,
    const FOnHttpRequestProgress& OnProgress,
    const FOnHttpRequestHeaderReceived& OnHeaderReceived,
    const FOnHttpRequestComplete& OnComplete)
{
    // Create an instance of the JsonHttpToolkit
    UJsonHttpToolkit* ToolkitInstance = NewObject<UJsonHttpToolkit>();

    // Send the HTTP request with the provided URL, JsonData, ApiKey, and bind delegates
    ToolkitInstance->SendHttpRequest(URL, JsonData, ApiKey, OnError, OnProgress, OnHeaderReceived, OnComplete);

    // Return the toolkit instance so the user can subscribe to the delegates
    return ToolkitInstance;
}

void UJsonHttpFunctionLibrary::SerializeObjectToJsonString(const UObject* InObject, FString& OutJsonString, bool& bSuccess)
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

void UJsonHttpFunctionLibrary::DeserializeJsonStringToObject(const FString& JsonString, UObject* OutObject, bool& bSuccess)
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
DEFINE_FUNCTION(UJsonHttpFunctionLibrary::execConvertJsonStringToStruct)
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
        UE_LOG(LogJsonHttp, Error, TEXT("Invalid struct or struct pointer address."));
        bSuccess = false;
        return;
    }

    if (JsonString.IsEmpty())
    {
        UE_LOG(LogJsonHttp, Warning, TEXT("JSON string is empty."));
        bSuccess = false;
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogJsonHttp, Warning, TEXT("Failed to parse JSON string."));
        bSuccess = false;
        return;
    }

    bool bDeserializationSuccess = FJsonObjectConverter::JsonObjectToUStruct(
        JsonObject.ToSharedRef(),
        StructProperty->Struct,
        StructPtr,
        0,
        0,
        true
    );

    if (!bDeserializationSuccess)
    {
        UE_LOG(LogJsonHttp, Error, TEXT("Failed to deserialize JSON into UStruct."));
        bSuccess = false;
        return;
    }

    bSuccess = true;

    P_NATIVE_END;
}

void UJsonHttpFunctionLibrary::GetFieldFromJsonString(const FString& JsonString, const FString& FieldName, FString& OutValue, bool bSearchAll, bool& bSuccess)
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

bool UJsonHttpFunctionLibrary::FindFieldInJsonObject(TSharedPtr<FJsonObject> JsonObject, const FString& FieldName, FString& OutValue, bool bSearchAll)
{
    TArray<FString> FieldParts;
    FieldName.ParseIntoArray(FieldParts, TEXT("."), true);

    return FindFieldInJsonObjectRecursive(JsonObject, FieldParts, 0, OutValue, bSearchAll);
}

bool UJsonHttpFunctionLibrary::FindFieldInJsonObjectRecursive(TSharedPtr<FJsonObject> JsonObject, const TArray<FString>& FieldParts, int32 PartIndex, FString& OutValue, bool bSearchAll)
{
    if (!JsonObject.IsValid())
    {
        return false;
    }

    FString CurrentField = FieldParts[PartIndex];

    if (JsonObject->HasField(CurrentField))
    {
        TSharedPtr<FJsonValue> FieldValue = JsonObject->TryGetField(CurrentField);
        if (FieldValue.IsValid())
        {
            if (PartIndex == FieldParts.Num() - 1)
            {
                // This is the final field, retrieve the value
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
                    OutValue = FString::SanitizeFloat(FieldValue->AsNumber());
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
            else
            {
                // Not the final field, recurse into the nested object
                if (FieldValue->Type == EJson::Object)
                {
                    TSharedPtr<FJsonObject> NestedObject = FieldValue->AsObject();
                    return FindFieldInJsonObjectRecursive(NestedObject, FieldParts, PartIndex + 1, OutValue, bSearchAll);
                }
                else
                {
                    // The field is not an object, cannot proceed further
                    return false;
                }
            }
        }
        else
        {
            return false;
        }
    }
    else if (bSearchAll)
    {
        // Search all nested objects and arrays
        auto LocalValues = JsonObject->Values;
        for (const auto& Pair : LocalValues)
        {
            TSharedPtr<FJsonValue> Value = Pair.Value;

            if (Value->Type == EJson::Object)
            {
                TSharedPtr<FJsonObject> NestedObject = Value->AsObject();
                if (FindFieldInJsonObjectRecursive(NestedObject, FieldParts, PartIndex, OutValue, bSearchAll))
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
                        TSharedPtr<FJsonObject> ArrayObject = ArrayElement->AsObject();
                        if (FindFieldInJsonObjectRecursive(ArrayObject, FieldParts, PartIndex, OutValue, bSearchAll))
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
DEFINE_FUNCTION(UJsonHttpFunctionLibrary::execConvertStructToJsonString)
{
    Stack.MostRecentProperty = nullptr;
    Stack.StepCompiledIn<FStructProperty>(nullptr);
    FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
    void* StructPtr = Stack.MostRecentPropertyAddress;
    P_FINISH;

    if (!StructProperty || !StructPtr)
    {
        UE_LOG(LogJsonHttp, Warning, TEXT("Invalid struct or address"));
        *(FString*)RESULT_PARAM = TEXT("{}");
        return;
    }

    TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    bool bSuccess = FJsonObjectConverter::UStructToJsonObject(StructProperty->Struct, StructPtr, JsonObject, 0, 0);

    if (!bSuccess)
    {
        UE_LOG(LogJsonHttp, Warning, TEXT("Failed to serialize struct to JSON"));
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
DEFINE_FUNCTION(UJsonHttpFunctionLibrary::execConvertStructToJsonStringWithExclusions)
{
    Stack.MostRecentProperty = nullptr;
    Stack.StepCompiledIn<FStructProperty>(nullptr);
    FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
    void* StructPtr = Stack.MostRecentPropertyAddress;
    P_GET_TARRAY(FString, FieldsToExclude);
    P_FINISH;

    if (!StructProperty || !StructPtr)
    {
        UE_LOG(LogJsonHttp, Warning, TEXT("Invalid struct or address"));
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

void UJsonHttpFunctionLibrary::RemoveNestedField(TSharedPtr<FJsonObject> JsonObject, const TArray<FString>& FieldNames)
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

DEFINE_FUNCTION(UJsonHttpFunctionLibrary::execConvertJsonArrayStringToArray)
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
        UE_LOG(LogJsonHttp, Error, TEXT("Invalid array parameter."));
        return;
    }

    // Parse the JSON string into a JSON array
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    TArray<TSharedPtr<FJsonValue>> JsonArray;
    if (!FJsonSerializer::Deserialize(Reader, JsonArray) || JsonArray.Num() == 0)
    {
        UE_LOG(LogJsonHttp, Error, TEXT("Failed to parse JSON array string."));
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
            UE_LOG(LogJsonHttp, Warning, TEXT("Unsupported property type in array conversion."));
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
            UE_LOG(LogJsonHttp, Warning, TEXT("Failed to convert JSON value to property type."));
        }

        // Clean up
        InnerProperty->DestroyValue(ValuePtr);
        FMemory::Free(ValuePtr);
    }

    // Set bSuccess to true if elements were added
    bSuccess = ArrayHelper.Num() > 0;
}