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



// =================== GPTChatAPIStructs.h ===================
#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "JsonObjectWrapper.h"
#include "JsonDomBuilder.h"
#include "Templates/SharedPointer.h"
#include "GPTChatAPIStructs.generated.h"


//Heh heh... you thought this was only going to be structs didn't you?

/******************************************************************
 *                          ENUMS                                 *
 ******************************************************************/



// =============== CHATGPT ENUMS ===============

UENUM(BlueprintType, Category = "AIITKStructs")
enum class EGPTResponseFormat : uint8
{
    Text UMETA(DisplayName = "Text"),
    JsonObject UMETA(DisplayName = "JSON Object")
};


UENUM(BlueprintType, Category = "AIITKStructs")
enum class EAPIKeyType : uint8
{
    OpenAI UMETA(DisplayName = "OpenAI"),
    Labs11 UMETA(DisplayName = "11Labs"),
    Meshy UMETA(DisplayName = "Meshy")
};

UENUM(BlueprintType, Category = "AIITKStructs")
enum class EFunctionCallMode : uint8
{
    Auto,
    None,
    SpecificFunction,
    Null
};

UENUM(BlueprintType, Category = "AIITKStructs")
enum class EGPTFieldTypes : uint8
{
    String,
    Number,
    Boolean,
    Enum,
    Object
};

UENUM(BlueprintType, Category = "AIITKStructs")
enum class EJsonEnumType : uint8
{
    String,
    Number
};

UENUM(BlueprintType, Category = "AIITKStructs")
enum class EContentType : uint8
{
    Text,
    Image
};

UENUM(BlueprintType, Category = "AIITKStructs")
enum class EGPTRole : uint8
{
    system UMETA(DisplayName = "system"),
    user UMETA(DisplayName = "user"),
    assistant UMETA(DisplayName = "assistant"),
    tool UMETA(DisplayName = "tool")
};


// =============== DALLE ENUMS ===============

UENUM(BlueprintType, Category = "AIITKStructs")
enum class EGenerationType : uint8
{
    Image UMETA(DisplayName = "Image"),
    Variation UMETA(DisplayName = "Variation"),
    Edit UMETA(DisplayName = "Edit")
};

// Define the image size options
UENUM(BlueprintType, Category = "AIITKStructs")
enum class EImageSize : uint8
{
    S256x256 UMETA(DisplayName = "256x256 - DALL-E 2"),
    S512x512 UMETA(DisplayName = "512x512 - DALL-E 2"),
    S1024x1024 UMETA(DisplayName = "1024x1024 - DALL-E 2/3"),
    S1792x1024 UMETA(DisplayName = "1792x1024 - DALL-E 3"),
    S1024x1792 UMETA(DisplayName = "1024x1792 - DALL-E 3")
};


// Define the image encoding options
UENUM(BlueprintType, Category = "AIITKStructs")
enum class EImageEncoding : uint8
{
    URL UMETA(DisplayName = "URL"),
    Base64 UMETA(DisplayName = "Base64")
};


// =============== WHISPER ENUMS ===============

UENUM(BlueprintType, Category = "AIITKStructs")
enum class EWhisperResponseFormat : uint8
{
    json        UMETA(DisplayName = "json"),
    text        UMETA(DisplayName = "text"),
    srt         UMETA(DisplayName = "srt"),
    verbose_json UMETA(DisplayName = "verbose_json"),
    vtt         UMETA(DisplayName = "vtt")
};


UENUM(BlueprintType, Category = "AIITKStructs")
enum class EWhisperEndpoint : uint8
{
    Transcription,
    Translation
};


// =============== 11LABS ENUMS ===============

UENUM(BlueprintType, Category = "AIITKStructs")
enum class EOutputFormat : uint8
{
    pcm_16000,
    pcm_22050,
    pcm_24000,
    pcm_44100,
    mp3_44100_64,
    mp3_44100_96,
    mp3_44100_128,
    mp3_44100_192
};

UENUM(BlueprintType, Category = "AIITKStructs")
enum class EAudioFileFormat : uint8
{
    PCM,
    MP3
};

// =============== MESHY ENUMS ===============

UENUM(BlueprintType, Category = "AIITKStructs")
enum class EMeshyTextureType : uint8
{
    BaseColor UMETA(DisplayName = "Base Color"),
    Metallic UMETA(DisplayName = "Metallic"),
    Normal UMETA(DisplayName = "Normal"),
    Roughness UMETA(DisplayName = "Roughness")
};

UENUM(BlueprintType, Category = "AIITKStructs")
enum class EMeshyTaskType : uint8
{
    TextTo3D,
    TextToTexture
};


/******************************************************************
 *                          STRUCTS                                 *
 ******************************************************************/









 // =============== CHATGPT STRUCTS ===============

// ---- Log Probs ----

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FTopLogProbs
{
    GENERATED_BODY()

    /** The token for which top log probabilities are calculated. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString Token = "";

    /** The log probability of the token. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    float LogProb = 0.0f;

    /** The UTF-8 bytes representation of the token. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<int32> Bytes;
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FLogProbContent
{
    GENERATED_BODY()

    /** The token for which top log probabilities are calculated. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString Token = "";

    /** The log probability of the token. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    float LogProb = 0.0f;

    /** The UTF-8 bytes representation of the token. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<int32> Bytes;

    /** An array of the top log probabilities for each token position. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<FTopLogProbs> TopLogProbs;
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FLogProbs
{
    GENERATED_BODY()

    /** An array of log probabilities for each content token. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<FLogProbContent> Content;
};



// ---- Message Types ----

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FToolCall
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int32 ToolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString ID = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString Type = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString FunctionName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString FunctionArguments = "";
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FSystemMessage
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString Role = TEXT("system");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString Content = "";
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FContentPart
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    EContentType Type = EContentType::Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString Content = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString Detail = "";
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FUserMessage
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AIITKStructs")
    FString Role = TEXT("user");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    TArray<FContentPart> Content;
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FAssistantMessage
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AIITKStructs")
    FString Role = TEXT("assistant");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString Content = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<FToolCall> ToolCalls;
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FToolMessage
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AIITKStructs")
    FString Role = TEXT("tool");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString Content = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString ToolCallID;
};



// --- Usage Info ---

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FUsageContent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int32 PromptTokens = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int32 CompletionTokens = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int32 TotalTokens = 0;
};


USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FGPTChatMessage
{

    GENERATED_BODY()

    /** Role of the message author (e.g., user, assistant, system, tool). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    EGPTRole Role = EGPTRole::system;

    /** Optional name for the model to differentiate between participants of the same role. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString Name = "";

    /** System Message Content. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (EditCondition = "Role==EGPTRole::system"))
    FSystemMessage SystemMessage;

    /** User Message Content. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (EditCondition = "Role==EGPTRole::user"))
    FUserMessage UserMessage;

    /** Assistant Message Content. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (EditCondition = "Role==EGPTRole::assistant"))
    FAssistantMessage AssistantMessage;

    /** Tool Message Content. This is for returning  */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (EditCondition = "Role==EGPTRole::tool"))
    FToolMessage ToolMessage;

    /** Raw JSON Message */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString RawMessage = "";
};




// ---- Completion Data ----

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FGPTChatChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    FString FinishReason = "";

    //The index of the choice in the list of choices. A.K.A. if N param is > 1, this indicates which generation it is.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    int32 Index = 0;

    /* Message from the API with the generated content.
    This is called 'Delta' in a chunk completion object. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    FGPTChatMessage Message;

    //Log probability information for the choice.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FLogProbs LogProbs;
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FGPTChatCompletionInfo
{
    GENERATED_BODY()

    //A unique identifier for the chat completion. Each streamed chunk has the same ID.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    FString ID = "";

    // The object type, chat.completion, chat.completion.chunk
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    FString Object = "default.completion";

    // The Unix timestamp(in seconds) of when the chat completion was created.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    int32 Created = 0;

    // The model used for the chat completion.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    FString Model = "default";

    /* This fingerprint represents the backend configuration that the model runs with.
    Can be used in conjunction with the seed request parameter to understand when backend
    changes have been made that might impact determinism.*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    FString SystemFingerprint = "fingerprint";

    // Usage info like token count, etc. NOT AVAILABLE WITH STREAMED RESPONSES
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    FUsageContent Usage;
};

// The completion object for normal and chunked responses
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FGPTChatCompletion
{
    GENERATED_BODY()

    // A list of chat completion choices. Can be more than one if n is greater than 1.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    TArray<FGPTChatChoice> Choices;

    // Statistical information about the response
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    FGPTChatCompletionInfo CompletionInfo;
};



// ---- CHATGPT FUNCTION/TOOL STRUCTS ----

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FJsonEnumData
{
    GENERATED_BODY()

        // Enum type to specify the type of data our enum holds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs|JSON")
        EJsonEnumType EnumType = EJsonEnumType::String;

    // Array of strings representing the enum data.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs|JSON")
        TArray<FString> EnumValues;

};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FObjectLevel3
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        bool bIsArray = false;

    // Use 0 for no limit. If X is 0, no minimum constraint. If Y is 0, no maximum constraint.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", Meta = (DisplayName = "Array Size (Min X, Max Y)", Tooltip = "Specify the minimum (X) and maximum (Y) allowed array size. Use 0 for no constraint.", EditCondition = "bIsArray == true"))
        FVector2D ArraySizeConstraints = FVector2D(0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        EGPTFieldTypes FieldType = EGPTFieldTypes::String;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        FString FieldName = "Field_Name_ObjectLevel3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        FString Description = "This is the description for the field.";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", Meta = (EditCondition = "FieldType == EGPTFieldTypes::Object"))
        TArray<FString> InputObject;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", Meta = (EditCondition = "FieldType == EGPTFieldTypes::Enum"))
        FJsonEnumData EnumData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
        bool bIsRequired = true;
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FObjectLevel2
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
        bool bIsArray = false;

    // Use 0 for no limit. If X is 0, no minimum constraint. If Y is 0, no maximum constraint.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", Meta = (DisplayName = "Array Size (Min X, Max Y)", Tooltip = "Specify the minimum (X) and maximum (Y) allowed array size. Use 0 for no constraint.", EditCondition = "bIsArray == true"))
        FVector2D ArraySizeConstraints = FVector2D(0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        EGPTFieldTypes FieldType = EGPTFieldTypes::String;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        FString FieldName = "Field_Name_ObjectLevel2";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        FString Description = "This is the description for the field.";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", Meta = (EditCondition = "FieldType == EGPTFieldTypes::Object"))
        TArray<FObjectLevel3> InputObject;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", Meta = (EditCondition = "FieldType == EGPTFieldTypes::Enum"))
        FJsonEnumData EnumData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
        bool bIsRequired = true;
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FObjectLevel1
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
        bool bIsArray = false;

    // Use 0 for no limit. If X is 0, no minimum constraint. If Y is 0, no maximum constraint.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", Meta = (DisplayName = "Array Size (Min X, Max Y)", Tooltip = "Specify the minimum (X) and maximum (Y) allowed array size. Use 0 for no constraint.", EditCondition = "bIsArray == true"))
        FVector2D ArraySizeConstraints = FVector2D(0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        EGPTFieldTypes FieldType = EGPTFieldTypes::String;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        FString FieldName = "Field_Name_ObjectLevel1";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        FString Description = "This is the description for the field.";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", Meta = (EditCondition = "FieldType == EGPTFieldTypes::Object"))
        TArray<FObjectLevel2> InputObject;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", Meta = (EditCondition = "FieldType == EGPTFieldTypes::Enum"))
        FJsonEnumData EnumData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
        bool bIsRequired = true;
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FBaseObject
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
        bool bIsArray = false;

    // Use 0 for no limit. If X is 0, no minimum constraint. If Y is 0, no maximum constraint.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", Meta = (DisplayName = "Array Size (Min X, Max Y)", Tooltip = "Specify the minimum (X) and maximum (Y) allowed array size. Use 0 for no constraint.", EditCondition = "bIsArray == true"))
        FVector2D ArraySizeConstraints = FVector2D(0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        EGPTFieldTypes FieldType = EGPTFieldTypes::String;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        FString FieldName = TEXT("Field_Name_BaseObject");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        FString Description = TEXT("This is the description for the field.");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", Meta = (EditCondition = "FieldType == EGPTFieldTypes::Object"))
        TArray<FObjectLevel1> InputObject;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", Meta = (EditCondition = "FieldType == EGPTFieldTypes::Enum"))
        FJsonEnumData EnumData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
        bool bIsRequired = true;
};

/*Function err... I mean "Tool" object...
that's of type 'function'... which is the only type...
I guess I should rename this to "FGPTChatTool" but that would break a lot of things.
Maybe I will rename it later, idk. */
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FGPTChatFunction 
{
    GENERATED_BODY()

    // 'function' is the only type of tool available.
    UPROPERTY(VisibleAnywhere, Category = "AIITKStructs")
        FString Type = "function";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        FString Name = "Chat_Function_Name";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        FString Description = "Default description for the function.";

    // This is where you put your JSON Schemas if created via struct
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        TArray<FBaseObject> Parameters;
    
    // This is where you put your JSON Schemas if created manually or with MakeGPTJson
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
        TArray<FString> RawJsonParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
        bool bIsRequired = true;
};



// ---- CHATGPT MAIN REQUEST STRUCTS ----

// Function related parameters
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FGPTChatFunctionParams
{
    GENERATED_BODY()

    // Define the mode for function call handling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    EFunctionCallMode FunctionCallMode = EFunctionCallMode::Null;

    /* Input the specific function name if the mode is SpecificFunction
    Creates an object in the format:
    {"type: "function", "function": {"name": "My_Function"}} */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString SpecificFunctionName = "My_Function";
};

// Tool related parameters
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FGPTChatToolParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "AIITKStructs", Meta = (ToolTip = "Parameters related to messages and functions."))
    FGPTChatFunctionParams ToolChoice;

    /* Previously 'Functions'. A list of the 'tools' (Json Schemas)
    that you want ChatGPT to return parameters for.
    'function' is the only supported tool type right now. */
    UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<FGPTChatFunction> Tools;
};

// Required Parameters
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FGPTChatRequiredParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", Meta = (ToolTip = "ID of the model to use."))
    FString Model = "gpt-3.5-turbo";

    UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "AIITKStructs", Meta = (ToolTip = "Please paste your API key into: ProjectSettings->Plugins->AIITKDeveloperSettings"))
    FString APIKey = "DO NOT USE Set API key in Project settings";

    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "AIITKStructs", BlueprintReadWrite, Meta = (ToolTip = "API endpoint for chat completions."))
    FString Endpoint = "https://api.openai.com/v1/chat/completions";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", Meta = (ToolTip = "Enable partial message streaming."))
    bool StreamResponse = false;
};

// Advanced Parameters
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FGPTChatAdvancedParams
{
    GENERATED_BODY()

    // Max tokens to generate in the completion.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int32 MaxTokens = 1000;

    // Sampling temperature between 0 and 2.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", Meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0", SliderExponent = "1", Delta = "0.01"))
    float Temperature = 1.0f;

    // Probability mass for nucleus sampling.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    float TopP = 1.0f;

    // Number of chat completion choices.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    int32 N = 1;

    // Max tokens to generate in the completion.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int32 Seed = -1;

    // Sequences where the API will stop generating tokens.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    FString Stop = "";

    // Penalty for new topics based on presence in text.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    float PresencePenalty = 0.0f;

    // Penalty for token frequency in the text.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    float FrequencyPenalty = 0.0f;

    // Bias values for specific tokens.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    TMap<int32, float> LogitBias;

    // Whether to return log probabilities of the output tokens.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    bool LogProbs = false;

    // Number of most likely tokens to return at each token position, with log probabilities.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    int32 TopLogProbs = 0;

    // Response timeout in seconds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    float ResponseTimeout = 30.0f;

    /* Setting to { "type": "json_object" } enables JSON mode, which guarantees the message the model generates is valid JSON.
    Important: when using JSON mode, you must also instruct the model to produce JSON yourself via a system or user message. 
    Without this, the model may generate an unending stream of whitespace until the generation reaches the token limit. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    EGPTResponseFormat ResponseFormat = EGPTResponseFormat::Text;

    // Unique identifier for end-user.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs")
    FString User;
};

// Main Struct
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FGPTChatPromptParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", Meta = (ToolTip = "List of messages in the conversation."))
    TArray<FGPTChatMessage> Messages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", Meta = (ToolTip = "Required parameters for the API."))
    FGPTChatRequiredParams RequiredParams;

    UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "AIITKStructs", Meta = (ToolTip = "Advanced options for chat completion."))
    FGPTChatAdvancedParams AdvancedParams;

    UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "AIITKStructs", Meta = (ToolTip = "Advanced options for chat completion."))
    FGPTChatToolParams ToolParams;

};


 // =============== DALLE STRUCTS ===============

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FDallEAPIParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "AIITKStructs")
    EGenerationType RequestType = EGenerationType::Image;

    UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "AIITKStructs", Meta = (ToolTip = "Please paste your API key into: ProjectSettings->Plugins->AIITKDeveloperSettings"))
        FString APIKey = "Set your API key in project settings";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
        FString Prompt = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
        int32 NumberOfImages = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", Meta = (DefaultValue = "EImageSize::S256x256"))
        EImageSize ImageSize = EImageSize::S256x256;

    UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "AIITKStructs", Meta = (DefaultValue = "EImageEncoding::Base64"))
        EImageEncoding Encoding = EImageEncoding::Base64;

    UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "AIITKStructs", Meta = (DefaultValue = "https://api.openai.com/v1/images/generations"))
        FString Endpoint = TEXT("https://api.openai.com/v1/images/generations");

    UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "AIITKStructs", Meta = (ToolTip = "ID of the model"))
        FString Model = "dall-e-2";

    UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "AIITKStructs", Meta = (ToolTip = "'hd' creates images with finer details and greater consistency across the image. This param is only supported for dall-e-3"))
        FString Quality = "standard";

    UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "AIITKStructs", Meta = (DefaultValue = "60.0"))
        float Timeout = 60.0f;

    UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "AIITKStructs", Meta = (EditCondition = "RequestType == EGenerationType::Variation || RequestType == EGenerationType::Edit"))
        FString ImageToSend = "";

    UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "AIITKStructs", Meta = (EditCondition = "RequestType == EGenerationType::Edit"))
        FString MaskToSend = "";
};


 // =============== WHISPER STRUCTS ===============

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FWhisperAPIParams
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "AIITKStructs", Meta = (ToolTip = "Please paste your API key into: ProjectSettings->Plugins->AIITKDeveloperSettings"))
    FString APIKey = "Set your API key in project settings";

    UPROPERTY(BlueprintReadWrite, Category = "AIITKStructs", EditAnywhere, Meta = (ToolTip = "Path to the audio file for translation"))
        FString FilePath = "ProjectDir/Sounds";

    UPROPERTY(BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", EditAnywhere, Meta = (ToolTip = "Format of the transcript output, defaults to JSON"))
        EWhisperResponseFormat ResponseFormat = EWhisperResponseFormat::json;

    UPROPERTY(BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", EditAnywhere, Meta = (ToolTip = "ID of the model, currently only whisper-1 is available"))
        FString Model = "whisper-1";

    UPROPERTY(BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", EditAnywhere, Meta = (ToolTip = "API endpoint for the request"))
        EWhisperEndpoint Endpoint = EWhisperEndpoint::Transcription;

    UPROPERTY(BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", EditAnywhere, Meta = (ToolTip = "Language for the transcription, defaults to English"))
        FString Language = "en";

    UPROPERTY(BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", EditAnywhere, Meta = (MultiLine = true, ToolTip = "Optional text to guide the model's style"))
        FString Prompt = "";

    UPROPERTY(BlueprintReadWrite, AdvancedDisplay, Category = "AIITKStructs", EditAnywhere, Meta = (ToolTip = "Sampling temperature, affects output randomness"))
        float Temperature = 0.0f;
};


 // =============== 11LABS STRUCTS ===============

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FVoiceInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString Name = "Josh";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString VoiceID = "EnterVoiceIDHere";
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FModelInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString Name = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString ModelID = "";

};

USTRUCT(BlueprintType)
struct FCharacterAlignment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<FString> Characters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<float> StartTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<float> EndTimes;

    // Normalized alignment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<FString> NormalizedCharacters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<float> NormalizedStartTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<float> NormalizedEndTimes;
};

USTRUCT(BlueprintType)
struct FCondensedAlignment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<FString> Characters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<float> StartTimes;
};


USTRUCT(BlueprintType, Category = "AIITKStructs")
struct F11LabsRequiredParams
{
    GENERATED_BODY()

    // Your unique API key for accessing the service
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString APIKey = "Set API key in project settings or use SetAPIKey";

    // Disk location followed by file name for converting MP3 data to WAV for use in UE (ex: ProjDir/Sounds/Temp/ConvertMe), Defaults to ProjDir/Sounds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", DisplayName = "MP3 Convert Location")
    FString FileName = "";

    // Identifier for the voice model to be used, retrieve a list of models avalable to you with the associated request function
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString VoiceModel = "eleven_multilingual_v2";

    // Unique identifier for the specific voice, retrieve a list of voices avalable to you with the associated request function
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString VoiceId = "TxGEqnHWrfWFTfGW9XjX";

    // Endpoint URL for the text-to-speech API
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString Endpoint = "https://api.elevenlabs.io/v1/text-to-speech/";

    // Option to include timestamps in the response
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    bool IncludeTimestamps = false;
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct F11LabsStreamParams
{
    GENERATED_BODY()

    // Enables or disables streaming of responses
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    bool bStreamedResponse = false;

    // Configures latency optimizations, affecting audio quality
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", Meta = (UIMin = "0", UIMax = "4"))
    int32 OptimizeStreamingLatency = 0;

    // Select the audio output format
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    EOutputFormat OutputFormat = EOutputFormat::pcm_24000;
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct F11LabsVoiceSettings
{
    GENERATED_BODY()

    // Adjusts the stability of voice generation, affecting the predictability and variability of output
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", Meta = (UIMin = "0", UIMax = "1"))
    float Stability = 0.5f;

    // Increases the likelihood of generating a voice similar to the target voice
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", Meta = (UIMin = "0", UIMax = "1"))
    float SimilarityBoost = 1;

    // Defines the stylistic aspects of voice generation, specific to V2+ models
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", Meta = (UIMin = "0", UIMax = "1"))
    float Style = 0;

    // Determines whether to enhance the speaker's distinct characteristics in the generated voice, specific to V2+ models
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    bool UseSpeakerBoost = true;
};

USTRUCT(BlueprintType, Category = "AIITKStructs")
struct F11LabsRequestParams
{
    GENERATED_BODY()

    // Text prompt for the voice generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString TextPrompt = "";

    // Required parameters for the request
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    F11LabsRequiredParams RequiredParams;

    // Parameters for stream handling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    F11LabsStreamParams StreamParams;

    // Settings for voice modulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    F11LabsVoiceSettings VoiceSettings;
};

// This is an experimental feature at the moment but seems to work okay with default settings
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FOpenAITTSRequestParams
{
    GENERATED_BODY()

    // Text prompt for the voice generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString TextPrompt = "";

    // The model to be used for text-to-speech conversion
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString Model = "tts-1";

    // The voice to be used for the speech
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString Voice = "alloy";

    // The response format of the audio (PCM 24000kHz only, use conversion helpers if needed)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    EAudioFileFormat ResponseFormat = EAudioFileFormat::PCM;

    // The speed of the generated audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    float Speed = 1.0;

    // Disk location followed by file name for converting MP3 data to WAV for use in UE (ex: ProjDir/Sounds/Temp/ConvertMe), Defaults to ProjDir/Sounds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", DisplayName = "MP3 Convert Location")
    FString FileName = "";

    // 24000 only right now, weird results if this is changed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int32 ProcAudioSampleRate = 24000;
};

// =============== MESHY STRUCTS ===============

// Struct for Model URLs
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FModelURLs
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString glb = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString fbx = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString usdz = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString obj = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString mtl = "";
};

// Struct for Texture URLs
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FTextureURL
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString base_color = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString metallic = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString normal = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString roughness = "";
};

// Struct for Task Error
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FTaskError
{
    GENERATED_BODY()

    // Detailed error message.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString message = "";
};

// Struct for Text to 3D Task Model that is returned by the API
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FTextTo3DTask
{
    GENERATED_BODY()

    // Unique identifier for the task.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString id = "";

    // Downloadable URLs to the model files.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FModelURLs model_urls;

    // Text prompt for generating the 3D model.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString prompt = "";

    // Text prompt for generating the texture.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString object_prompt = "";

    // Text prompt for the style of the texture.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString style_prompt = "";

    // Text prompt for excluding certain elements.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString negative_prompt = "";

    // The `art_style` parameter allows you to specify the desired art style of the object.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString art_style = "";

    // Text prompt for texture richness.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString texture_richness = "";

    // URL for the thumbnail image.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString thumbnail_url = "";

    // Progress of the task.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int32 progress = 0;

    // Timestamp of when the task started, in milliseconds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int64 started_at = 0;

    // Timestamp of when the task was created, in milliseconds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int64 created_at = 0;

    // Timestamp of when the task result expires, in milliseconds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int64 expires_at = 0;

    // Timestamp of when the task was finished, in milliseconds.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int64 finished_at = 0;

    // Status of the task.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString status = "";

    // Error object that contains the error message if the task failed.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FTaskError task_error;

    // Array of texture URL objects that are generated from the task.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    TArray<FTextureURL> texture_urls;
};

// Struct for creating a preview task
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FMeshyPreviewTaskParams
{
    GENERATED_BODY()

    // Mode for the task, should be "preview".
    UPROPERTY(VisibleAnywhere, Category = "AIITKStructs")
    FString mode = "preview";

    // Text prompt for generating the preview.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString prompt = "";

    // Text prompt for excluding certain elements.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString negative_prompt = "";

    /**
     * The `art_style` parameter allows you to specify the desired art style of the object.
     * If not specified, it defaults to "realistic".
     * Available values are:
     *     - realistic: Realistic style
     *     - cartoon: Cartoon style
     *     - low-poly: Low Poly style
     *     - sculpture: Sculpture style
     *     - pbr: PBR style
     * Usage: Set the `art_style` parameter to one of the above values to change the art style of the object.
     *
     * Example:
     *     art_style = "cartoon";  // Sets the art style to Cartoon style
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString art_style = "realistic";

    // Seed for generating the preview. Using the same seed will generate the same result.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    int32 seed = 0;
};

// Struct for creating a refine task
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FMeshyRefineTaskParams
{
    GENERATED_BODY()

    // Mode for the task, should be "refine".
    UPROPERTY(VisibleAnywhere, Category = "AIITKStructs")
    FString mode = "refine";

    // ID of the corresponding preview task.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString preview_task_id = "";

    // Desired texture richness.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString texture_richness = "";
};

// Struct for creating a Text to Texture Task
USTRUCT(BlueprintType, Category = "AIITKStructs")
struct FMeshyTextToTextureTaskParams
{
    GENERATED_BODY()

    // URL to the 3D model.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString model_url = "";

    // Text prompt for the object.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString object_prompt = "";

    // Text prompt for the style of the texture.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString style_prompt = "";

    // Use the original UV of the model.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    bool enable_original_uv = true;

    // Generate PBR Maps (metallic, roughness, normal) in addition to the base color.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    bool enable_pbr = true;

    // Text prompt for excluding certain elements.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs", meta = (MultiLine = true))
    FString negative_prompt = "";

    // Resolution of the generated textures. Available values: 1024, 2048, 4096.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString resolution = "1024";

   /**
    * The `art_style` parameter allows you to specify the desired art style of the object.
    * If not specified, it defaults to "realistic".
    * Available values are:
    *     - realistic: Realistic style
    *     - fake-3d-cartoon: 2.5D Cartoon style
    *     - japanese-anime: Japanese Anime style
    *     - cartoon-line-art: Cartoon Line Art style
    *     - realistic-hand-drawn: Realistic Hand-drawn style
    *     - fake-3d-hand-drawn: 2.5D Hand-drawn style
    *     - oriental-comic-ink: Oriental Comic Ink style
    * Usage: Set the `art_style` parameter to one of the above values to change the art style of the object.
    *
    * Example:
    *     art_style = "japanese-anime";  // Sets the art style to Japanese Anime style
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIITKStructs")
    FString art_style = "realistic";
};

