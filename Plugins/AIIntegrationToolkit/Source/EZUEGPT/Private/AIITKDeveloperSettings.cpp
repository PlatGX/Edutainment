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



// =================== AIITKDeveloperSettings.cpp ===================

#include "AIITKDeveloperSettings.h"
#include "Misc/OutputDeviceRedirector.h"
#include "OpenAIFunctionLibrary.h"

// Always use defaultgame.ini
FString CurrentConfigPath = FConfigCacheIni::NormalizeConfigIniPath(FPaths::ProjectDir() + TEXT("Config/DefaultGame.ini"));

void UAIITKDeveloperSettings::PostInitProperties()
{
    Super::PostInitProperties();

    UE_LOG(AIITKLog, Log, TEXT("ConfigPath: %s"), *CurrentConfigPath);

    GConfig->GetString(
        TEXT("/Script/EZUEGPT.UAIITKDeveloperSettings"),
        TEXT("EncryptedOpenAIApiKey"),
        EncryptedOpenAIApiKey,
        CurrentConfigPath
    );

    GConfig->GetString(
        TEXT("/Script/EZUEGPT.UAIITKDeveloperSettings"),
        TEXT("Encrypted11LabsAPIKey"),
        Encrypted11LabsAPIKey,
        CurrentConfigPath
    );

    GConfig->GetString(
        TEXT("/Script/EZUEGPT.UAIITKDeveloperSettings"),
        TEXT("EncryptedMeshyAPIKey"),
        EncryptedMeshyAPIKey,
        CurrentConfigPath
    );
}

#if WITH_EDITOR
void UAIITKDeveloperSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property)
    {
        FName PropertyName = PropertyChangedEvent.Property->GetFName();

        // Encryption and masking logic
        if (PropertyName == GET_MEMBER_NAME_CHECKED(UAIITKDeveloperSettings, PlainTextOpenAIApiKey))
        {
            EncryptAndStoreApiKey(PlainTextOpenAIApiKey, EncryptedOpenAIApiKey, TEXT("EncryptedOpenAIApiKey"));
            PlainTextOpenAIApiKey = FString::ChrN(PlainTextOpenAIApiKey.Len(), '*');
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UAIITKDeveloperSettings, PlainText11LabsAPIKey))
        {
            EncryptAndStoreApiKey(PlainText11LabsAPIKey, Encrypted11LabsAPIKey, TEXT("Encrypted11LabsAPIKey"));
            PlainText11LabsAPIKey = FString::ChrN(PlainText11LabsAPIKey.Len(), '*');
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UAIITKDeveloperSettings, PlainTextMeshyAPIKey))
        {
            EncryptAndStoreApiKey(PlainTextMeshyAPIKey, EncryptedMeshyAPIKey, TEXT("EncryptedMeshyAPIKey"));
            PlainTextMeshyAPIKey = FString::ChrN(PlainTextMeshyAPIKey.Len(), '*');
        }
    }
}
#endif

void UAIITKDeveloperSettings::SetAPIKey(EAPIKeyType KeyType, const FString& ApiKey)
{
    UAIITKDeveloperSettings* Instance = NewObject<UAIITKDeveloperSettings>();

    FString EncryptedApiKey;
    Instance->EncryptAndStoreApiKey(ApiKey, EncryptedApiKey, GetConfigKeyName(KeyType));

    Instance->ConditionalBeginDestroy();
}

FString UAIITKDeveloperSettings::GetAPIKey(EAPIKeyType KeyType)
{
    UAIITKDeveloperSettings* Instance = NewObject<UAIITKDeveloperSettings>();

    FString EncryptedApiKey;
    LoadEncryptedApiKey(GetConfigKeyName(KeyType), EncryptedApiKey);

    FString DecryptedApiKey = Instance->GetDecryptedApiKey(EncryptedApiKey);

    Instance->ConditionalBeginDestroy();

    return DecryptedApiKey;
}

void UAIITKDeveloperSettings::LoadEncryptedApiKey(const FString& KeyName, FString& OutApiKey)
{
    GConfig->GetString(
        TEXT("/Script/EZUEGPT.UAIITKDeveloperSettings"),
        *KeyName,
        OutApiKey,
        CurrentConfigPath
    );
}

FString UAIITKDeveloperSettings::GetConfigKeyName(EAPIKeyType KeyType)
{
    switch (KeyType)
    {
    case EAPIKeyType::OpenAI:
        return TEXT("EncryptedOpenAIApiKey");
    case EAPIKeyType::Labs11:
        return TEXT("Encrypted11LabsAPIKey");
    case EAPIKeyType::Meshy:
        return TEXT("EncryptedMeshyAPIKey");
    default:
        return TEXT("");
    }
}

void UAIITKDeveloperSettings::EncryptAndStoreApiKey(const FString& PlainTextApiKey, FString& EncryptedApiKey, const FString& ConfigKeyName)
{
    TArray<uint8> Data;
    Data.Append((uint8*)TCHAR_TO_UTF8(*PlainTextApiKey), PlainTextApiKey.Len());

    while (Data.Num() % 16 != 0)
    {
        Data.Add(0);
    }

    FAES::EncryptData(Data.GetData(), Data.Num(), EncryptionKey.GetData(), EncryptionKey.Num());

    EncryptedApiKey = FBase64::Encode(Data);

    GConfig->SetString(
        TEXT("/Script/EZUEGPT.UAIITKDeveloperSettings"),
        *ConfigKeyName,
        *EncryptedApiKey,
        CurrentConfigPath
    );
    GConfig->Flush(false, CurrentConfigPath);

    UE_LOG(AIITKLog, Warning, TEXT("Stored Encrypted API Key at: %s"), *CurrentConfigPath);
}

FString UAIITKDeveloperSettings::GetDecryptedApiKey(const FString& EncryptedApiKey)
{
    TArray<uint8> EncodedData;
    FBase64::Decode(EncryptedApiKey, EncodedData);

    // Decrypt the data using AES
    FAES::DecryptData(EncodedData.GetData(), EncodedData.Num(), EncryptionKey.GetData(), EncryptionKey.Num());

    // Convert the buffer to a string, excluding null characters at the end
    FString DecryptedApiKey;
    FFileHelper::BufferToString(DecryptedApiKey, EncodedData.GetData(), EncodedData.Num());

    // Trim any null termination characters that might have been added as padding during encryption
    DecryptedApiKey = DecryptedApiKey.Replace(TEXT("\\u0000"), TEXT(""));

    return DecryptedApiKey;
}

