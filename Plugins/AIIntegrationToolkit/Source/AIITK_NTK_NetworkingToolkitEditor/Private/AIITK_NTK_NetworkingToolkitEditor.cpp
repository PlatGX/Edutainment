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


// ========== AIITK_NTK_NetworkingToolkitEditor.cpp ==========

#include "AIITK_NTK_NetworkingToolkitEditor.h"

#if WITH_EDITOR

DEFINE_LOG_CATEGORY(AIITK_NTK_NetworkingToolkitEditorLog);

#define LOCTEXT_NAMESPACE "FAIITK_NTK_NetworkingToolkitEditor"

void FAIITK_NTK_NetworkingToolkitEditor::StartupModule()
{
    UE_LOG(AIITK_NTK_NetworkingToolkitEditorLog, Log, TEXT("AIITK_NTK_NetworkingToolkitEditor module has been loaded"));
}

void FAIITK_NTK_NetworkingToolkitEditor::ShutdownModule()
{
    UE_LOG(AIITK_NTK_NetworkingToolkitEditorLog, Log, TEXT("AIITK_NTK_NetworkingToolkitEditor module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

#endif // WITH_EDITOR

IMPLEMENT_MODULE(FAIITK_NTK_NetworkingToolkitEditor, AIITK_NTK_NetworkingToolkitEditor)
