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


// ========== AIITK_NTK_WetSocketFunctionLibrary.h ==========


#pragma once

#include "Misc/EngineVersion.h"
#include "Runtime/Launch/Resources/Version.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AIITK_NTK_WetSocketManager.h"

#include "AIITK_NTK_WetSocketFunctionLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(NTKWetSockLog, Log, All);


/**
 * UAIITK_NTK_WetSocketFunctionLibrary: Blueprint Function Library for creating and working with AIITK_NTK_WetSocketManager.
 */
UCLASS()
class AIITK_NTK_NETWORKINGTOOLKIT_API UAIITK_NTK_WetSocketFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Creates an instance of UAIITK_NTK_WetSocketManager.
     * @return A new instance of UAIITK_NTK_WetSocketManager.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|NetworkingToolkit|WetSocket")
    static UAIITK_NTK_WetSocketManager* CreateWetSocketManager();
};
