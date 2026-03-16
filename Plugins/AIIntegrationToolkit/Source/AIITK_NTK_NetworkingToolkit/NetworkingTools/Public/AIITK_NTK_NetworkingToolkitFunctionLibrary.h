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


// ========== AIITK_NTK_NetworkingToolkitFunctionLibrary.h ==========

#pragma once

#include "Misc/EngineVersion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Texture2D.h"

#include "Http.h"
#include "AIITK_NTK_NetworkingToolkitFunctionLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNetworkingToolkit, Log, All);

DECLARE_DYNAMIC_DELEGATE_OneParam(FNTK_DownloadImageDelegate, UTexture2D*, DownloadedImage);

/**
 * Provides blueprint-callable networking and utility functions.
 */
UCLASS()
class AIITK_NTK_NETWORKINGTOOLKIT_API UAIITK_NTK_NetworkingToolkitFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    // ------------------------
    // General Utility
    // ------------------------

    /**
    * Opens the specified URL in the default system web browser.
    *
    * @param URL - The full web address to open (e.g. "https://www.kalebknoettgen.com").
    */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|Utility")
    static void OpenURLInBrowser(const FString& URL);

    /**
     * Searches the input string for all occurrences of the specified regex pattern.
     * @param InputString   The string to search.
     * @param Pattern       The regular expression pattern to match.
     * @param CaptureGroup  The capture group index to extract; if not found, the entire match is used.
     * @param OutMatches    (Output) Array populated with all matched substrings.
     * @return True if one or more matches were found; false otherwise.
     */
    UFUNCTION(BlueprintPure, Category = "*AIITK|NetworkingToolkit|Utility")
    static bool RegexSearch(
        const FString& InputString,
        const FString& Pattern,
        int32 CaptureGroup,
        TArray<FString>& OutMatches
    );

    /**
     * Calls a function on the specified UObject by name.
     * @param Target        The object to invoke the function on.
     * @param FunctionName  The name of the function to call.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|Utility", meta = (DefaultToSelf = "Target"))
    static void CallFunctionByName(
        UObject* Target,
        const FString& FunctionName
    );

    /**
     * Converts an enum type to an array of its string values.
     * @param Enum  The enum type to convert.
     * @return An array of strings representing each enumerator name.
     */
    UFUNCTION(BlueprintPure, Category = "*AIITK|NetworkingToolkit|Utility", DisplayName = "Enum To String Array")
    static TArray<FString> Conv_EnumToStringArray(
        const UEnum* Enum
    );

    /**
     * Converts a string to the corresponding enum value.
     * @param Enum         The enum type to parse.
     * @param StringValue  The string representation of the enum value.
     * @return The enum value as a byte; returns 0 if not found.
     */
    UFUNCTION(BlueprintPure, Category = "*AIITK|NetworkingToolkit|Utility", meta = (DisplayName = "String To Enum"))
    static uint8 Conv_StringToEnum(
        const UEnum* Enum,
        const FString& StringValue
    );

    // ------------------------
    // Image Processing
    // ------------------------

    /**
     * Converts a Base64-encoded string to a Texture2D object.
     * @param Base64String The Base64 string to decode.
     * @return A Texture2D created from the decoded data; nullptr on failure.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|Image Processing")
    static UTexture2D* Base64ToTexture(
        const FString& Base64String
    );

    /**
     * Encodes a Texture2D object to a Base64 string.
     * @param Texture  The Texture2D to encode.
     * @return A Base64-encoded string of the texture data; empty on failure.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|Image Processing")
    static FString TextureToBase64(
        UTexture2D* Texture
    );

    /**
     * Saves a Texture2D object to disk as an image file.
     * @param Texture   The texture to save.
     * @param FileName  The full file path (including extension) to save to.
     * @return True if the file was saved successfully; false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|Image Processing")
    static bool SaveTextureToDisk(
        UTexture2D* Texture,
        const FString& FileName
    );

    /**
     * Loads an image file from disk into a Texture2D object.
     * @param FileName  The full file path (including extension) to load.
     * @return A Texture2D loaded from the file; nullptr on failure.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|Image Processing")
    static UTexture2D* LoadTextureFromDisk(
        const FString& FileName
    );

    /**
     * Downloads an image from the specified URL and returns it via a delegate.
     * @param URL               The URL of the image to download.
     * @param OnImageDownloaded Delegate called on completion; provides the downloaded Texture2D (or nullptr on failure).
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|Image Processing")
    static void DownloadImageFromURL(
        const FString& URL,
        FNTK_DownloadImageDelegate OnImageDownloaded
    );

private:
    template<typename TEnum>
    static FString GetEnumValueAsString(const FString& Name, TEnum Value);
};
