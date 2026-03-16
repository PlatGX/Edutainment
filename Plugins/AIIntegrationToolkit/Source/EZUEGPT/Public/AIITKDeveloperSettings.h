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

// =================== AIITKDeveloperSettings.h ===================

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Misc/AES.h"
#include "Runtime/Core/Public/Misc/Base64.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "GPTChatAPIStructs.h"
#include "AIITKDeveloperSettings.generated.h"

// -------------------------------------------------------------------------
// 
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// 
//      AIITK V2 Developer Settings
// 
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// 
// Configuration Specifics:
// 
//  - Encrypted keys are read from and saved to: YourProject\Config\DefaultGame.ini
//  - If you remove the ini you will need to re-enter your API keys.
//  - Be aware that your API key will most likely get pushed to source control if you use it,
//      add DefaultGame.ini to the respective ignore list to prevent this.
//  - See documentation for info on packaging API keys.
//
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//      Encryption/Decryption
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// 
// Security Note:
// 
//  - For enhanced security, consider fetching 
//    the encryption key from a secure external server.
//  - Although the encryption key is compiled and the API 
//    keys are encrypted, take additional steps to harden 
//    security as per your project requirements.
//  - To add a layer of protection, open AIITKDeveloperSettings.h
//    and re-seed the EncryptionKey values.
// 
//  ~Thank you
// 
// -------------------------------------------------------------------------


UCLASS(config = Game)
class EZUEGPT_API UAIITKDeveloperSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "*AIITK|API Settings")
    FString PlainTextOpenAIApiKey;

    UPROPERTY(EditAnywhere, Category = "*AIITK|API Settings")
    FString PlainText11LabsAPIKey;

    UPROPERTY(EditAnywhere, Category = "*AIITK|API Settings")
    FString PlainTextMeshyAPIKey;

    UPROPERTY(config, VisibleAnywhere, Category = "*AIITK|API Settings")
    FString EncryptedOpenAIApiKey;

    UPROPERTY(config, VisibleAnywhere, Category = "*AIITK|API Settings")
    FString Encrypted11LabsAPIKey;

    UPROPERTY(config, VisibleAnywhere, Category = "*AIITK|API Settings")
    FString EncryptedMeshyAPIKey;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    // Allows user set API key at runtime
    UFUNCTION(BlueprintCallable, Category = "*AIITK|API Settings")
    static void SetAPIKey(EAPIKeyType KeyType, const FString& ApiKey);

    // !Warning! Retrieves the decrypted API key from config in plain text!
    UFUNCTION(BlueprintPure, Category = "*AIITK|API Settings")
    static FString GetAPIKey(EAPIKeyType KeyType);

    virtual FName GetCategoryName() const override
    {
        return TEXT("Plugins");
    }

    virtual void PostInitProperties() override;

private:
    static void LoadEncryptedApiKey(const FString& KeyName, FString& OutApiKey);
    static FString GetConfigKeyName(EAPIKeyType KeyType);
    void EncryptAndStoreApiKey(const FString& PlainTextApiKey, FString& EncryptedApiKey, const FString& ConfigKeyName);
    FString GetDecryptedApiKey(const FString& EncryptedApiKey);

    // I recommend re-seeding this Encryption key if you are distributing the API key with the game files
    const TArray<uint8> EncryptionKey = {
        0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
        0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
        0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
        0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
    };

    // This is not used but can be incorporated to improve security
    const TArray<uint8> InitializationVector = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };

};

