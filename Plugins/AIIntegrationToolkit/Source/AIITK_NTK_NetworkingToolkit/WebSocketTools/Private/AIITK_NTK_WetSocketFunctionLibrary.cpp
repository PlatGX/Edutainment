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


// ========== AIITK_NTK_WetSocketFunctionLibrary.cpp ==========


#include "AIITK_NTK_WetSocketFunctionLibrary.h"
#include "AIITK_NTK_WetSocketManager.h"

DEFINE_LOG_CATEGORY(NTKWetSockLog);


UAIITK_NTK_WetSocketManager* UAIITK_NTK_WetSocketFunctionLibrary::CreateWetSocketManager()
{
    // Create an instance of UAIITK_NTK_WetSocketManager
    UAIITK_NTK_WetSocketManager* WetSocketManagerInstance = NewObject<UAIITK_NTK_WetSocketManager>();

    // Ensure that the instance is valid
    if (WetSocketManagerInstance)
    {
        UE_LOG(NTKWetSockLog, Log, TEXT("AIITK_NTK_WetSocketManager instance created successfully."));
    }
    else
    {
        UE_LOG(NTKWetSockLog, Error, TEXT("Failed to create AIITK_NTK_WetSocketManager instance."));
    }

    return WetSocketManagerInstance;
}
