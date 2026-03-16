// Copyright 2023 Kaleb Knoettgen. All rights reserved.

// UCustomK2Node_MakeGPTParameter.cpp

#include "CustomK2Node_MakeGPTParameter.h"
#include "Math/Vector2D.h"
#include "EZUEGPTEditor.h"


FName UCustomK2Node_MakeGPTParameter::FieldTypePinName(TEXT("FieldType"));
FName UCustomK2Node_MakeGPTParameter::FieldNamePinName(TEXT("FieldName"));
FName UCustomK2Node_MakeGPTParameter::DescriptionPinName(TEXT("Description"));
FName UCustomK2Node_MakeGPTParameter::InputObjectPinName(TEXT("InputObject"));
FName UCustomK2Node_MakeGPTParameter::TogglePinName(TEXT("IsArray"));
FName UCustomK2Node_MakeGPTParameter::ArraySizeConstraintsPinName(TEXT("ArraySizeConstraints"));
FName UCustomK2Node_MakeGPTParameter::RequiredPinName(TEXT("Required"));
FName UCustomK2Node_MakeGPTParameter::EnumTypePinName(TEXT("EnumType"));
FName UCustomK2Node_MakeGPTParameter::EnumValuesPinName(TEXT("EnumValues"));



void UCustomK2Node_MakeGPTParameter::AllocateDefaultPins()
{
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Boolean, TogglePinName);

    UStruct* Vector2DStruct = TBaseStructure<FVector2D>::Get();
    UEdGraphPin* ArraySizeConstraintsPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, Vector2DStruct, ArraySizeConstraintsPinName);
    if (!bIsArray)
    {
        ArraySizeConstraintsPin->bHidden = true;
    }

    // Use StaticEnum to get the UEnum for EGPTFieldTypes
    UEnum* EnumClass = StaticEnum<EGPTFieldTypes>();
    if (EnumClass)
    {
        UEdGraphPin* EnumPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Byte, EnumClass, FieldTypePinName);
        EnumPin->DefaultValue = CurrentFieldType.IsEmpty() ? "String" : CurrentFieldType;
    }
    else
    {
        UE_LOG(AIITKEditorLog, Warning, TEXT("A.I.I.T.K: Failed to find the EGPTFieldTypes enum class!"));
    }

    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, FieldNamePinName)->DefaultValue = "DefaultFieldName";
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, DescriptionPinName)->DefaultValue = "DefaultDescription";

    // Input Object pin as an array of strings, regardless of the current FieldType value
    UEdGraphPin* InputObjectPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, InputObjectPinName);
    InputObjectPin->PinType.ContainerType = EPinContainerType::Array;
    InputObjectPin->bHidden = CurrentFieldType != TEXT("Object");

    // Use StaticEnum to get the UEnum for EJsonEnumType
    UEnum* JsonEnumClass = StaticEnum<EJsonEnumType>();
    UEdGraphPin* EnumTypePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Byte, JsonEnumClass, EnumTypePinName);
    EnumTypePin->DefaultValue = CurrentEnumFieldType;
    EnumTypePin->bHidden = CurrentFieldType != TEXT("Enum");

    UEdGraphPin* EnumValuesPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, EnumValuesPinName);
    EnumValuesPin->PinType.ContainerType = EPinContainerType::Array;
    EnumValuesPin->bHidden = CurrentFieldType != TEXT("Enum");

    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Boolean, RequiredPinName)->DefaultValue = "true";
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_String, FName("Output Schema"))->DefaultValue = TEXT("");
}




void UCustomK2Node_MakeGPTParameter::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    // Call the parent's implementation first
    Super::ExpandNode(CompilerContext, SourceGraph);

    // Use the class member variable bIsArray and bRequired directly
    bool bIsArrayLocal = bIsArray;
    //bool bRequiredLocal = bRequired;

    // Get the field type
    UEdGraphPin* FieldTypePin = FindPin(FieldTypePinName);
    FString FieldTypeValue = FieldTypePin ? FieldTypePin->GetDefaultAsString() : TEXT("");
    bool isObjectField = FieldTypeValue == TEXT("object");
    bool isEnumField = FieldTypeValue == TEXT("Enum");

    // Debug logs
    //UE_LOG(AIITKEditorLog, Warning, TEXT("bIsArray Member: %s"), bIsArrayLocal ? TEXT("True") : TEXT("False"));
    //UE_LOG(AIITKEditorLog, Warning, TEXT("isObjectField: %s"), isObjectField ? TEXT("True") : TEXT("False"));
    //UE_LOG(AIITKEditorLog, Warning, TEXT("isEnumField: %s"), isEnumField ? TEXT("True") : TEXT("False"));

    // Choose the right function to call based on conditions
    UFunction* FunctionToCall = nullptr;
    if (isObjectField)
    {
        FunctionToCall = UOpenAIFunctionLibrary::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UOpenAIFunctionLibrary, CreateJSONSchemaForObjectArray));
    }
    else if (isEnumField)
    {
        FunctionToCall = UOpenAIFunctionLibrary::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UOpenAIFunctionLibrary, CreateJSONSchemaForEnum));
    }
    else
    {
        FunctionToCall = UOpenAIFunctionLibrary::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UOpenAIFunctionLibrary, CreateJSONSchema));
    }

    // Log the selected function
    //UE_LOG(AIITKEditorLog, Warning, TEXT("A.I.I.T.K.: Selected Function: %s"), FunctionToCall ? *FunctionToCall->GetName() : TEXT("None"));

    // Create the function call node
    UK2Node_CallFunction* CallFuncNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
    CallFuncNode->FunctionReference.SetExternalMember(FunctionToCall->GetFName(), UOpenAIFunctionLibrary::StaticClass());
    CallFuncNode->AllocateDefaultPins();

    // Explicitly set the value of the IsArray pin in the function call node
    if (UEdGraphPin* CallFuncIsArrayPin = CallFuncNode->FindPin(TEXT("bIsArray")))
    {
        CallFuncIsArrayPin->DefaultValue = bIsArrayLocal ? TEXT("True") : TEXT("False");
    }

    // Explicitly set the value of the Required pin in the function call node
    if (UEdGraphPin* CallFuncIsRequiredPin = CallFuncNode->FindPin(TEXT("bRequired")))
    {
        CallFuncIsRequiredPin->DefaultValue = bRequired ? TEXT("True") : TEXT("False");
    }

    // Move the pin links to the function call node
    for (UEdGraphPin* OriginalPin : Pins)
    {
        if (OriginalPin->Direction == EEdGraphPinDirection::EGPD_Input)
        {
            UEdGraphPin* CallFuncPin = CallFuncNode->FindPin(OriginalPin->PinName);
            if (!CallFuncPin) continue;

            CompilerContext.MovePinLinksToIntermediate(*OriginalPin, *CallFuncPin);
        }
    }

    // Handle the output pin
    UEdGraphPin* CallFuncReturnPin = CallFuncNode->GetReturnValuePin();
    UEdGraphPin* OriginalOutputPin = FindPin(FName("Output Schema"));
    CompilerContext.MovePinLinksToIntermediate(*OriginalOutputPin, *CallFuncReturnPin);

    // Clean up
    BreakAllNodeLinks();
}


void UCustomK2Node_MakeGPTParameter::PinDefaultValueChanged(UEdGraphPin* Pin)
{
    if (Pin && Pin->PinName == TogglePinName)
    {
        bIsArray = Pin->GetDefaultAsString().Equals(TEXT("true"), ESearchCase::IgnoreCase);
        UEdGraphPin* ArraySizeConstraintsPin = FindPin(ArraySizeConstraintsPinName);
        ArraySizeConstraintsPin->bHidden = !bIsArray;
    }

    if (Pin && Pin->PinName == FieldTypePinName)
    {
        CurrentFieldType = Pin->GetDefaultAsString();

        // Object related pin
        UEdGraphPin* InputObjectPin = FindPin(InputObjectPinName);
        InputObjectPin->bHidden = CurrentFieldType != TEXT("object");

        // Enum related pins
        UEdGraphPin* EnumTypePin = FindPin(EnumTypePinName);
        UEdGraphPin* EnumValuesPin = FindPin(EnumValuesPinName);

        bool isEnumField = CurrentFieldType == TEXT("Enum");
        EnumTypePin->bHidden = !isEnumField;
        EnumValuesPin->bHidden = !isEnumField;

        EnumTypePin->DefaultValue = CurrentEnumFieldType;
    }

    if (Pin && Pin->PinName == EnumTypePinName)
    {
        CurrentEnumFieldType = Pin->DefaultValue;
    }

    if (Pin && Pin->PinName == RequiredPinName)
    {
        bRequired = Pin->GetDefaultAsString().Equals(TEXT("true"), ESearchCase::IgnoreCase);
    }

    GetGraph()->NotifyGraphChanged();
    Super::PinDefaultValueChanged(Pin);
}




void UCustomK2Node_MakeGPTParameter::PostPasteNode()
{
    Super::PostPasteNode();

    // Update the internal state variables based on the pin values
    UEdGraphPin* TogglePin = FindPin(TogglePinName);
    UEdGraphPin* FieldTypePin = FindPin(FieldTypePinName);

    if (TogglePin)
    {
        bIsArray = TogglePin->GetDefaultAsString().Equals(TEXT("true"), ESearchCase::IgnoreCase);
    }

    if (FieldTypePin)
    {
        CurrentFieldType = FieldTypePin->GetDefaultAsString();
    }

    UEdGraphPin* InputObjectPin = FindPin(InputObjectPinName);

    // Remove all existing input object pins
    if (InputObjectPin)
    {
        InputObjectPin->BreakAllPinLinks(true);
        RemovePin(InputObjectPin);
    }

    // Add the appropriate input object pin based on bIsArray
    InputObjectPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, InputObjectPinName);
    if (bIsArray)
    {
        InputObjectPin->PinType.ContainerType = EPinContainerType::Array;
    }

    GetGraph()->NotifyGraphChanged();
}



void UCustomK2Node_MakeGPTParameter::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    if (ActionRegistrar.IsOpenForRegistration(GetClass()))
    {
        UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
        ensure(NodeSpawner);  // Make sure the spawner is valid

        // Set up the menu entry details
        FBlueprintActionUiSpec UiSpec;
        UiSpec.MenuName = NSLOCTEXT("MakeGPTParam", "MenuName", "Make GPT JSON Parameter");
        UiSpec.Tooltip = NSLOCTEXT("MakeGPTParam", "Tooltip", "Creates a GPT Parameter.");
        UiSpec.Category = NSLOCTEXT("MakeGPTParam", "Category", "*AIITK|ChatGPT|JSON");
        UiSpec.Keywords = NSLOCTEXT("MakeGPTParam", "Keywords", "GPT, Parameter, Make");
        NodeSpawner->DefaultMenuSignature = UiSpec;

        // Register the spawner with the registrar
        ActionRegistrar.AddBlueprintAction(GetClass(), NodeSpawner);
    }
}

