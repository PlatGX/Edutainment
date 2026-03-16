// Copyright 2023 Kaleb Knoettgen. All rights reserved.

// UCustomK2Node_MakeGPTParameter.h

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "K2Node.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "GPTChatAPIStructs.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"
#include "OpenAIFunctionLibrary.h"
#include "K2Node_MakeArray.h"
#include "UObject/EnumProperty.h"
#include "CustomK2Node_MakeGPTParameter.generated.h"

UCLASS()
class EZUEGPTEDITOR_API UCustomK2Node_MakeGPTParameter : public UK2Node
{
    GENERATED_BODY()

public:

    // Pin Names
    static FName InputObjectPinName;
    static FName EnumTypePinName;
    static FName EnumValuesPinName;
    static FName TogglePinName;
    static FName ArraySizeConstraintsPinName;
    static FName FieldTypePinName;
    static FName FieldNamePinName;
    static FName DescriptionPinName;
    static FName RequiredPinName;


    // Create the pins for our node.
    virtual void AllocateDefaultPins() override;

    // Modify the node based on external changes
    virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;

    // Register the node in Blueprint Editor's menu
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;

    // Validate the node.
    virtual bool IsNodeSafeToIgnore() const override { return true; }

    // Determine if the node is pure
    virtual bool IsNodePure() const override { return true; }

    // Handle node expansion
    virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

    // Handle copy/paste
    virtual void PostPasteNode() override;

   // virtual void PostInitProperties() override;


    // Visual
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override
    {
        return NSLOCTEXT("YourNamespace", "YourKey", "Make GPT JSON Parameter");
    }

    virtual FLinearColor GetNodeTitleColor() const override
    {
        return FLinearColor(0.03f, 0.03f, 0.55f);  // Deep indigo

    }


private:

    UPROPERTY()
        bool bIsArray = false;

    UPROPERTY()
        bool bRequired = true;

    UPROPERTY()
        FString CurrentFieldType = TEXT("String");

    UPROPERTY()
        FString CurrentEnumFieldType = TEXT("String");

    UPROPERTY()
        FVector2D ArraySizeConstraints = FVector2D(0, 0);

};
