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


// ========== AIITK_NTK_NetworkingToolkitSettings.h ==========

#pragma once

#include "Misc/EngineVersion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "AIITK_NTK_NetworkingToolkitSettings.generated.h"

/**
 * -- AI Integration Toolkit/Networking Toolkit --
 * --          Secure Key Storage              --
 *
 * Store your sensitive data here for safe retrieval in your blueprints.
 *
 * Add a key to the list and it will automatically be encrypted and
 * stored in the runtime version of DefaultGame.ini in 'ProjDir/Saved/Config/WindowsEditor/Game.ini'
 *
 * Use 'SetKey' and 'GetDecryptedKey' in blueprints to get/set/add keys at runtime or procedurally
 *
 * !IMPORTANT!
 * If you want the keys you enter to be included in a package you need to hit "Set As Default" below.
 * You need to do this every time you want to update the default keys packaged with the game.
 *
 * -Thanks!
 */
UCLASS(config = Game, meta = (DisplayName = "🔑 AIITK Key Storage"))
class AIITK_NTK_NETWORKINGTOOLKIT_API UAIITK_NTK_NetworkingToolkitSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    virtual FName GetCategoryName() const override
    {
        return TEXT("Project");
    }

    // Map of key names to encrypted keys; this property is serialized to Game.ini.
    UPROPERTY(config, EditAnywhere, Category = "Storage")
    TMap<FString, FString> EncryptedKeys;

#if WITH_EDITOR
    /**
     * Called after a property is edited in the editor.
     * @param PropertyChangedEvent - The details of the property change.
     */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;


    /**
     * Called after the settings object has been initialized.
     * Encrypts any plaintext keys in EncryptedKeys and saves the config if needed.
     */
    virtual void PostInitProperties() override;
#endif

    /**
     * Retrieves the decrypted key for a given key name.
     * @param KeyName - The name of the key to retrieve.
     * @return The decrypted plaintext value, or empty if not found or invalid.
     */
    UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Key From Storage"), Category = "*AIITK|NetworkingToolkit|Encryption")
    static FString GetDecryptedKey(const FString& KeyName);

    /**
     * Sets (encrypts and saves) the key for a given key name.
     * @param KeyName - The name of the key to set.
     * @param PlainTextKey - The plaintext key value to encrypt and store.
     */
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Key In Storage"), Category = "*AIITK|NetworkingToolkit|Encryption")
    static void SetKey(const FString& KeyName, const FString& PlainTextKey);



protected:
    /**
     * Encrypts a given plain-text key using AES and Base64.
     * @param PlainTextKey - The plaintext key to encrypt.
     * @return Base64-encoded encrypted string.
     */
    FString EncryptKey(const FString& PlainTextKey) const;

    /**
     * Decrypts the provided encrypted key (expects a value prefixed with "AES:").
     * @param EncryptedKey - The encrypted key string to decrypt.
     * @return The decrypted plaintext string after sanitization, or empty if invalid.
     */
    FString DecryptKey(const FString& EncryptedKey) const;

private:
    /**
     * Sanitizes a decrypted string by removing literal "\\u0000" sequences,
     * null characters, carriage returns, line feeds, and tabs.
     * @param OutputString - The string to sanitize in place.
     */
    static void SanitizeOutputString(FString& OutputString);

    // Marker prefix to indicate that a value is encrypted.
    static constexpr const TCHAR* EncryptionPrefix = TEXT("AES:");

    // The AES key for encryption/decryption.
    const TArray<uint8> EncryptionKey = {
        0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
        0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
        0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
        0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
    };

    // An optional initialization vector.
    const TArray<uint8> InitializationVector = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };
};
