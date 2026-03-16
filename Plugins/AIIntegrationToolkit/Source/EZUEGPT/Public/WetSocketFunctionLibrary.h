
#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WetSocketManager.h"
#include "WetSocketFunctionLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(WetSockLog, Log, All);


/**
 * UWetSocketFunctionLibrary: Blueprint Function Library for creating and working with WetSocketManager.
 */
UCLASS()
class EZUEGPT_API UWetSocketFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Creates an instance of UWetSocketManager.
     * @return A new instance of UWetSocketManager.
     */
    UFUNCTION(BlueprintCallable, Category = "*AIITK|WetSocket")
    static UWetSocketManager* CreateWetSocketManager();
};
