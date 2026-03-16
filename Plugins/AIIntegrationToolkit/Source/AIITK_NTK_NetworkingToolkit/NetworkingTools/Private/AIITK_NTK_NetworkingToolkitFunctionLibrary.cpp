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


// ========== AIITK_NTK_NetworkingToolkitFunctionLibrary.cpp ==========

#include "AIITK_NTK_NetworkingToolkitFunctionLibrary.h"
#include "Internationalization/Regex.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Modules/ModuleManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Async/Async.h"
#include "TextureResource.h"




DEFINE_LOG_CATEGORY(LogNetworkingToolkit);


void UAIITK_NTK_NetworkingToolkitFunctionLibrary::OpenURLInBrowser(const FString& URL)
{
    if (URL.IsEmpty())
    {
        UE_LOG(LogNetworkingToolkit, Warning, TEXT("OpenURLInBrowser: URL is empty."));
        return;
    }

    FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
}

bool UAIITK_NTK_NetworkingToolkitFunctionLibrary::RegexSearch(const FString& InputString, const FString& Pattern, int32 CaptureGroup, TArray<FString>& OutMatches)
{
    OutMatches.Empty();
    FRegexPattern RegexPattern(Pattern, ERegexPatternFlags::None);
    FRegexMatcher Matcher(RegexPattern, InputString);
    bool bFound = false;

    while (Matcher.FindNext())
    {
        FString Match;
        if (Matcher.GetCaptureGroupBeginning(CaptureGroup) != INDEX_NONE)
        {
            Match = Matcher.GetCaptureGroup(CaptureGroup);
        }
        else
        {
            int32 Begin = Matcher.GetMatchBeginning();
            int32 End = Matcher.GetMatchEnding();
            Match = InputString.Mid(Begin, End - Begin);
        }
        OutMatches.Add(Match);
        bFound = true;
    }

    return bFound;
}

void UAIITK_NTK_NetworkingToolkitFunctionLibrary::CallFunctionByName(UObject* Target, const FString& FunctionName)
{
    if (!Target)
    {
        UE_LOG(LogNetworkingToolkit, Warning, TEXT("CallFunctionByName: Target is null."));
        return;
    }
    if (FunctionName.IsEmpty())
    {
        UE_LOG(LogNetworkingToolkit, Warning, TEXT("CallFunctionByName: FunctionName is empty."));
        return;
    }

    FName FuncFName(*FunctionName);
    UFunction* Func = Target->FindFunction(FuncFName);
    if (!Func)
    {
        UE_LOG(LogNetworkingToolkit, Warning, TEXT("Function '%s' not found on '%s'."), *FunctionName, *Target->GetName());
        return;
    }
    if (Func->NumParms > 0)
    {
        UE_LOG(LogNetworkingToolkit, Warning, TEXT("Function '%s' has parameters; only no-param functions supported."), *FunctionName);
        return;
    }

    Target->ProcessEvent(Func, nullptr);
}

TArray<FString> UAIITK_NTK_NetworkingToolkitFunctionLibrary::Conv_EnumToStringArray(const UEnum* Enum)
{
    TArray<FString> Names;
    if (!Enum) return Names;

    for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
    {
        Names.Add(Enum->GetDisplayNameTextByIndex(i).ToString());
    }
    return Names;
}

uint8 UAIITK_NTK_NetworkingToolkitFunctionLibrary::Conv_StringToEnum(const UEnum* Enum, const FString& StringValue)
{
    if (!Enum)
    {
        UE_LOG(LogNetworkingToolkit, Warning, TEXT("Conv_StringToEnum: Invalid enum pointer."));
        return 0;
    }

    for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
    {
        if (Enum->GetDisplayNameTextByIndex(i).ToString().Equals(StringValue, ESearchCase::IgnoreCase))
        {
            int64 Val = Enum->GetValueByIndex(i);
            UE_LOG(LogNetworkingToolkit, Log, TEXT("Converted '%s' to enum value %lld."), *StringValue, Val);
            return static_cast<uint8>(Val);
        }
    }

    UE_LOG(LogNetworkingToolkit, Warning, TEXT("Conv_StringToEnum: '%s' not found."), *StringValue);
    return 0;
}

UTexture2D* UAIITK_NTK_NetworkingToolkitFunctionLibrary::Base64ToTexture(const FString& Base64String)
{
    TArray<uint8> DecodedBytes;
    if (!FBase64::Decode(Base64String, DecodedBytes))
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("Base64ToTexture: Failed to decode Base64 string."));
        return nullptr;
    }

    IImageWrapperModule& WrapperMod = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
    TSharedPtr<IImageWrapper> Wrapper = WrapperMod.CreateImageWrapper(EImageFormat::PNG);

    if (!Wrapper.IsValid() || !Wrapper->SetCompressed(DecodedBytes.GetData(), DecodedBytes.Num()))
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("Base64ToTexture: Failed to load image from decoded bytes."));
        return nullptr;
    }

    TArray<uint8> RawBGRA;
    if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, RawBGRA))
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("Base64ToTexture: Failed to get raw image data."));
        return nullptr;
    }

    UTexture2D* Texture = UTexture2D::CreateTransient(Wrapper->GetWidth(), Wrapper->GetHeight());
    if (!Texture)
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("Base64ToTexture: Failed to create transient texture."));
        return nullptr;
    }

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 27
    // UE4.27 and earlier
    FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
    void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
#else
    // UE5+
    void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
#endif

    FMemory::Memcpy(TextureData, RawBGRA.GetData(), RawBGRA.Num());

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 27
    Mip.BulkData.Unlock();
#else
    Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
#endif
#undef UpdateResource

    Texture->UpdateResource();
    return Texture;
}

FString UAIITK_NTK_NetworkingToolkitFunctionLibrary::TextureToBase64(UTexture2D* Texture)
{
    if (!Texture)
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("TextureToBase64: Invalid Texture."));
        return FString();
    }

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 27
    // UE4.27 and earlier
    FTexture2DMipMap& MipMap = Texture->PlatformData->Mips[0];
    void* Data = MipMap.BulkData.Lock(LOCK_READ_ONLY);
#else
    // UE5+
    void* Data = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_ONLY);
#endif

    int32 Width = Texture->GetSizeX();
    int32 Height = Texture->GetSizeY();
    int32 DataSize = Width * Height * 4;  // BGRA8

    TArray<uint8> RawData;
    RawData.AddUninitialized(DataSize);
    FMemory::Memcpy(RawData.GetData(), Data, DataSize);

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 27
    MipMap.BulkData.Unlock();
#else
    Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
#endif

    // Compress to PNG
    IImageWrapperModule& WrapperMod = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
    TSharedPtr<IImageWrapper> Wrapper = WrapperMod.CreateImageWrapper(EImageFormat::PNG);

    if (!Wrapper.IsValid() ||
        !Wrapper->SetRaw(RawData.GetData(), RawData.Num(), Width, Height, ERGBFormat::RGBA, 8))
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("TextureToBase64: Failed to set raw data for image wrapper."));
        return FString();
    }

    // Grab compressed bytes (always 8-bit PNG)
    const TArray<uint8, FDefaultAllocator64>& Compressed = Wrapper->GetCompressed();

    // Convert to standard TArray<uint8>
    TArray<uint8> StdBytes;
    StdBytes.Append(Compressed.GetData(), Compressed.Num());

    return FBase64::Encode(StdBytes);
}

bool UAIITK_NTK_NetworkingToolkitFunctionLibrary::SaveTextureToDisk(UTexture2D* Texture, const FString& FileName)
{
    if (!Texture)
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("Invalid texture."));
        return false;
    }

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 27
    // For UE 4.27 and earlier
    FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
#else
    // For UE 5 and later
    FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
#endif

    uint8* TextureData = const_cast<uint8*>(static_cast<const uint8*>(Mip.BulkData.LockReadOnly()));
    TArray<uint8> UncompressedRGBA;
    UncompressedRGBA.Append(TextureData, Mip.BulkData.GetBulkDataSize());
    Mip.BulkData.Unlock();

    // Decompressing the texture into raw FColor data
    int32 PixelCount = Texture->GetSizeX() * Texture->GetSizeY();
    FColor* FormattedImageData = new FColor[PixelCount];
    FMemory::Memcpy(FormattedImageData, UncompressedRGBA.GetData(), UncompressedRGBA.Num());

    TArray<FColor> UncompressedColors;
    UncompressedColors.Reserve(PixelCount);
    for (int32 i = 0; i < PixelCount; ++i)
    {
        UncompressedColors.Add(FormattedImageData[i]);
    }
    delete[] FormattedImageData;

#if ENGINE_MAJOR_VERSION > 4
    // UE5+ requires a 64-bit allocator and a TArrayView64
    TArray64<uint8> CompressedBitmap;
    FImageUtils::PNGCompressImageArray(
        Texture->GetSizeX(),
        Texture->GetSizeY(),
        TArrayView64<const FColor>(UncompressedColors),
        CompressedBitmap
    );
#else
    // UE4 uses the 32-bit allocator overload
    TArray<uint8> CompressedBitmap;
    FImageUtils::CompressImageArray(
        Texture->GetSizeX(),
        Texture->GetSizeY(),
        UncompressedColors,
        CompressedBitmap
    );
#endif

    if (CompressedBitmap.Num() == 0)
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("Failed to compress texture data."));
        return false;
    }

    if (!FFileHelper::SaveArrayToFile(CompressedBitmap, *FileName))
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("Failed to save texture to disk."));
        return false;
    }

    UE_LOG(LogNetworkingToolkit, Log, TEXT("Texture saved to %s"), *FileName);
    return true;
}


UTexture2D* UAIITK_NTK_NetworkingToolkitFunctionLibrary::LoadTextureFromDisk(const FString& FileName)
{
    TArray<uint8> CompressedData;
    if (!FFileHelper::LoadFileToArray(CompressedData, *FileName))
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("Failed to load file from disk."));
        return nullptr;
    }

    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

    if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(CompressedData.GetData(), CompressedData.Num()))
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("Failed to initialize ImageWrapper."));
        return nullptr;
    }

    TArray64<uint8> UncompressedRGBA;
    if (!ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, UncompressedRGBA))
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("Failed to decompress image data."));
        return nullptr;
    }

    UTexture2D* Texture = nullptr;
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 27
    Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_R8G8B8A8);
#else
    Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), EPixelFormat::PF_R8G8B8A8);
#endif

    if (!Texture)
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("Failed to create texture."));
        return nullptr;
    }

    // Ensure PlatformData is valid
    if (!Texture->GetPlatformData())
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("PlatformData is invalid."));
        return nullptr;
    }

    // Fill texture with raw data
    FTexturePlatformData* PlatformData = Texture->GetPlatformData();
    void* TextureData = PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(TextureData, UncompressedRGBA.GetData(), UncompressedRGBA.Num());
    PlatformData->Mips[0].BulkData.Unlock();

    // Update texture properties
    Texture->UpdateResource();

    // Log where the texture was loaded from
    UE_LOG(LogNetworkingToolkit, Log, TEXT("Texture loaded from %s"), *FileName);

    return Texture;
}

void UAIITK_NTK_NetworkingToolkitFunctionLibrary::DownloadImageFromURL(const FString& URL, FNTK_DownloadImageDelegate OnImageDownloaded)
{
    if (URL.IsEmpty())
    {
        UE_LOG(LogNetworkingToolkit, Error, TEXT("DownloadImageFromURL: empty URL."));
        AsyncTask(ENamedThreads::GameThread, [OnImageDownloaded]() {
            OnImageDownloaded.ExecuteIfBound(nullptr);
        });
        return;
    }

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    Req->SetURL(URL);
    Req->SetVerb(TEXT("GET"));

    Req->OnProcessRequestComplete().BindLambda(
        [OnImageDownloaded = MoveTemp(OnImageDownloaded)](
            FHttpRequestPtr ReqPtr, FHttpResponsePtr Resp, bool bOk)
    {
        UTexture2D* Result = nullptr;
        if (bOk && Resp.IsValid() && Resp->GetContentLength() > 0)
        {
            TArray<uint8> Bytes = Resp->GetContent();
            IImageWrapperModule& IWMod = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
            EImageFormat Format = IWMod.DetectImageFormat(Bytes.GetData(), Bytes.Num());
            TSharedPtr<IImageWrapper> Wrapper = IWMod.CreateImageWrapper(Format);

            if (Wrapper->SetCompressed(Bytes.GetData(), Bytes.Num()))
            {
                TArray<uint8> Raw;
                if (Wrapper->GetRaw(ERGBFormat::BGRA, 8, Raw))
                {
                    Result = UTexture2D::CreateTransient(Wrapper->GetWidth(), Wrapper->GetHeight(), PF_B8G8R8A8);
                    void* Data = Result->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
                    FMemory::Memcpy(Data, Raw.GetData(), Raw.Num());
                    Result->GetPlatformData()->Mips[0].BulkData.Unlock();
                    Result->UpdateResource();
                }
            }
        }
        else
        {
            UE_LOG(LogNetworkingToolkit, Error, TEXT("DownloadImageFromURL: failed download."));
        }

        AsyncTask(ENamedThreads::GameThread, [OnImageDownloaded, Result]() {
            OnImageDownloaded.ExecuteIfBound(Result);
        });
    });

    Req->ProcessRequest();
}
