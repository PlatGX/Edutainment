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


// ========== AIITK_NTK_NetworkingToolkitEditor.build.cs ==========

using System.IO;
using UnrealBuildTool;

public class AIITK_NTK_NetworkingToolkitEditor : ModuleRules
{
    public AIITK_NTK_NetworkingToolkitEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        if (Target.bBuildEditor)
        {
			PublicDependencyModuleNames.AddRange(new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Kismet",
				"UnrealEd",
				"Json",
				"JsonUtilities",
				"KismetCompiler",
				"AssetTools",
                "AIITK_NTK_NetworkingToolkit",
			});

			PrivateDependencyModuleNames.AddRange(new string[] {
				"Slate",
				"SlateCore",
				"UnrealEd",
				"BlueprintGraph",
                "Projects",
            });

			string ModulePath = ModuleDirectory;

			PublicIncludePaths.AddRange(new string[] {
				Path.Combine(ModulePath, "Public")
			});

			PrivateIncludePaths.AddRange(new string[] {
				Path.Combine(ModulePath, "Private")
			});
		}
	}
}
