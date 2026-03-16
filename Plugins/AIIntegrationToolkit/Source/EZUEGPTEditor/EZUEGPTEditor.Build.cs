// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class EZUEGPTEditor : ModuleRules
{
    public EZUEGPTEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "KismetCompiler",
                "AssetTools",
                "EZUEGPT",
                "Json",
                "JsonUtilities",
                "UnrealEd",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "UnrealEd",
                "BlueprintGraph",
                "Kismet",
                "LevelEditor",
            }
        );
    }
}
