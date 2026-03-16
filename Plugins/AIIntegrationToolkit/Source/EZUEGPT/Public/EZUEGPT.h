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

// =================== EZUEGPT.h ===================

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "GPTChatAPI.h"
#include "TextToSpeechAPI.h"

class FEZUEGPTModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    UGPTChatAPI* CreateGPTChatAPIInstance();
    UTextToSpeechAPI* CreateTextToSpeechAPIInstance();
    void AddRequest(TStrongObjectPtr<UObject> Request);
    void RemoveRequest(TStrongObjectPtr<UObject> Request);

private:
    TArray<TStrongObjectPtr<UObject>> OngoingRequests;
};
