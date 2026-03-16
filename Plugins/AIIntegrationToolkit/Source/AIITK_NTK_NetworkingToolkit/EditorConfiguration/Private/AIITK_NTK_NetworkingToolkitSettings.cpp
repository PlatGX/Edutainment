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


// ========== AIITK_NTK_NetworkingToolkitSettings.cpp ==========

// AIITK_NTK_NetworkingToolkitSettings.cpp

#include "AIITK_NTK_NetworkingToolkitSettings.h"
#include "Misc/AES.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"

//---------------------------------------------------------------------
//  Helpers
//---------------------------------------------------------------------
void UAIITK_NTK_NetworkingToolkitSettings::SanitizeOutputString(FString& OutputString)
{
    // Remove any literal \u0000 sequences (six characters: backslash, u,0,0,0,0)
    OutputString = OutputString.Replace(TEXT("\\u0000"), TEXT(""));

    // Strip out any actual null char, carriage return, newline, or tab
    OutputString.ReplaceInline(TEXT("\0"), TEXT(""), ESearchCase::CaseSensitive);
    OutputString = OutputString.Replace(TEXT("\r"), TEXT(""));
    OutputString = OutputString.Replace(TEXT("\n"), TEXT(""));
    OutputString = OutputString.Replace(TEXT("\t"), TEXT(""));
}

#if WITH_EDITOR
//---------------------------------------------------------------------
//  Post Initialization / Editor Change Handlers
//---------------------------------------------------------------------
void UAIITK_NTK_NetworkingToolkitSettings::PostInitProperties()
{
    Super::PostInitProperties();
    bool bUpdated = false;
    for (auto& Pair : EncryptedKeys)
    {
        if (!Pair.Value.StartsWith(EncryptionPrefix) && !Pair.Value.IsEmpty())
        {
            FString Encrypted = EncryptKey(Pair.Value);
            Pair.Value = FString(EncryptionPrefix) + Encrypted;
            bUpdated = true;
        }
    }
    if (bUpdated)
    {
        SaveConfig();
    }
}


void UAIITK_NTK_NetworkingToolkitSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    bool bUpdated = false;
    for (auto& Pair : EncryptedKeys)
    {
        if (!Pair.Value.StartsWith(EncryptionPrefix) && !Pair.Value.IsEmpty())
        {
            FString Encrypted = EncryptKey(Pair.Value);
            Pair.Value = FString(EncryptionPrefix) + Encrypted;
            bUpdated = true;
        }
    }
    if (bUpdated)
    {
        SaveConfig();
    }
}
#endif

//---------------------------------------------------------------------
//  Encryption/Decryption Methods
//---------------------------------------------------------------------
FString UAIITK_NTK_NetworkingToolkitSettings::EncryptKey(const FString& PlainTextKey) const
{
    TArray<uint8> Data;
    Data.Append(reinterpret_cast<const uint8*>(TCHAR_TO_UTF8(*PlainTextKey)), PlainTextKey.Len());

    // Pad the data to a multiple of 16 bytes.
    while (Data.Num() % 16 != 0)
    {
        Data.Add(0);
    }

    FAES::EncryptData(Data.GetData(), Data.Num(), EncryptionKey.GetData(), EncryptionKey.Num());
    return FBase64::Encode(Data);
}

FString UAIITK_NTK_NetworkingToolkitSettings::DecryptKey(const FString& EncryptedKey) const
{
    if (!EncryptedKey.StartsWith(EncryptionPrefix))
    {
        return FString();
    }

    FString Base64Encoded = EncryptedKey.RightChop(FCString::Strlen(EncryptionPrefix));
    TArray<uint8> Data;
    if (!FBase64::Decode(Base64Encoded, Data))
    {
        return FString();
    }

    FAES::DecryptData(Data.GetData(), Data.Num(), EncryptionKey.GetData(), EncryptionKey.Num());

    // Convert raw bytes into FString, then sanitize
    FString Decrypted;
    FFileHelper::BufferToString(Decrypted, Data.GetData(), Data.Num());
    SanitizeOutputString(Decrypted);

    return Decrypted;
}

//---------------------------------------------------------------------
//  Static Accessors / Mutators
//---------------------------------------------------------------------
FString UAIITK_NTK_NetworkingToolkitSettings::GetDecryptedKey(const FString& KeyName)
{
    const UAIITK_NTK_NetworkingToolkitSettings* Settings = GetDefault<UAIITK_NTK_NetworkingToolkitSettings>();
    if (Settings)
    {
        if (const FString* Encrypted = Settings->EncryptedKeys.Find(KeyName))
        {
            return Settings->DecryptKey(*Encrypted);
        }
    }
    return FString();
}

void UAIITK_NTK_NetworkingToolkitSettings::SetKey(const FString& KeyName, const FString& PlainTextKey)
{
    UAIITK_NTK_NetworkingToolkitSettings* Settings = GetMutableDefault<UAIITK_NTK_NetworkingToolkitSettings>();
    if (!Settings) return;

    FString Encrypted = Settings->EncryptKey(PlainTextKey);
    Settings->EncryptedKeys.Add(KeyName, FString(EncryptionPrefix) + Encrypted);
    Settings->SaveConfig();
}
