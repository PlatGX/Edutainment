
#include "WetSocketFunctionLibrary.h"
#include "WetSocketManager.h"

DEFINE_LOG_CATEGORY(WetSockLog);


UWetSocketManager* UWetSocketFunctionLibrary::CreateWetSocketManager()
{
    // Create an instance of UWetSocketManager
    UWetSocketManager* WetSocketManagerInstance = NewObject<UWetSocketManager>();

    // Ensure that the instance is valid
    if (WetSocketManagerInstance)
    {
        UE_LOG(WetSockLog, Log, TEXT("WetSocketManager instance created successfully."));
    }
    else
    {
        UE_LOG(WetSockLog, Error, TEXT("Failed to create WetSocketManager instance."));
    }

    return WetSocketManagerInstance;
}
