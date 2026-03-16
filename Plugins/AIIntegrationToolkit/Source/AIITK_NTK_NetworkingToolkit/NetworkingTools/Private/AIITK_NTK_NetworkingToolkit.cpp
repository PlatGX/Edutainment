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


// ========== AIITK_NTK_NetworkingToolkit.cpp ==========

#include "AIITK_NTK_NetworkingToolkit.h"

DEFINE_LOG_CATEGORY(NetworkingToolkitLog);

#define LOCTEXT_NAMESPACE "FNetworkingToolkit"

void FNetworkingToolkit::StartupModule()
{
	UE_LOG(NetworkingToolkitLog, Warning, TEXT("AIITK_NTK_NetworkingToolkit module has been loaded"));
}

void FNetworkingToolkit::ShutdownModule()
{
	UE_LOG(NetworkingToolkitLog, Warning, TEXT("AIITK_NTK_NetworkingToolkit module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNetworkingToolkit, AIITK_NTK_NetworkingToolkit)