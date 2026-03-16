#include "AIITKEditorFunctionLibrary.h"
#include "AssetToolsModule.h"
#include "Engine/AssetUserData.h"
#include "EZUEGPTEditor.h"
#include "FileHelpers.h"
#include "IAssetTools.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Kismet2/StructureEditorUtils.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"

bool UAIITKEditorFunctionLibrary::CreateBlueprintableStructFromJson(const FString& JsonInput, const FString& DirectoryPath, const FString& StructName)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonInput);

    // Parse JSON
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(AIITKEditorLog, Error, TEXT("Failed to parse JSON input."));
        return false;
    }

    // Correct the path to ensure it uses a valid mount point
    FString PackageName = FPaths::Combine(DirectoryPath, StructName);
    FString FullPath = FPaths::ConvertRelativePathToFull(PackageName);

    if (!FPaths::ValidatePath(FullPath))
    {
        UE_LOG(AIITKEditorLog, Error, TEXT("Invalid directory path: %s"), *FullPath);
        return false;
    }

    // Create the package for the struct
    UPackage* Package = CreatePackage(*PackageName);
    if (!Package)
    {
        UE_LOG(AIITKEditorLog, Error, TEXT("Failed to create package at path: %s"), *PackageName);
        return false;
    }

    // Use native methods to create the struct
    UUserDefinedStruct* NewStruct = NewObject<UUserDefinedStruct>(Package, FName(*StructName), RF_Public | RF_Standalone);
    if (!NewStruct)
    {
        UE_LOG(AIITKEditorLog, Error, TEXT("Failed to create UUserDefinedStruct."));
        return false;
    }

    // Initialize the struct's editor data
    NewStruct->EditorData = NewObject<UUserDefinedStructEditorData>(NewStruct, NAME_None, RF_Public | RF_Transactional);
    if (!NewStruct->EditorData)
    {
        UE_LOG(AIITKEditorLog, Error, TEXT("Failed to initialize EditorData for struct: %s"), *StructName);
        return false;
    }

    NewStruct->EditorData->Modify();

    // Convert JSON fields into struct variables
    for (const auto& Entry : JsonObject->Values)
    {
        const FString& Key = Entry.Key;
        const TSharedPtr<FJsonValue>& Value = Entry.Value;

        // Create properties based on JSON value type
        FEdGraphPinType PinType;
        FString DefaultValue;

        if (Value->Type == EJson::String)
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_String;
            DefaultValue = Value->AsString();
        }
        else if (Value->Type == EJson::Number)
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Float;
            DefaultValue = FString::SanitizeFloat(Value->AsNumber());
        }
        else if (Value->Type == EJson::Boolean)
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
            DefaultValue = Value->AsBool() ? TEXT("true") : TEXT("false");
        }
        else if (Value->Type == EJson::Object)
        {
            // Create a sub-struct for nested objects
            UUserDefinedStruct* SubStruct = CreateSubStructFromJsonObject(Value->AsObject(), StructName + TEXT("_") + Key, Package);
            if (SubStruct)
            {
                PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                PinType.PinSubCategoryObject = SubStruct;
            }
            else
            {
                UE_LOG(AIITKEditorLog, Error, TEXT("Failed to create sub-struct for key: %s"), *Key);
                continue;
            }
        }
        else if (Value->Type == EJson::Array)
        {
            PinType.ContainerType = EPinContainerType::Array;
            const TArray<TSharedPtr<FJsonValue>>& JsonArray = Value->AsArray();

            if (JsonArray.Num() > 0)
            {
                TSharedPtr<FJsonValue> FirstElement = JsonArray[0];

                // Determine the type of the array elements
                if (FirstElement->Type == EJson::String)
                {
                    PinType.PinCategory = UEdGraphSchema_K2::PC_String;

                    // Build default value for the array
                    TArray<FString> StringValues;
                    for (const auto& ArrayValue : JsonArray)
                    {
                        StringValues.Add(FString::Printf(TEXT("\"%s\""), *ArrayValue->AsString()));
                    }
                    DefaultValue = FString::Printf(TEXT("(%s)"), *FString::Join(StringValues, TEXT(",")));
                }
                else if (FirstElement->Type == EJson::Number)
                {
                    PinType.PinCategory = UEdGraphSchema_K2::PC_Float;

                    // Build default value for the array
                    TArray<FString> NumberValues;
                    for (const auto& ArrayValue : JsonArray)
                    {
                        NumberValues.Add(FString::SanitizeFloat(ArrayValue->AsNumber()));
                    }
                    DefaultValue = FString::Printf(TEXT("(%s)"), *FString::Join(NumberValues, TEXT(",")));
                }
                else if (FirstElement->Type == EJson::Boolean)
                {
                    PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;

                    // Build default value for the array
                    TArray<FString> BoolValues;
                    for (const auto& ArrayValue : JsonArray)
                    {
                        BoolValues.Add(ArrayValue->AsBool() ? TEXT("true") : TEXT("false"));
                    }
                    DefaultValue = FString::Printf(TEXT("(%s)"), *FString::Join(BoolValues, TEXT(",")));
                }
                else if (FirstElement->Type == EJson::Object)
                {
                    // Create a sub-struct for the array element type
                    UUserDefinedStruct* ElementSubStruct = CreateSubStructFromJsonObject(FirstElement->AsObject(), StructName + TEXT("_") + Key + TEXT("_Element"), Package);
                    if (ElementSubStruct)
                    {
                        PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                        PinType.PinSubCategoryObject = ElementSubStruct;

                        // Note: Setting default values for arrays of structs is complex and often not necessary
                        DefaultValue = TEXT(""); // Leave default empty for arrays of structs
                    }
                    else
                    {
                        UE_LOG(AIITKEditorLog, Error, TEXT("Failed to create sub-struct for array element key: %s"), *Key);
                        continue;
                    }
                }
                else
                {
                    UE_LOG(AIITKEditorLog, Warning, TEXT("Unsupported array element type for key: %s"), *Key);
                    continue;
                }
            }
            else
            {
                // Empty array, set to empty parentheses
                PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
                DefaultValue = TEXT("()");
            }
        }
        else
        {
            UE_LOG(AIITKEditorLog, Warning, TEXT("Unsupported JSON value type for key: %s"), *Key);
            continue;
        }

        // Add the variable to the struct
        if (!AddVariableToStruct(NewStruct, *Key, PinType, DefaultValue))
        {
            UE_LOG(AIITKEditorLog, Error, TEXT("Failed to add variable %s to struct %s"), *Key, *StructName);
        }
    }

    // Finalize the struct setup
    FStructureEditorUtils::OnStructureChanged(NewStruct);
    NewStruct->SetMetaData(TEXT("BlueprintType"), TEXT("true"));

    // Save the package to disk
    FEditorFileUtils::SaveDirtyPackages(true, true, true, false, true, true);
    UE_LOG(AIITKEditorLog, Log, TEXT("Successfully created struct: %s"), *StructName);

    return true;
}

UUserDefinedStruct* UAIITKEditorFunctionLibrary::CreateSubStructFromJsonObject(const TSharedPtr<FJsonObject>& JsonObject, const FString& StructName, UPackage* Package)
{
    UUserDefinedStruct* SubStruct = NewObject<UUserDefinedStruct>(Package, FName(*StructName), RF_Public | RF_Standalone);
    if (!SubStruct)
    {
        UE_LOG(AIITKEditorLog, Error, TEXT("Failed to create sub-struct: %s"), *StructName);
        return nullptr;
    }

    SubStruct->EditorData = NewObject<UUserDefinedStructEditorData>(SubStruct, NAME_None, RF_Public | RF_Transactional);
    if (!SubStruct->EditorData)
    {
        UE_LOG(AIITKEditorLog, Error, TEXT("Failed to initialize EditorData for sub-struct: %s"), *StructName);
        return nullptr;
    }

    SubStruct->EditorData->Modify();

    for (const auto& Entry : JsonObject->Values)
    {
        const FString& Key = Entry.Key;
        const TSharedPtr<FJsonValue>& Value = Entry.Value;

        FEdGraphPinType PinType;
        FString DefaultValue;

        if (Value->Type == EJson::String)
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_String;
            DefaultValue = Value->AsString();
        }
        else if (Value->Type == EJson::Number)
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Float;
            DefaultValue = FString::SanitizeFloat(Value->AsNumber());
        }
        else if (Value->Type == EJson::Boolean)
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
            DefaultValue = Value->AsBool() ? TEXT("true") : TEXT("false");
        }
        else if (Value->Type == EJson::Object)
        {
            // Recursively create sub-structs for nested objects
            UUserDefinedStruct* NestedSubStruct = CreateSubStructFromJsonObject(Value->AsObject(), StructName + TEXT("_") + Key, Package);
            if (NestedSubStruct)
            {
                PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                PinType.PinSubCategoryObject = NestedSubStruct;
            }
            else
            {
                UE_LOG(AIITKEditorLog, Error, TEXT("Failed to create nested sub-struct for key: %s"), *Key);
                continue;
            }
        }
        else if (Value->Type == EJson::Array)
        {
            PinType.ContainerType = EPinContainerType::Array;
            const TArray<TSharedPtr<FJsonValue>>& JsonArray = Value->AsArray();

            if (JsonArray.Num() > 0)
            {
                TSharedPtr<FJsonValue> FirstElement = JsonArray[0];

                // Determine the type of the array elements
                if (FirstElement->Type == EJson::String)
                {
                    PinType.PinCategory = UEdGraphSchema_K2::PC_String;

                    // Build default value for the array
                    TArray<FString> StringValues;
                    for (const auto& ArrayValue : JsonArray)
                    {
                        StringValues.Add(FString::Printf(TEXT("\"%s\""), *ArrayValue->AsString()));
                    }
                    DefaultValue = FString::Printf(TEXT("(%s)"), *FString::Join(StringValues, TEXT(",")));
                }
                else if (FirstElement->Type == EJson::Number)
                {
                    PinType.PinCategory = UEdGraphSchema_K2::PC_Float;

                    // Build default value for the array
                    TArray<FString> NumberValues;
                    for (const auto& ArrayValue : JsonArray)
                    {
                        NumberValues.Add(FString::SanitizeFloat(ArrayValue->AsNumber()));
                    }
                    DefaultValue = FString::Printf(TEXT("(%s)"), *FString::Join(NumberValues, TEXT(",")));
                }
                else if (FirstElement->Type == EJson::Boolean)
                {
                    PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;

                    // Build default value for the array
                    TArray<FString> BoolValues;
                    for (const auto& ArrayValue : JsonArray)
                    {
                        BoolValues.Add(ArrayValue->AsBool() ? TEXT("true") : TEXT("false"));
                    }
                    DefaultValue = FString::Printf(TEXT("(%s)"), *FString::Join(BoolValues, TEXT(",")));
                }
                else if (FirstElement->Type == EJson::Object)
                {
                    // Create a sub-struct for the array element type
                    UUserDefinedStruct* ElementSubStruct = CreateSubStructFromJsonObject(FirstElement->AsObject(), StructName + TEXT("_") + Key + TEXT("_Element"), Package);
                    if (ElementSubStruct)
                    {
                        PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                        PinType.PinSubCategoryObject = ElementSubStruct;

                        // Note: Setting default values for arrays of structs is complex and often not necessary
                        DefaultValue = TEXT(""); // Leave default empty for arrays of structs
                    }
                    else
                    {
                        UE_LOG(AIITKEditorLog, Error, TEXT("Failed to create sub-struct for array element key: %s"), *Key);
                        continue;
                    }
                }
                else
                {
                    UE_LOG(AIITKEditorLog, Warning, TEXT("Unsupported array element type for key: %s"), *Key);
                    continue;
                }
            }
            else
            {
                // Empty array, set to empty parentheses
                PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
                DefaultValue = TEXT("()");
            }
        }
        else
        {
            UE_LOG(AIITKEditorLog, Warning, TEXT("Unsupported JSON value type for sub-struct key: %s"), *Key);
            continue;
        }

        // Add the variable to the sub-struct
        if (!AddVariableToStruct(SubStruct, *Key, PinType, DefaultValue))
        {
            UE_LOG(AIITKEditorLog, Error, TEXT("Failed to add variable %s to sub-struct %s"), *Key, *StructName);
        }
    }

    FStructureEditorUtils::OnStructureChanged(SubStruct);
    SubStruct->SetMetaData(TEXT("BlueprintType"), TEXT("true"));

    return SubStruct;
}

bool UAIITKEditorFunctionLibrary::AddVariableToStruct(UUserDefinedStruct* Struct, const FName& VarName, const FEdGraphPinType& PinType, const FString& DefaultValue)
{
    // Attempt to add the variable to the struct
    if (!FStructureEditorUtils::AddVariable(Struct, PinType))
    {
        UE_LOG(AIITKEditorLog, Error, TEXT("Cannot add variable of type: %s"), *PinType.PinCategory.ToString());
        return false;
    }

    // Retrieve the description of the newly added variable
    TArray<FStructVariableDescription>& VarDesc = FStructureEditorUtils::GetVarDesc(Struct);
    if (VarDesc.Num() > 0)
    {
        FStructVariableDescription& NewVarDesc = VarDesc.Last();

        // Rename the variable using the provided utility function
        if (!FStructureEditorUtils::RenameVariable(Struct, NewVarDesc.VarGuid, VarName.ToString()))
        {
            UE_LOG(AIITKEditorLog, Error, TEXT("Failed to rename variable to %s"), *VarName.ToString());
            return false;
        }

        // Set the initial default value using FStructureEditorUtils
        if (!DefaultValue.IsEmpty())
        {
            FStructureEditorUtils::ChangeVariableDefaultValue(Struct, NewVarDesc.VarGuid, DefaultValue);
        }
    }

    FStructureEditorUtils::OnStructureChanged(Struct);
    return true;
}

