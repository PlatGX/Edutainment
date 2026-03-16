// Copyright 2023 Kaleb Knoettgen. All rights reserved.

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

DECLARE_LOG_CATEGORY_EXTERN(AIITKEditorLog, Log, All);


class FEZUEGPTEditorModule : public IModuleInterface
{
public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    /** Opens a window containing links to obtain API keys */
    void OpenAPIKeysWindow();

private:
    void AddMenuEntry(FMenuBuilder& MenuBuilder);

    TSharedPtr<FExtender> MenuExtender;
};
