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


// ========== AIITK_NTK_OAuth2FunctionLibrary.cpp ==========

#include "AIITK_NTK_OAuth2FunctionLibrary.h"
#include "AIITK_NTK_OAuth2Config.h"
#include "GenericPlatform/GenericPlatformHttp.h"

UAIITK_NTK_OAuth2Manager* UAIITK_NTK_OAuth2FunctionLibrary::CreateAndStartOAuthFlow(const FAIITK_NTK_OAuth2Config& Config)
{
    UAIITK_NTK_OAuth2Manager* OAuthManager = NewObject<UAIITK_NTK_OAuth2Manager>();
    if (!OAuthManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create AIITK_NTK_OAuth2Manager instance."));
        return nullptr;
    }

    OAuthManager->InitializeOAuthFlow(Config);
    OAuthManager->StartAuthFlow();

    return OAuthManager;
}

FString UAIITK_NTK_OAuth2FunctionLibrary::BuildQueryString(const TMap<FString, FString>& Params)
{
    FString QueryString;
    for (const auto& Pair : Params)
    {
        if (!QueryString.IsEmpty())
        {
            QueryString += TEXT("&");
        }
        QueryString += FString::Printf(TEXT("%s=%s"), *Pair.Key, *FGenericPlatformHttp::UrlEncode(Pair.Value));
    }
    return QueryString;
}
