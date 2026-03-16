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

#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "AudioCaptureComponent.h"
#include "Sound/SoundSubmix.h"
#include "WhisperAudioCaptureComponent.generated.h"

UCLASS(ClassGroup = (AIITK), meta = (BlueprintSpawnableComponent), Blueprintable, Category = "*AIITK|Audio")
class EZUEGPT_API UWhisperAudioCaptureComponent : public UAudioCaptureComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "*AIITK|Whisper API|Components")
    UWhisperAudioCaptureComponent* SetBaseSubmix(USoundSubmix* NewSubmix)
    {
        bEnableBaseSubmix = true;  // Enable base submix routing
        SoundSubmix = NewSubmix;   // Set the base submix

        return this;
    }

    // Override Activate to manage root status
    virtual void Activate(bool bReset = false) override
    {
        Super::Activate(bReset);
        if (!IsRooted())
        {
            AddToRoot();  // Add to the root set to prevent GC
        }
    }

    // Override Deactivate to manage root status
    virtual void Deactivate() override
    {
        Super::Deactivate();
        if (IsRooted())
        {
            RemoveFromRoot();  // Remove from root set to allow GC
        }
    }

    // Optionally, override BeginDestroy to ensure cleanup
    virtual void BeginDestroy() override
    {
        if (IsRooted())
        {
            RemoveFromRoot();  // Ensure removal from root on destruction
        }
        Super::BeginDestroy();
    }
};

