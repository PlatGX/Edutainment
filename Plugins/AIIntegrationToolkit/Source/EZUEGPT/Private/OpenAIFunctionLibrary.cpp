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



// =================== OpenAIFunctionLibrary.cpp ===================

#include "OpenAIFunctionLibrary.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Base64.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture.h"
#include "Modules/ModuleManager.h"
#include "Streaming/TextureMipDataProvider.h"
#include "TextureResource.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Math/Color.h"
#include "Containers/Array.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Animation/AnimationAsset.h"



DEFINE_LOG_CATEGORY(AIITKLog);


// INIT

UOpenAIFunctionLibrary::UOpenAIFunctionLibrary()
{
    //Constructor
}


void UOpenAIFunctionLibrary::CallFunctionByName(UObject* Target, const FString& FunctionName)
{
    if (!Target)
    {
        UE_LOG(AIITKLog, Warning, TEXT("CallFunctionByName: Target is null."));
        return;
    }

    if (FunctionName.IsEmpty())
    {
        UE_LOG(AIITKLog, Warning, TEXT("CallFunctionByName: FunctionName is empty."));
        return;
    }

    FName FunctionFName(*FunctionName);
    UFunction* Function = Target->FindFunction(FunctionFName);

    if (!Function)
    {
        UE_LOG(AIITKLog, Warning, TEXT("CallFunctionByName: Function '%s' not found in Target '%s'."), *FunctionName, *Target->GetName());
        return;
    }

    // Ensure the function has no parameters
    if (Function->NumParms > 0)
    {
        UE_LOG(AIITKLog, Warning, TEXT("CallFunctionByName: Function '%s' has parameters. This method only supports functions with no parameters."), *FunctionName);
        return;
    }

    // Call the function
    Target->ProcessEvent(Function, nullptr);
}

// SERILIZATION/JSON CREATION



bool AddFieldIfUnique(TArray<FString>& FieldNames, const FString& FieldName, TSharedPtr<FJsonObject> PropertiesObject, TSharedPtr<FJsonValue> Value, TArray<TSharedPtr<FJsonValue>>& RequiredFields, bool bIsRequired)
{
    if (FieldNames.Contains(FieldName))
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Duplicate field name: %s"), *FieldName);
        return false;
    }
    else
    {
        FieldNames.Add(FieldName);
        if (Value->AsObject()->HasField(TEXT("RequiredField")))
        {
            Value->AsObject()->RemoveField(TEXT("RequiredField"));
        }
        PropertiesObject->SetField(FieldName, Value);
        if (bIsRequired)
        {
            RequiredFields.Add(MakeShareable(new FJsonValueString(FieldName)));
        }
        return true;
    }
}

FString SetItemConstraints(TSharedPtr<FJsonObject>& JsonObject, const FString& Description, FVector2D ArraySizeConstraints)
{
    FString ModifiedDescription = Description;
    FString ConstraintsText = "";

    if (ArraySizeConstraints.X > 0 && ArraySizeConstraints.Y > 0)
    {
        if (ArraySizeConstraints.X > ArraySizeConstraints.Y)
        {
            ConstraintsText = FString::Printf(TEXT("(Min items: %d)"), static_cast<int32>(ArraySizeConstraints.X));
            JsonObject->SetNumberField(TEXT("minItems"), static_cast<int32>(ArraySizeConstraints.X));
            UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: Invalid range specified: min (%d) is greater than max (%d). maxItems has been omitted."), static_cast<int32>(ArraySizeConstraints.X), static_cast<int32>(ArraySizeConstraints.Y));
        }
        else
        {
            ConstraintsText = FString::Printf(TEXT("(Min items: %d, Max items: %d)"), static_cast<int32>(ArraySizeConstraints.X), static_cast<int32>(ArraySizeConstraints.Y));
            JsonObject->SetNumberField(TEXT("minItems"), static_cast<int32>(ArraySizeConstraints.X));
            JsonObject->SetNumberField(TEXT("maxItems"), static_cast<int32>(ArraySizeConstraints.Y));
        }
    }
    else if (ArraySizeConstraints.X > 0)
    {
        ConstraintsText = FString::Printf(TEXT("(Min items: %d)"), static_cast<int32>(ArraySizeConstraints.X));
        JsonObject->SetNumberField(TEXT("minItems"), static_cast<int32>(ArraySizeConstraints.X));
    }
    else if (ArraySizeConstraints.Y > 0)
    {
        ConstraintsText = FString::Printf(TEXT("(Max items: %d)"), static_cast<int32>(ArraySizeConstraints.Y));
        JsonObject->SetNumberField(TEXT("maxItems"), static_cast<int32>(ArraySizeConstraints.Y));
    }

    if (!ConstraintsText.IsEmpty())
    {
        ModifiedDescription = FString::Printf(TEXT("%s %s"), *Description, *ConstraintsText);
    }

    return ModifiedDescription;
}

FString UOpenAIFunctionLibrary::CreateJSONSchema(EGPTFieldTypes FieldType, const FString& FieldName, const FString& Description, const FString& InputObject, bool bIsArray, bool bRequired, FVector2D ArraySizeConstraints)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> InnerObject;

    FString ModifiedDescription = Description;  // Set to the original description by default

    if (bIsArray)
    {
        InnerObject = MakeShareable(new FJsonObject);
        JsonObject->SetStringField(TEXT("type"), TEXT("array"));
        ModifiedDescription = SetItemConstraints(JsonObject, Description, ArraySizeConstraints);
        JsonObject->SetObjectField(TEXT("items"), InnerObject);
    }
    else
    {
        InnerObject = JsonObject;
    }

    InnerObject->SetStringField(TEXT("description"), ModifiedDescription);

    switch (FieldType)
    {
    case EGPTFieldTypes::String:
        InnerObject->SetStringField(TEXT("type"), TEXT("string"));
        break;
    case EGPTFieldTypes::Number:
        InnerObject->SetStringField(TEXT("type"), TEXT("number"));
        break;
    case EGPTFieldTypes::Boolean:
        InnerObject->SetStringField(TEXT("type"), TEXT("boolean"));
        break;
    case EGPTFieldTypes::Enum:
        InnerObject->SetStringField(TEXT("type"), TEXT("string"));
        break;
    default:
        break;
    }

    if (bRequired)
    {
        JsonObject->SetBoolField(TEXT("RequiredField"), true);
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    return FString::Printf(TEXT("{ \"%s\": %s }"), *FieldName, *OutputString);
}

FString UOpenAIFunctionLibrary::CreateJSONSchemaForObjectArray(const FString& FieldName, const FString& Description, const TArray<FString>& InputObject, bool bIsArray, bool bRequired, FVector2D ArraySizeConstraints)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> InnerObject;

    FString ModifiedDescription = Description;  // Set to the original description by default

    if (bIsArray)
    {
        InnerObject = MakeShareable(new FJsonObject);
        JsonObject->SetStringField(TEXT("type"), TEXT("array"));
        ModifiedDescription = SetItemConstraints(JsonObject, Description, ArraySizeConstraints);
        JsonObject->SetObjectField(TEXT("items"), InnerObject);
    }
    else
    {
        InnerObject = JsonObject;
    }

    InnerObject->SetStringField(TEXT("description"), ModifiedDescription);
    InnerObject->SetStringField(TEXT("type"), TEXT("object"));

    TSharedPtr<FJsonObject> PropertiesObject = MakeShareable(new FJsonObject);
    TArray<TSharedPtr<FJsonValue>> RequiredFields;
    for (const FString& Property : InputObject)
    {
        FString ValidJsonString;
        if (Property.StartsWith("{") && Property.EndsWith("}"))
        {
            ValidJsonString = Property;
        }
        else
        {
            ValidJsonString = FString("{") + Property + FString("}");
        }

        TSharedPtr<FJsonObject> FullPropertySchema;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ValidJsonString);
        if (FJsonSerializer::Deserialize(Reader, FullPropertySchema) && FullPropertySchema.IsValid())
        {
            for (const auto& Pair : FullPropertySchema->Values)
            {
                if (Pair.Value->AsObject()->HasField(TEXT("RequiredField")))
                {
                    Pair.Value->AsObject()->RemoveField(TEXT("RequiredField"));
                }
                PropertiesObject->SetField(Pair.Key, Pair.Value);
                if (bRequired)
                {
                    RequiredFields.Add(MakeShareable(new FJsonValueString(Pair.Key)));
                }
            }
        }
        else
        {
            UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: Failed to deserialize a property: %s"), *ValidJsonString);
        }
    }

    InnerObject->SetObjectField(TEXT("properties"), PropertiesObject);
    if (RequiredFields.Num() > 0)
    {
        JsonObject->SetArrayField(TEXT("required"), RequiredFields);
    }

    if (bRequired)
    {
        JsonObject->SetBoolField(TEXT("RequiredField"), true);
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    return FString::Printf(TEXT("\"%s\": %s"), *FieldName, *OutputString);
}

FString UOpenAIFunctionLibrary::CreateJSONSchemaForEnum(const FString& FieldName, const FString& Description, EJsonEnumType EnumType, const TArray<FString>& EnumValues, bool bIsArray, bool bRequired, FVector2D ArraySizeConstraints)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    TArray<TSharedPtr<FJsonValue>> ProcessedEnumValues;
    for (const FString& Item : EnumValues)
    {
        if (EnumType == EJsonEnumType::String)
        {
            ProcessedEnumValues.Add(MakeShareable(new FJsonValueString(Item)));
        }
        else if (EnumType == EJsonEnumType::Number)
        {
            double NumberValue;
            if (Item.IsNumeric())
            {
                LexFromString(NumberValue, *Item);
                ProcessedEnumValues.Add(MakeShareable(new FJsonValueNumber(NumberValue)));
            }
            else
            {
                UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: Invalid numeric value for enum item: %s"), *Item);
            }
        }
    }

    FString ModifiedDescription = Description; // Set default to provided description
    JsonObject->SetStringField(TEXT("description"), ModifiedDescription);

    if (bIsArray)
    {
        JsonObject->SetStringField(TEXT("type"), TEXT("array"));

        TSharedPtr<FJsonObject> ItemsObject = MakeShareable(new FJsonObject);
        ItemsObject->SetStringField(TEXT("type"), EnumType == EJsonEnumType::String ? TEXT("string") : TEXT("number"));
        ItemsObject->SetArrayField(TEXT("enum"), ProcessedEnumValues);
        JsonObject->SetObjectField(TEXT("items"), ItemsObject);
        ModifiedDescription = SetItemConstraints(JsonObject, Description, ArraySizeConstraints);
    }
    else
    {
        JsonObject->SetStringField(TEXT("type"), EnumType == EJsonEnumType::String ? TEXT("string") : TEXT("number"));
        JsonObject->SetArrayField("enum", ProcessedEnumValues);
    }

    if (bRequired)
    {
        JsonObject->SetBoolField(TEXT("RequiredField"), true);
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    return FString::Printf(TEXT("\"%s\": %s"), *FieldName, *OutputString);
}

FString UOpenAIFunctionLibrary::SerializeField(const EGPTFieldTypes& FieldType, const EJsonEnumType EnumFieldType, const TArray<FString> EnumValues, const FString& Description, bool bIsRequired, bool bIsArray, FVector2D ArraySizeConstraints, TSharedPtr<FJsonObject> NestedProperties)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    FString ModifiedDescription = Description;
    if (bIsArray)
    {
        FString ConstraintsText = "";

        if (ArraySizeConstraints.X != 0 && ArraySizeConstraints.Y != 0)
        {
            if (ArraySizeConstraints.X > ArraySizeConstraints.Y)
            {
                ConstraintsText = FString::Printf(TEXT("(Min items: %d)"), static_cast<int32>(ArraySizeConstraints.X));
                UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: Invalid range specified: min (%d) is greater than max (%d). maxItems has been omitted."), static_cast<int32>(ArraySizeConstraints.X), static_cast<int32>(ArraySizeConstraints.Y));
            }
            else
            {
                ConstraintsText = FString::Printf(TEXT("(Min items: %d, Max items: %d)"), static_cast<int32>(ArraySizeConstraints.X), static_cast<int32>(ArraySizeConstraints.Y));
            }
        }

        if (!ConstraintsText.IsEmpty())
        {
            ModifiedDescription = FString::Printf(TEXT("%s %s"), *Description, *ConstraintsText);
        }
    }

    JsonObject->SetStringField(TEXT("description"), ModifiedDescription);

    if (bIsArray)
    {
        JsonObject->SetStringField(TEXT("type"), "array");
        TSharedPtr<FJsonObject> ItemsObject = MakeShareable(new FJsonObject);
        ItemsObject->SetStringField(TEXT("type"), FieldTypeEnumToString(FieldType));

        if (ArraySizeConstraints.X > 0 && ArraySizeConstraints.Y > 0)
        {
            if (ArraySizeConstraints.X > ArraySizeConstraints.Y)
            {
                JsonObject->SetNumberField(TEXT("minItems"), ArraySizeConstraints.X);
            }
            else
            {
                JsonObject->SetNumberField(TEXT("minItems"), ArraySizeConstraints.X);
                JsonObject->SetNumberField(TEXT("maxItems"), ArraySizeConstraints.Y);
            }
        }

        JsonObject->SetObjectField(TEXT("items"), ItemsObject);
    }
    else
    {
        if (FieldType == EGPTFieldTypes::Enum)
        {
            JsonObject->SetStringField(TEXT("type"), "string");

            TArray<TSharedPtr<FJsonValue>> JsonEnumValues;
            for (const FString& Value : EnumValues)
            {
                JsonEnumValues.Add(MakeShareable(new FJsonValueString(Value)));
            }
            JsonObject->SetArrayField(TEXT("enum"), JsonEnumValues);
        }
        else
        {
            JsonObject->SetStringField(TEXT("type"), FieldTypeEnumToString(FieldType));
        }
    }

    if (FieldType == EGPTFieldTypes::Object && NestedProperties.IsValid())
    {
        JsonObject->SetObjectField(TEXT("properties"), NestedProperties);
    }

    if (FieldType == EGPTFieldTypes::Object && bIsRequired)
    {
        TArray<TSharedPtr<FJsonValue>> RequiredArray;
        for (const auto& Pair : NestedProperties->Values)
        {
            RequiredArray.Add(MakeShareable(new FJsonValueString(Pair.Key)));
        }
        JsonObject->SetArrayField(TEXT("required"), RequiredArray);
    }

    if (bIsRequired)
    {
        JsonObject->SetBoolField(TEXT("RequiredField"), true);
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    return OutputString;
}

FString UOpenAIFunctionLibrary::SerializeObjectLevel3(const FObjectLevel3& Obj)
{
    TSharedPtr<FJsonObject> NestedProperties = MakeShareable(new FJsonObject);

    TSharedPtr<FJsonObject> DeserializedObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Obj.InputObject[0]);
    if (FJsonSerializer::Deserialize(Reader, DeserializedObject) && DeserializedObject.IsValid())
    {
        NestedProperties = DeserializedObject;
    }

    return SerializeField(Obj.FieldType, Obj.EnumData.EnumType, Obj.EnumData.EnumValues, Obj.Description, Obj.bIsRequired, Obj.bIsArray, Obj.ArraySizeConstraints, NestedProperties);
}

FString UOpenAIFunctionLibrary::SerializeObjectLevel2(const FObjectLevel2& Obj)
{
    TSharedPtr<FJsonObject> NestedProperties = MakeShareable(new FJsonObject);
    for (const FObjectLevel3& Object : Obj.InputObject)
    {
        FString SerializedObject = SerializeObjectLevel3(Object);
        TSharedPtr<FJsonObject> DeserializedObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(SerializedObject);
        if (FJsonSerializer::Deserialize(Reader, DeserializedObject) && DeserializedObject.IsValid())
        {
            NestedProperties->SetObjectField(Object.FieldName, DeserializedObject);
        }
    }
    return SerializeField(Obj.FieldType, Obj.EnumData.EnumType, Obj.EnumData.EnumValues, Obj.Description, Obj.bIsRequired, Obj.bIsArray, Obj.ArraySizeConstraints, NestedProperties);
}

FString UOpenAIFunctionLibrary::SerializeObjectLevel1(const FObjectLevel1& Obj)
{
    TSharedPtr<FJsonObject> NestedProperties = MakeShareable(new FJsonObject);
    for (const FObjectLevel2& Object : Obj.InputObject)
    {
        FString SerializedObject = SerializeObjectLevel2(Object);
        TSharedPtr<FJsonObject> DeserializedObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(SerializedObject);
        if (FJsonSerializer::Deserialize(Reader, DeserializedObject) && DeserializedObject.IsValid())
        {
            NestedProperties->SetObjectField(Object.FieldName, DeserializedObject);
        }
    }
    return SerializeField(Obj.FieldType, Obj.EnumData.EnumType, Obj.EnumData.EnumValues, Obj.Description, Obj.bIsRequired, Obj.bIsArray, Obj.ArraySizeConstraints, NestedProperties);
}

FString UOpenAIFunctionLibrary::SerializeBaseObject(const FBaseObject& Obj)
{
    TSharedPtr<FJsonObject> NestedProperties = MakeShareable(new FJsonObject);
    for (const FObjectLevel1& Object : Obj.InputObject)
    {
        FString SerializedObject = SerializeObjectLevel1(Object);
        TSharedPtr<FJsonObject> DeserializedObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(SerializedObject);
        if (FJsonSerializer::Deserialize(Reader, DeserializedObject) && DeserializedObject.IsValid())
        {
            NestedProperties->SetObjectField(Object.FieldName, DeserializedObject);
        }
    }
    return SerializeField(Obj.FieldType, Obj.EnumData.EnumType, Obj.EnumData.EnumValues, Obj.Description, Obj.bIsRequired, Obj.bIsArray, Obj.ArraySizeConstraints, NestedProperties);
}

FString UOpenAIFunctionLibrary::SerializeGPTChatFunction(const FGPTChatFunction& Function)
{
    TSharedPtr<FJsonObject> JsonFunctionObject = MakeShareable(new FJsonObject);
    JsonFunctionObject->SetStringField(TEXT("name"), Function.Name);
    JsonFunctionObject->SetStringField(TEXT("description"), Function.Description);

    TSharedPtr<FJsonObject> ParametersObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> PropertiesObject = MakeShareable(new FJsonObject);
    TArray<FString> FieldNames;
    TArray<TSharedPtr<FJsonValue>> RequiredFields;

    for (const FString& RawJson : Function.RawJsonParameters)
    {
        TSharedPtr<FJsonObject> RawParametersObject;
        TSharedRef<TJsonReader<>> InitialReader = TJsonReaderFactory<>::Create(RawJson);
        bool bIsValidJson = FJsonSerializer::Deserialize(InitialReader, RawParametersObject) && RawParametersObject.IsValid();

        FString ValidJsonString;
        if (!bIsValidJson)
        {
            ValidJsonString = FString("{") + RawJson + FString("}");
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ValidJsonString);
            bIsValidJson = FJsonSerializer::Deserialize(Reader, RawParametersObject) && RawParametersObject.IsValid();
        }
        else
        {
            ValidJsonString = RawJson;
        }

        if (bIsValidJson)
        {
            for (const auto& Pair : RawParametersObject->Values)
            {
                bool bIsRawJsonParamRequired = Pair.Value->AsObject()->HasField(TEXT("RequiredField")) && Pair.Value->AsObject()->GetBoolField(TEXT("RequiredField"));
                AddFieldIfUnique(FieldNames, Pair.Key, PropertiesObject, Pair.Value, RequiredFields, bIsRawJsonParamRequired);
            }
        }
        else
        {
            UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: RawJsonParameters is not a valid JSON: %s"), *RawJson);
        }
    }

    for (const FBaseObject& BaseObject : Function.Parameters)
    {
        FString SerializedObject = SerializeBaseObject(BaseObject);
        TSharedPtr<FJsonObject> DeserializedObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(SerializedObject);
        if (FJsonSerializer::Deserialize(Reader, DeserializedObject) && DeserializedObject.IsValid())
        {
            TSharedPtr<FJsonValueObject> DeserializedValue = MakeShareable(new FJsonValueObject(DeserializedObject));
            AddFieldIfUnique(FieldNames, BaseObject.FieldName, PropertiesObject, DeserializedValue, RequiredFields, BaseObject.bIsRequired);
        }
    }

    if (FieldNames.Num() > 0)
    {
        ParametersObject->SetStringField(TEXT("type"), TEXT("object"));
        ParametersObject->SetObjectField(TEXT("properties"), PropertiesObject);
        if (RequiredFields.Num() > 0)
        {
            ParametersObject->SetArrayField(TEXT("required"), RequiredFields);
        }

        JsonFunctionObject->SetObjectField(TEXT("parameters"), ParametersObject);
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonFunctionObject.ToSharedRef(), Writer);

    return OutputString;
}

TArray<TSharedPtr<FJsonValue>> UOpenAIFunctionLibrary::CreateFunctionsArrayForAPI(const TArray<FGPTChatFunction>& Functions)
{
    TArray<TSharedPtr<FJsonValue>> JsonFunctionsArray;

    for (const FGPTChatFunction& Function : Functions)
    {
        // Serialize the FGPTChatFunction into a JSON string
        FString SerializedFunction = SerializeGPTChatFunction(Function);

        // Deserialize the JSON string back into a JSON object
        TSharedPtr<FJsonObject> DeserializedFunction;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(SerializedFunction);
        if (FJsonSerializer::Deserialize(Reader, DeserializedFunction) && DeserializedFunction.IsValid())
        {
            // Create a new JSON object to encapsulate the function within a 'type': 'function' structure
            TSharedPtr<FJsonObject> FunctionWithType = MakeShareable(new FJsonObject);
            FunctionWithType->SetStringField(TEXT("type"), TEXT("function"));
            FunctionWithType->SetObjectField(TEXT("function"), DeserializedFunction);

            // Add the new function object to the array
            JsonFunctionsArray.Add(MakeShareable(new FJsonValueObject(FunctionWithType)));
        }
        else
        {
            UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: Failed to deserialize the serialized FGPTChatFunction: %s"), *SerializedFunction);
        }
    }

    return JsonFunctionsArray;
}



// PARSING

FString UOpenAIFunctionLibrary::HandleJsonArray(const TArray<TSharedPtr<FJsonValue>>& JsonArray)
{
    TArray<FString> ArrayStrings;
    for (const TSharedPtr<FJsonValue>& Value : JsonArray)
    {
        switch (Value->Type)
        {
        case EJson::String:
        case EJson::Number:
        case EJson::Boolean:
            ArrayStrings.Add(Value->AsString());
            break;
        case EJson::Array:
            ArrayStrings.Add(HandleJsonArray(Value->AsArray()));
            break;
        case EJson::Object:
        {
            FString OutputString;
            TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
            FJsonSerializer::Serialize(Value->AsObject().ToSharedRef(), Writer);
            ArrayStrings.Add(OutputString);
        }
        break;
        case EJson::Null:
            ArrayStrings.Add(TEXT("null"));
            break;
        default:
            break;
        }
    }
    return FString::Join(ArrayStrings, TEXT(", "));
}

void UOpenAIFunctionLibrary::ParseFunctionArgumentsRecursive(TSharedPtr<FJsonObject> JsonObject, const FString& ArgumentToFind, TMap<FString, FString>& FunctionArguments, FString& FoundArgumentValue)
{
    if (!JsonObject.IsValid())
    {
        UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: Invalid JsonObject passed to ParseFunctionArgumentsRecursive."));
        return;
    }

    for (auto& Pair : JsonObject->Values)
    {
        if (!Pair.Value.IsValid()) // Robustness: Check each value's validity.
        {
            UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: Null value detected in JsonObject during parsing."));
            continue; // Skip this iteration
        }

        FString ValueAsString;
        switch (Pair.Value->Type)
        {
        case EJson::String:
        case EJson::Number:
        case EJson::Boolean:
            ValueAsString = Pair.Value->AsString();
            break;
        case EJson::Array:
            ValueAsString = HandleJsonArray(Pair.Value->AsArray());
            break;
        case EJson::Object:
        {
            ParseFunctionArgumentsRecursive(Pair.Value->AsObject(), ArgumentToFind, FunctionArguments, FoundArgumentValue);
            if (!FoundArgumentValue.IsEmpty()) return; // Found the argument, exit early
            FString OutputString;
            TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
            FJsonSerializer::Serialize(Pair.Value->AsObject().ToSharedRef(), Writer);
            ValueAsString = OutputString;
        }
        break;
        case EJson::Null:
            ValueAsString = TEXT("null");
            break;
        default:
            UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: Encountered unhandled JSON type during parsing."));
            continue; // Skip this iteration
        }

        FunctionArguments.Add(Pair.Key, ValueAsString);

        if (Pair.Key == ArgumentToFind)
        {
            FoundArgumentValue = ValueAsString;
            return; // Found the argument, exit early
        }
    }
}

FString UOpenAIFunctionLibrary::PrintStringMap(const TMap<FString, FString>& InputMap)
{
    FString Result;

    for (const auto& KVPair : InputMap)
    {
        Result.Append(FString::Printf(TEXT("Key: %s, Value: %s\n"), *KVPair.Key, *KVPair.Value));
    }

    //UE_LOG(AIITKLog, Warning, TEXT("Result Map:\n %s"), *Result);
    return Result;
}

void UOpenAIFunctionLibrary::ParseArgumentsFromJsonObject(TSharedPtr<FJsonObject> JsonObject, TMap<FString, FString>& OutMap, const FString& ParentKey = "")
{
    if (!JsonObject.IsValid())
        return;

    for (const auto& Pair : JsonObject->Values)
    {
        FString CurrentKey = ParentKey.IsEmpty() ? Pair.Key : ParentKey + "." + Pair.Key;

        if (Pair.Value.IsValid())
        {
            switch (Pair.Value->Type)
            {
            case EJson::String:
                OutMap.Add(CurrentKey, Pair.Value->AsString());
                break;
            case EJson::Number:
                OutMap.Add(CurrentKey, FString::Printf(TEXT("%f"), Pair.Value->AsNumber()));
                break;
            case EJson::Object:
                ParseArgumentsFromJsonObject(Pair.Value->AsObject(), OutMap, CurrentKey);
                break;
            case EJson::Array:
            {
                TArray<FString> EncodedItems;
                for (const auto& Item : Pair.Value->AsArray())
                {
                    FString ItemAsString = Item->AsString();
                    FString Base64EncodedItem = FBase64::Encode(ItemAsString);
                    EncodedItems.Add(Base64EncodedItem);
                }
                FString FinalArrayString = "[" + FString::Join(EncodedItems, TEXT(",")) + "]";
                OutMap.Add(CurrentKey, FinalArrayString);
            }
            break;


            default:
                continue; // Skip other types for simplicity.
            }
        }
    }
}

void UOpenAIFunctionLibrary::RetrieveArgumentValue(const TMap<FString, FString>& InputMap, const FString& Key, FString& OutValue, TArray<FString>& OutArray, bool& bValueFound)
{
    OutValue = TEXT(""); // Default values
    OutArray.Empty();
    bValueFound = false; // Changed from bIsArray to bValueFound

    const FString* FoundValue = InputMap.Find(Key);
    if (!FoundValue)
        return;

    // Setting bValueFound to true since a value is found
    bValueFound = true;

    if (FoundValue->StartsWith("[") && FoundValue->EndsWith("]"))
    {
        FString CleanedValue = FoundValue->Mid(1, FoundValue->Len() - 2); // remove the brackets
        TArray<FString> Items;
        CleanedValue.ParseIntoArray(Items, TEXT(","), true); // true to keep empty entries

        for (FString& Item : Items)
        {
            Item = Item.TrimStartAndEnd(); // Trim whitespace
            FString DecodedItem;

            // Attempt to decode, if it fails, assume it's not Base64 encoded
            if (!FBase64::Decode(Item, DecodedItem))
            {
                // Decoding failed, use the original item
                DecodedItem = Item;
            }

            OutArray.Add(DecodedItem);
        }
    }
    else
    {
        OutValue = *FoundValue;
    }
}

TMap<FString, FString> UOpenAIFunctionLibrary::ParseFunctionArguments(const FString& JsonString, bool bEncodeArrayValues)
{
    TMap<FString, FString> ParsedMap;

    // Remove newline characters from the JSON string
    FString SanitizedJsonString = JsonString.Replace(TEXT("\\n"), TEXT(""));

    // Create a JSON Reader from the sanitized JSON string
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(SanitizedJsonString);

    // Deserialize the JSON data
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        // Iterate through all the key-value pairs in the JSON object
        for (const auto& Elem : JsonObject->Values)
        {
            // Check the type of the value to handle different types accordingly
            switch (Elem.Value->Type)
            {
            case EJson::Object:
            {
                TSharedPtr<FJsonObject> SubObject = Elem.Value->AsObject();
                for (const auto& SubElem : SubObject->Values)
                {
                    ParsedMap.Add(SubElem.Key, SubElem.Value->AsString());
                    UE_LOG(AIITKLog, Warning, TEXT("Object - Key: %s, Value: %s"), *SubElem.Key, *SubElem.Value->AsString());
                }
                break;
            }
            case EJson::Array:
            {
                TArray<TSharedPtr<FJsonValue>> Array = Elem.Value->AsArray();
                FString ArrayString = "[";
                for (int32 Index = 0; Index < Array.Num(); ++Index)
                {
                    FString ValueStr;

                    // Check the type of each element in the array and convert to string accordingly
                    if (Array[Index]->Type == EJson::Number)
                    {
                        // Handle numeric values
                        ValueStr = FString::SanitizeFloat(Array[Index]->AsNumber());
                    }
                    else if (Array[Index]->Type == EJson::String)
                    {
                        // Handle string values
                        ValueStr = Array[Index]->AsString();
                    }
                    else
                    {
                        // Handle other types or log an error
                        UE_LOG(AIITKLog, Warning, TEXT("Unsupported JSON array element type."));
                        continue;
                    }

                    FString EncodedValue = bEncodeArrayValues ? FBase64::Encode(ValueStr) : ValueStr;
                    ArrayString += EncodedValue;
                    if (Index < Array.Num() - 1)
                    {
                        ArrayString += ",";
                    }
                }
                ArrayString += "]";
                ParsedMap.Add(Elem.Key, ArrayString);
                UE_LOG(AIITKLog, Warning, TEXT("Array - Key: %s, Value: %s"), *Elem.Key, *ArrayString);
                break;
            }
            case EJson::String:
            {
                ParsedMap.Add(Elem.Key, Elem.Value->AsString());
                UE_LOG(AIITKLog, Warning, TEXT("String - Key: %s, Value: %s"), *Elem.Key, *Elem.Value->AsString());
                break;
            }
            default:
                UE_LOG(AIITKLog, Warning, TEXT("Unsupported JSON value type for Key: %s"), *Elem.Key);
                break;
            }
        }
        UE_LOG(AIITKLog, Warning, TEXT("Successfully parsed JSON to TMap."));
    }
    else
    {
        UE_LOG(AIITKLog, Warning, TEXT("Failed to parse JSON to TMap."));
    }

    return ParsedMap; // Return the parsed map, which will be empty if parsing failed
}



// AUDIO
FString UOpenAIFunctionLibrary::SaveRecordingOutput(UObject* WorldContext, FString FilePath, FString FileName, USoundSubmix* SubmixToRecord)
{
    // Log the initiation of the saving process
    UE_LOG(AIITKLog, Warning, TEXT("Starting the SaveRecordingOutput process."));

    // Log the FilePath and FileName
    UE_LOG(AIITKLog, Warning, TEXT("File Path: %s"), *FilePath);
    UE_LOG(AIITKLog, Warning, TEXT("File Name: %s"), *FileName);

    // Check if directory exists; if not, create it
    if (!IFileManager::Get().DirectoryExists(*FilePath))
    {
        IFileManager::Get().MakeDirectory(*FilePath, true);
        UE_LOG(AIITKLog, Warning, TEXT("Directory did not exist; created new one."));
    }

    // Check if file already exists; if yes, remove it
    FString FullFilePath = FPaths::Combine(FilePath, FileName + FString(".wav"));
    if (IFileManager::Get().FileExists(*FullFilePath))
    {
        IFileManager::Get().Delete(*FullFilePath);
        UE_LOG(AIITKLog, Warning, TEXT("Existing file detected and removed."));
    }

    // Log the full file path
    UE_LOG(AIITKLog, Warning, TEXT("Full File Path: %s"), *FullFilePath);

    // Stop recording and get the USoundWave object
    USoundWave* RecordingOutput = UAudioMixerBlueprintLibrary::StopRecordingOutput(
        WorldContext,
        EAudioRecordingExportType::WavFile,
        FileName,
        FilePath,
        SubmixToRecord,
        nullptr
    );

    return FullFilePath;
}

FString UOpenAIFunctionLibrary::ResponseFormatToString(EGPTResponseFormat ResponseFormat)
{
    switch (ResponseFormat)
    {
    case EGPTResponseFormat::Text:
        return TEXT("text");
    case EGPTResponseFormat::JsonObject:
        return TEXT("{\"type\": \"json_object\"}");
    default:
        return TEXT("");
    }
}




FString UOpenAIFunctionLibrary::OutputFormatEnumToString(EOutputFormat OutputFormat)
{
    FString typeString;
    switch (OutputFormat)
    {
    case EOutputFormat::pcm_16000:
        typeString = TEXT("pcm_16000");
        break;
    case EOutputFormat::pcm_22050:
        typeString = TEXT("pcm_22050");
        break;
    case EOutputFormat::pcm_24000:
        typeString = TEXT("pcm_24000");
        break;
    case EOutputFormat::pcm_44100:
        typeString = TEXT("pcm_44100");
        break;
    case EOutputFormat::mp3_44100_64:
        typeString = TEXT("mp3_44100_64");
        break;
    case EOutputFormat::mp3_44100_96:
        typeString = TEXT("mp3_44100_96");
        break;
    case EOutputFormat::mp3_44100_128:
        typeString = TEXT("mp3_44100_128");
        break;
    case EOutputFormat::mp3_44100_192:
        typeString = TEXT("mp3_44100_19");
        break;
    default:
        typeString = TEXT("unknown");
        break;
    }
    return typeString;
}


FString UOpenAIFunctionLibrary::ImageSizeToString(EImageSize ImageSize)
{
    switch (ImageSize)
    {
    case EImageSize::S256x256:
        return TEXT("256x256");
    case EImageSize::S512x512:
        return TEXT("512x512");
    case EImageSize::S1024x1024:
        return TEXT("1024x1024");
    case EImageSize::S1792x1024:
        return TEXT("1792x1024");
    case EImageSize::S1024x1792:
        return TEXT("1024x1792");
    default:
        return TEXT("256x256");
    }
}


int32 UOpenAIFunctionLibrary::OutputFormatToSampleRate(EOutputFormat OutputFormat)
{
    switch (OutputFormat)
    {
    case EOutputFormat::pcm_16000:
        return 16000;
    case EOutputFormat::pcm_22050:
        return 22050;
    case EOutputFormat::pcm_24000:
        return 24000;
    case EOutputFormat::pcm_44100:
        return 44100;
    case EOutputFormat::mp3_44100_64:
    case EOutputFormat::mp3_44100_96:
    case EOutputFormat::mp3_44100_128:
    case EOutputFormat::mp3_44100_192:
        return 44100;
    default:
        return -1;
    }
}

FString UOpenAIFunctionLibrary::AudioFileFormatEnumToString(EAudioFileFormat AudioFileFormat)
{
    FString formatString;
    switch (AudioFileFormat)
    {
    case EAudioFileFormat::PCM:
        formatString = TEXT("pcm");
        break;
    case EAudioFileFormat::MP3:
        formatString = TEXT("mp3");
        break;
    default:
        formatString = TEXT("unknown");
        break;
    }
    return formatString;
}



// IMAGE

UTexture2D* UOpenAIFunctionLibrary::Base64ToTexture(const FString& Base64String)
{
    TArray<uint8> DecodedBytes;
    if (!FBase64::Decode(Base64String, DecodedBytes))
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to decode Base64 string."));
        return nullptr;
    }

    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

    if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(DecodedBytes.GetData(), DecodedBytes.Num()))
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to load image from decoded bytes."));
        return nullptr;
    }

    TArray<uint8> UncompressedBGRA;
    if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to get raw image data."));
        return nullptr;
    }

    UTexture2D* Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight());
    if (!Texture)
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to create transient texture."));
        return nullptr;
    }

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 27
    // For UE 4.27 and earlier
    FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
    void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
#else
    // For UE 5 and later
    void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
#endif
    FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 27
    Mip.BulkData.Unlock();
#else
    Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
#endif

    Texture->UpdateResource();
    return Texture;
}

FString UOpenAIFunctionLibrary::TextureToBase64(UTexture2D* Texture)
{
    if (!Texture)
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Invalid Texture."));
        return "";
    }

    // Get the texture's data
    FTexture2DMipMap* MipMap = &Texture->GetPlatformData()->Mips[0];
    if (!MipMap)
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to get MipMap."));
        return "";
    }

    void* Data = MipMap->BulkData.Lock(LOCK_READ_ONLY);
    if (!Data)
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to lock MipMap data."));
        return "";
    }

    const int32 Width = Texture->GetSizeX();
    const int32 Height = Texture->GetSizeY();
    const int32 DataSize = Width * Height * 4; // Assuming BGRA format

    TArray<uint8> RawData;
    RawData.AddUninitialized(DataSize);
    FMemory::Memcpy(RawData.GetData(), Data, DataSize);

    MipMap->BulkData.Unlock();

    // Compress the raw data to PNG
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

    if (!ImageWrapper.IsValid())
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to create image wrapper."));
        return "";
    }

    if (!ImageWrapper->SetRaw(RawData.GetData(), RawData.Num(), Width, Height, ERGBFormat::RGBA, 8))
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to set raw data for image wrapper."));
        return "";
    }

    // Get the compressed PNG data
    const TArray<uint8, FDefaultAllocator64>& PngData64 = ImageWrapper->GetCompressed();

    // Convert to TArray<uint8, FDefaultAllocator>
    TArray<uint8, FDefaultAllocator> PngData;
    PngData.Append(PngData64.GetData(), PngData64.Num());

    // Encode the PNG data to Base64
    FString Base64String = FBase64::Encode(PngData);

    return Base64String;
}

bool UOpenAIFunctionLibrary::SaveTextureToDisk(UTexture2D* Texture, const FString& FileName)
{
    if (!Texture)
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Invalid texture."));
        return false;
    }

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 27
    // For UE 4.27 and earlier
    FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
#else
    // For UE 5 and later
    FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
#endif
    uint8* TextureData = const_cast<uint8*>(static_cast<const uint8*>(Mip.BulkData.LockReadOnly()));
    TArray<uint8> UncompressedRGBA;
    UncompressedRGBA.Append(TextureData, Mip.BulkData.GetBulkDataSize());
    Mip.BulkData.Unlock();

    // Decompressing the texture into raw data
    FColor* FormatedImageData = new FColor[Texture->GetSizeX() * Texture->GetSizeY()];
    FMemory::Memcpy(FormatedImageData, UncompressedRGBA.GetData(), UncompressedRGBA.Num());

    TArray<FColor> UncompressedColors;
    for (int i = 0; i < Texture->GetSizeX() * Texture->GetSizeY(); i++)
    {
        UncompressedColors.Add(FormatedImageData[i]);
    }

#if ENGINE_MAJOR_VERSION > 4
    TArray64<uint8> CompressedBitmap;
    FImageUtils::PNGCompressImageArray(Texture->GetSizeX(), Texture->GetSizeY(), TArrayView64<const FColor>(UncompressedColors), CompressedBitmap);
#else
    TArray<uint8> CompressedBitmap;
    FImageUtils::CompressImageArray(Texture->GetSizeX(), Texture->GetSizeY(), UncompressedColors, CompressedBitmap);
#endif

    if (CompressedBitmap.Num() == 0)
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to compress texture data."));
        return false;
    }

    if (!FFileHelper::SaveArrayToFile(CompressedBitmap, *FileName))
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to save texture to disk."));
        return false;
    }

    // Log where the texture was saved
    UE_LOG(AIITKLog, Log, TEXT("A.I.I.T.K: Texture saved to %s"), *FileName);

    return true;
}


UTexture2D* UOpenAIFunctionLibrary::LoadTextureFromDisk(const FString& FileName)
{
    TArray<uint8> CompressedData;
    if (!FFileHelper::LoadFileToArray(CompressedData, *FileName))
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to load file from disk."));
        return nullptr;
    }

    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

    if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(CompressedData.GetData(), CompressedData.Num()))
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to initialize ImageWrapper."));
        return nullptr;
    }

    TArray64<uint8> UncompressedRGBA;
    if (!ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, UncompressedRGBA))
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to decompress image data."));
        return nullptr;
    }

    UTexture2D* Texture = nullptr;
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 27
    Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_R8G8B8A8);
#else
    Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), EPixelFormat::PF_R8G8B8A8);
#endif

    if (!Texture)
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Failed to create texture."));
        return nullptr;
    }

    // Ensure PlatformData is valid
    if (!Texture->GetPlatformData())
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: PlatformData is invalid."));
        return nullptr;
    }

    // Fill texture with raw data
    FTexturePlatformData* PlatformData = Texture->GetPlatformData();
    void* TextureData = PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(TextureData, UncompressedRGBA.GetData(), UncompressedRGBA.Num());
    PlatformData->Mips[0].BulkData.Unlock();

    // Update texture properties
    Texture->UpdateResource();

    // Log where the texture was loaded from
    UE_LOG(AIITKLog, Log, TEXT("A.I.I.T.K: Texture loaded from %s"), *FileName);

    return Texture;
}


FString UOpenAIFunctionLibrary::ExtractBase64FromDataURL(const FString& DataURL)
{
    FString Base64String = DataURL;
    Base64String.ReplaceInline(TEXT(" "), TEXT(""));
    return Base64String;
}

bool UOpenAIFunctionLibrary::IsValidURL(const FString& URL)
{
    if (URL.Len() < 5)
    {
        return false;
    }

    FString Protocol = URL.Left(4).ToLower();

    int32 ProtocolSeparatorIndex;
    if (!URL.FindChar(':', ProtocolSeparatorIndex))
    {
        return false;
    }

    FString AfterProtocol = URL.RightChop(ProtocolSeparatorIndex + 1);
    if (!AfterProtocol.StartsWith(TEXT("//")))
    {
        return false;
    }

    FString HostAndPath = AfterProtocol.RightChop(2);
    if (HostAndPath.IsEmpty())
    {
        return false;
    }

    return true;
}

void UOpenAIFunctionLibrary::DownloadImageFromURL(const FString& URL, const FDownloadImageDelegate& OnImageDownloaded)
{
    if (URL.IsEmpty())
    {
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: Invalid URL provided. Probably..."));
        OnImageDownloaded.ExecuteIfBound(nullptr);
        return;
    }

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(URL);
    HttpRequest->SetVerb("GET");

    HttpRequest->OnProcessRequestComplete().BindStatic(&UOpenAIFunctionLibrary::HandleImageDownloadResponse, OnImageDownloaded);

    HttpRequest->ProcessRequest();
}

void UOpenAIFunctionLibrary::HandleImageDownloadResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FDownloadImageDelegate OnImageDownloaded)
{
    if (bSucceeded && HttpResponse.IsValid() && HttpResponse->GetContentLength() > 0)
    {
        TArray<uint8> ImageData = HttpResponse->GetContent();

        IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
        EImageFormat ImageFormat = EImageFormat::JPEG;

        if (ImageData.Num() > 8)
        {
            // Check the header to guess the image format
            if (ImageData[0] == 0x89 && ImageData[1] == 0x50 && ImageData[2] == 0x4E && ImageData[3] == 0x47)
            {
                ImageFormat = EImageFormat::PNG;
            }
        }

        TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
        if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(ImageData.GetData(), ImageData.Num()))
        {
            TArray<uint8> RawData;
            if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
            {
                UTexture2D* Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);

                if (Texture)
                {
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 27
                    // For UE 4.27 and earlier
                    FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
                    void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
#else
                    // For UE 5 and later
                    void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
#endif

                    FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 27
                    Mip.BulkData.Unlock();
#else
                    Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
#endif

                    Texture->UpdateResource();
                    //AsyncTask(ENamedThreads::GameThread, [OnImageDownloaded, Texture]()
                    //{
                        OnImageDownloaded.ExecuteIfBound(Texture);
                    //});

                    return;
                }
            }
        }
    }

    UE_LOG(AIITKLog, Error, TEXT("Failed to download image."));
    //AsyncTask(ENamedThreads::GameThread, [OnImageDownloaded]()
    //{
        OnImageDownloaded.ExecuteIfBound(nullptr);
    //});
}


// UTILITY

template<typename TEnum>
static FString GetEnumValueAsString(TEnum Value)
{
    static_assert(TIsEnum<TEnum>::Value || TIsEnumClass<TEnum>::Value,
        "'TEnum' template parameter to GetEnumValueAsString must be an enum type");

    UEnum* Enum = StaticEnum<TEnum>();
    if (!Enum)
    {
        return FString("Invalid Enum");
    }
    return Enum->GetNameByValue((int64)Value).ToString();
}

TMap<FString, FString> UOpenAIFunctionLibrary::ConvertChatCompletionToMap(const FGPTChatCompletion& ChatCompletion)
{
    TMap<FString, FString> ResultMap;

    // Extracting completion info
    ResultMap.Add("CompletionID", ChatCompletion.CompletionInfo.ID);
    ResultMap.Add("Model", ChatCompletion.CompletionInfo.Model);
    ResultMap.Add("SystemFingerprint", ChatCompletion.CompletionInfo.SystemFingerprint);
    ResultMap.Add("Object", ChatCompletion.CompletionInfo.Object);
    ResultMap.Add("Created", FString::FromInt(ChatCompletion.CompletionInfo.Created));

    // Extracting usage content
    ResultMap.Add("PromptTokens", FString::FromInt(ChatCompletion.CompletionInfo.Usage.PromptTokens));
    ResultMap.Add("CompletionTokens", FString::FromInt(ChatCompletion.CompletionInfo.Usage.CompletionTokens));
    ResultMap.Add("TotalTokens", FString::FromInt(ChatCompletion.CompletionInfo.Usage.TotalTokens));

    // Iterating through choices
    for (int i = 0; i < ChatCompletion.Choices.Num(); ++i)
    {
        // Choice information
        const auto& Choice = ChatCompletion.Choices[i];
        FString IndexPrefix = FString::Printf(TEXT("Choice%d_"), i);
        ResultMap.Add(IndexPrefix + "FinishReason", Choice.FinishReason);
        ResultMap.Add(IndexPrefix + "Index", FString::FromInt(Choice.Index));
        ResultMap.Add(IndexPrefix + "MessageRole", GetRoleAsString(Choice.Message.Role));

        // Handling different message roles
        switch (Choice.Message.Role)
        {
        case EGPTRole::system:
            ResultMap.Add(IndexPrefix + "MessageContent", Choice.Message.SystemMessage.Content);
            break;
        case EGPTRole::assistant:
            ResultMap.Add(IndexPrefix + "MessageContent", Choice.Message.AssistantMessage.Content);
            // Iterating through tool calls within assistant message
            for (int t = 0; t < Choice.Message.AssistantMessage.ToolCalls.Num(); ++t)
            {
                const auto& ToolCall = Choice.Message.AssistantMessage.ToolCalls[t];
                FString ToolCallPrefix = IndexPrefix + FString::Printf(TEXT("ToolCall%d_"), t);
                ResultMap.Add(ToolCallPrefix + "ID", ToolCall.ID);
                ResultMap.Add(ToolCallPrefix + "Type", ToolCall.Type);
                ResultMap.Add(ToolCallPrefix + "FunctionName", ToolCall.FunctionName);
                ResultMap.Add(ToolCallPrefix + "FunctionArguments", ToolCall.FunctionArguments);
            }
            break;
        case EGPTRole::tool:
            ResultMap.Add(IndexPrefix + "MessageContent", Choice.Message.ToolMessage.Content);
            break;
            // Add additional cases for other roles if necessary
        }

        // Iterating through top log probabilities
        for (int j = 0; j < Choice.LogProbs.Content.Num(); ++j)
        {
            const auto& LogProbContent = Choice.LogProbs.Content[j];
            FString LogProbPrefix = IndexPrefix + FString::Printf(TEXT("LogProbContent%d_"), j);
            ResultMap.Add(LogProbPrefix + "Token", LogProbContent.Token);
            ResultMap.Add(LogProbPrefix + "LogProb", FString::SanitizeFloat(LogProbContent.LogProb));

            // Iterating through bytes
            FString BytesStr;
            for (int Byte : LogProbContent.Bytes)
            {
                BytesStr += FString::Printf(TEXT("%d "), Byte);
            }
            ResultMap.Add(LogProbPrefix + "Bytes", BytesStr);

            // Iterating through top log probabilities
            for (int k = 0; k < LogProbContent.TopLogProbs.Num(); ++k)
            {
                const auto& TopLogProb = LogProbContent.TopLogProbs[k];
                FString TopLogProbPrefix = LogProbPrefix + FString::Printf(TEXT("TopLogProb%d_"), k);
                ResultMap.Add(TopLogProbPrefix + "Token", TopLogProb.Token);
                ResultMap.Add(TopLogProbPrefix + "LogProb", FString::SanitizeFloat(TopLogProb.LogProb));

                FString TopBytesStr;
                for (int TopByte : TopLogProb.Bytes)
                {
                    TopBytesStr += FString::Printf(TEXT("%d "), TopByte);
                }
                ResultMap.Add(TopLogProbPrefix + "Bytes", TopBytesStr);
            }
        }
    }

    return ResultMap;
}

FString UOpenAIFunctionLibrary::GetRoleAsString(EGPTRole Role)
{
    switch (Role)
    {
    case EGPTRole::system:
        return TEXT("system");
    case EGPTRole::user:
        return TEXT("user");
    case EGPTRole::assistant:
        return TEXT("assistant");
    case EGPTRole::tool:
        return TEXT("tool");
    default:
        return TEXT("Unknown");
    }
}

bool UOpenAIFunctionLibrary::DeleteFile(FString FilePath)
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    if (PlatformFile.FileExists(*FilePath))
    {
        if (PlatformFile.DeleteFile(*FilePath))
        {
            // File deleted successfully
            UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: File deleted successfully: %s"), *FilePath);
            return true;
        }
        else
        {
            // File could not be deleted
            UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: File could not be deleted: %s"), *FilePath);
            return false;
        }
    }
    else
    {
        // File does not exist
        UE_LOG(AIITKLog, Error, TEXT("A.I.I.T.K: File does not exist, okay to write: %s"), *FilePath);
        return true;
    }
}

FString UOpenAIFunctionLibrary::ConvertToPascalCase(const FString& InputString)
{
    FString Result;
    TArray<FString> Words;

    // Split the input string into words
    InputString.ParseIntoArray(Words, TEXT(" "), true);

    // Limit to first 4 words
    int32 WordCount = FMath::Min(4, Words.Num());

    // Iterate over each word
    for (int32 Index = 0; Index < WordCount; ++Index)
    {
        FString& Word = Words[Index];
        FString SanitizedWord;

        // Iterate over each character
        for (TCHAR& Char : Word)
        {
            // If the character is alphanumeric, add it to the sanitized word
            if (FChar::IsAlnum(Char))
            {
                SanitizedWord.AppendChar(Char);
            }
        }

        // Replace the word with the sanitized version
        Word = SanitizedWord;

        // Convert to Pascal
        if (Word.Len() > 0)
        {
            Word[0] = FChar::ToUpper(Word[0]);
            if (Word.Len() > 1)
            {
                for (int32 i = 1; i < Word.Len(); i++)
                {
                    Word[i] = FChar::ToLower(Word[i]);
                }
            }

            Result += Word;
        }
    }

    return Result;
}

TSharedPtr<FJsonObject> UOpenAIFunctionLibrary::CreateLogitBiasObject(const TMap<int32, float>& LogitBiasMap)
{
    TSharedPtr<FJsonObject> LogitBiasObject = MakeShareable(new FJsonObject);

    for (const auto& Pair : LogitBiasMap)
    {
        LogitBiasObject->SetNumberField(FString::Printf(TEXT("%d"), Pair.Key), Pair.Value);
    }

    return LogitBiasObject;
}

uint8 UOpenAIFunctionLibrary::Conv_StringToEnum(const UEnum* Enum, const FString& StringValue)
{
    // Print the string value we are about to convert
    //UE_LOG(AIITKLog, Warning, TEXT("Attempting to convert string '%s' to enum"), *StringValue);

    if (!Enum)
    {
        UE_LOG(AIITKLog, Warning, TEXT("Invalid Enum"));
        return 0; // Return 0 or some invalid value
    }

    // Iterate through the enum entries and compare the display names
    for (int32 i = 0; i < Enum->NumEnums() - 1; ++i) // Exclude the _MAX auto-generated entry
    {
        FText DisplayName = Enum->GetDisplayNameTextByIndex(i);
        FString EnumDisplayName = DisplayName.ToString();
        if (EnumDisplayName.Equals(StringValue, ESearchCase::IgnoreCase))
        {
            // Found the matching entry, return the corresponding value
            int64 EnumValue = Enum->GetValueByIndex(i);
            UE_LOG(AIITKLog, Log, TEXT("Successfully converted string '%s' to enum value %lld"), *StringValue, EnumValue);
            return static_cast<uint8>(EnumValue);
        }
    }

    // No matching entry found
    UE_LOG(AIITKLog, Warning, TEXT("Invalid string '%s' for enum conversion"), *StringValue);
    return 0; // Return 0 or some invalid value
}



int UOpenAIFunctionLibrary::CalculateTokens(const FGPTChatMessage& Message)
{
    int TokenCount = 0;

    switch (Message.Role)
    {
    case EGPTRole::system:
        TokenCount = Message.SystemMessage.Content.Len();
        break;
    case EGPTRole::user:
        // Sum the length of all content parts for user messages
        for (const FContentPart& Part : Message.UserMessage.Content)
        {
            TokenCount += Part.Content.Len(); // Count tokens for each part of the content
        }
        break;
    case EGPTRole::assistant:
        TokenCount = Message.AssistantMessage.Content.Len();
        // Optionally include tool calls in token count
        for (const FToolCall& ToolCall : Message.AssistantMessage.ToolCalls)
        {
            TokenCount += ToolCall.FunctionName.Len() + ToolCall.FunctionArguments.Len();
        }
        break;
    case EGPTRole::tool:
        TokenCount = Message.ToolMessage.Content.Len();
        break;
        // Handle other roles if necessary
    }

    return TokenCount / 4; // Assuming 4 characters per token
}

FString UOpenAIFunctionLibrary::GetMessageContent(const FGPTChatMessage& Message)
{
    switch (Message.Role)
    {
    case EGPTRole::system:
        return Message.SystemMessage.Content;
    case EGPTRole::user:
    {
        FString CombinedContent;
        for (const FContentPart& Part : Message.UserMessage.Content)
        {
            CombinedContent += Part.Content + TEXT(" "); // Add a space between parts
        }
        return CombinedContent;
    }
    case EGPTRole::assistant:
        return Message.AssistantMessage.Content;
    case EGPTRole::tool:
        return Message.ToolMessage.Content;
    default:
        return TEXT("Unknown Role");
    }
}

FGPTChatMessage UOpenAIFunctionLibrary::GetResponseMessage(const FGPTChatCompletion& Completion, const int32 ChoiceIndex)
{
    if (Completion.Choices.IsValidIndex(ChoiceIndex))
    {
        return Completion.Choices[ChoiceIndex].Message;
    }
    else
    {
        UE_LOG(AIITKLog, Warning, TEXT("GetMessage: Invalid Choice Index %d"), ChoiceIndex);
        // Return a default FGPTChatMessage
        FGPTChatMessage DefaultMessage;
        return DefaultMessage;
    }
}

TArray<FString> UOpenAIFunctionLibrary::GetToolCallArguments(const FGPTChatMessage& Message)
{
    TArray<FString> ArgumentsArray;

    // Check if the role is Assistant, as Tool Calls are only relevant here
    if (Message.Role == EGPTRole::assistant)
    {
        for (const FToolCall& ToolCall : Message.AssistantMessage.ToolCalls)
        {
            // Add the FunctionArguments of each ToolCall to the array
            ArgumentsArray.Add(ToolCall.FunctionArguments);
        }
    }

    return ArgumentsArray;
}

void UOpenAIFunctionLibrary::GetArgumentFromCompletion(const FGPTChatCompletion& Completion, const FString& ArgumentName, const int32 ChoiceIndex, const int32 ToolCallIndex, FString& OutValue, TArray<FString>& OutArray, bool& bValueFound)
{
    // Initialize output parameters with default values
    OutValue = TEXT("");
    OutArray.Empty();
    bValueFound = false;

    // Check for valid choice index in Completion
    if (!Completion.Choices.IsValidIndex(ChoiceIndex))
    {
        UE_LOG(AIITKLog, Warning, TEXT("Invalid Choice Index %d"), ChoiceIndex);
        return;
    }

    // Get the response message based on ChoiceIndex
    FGPTChatMessage TempMessage = GetResponseMessage(Completion, ChoiceIndex);

    // Get tool call arguments from the message
    TArray<FString> TempArguments = GetToolCallArguments(TempMessage);

    // Check for valid ToolCallIndex in TempArguments
    if (!TempArguments.IsValidIndex(ToolCallIndex))
    {
        UE_LOG(AIITKLog, Warning, TEXT("Invalid Tool Call Index %d"), ToolCallIndex);
        return;
    }

    // Log the JSON string being parsed
    if (TempArguments.IsValidIndex(ToolCallIndex))
    {
        UE_LOG(AIITKLog, Log, TEXT("JSON String to Parse: %s"), *TempArguments[ToolCallIndex]);
    }

    // Parse the function arguments from the selected tool call
    TMap<FString, FString> FoundArgument = ParseFunctionArguments(TempArguments[ToolCallIndex], true);

    // Check for parsing errors
    if (FoundArgument.Num() == 0)
    {
        UE_LOG(AIITKLog, Error, TEXT("Failed to parse JSON for Tool Call Index %d"), ToolCallIndex);
        return;
    }

    // Find the specified argument
    const FString* FoundValue = FoundArgument.Find(ArgumentName);
    if (!FoundValue)
    {
        UE_LOG(AIITKLog, Warning, TEXT("Argument Not Found: %s"), *ArgumentName);
        return;
    }

    // Set flag to indicate value was found
    bValueFound = true;

    // Process the found value
    if (FoundValue->StartsWith("[") && FoundValue->EndsWith("]"))
    {
        // Handling array value
        FString CleanedValue = FoundValue->Mid(1, FoundValue->Len() - 2); // Remove brackets
        TArray<FString> Items;
        CleanedValue.ParseIntoArray(Items, TEXT(","), true); // Keep empty entries

        for (FString& Item : Items)
        {
            Item = Item.TrimStartAndEnd(); // Trim whitespace
            FString DecodedItem;

            // Decode item, fallback to original if decoding fails
            if (!FBase64::Decode(Item, DecodedItem))
            {
                DecodedItem = Item;
            }

            OutArray.Add(DecodedItem);
        }
    }
    else
    {
        // Handling single value
        OutValue = *FoundValue;
    }
}

void UOpenAIFunctionLibrary::GetToolIDFromCompletion(const FGPTChatCompletion& Completion, const int32 ChoiceIndex, const int32 ToolCallIndex, FString& OutValue, bool& bIDFound)
{
    // Initialize output parameters with default values
    OutValue = TEXT("");
    bIDFound = false;

    // Check for valid choice index in Completion
    if (!Completion.Choices.IsValidIndex(ChoiceIndex))
    {
        UE_LOG(AIITKLog, Warning, TEXT("Invalid Choice Index %d"), ChoiceIndex);
        return;
    }

    // Get the response message based on ChoiceIndex
    FGPTChatMessage TempMessage = Completion.Choices[ChoiceIndex].Message;

    // Ensure the message role is assistant
    if (TempMessage.Role != EGPTRole::assistant)
    {
        UE_LOG(AIITKLog, Warning, TEXT("Message Role is not of type 'assistant' for Choice Index %d"), ChoiceIndex);
        return;
    }

    // Check for valid ToolCallIndex in ToolCalls array
    if (!TempMessage.AssistantMessage.ToolCalls.IsValidIndex(ToolCallIndex))
    {
        UE_LOG(AIITKLog, Warning, TEXT("Invalid Tool Call Index %d"), ToolCallIndex);
        return;
    }

    // Retrieve the tool call based on ToolCallIndex
    FToolCall ToolCall = TempMessage.AssistantMessage.ToolCalls[ToolCallIndex];

    // Set the output value to the ID of the tool call
    OutValue = ToolCall.ID;

    // Set flag to indicate ID was found
    bIDFound = true;
}

int UOpenAIFunctionLibrary::CalculateConversationTokens(const TArray<FGPTChatMessage>& Messages)
{
    int TotalTokenCount = 0;
    for (const auto& Message : Messages)
    {
        TotalTokenCount += CalculateTokens(Message);
    }
    return TotalTokenCount;
}

TArray<FGPTChatMessage> UOpenAIFunctionLibrary::TruncateConversation(TArray<FGPTChatMessage> Conversation, int MaxTokens)
{
    int CurrentTokenCount = CalculateConversationTokens(Conversation);

    while (CurrentTokenCount > MaxTokens)
    {
        if (Conversation[0].Role != EGPTRole::system)
        {
            CurrentTokenCount -= CalculateTokens(Conversation[0]);
            Conversation.RemoveAt(0);
        }

        if (Conversation.Num() > 1)
        {
            CurrentTokenCount -= CalculateTokens(Conversation[1]);
            Conversation.RemoveAt(1);
        }

        UE_LOG(AIITKLog, Warning, TEXT("A.I.I.T.K: Conversation too long, pair removed"));
    }

    return Conversation;
}

FString UOpenAIFunctionLibrary::EnumFieldTypeToString(const EJsonEnumType& EnumType)
{
    FString Result;
    switch (EnumType)
    {
    case EJsonEnumType::String:
        Result = TEXT("string");
        break;
    case EJsonEnumType::Number:
        Result = TEXT("number");
        break;
    default:
        Result = TEXT("unknown");
        break;
    }
    return Result;
}

FString UOpenAIFunctionLibrary::FieldTypeEnumToString(const EGPTFieldTypes& FieldType)
{
    FString typeString;
    switch (FieldType)
    {
    case EGPTFieldTypes::String:
        typeString = TEXT("string");
        break;
    case EGPTFieldTypes::Number:
        typeString = TEXT("number");
        break;
    case EGPTFieldTypes::Boolean:
        typeString = TEXT("boolean");
        break;
    case EGPTFieldTypes::Enum:
        typeString = TEXT("enum");
        break;
    case EGPTFieldTypes::Object:
        typeString = TEXT("object");
        break;
    default:
        typeString = TEXT("unknown");
        break;
    }
    return typeString;
}

TMap<FString, FString> UOpenAIFunctionLibrary::ConvertVoiceArrayToMap(const TArray<FVoiceInfo>& VoicesArray)
{
    TMap<FString, FString> VoiceMap;
    if (VoicesArray.Num() == 0)
    {
        UE_LOG(AIITKLog, Error, TEXT("VoicesArray is empty"));
        return VoiceMap;
    }
    for (const auto& Voice : VoicesArray)
    {
        VoiceMap.Add(Voice.Name, Voice.VoiceID);
    }
    return VoiceMap;
}

TMap<FString, FString> UOpenAIFunctionLibrary::ConvertModelArrayToMap(const TArray<FModelInfo>& ModelsArray)
{
    TMap<FString, FString> ModelMap;
    if (ModelsArray.Num() == 0)
    {
        UE_LOG(AIITKLog, Error, TEXT("ModelsArray is empty"));
        return ModelMap;
    }
    for (const auto& Model : ModelsArray)
    {
        ModelMap.Add(Model.Name, Model.ModelID);
    }
    return ModelMap;
}

TArray<FString> UOpenAIFunctionLibrary::SimpleRegexSearch(const FString& Pattern, const FString& SearchString, const int CaptureGroup)
{
    FRegexPattern MyPattern(Pattern);
    FRegexMatcher MyMatcher(MyPattern, SearchString);

    TArray<FString> Results;

    while (MyMatcher.FindNext())
    {
        Results.Add(MyMatcher.GetCaptureGroup(CaptureGroup));  // Group 0 is the entire match
    }

    return Results;
}

bool UOpenAIFunctionLibrary::IsStringValidJSON(const FString& JSONString)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        return true;
    }
    else
    {
        return false;
    }
}

TArray<FString> UOpenAIFunctionLibrary::Conv_EnumToStringArray(const UEnum* Enum)
{
    TArray<FString> EnumFriendlyNames;
    if (!Enum) return EnumFriendlyNames;

    for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
    {
        FString EnumFriendlyName = Enum->GetDisplayNameTextByIndex(i).ToString();
        EnumFriendlyNames.Add(EnumFriendlyName);
    }

    return EnumFriendlyNames;
}

FString UOpenAIFunctionLibrary::GetColorName(const FLinearColor& Color)
{
    static const TArray<TPair<FLinearColor, FString>> ColorArray = {
        // ROYGBIV Colors
        TPair<FLinearColor, FString>(FLinearColor::Red, TEXT("Red")),
        TPair<FLinearColor, FString>(FLinearColor(1.0f, 0.5f, 0.0f), TEXT("Orange")),
        TPair<FLinearColor, FString>(FLinearColor::Yellow, TEXT("Yellow")),
        TPair<FLinearColor, FString>(FLinearColor::Green, TEXT("Green")),
        TPair<FLinearColor, FString>(FLinearColor::Blue, TEXT("Blue")),
        TPair<FLinearColor, FString>(FLinearColor(0.294f, 0.0f, 0.51f), TEXT("Indigo")),
        TPair<FLinearColor, FString>(FLinearColor(0.933f, 0.51f, 0.933f), TEXT("Violet")),

        // Light Variations
        TPair<FLinearColor, FString>(FLinearColor(1.0f, 0.75f, 0.75f), TEXT("Light Red")),
        TPair<FLinearColor, FString>(FLinearColor(1.0f, 0.75f, 0.5f), TEXT("Light Orange")),
        TPair<FLinearColor, FString>(FLinearColor(1.0f, 1.0f, 0.75f), TEXT("Light Yellow")),
        TPair<FLinearColor, FString>(FLinearColor(0.75f, 1.0f, 0.75f), TEXT("Light Green")),
        TPair<FLinearColor, FString>(FLinearColor(0.75f, 0.75f, 1.0f), TEXT("Light Blue")),
        TPair<FLinearColor, FString>(FLinearColor(0.647f, 0.5f, 0.76f), TEXT("Light Indigo")),
        TPair<FLinearColor, FString>(FLinearColor(1.0f, 0.76f, 1.0f), TEXT("Light Violet")),

        // Dark Variations
        TPair<FLinearColor, FString>(FLinearColor(0.5f, 0.0f, 0.0f), TEXT("Dark Red")),
        TPair<FLinearColor, FString>(FLinearColor(0.5f, 0.25f, 0.0f), TEXT("Dark Orange")),
        TPair<FLinearColor, FString>(FLinearColor(0.5f, 0.5f, 0.0f), TEXT("Dark Yellow")),
        TPair<FLinearColor, FString>(FLinearColor(0.0f, 0.5f, 0.0f), TEXT("Dark Green")),
        TPair<FLinearColor, FString>(FLinearColor(0.0f, 0.0f, 0.5f), TEXT("Dark Blue")),
        TPair<FLinearColor, FString>(FLinearColor(0.147f, 0.0f, 0.255f), TEXT("Dark Indigo")),
        TPair<FLinearColor, FString>(FLinearColor(0.467f, 0.255f, 0.467f), TEXT("Dark Violet")),
    };

    FString closestColorName = TEXT("Unknown");
    float minDistance = FLT_MAX;

    for (const TPair<FLinearColor, FString>& ColorPair : ColorArray)
    {
        float distance = FLinearColor::Dist(Color, ColorPair.Key);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestColorName = ColorPair.Value;
        }
    }

    return closestColorName;
}