// AIITKEditorFunctionLibrary.h
#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/UserDefinedStruct.h"
#include "EdGraph/EdGraphPin.h"

// Include StructUtils/UserDefinedStruct.h only for Unreal Engine version 5.5 and above
#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5)
#include "StructUtils/UserDefinedStruct.h"
#endif

#include "AIITKEditorFunctionLibrary.generated.h"

UCLASS()
class EZUEGPTEDITOR_API UAIITKEditorFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Creates a blueprintable struct from a JSON schema or JSON data string.
     * @param JsonInput - The JSON input string.
     * @param DirectoryPath - The directory path to save the struct.
     * @param StructName - The name of the struct to be created.
     * @return Whether the struct creation was successful.
     */
     /** Creates a blueprintable struct from a JSON schema or JSON data string */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Editor Utilities")
    static bool CreateBlueprintableStructFromJson(const FString& JsonInput, const FString& DirectoryPath, const FString& StructName);

private:
    /**
     * Helper function to add a variable to the struct with correct validation.
     * @param Struct - The struct to which the variable will be added.
     * @param VarName - The name of the variable to add.
     * @param PinType - The type of the variable.
     * @param DefaultValue - The default value for the variable.
     * @return Whether the variable was added successfully.
     */
     /** Helper function to add a variable to the struct with correct validation */
    static bool AddVariableToStruct(UUserDefinedStruct* Struct, const FName& VarName, const FEdGraphPinType& PinType, const FString& DefaultValue);

    /** Helper function to create sub-structs from JSON objects */
    /**
     * Helper function to create sub-structs from JSON objects.
     * @param JsonObject - The JSON object to convert into a sub-struct.
     * @param ParentName - The name of the parent struct.
     * @param Package - The package where the sub-struct will be created.
     * @return The created sub-struct, or nullptr if creation failed.
     */
    static UUserDefinedStruct* CreateSubStructFromJsonObject(const TSharedPtr<FJsonObject>& JsonObject, const FString& ParentName, UPackage* Package);
};
