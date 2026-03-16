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


 // ========== AIITK_NTK_NetworkingToolkitEditorFunctionLibrary.h ==========

#pragma once

#include "Misc/EngineVersion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "CoreMinimal.h"


#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/UserDefinedStruct.h"
#include "EdGraph/EdGraphPin.h"

#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5)
#include "StructUtils/UserDefinedStruct.h"
#endif

#include "AIITK_NTK_NetworkingToolkitEditorFunctionLibrary.generated.h"

UCLASS()
class AIITK_NTK_NETWORKINGTOOLKITEDITOR_API UAIITK_NTK_NetworkingToolkitEditorFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
    * Creates a Blueprint struct (UUserDefinedStruct) from the provided JSON input.
    *
    * @param JsonInput - A JSON string describing the struct's fields and values.
    * @param DirectoryPath - The content browser path where the struct should be created (e.g. "/Game/MyFolder").
    * @param StructName - The name to assign to the new struct.
    * @return true if the struct was created successfully; false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|EditorUtilities")
    static bool CreateBPStructFromJson(const FString& JsonInput, const FString& DirectoryPath, const FString& StructName);

    /**
    * Splits a Blueprint struct with nested sub-structs into multiple asset packages.
    * Useful for organizing large or deeply nested struct assets into separate files.
    *
    * @param RootStructObject - The root UUserDefinedStruct to split.
    * @param BaseDirectory - The content path to place the new struct assets (e.g. "/Game/Structs/SubParts").
    * @return true if the operation succeeded; false otherwise.
    */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|EditorUtilities")
    static bool SplitBPStructIntoMultiplePackages(UObject* RootStructObject, const FString& BaseDirectory);

    /**
    * Retrieves the directory (i.e. the folder path portion) for the asset represented by the passed-in object.
    * For example, if the asset path is "/Game/MyFolder/MyAsset", this function will return "/Game/MyFolder".
    *
    * @param Asset - The asset to query.
    * @return The directory path portion of the asset's package, or an empty string if invalid.
    */
    UFUNCTION(BlueprintPure, Category = "*AIITK|NetworkingToolkit|Editor Utilities")
    static FString GetAssetDirectory(UObject* Asset);

private:
    static bool AddVariableToStruct(UUserDefinedStruct* Struct, const FName& VarName, const FEdGraphPinType& PinType, const FString& DefaultValue);
    static UUserDefinedStruct* CreateSubStructFromJsonObject(const TSharedPtr<FJsonObject>& JsonObject, const FString& ParentName, UPackage* Package);
};
