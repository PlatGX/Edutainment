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


// ========== AIITK_NTK_NetworkingToolkit.Build.cs ==========

using System.IO;
using UnrealBuildTool;

public class AIITK_NTK_NetworkingToolkit : ModuleRules
{
    public AIITK_NTK_NetworkingToolkit(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "HTTP",
            "Json",
            "JsonUtilities",
            "DeveloperSettings",
            "HTTPServer",
            "WebSockets",
            "InputCore",
            "Projects",
            "RHI",
            "Renderer",
            "RenderCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
        });



        string ModulePath = ModuleDirectory;
        string PluginPath = Path.GetFullPath(Path.Combine(ModulePath, "../../"));

        PublicIncludePaths.AddRange(new string[] {
            Path.Combine(ModulePath, "NetworkingTools/Public"),
            Path.Combine(ModulePath, "JsonHttpTools/Public"),
            Path.Combine(ModulePath, "AuthorizationTools/Public"),
            Path.Combine(ModulePath, "WebSocketTools/Public"),
            Path.Combine(ModulePath, "EditorConfiguration/Public"),
        });

        PrivateIncludePaths.AddRange(new string[] {
            Path.Combine(ModulePath, "NetworkingTools/Private"),
            Path.Combine(ModulePath, "JsonHttpTools/Private"),
            Path.Combine(ModulePath, "AuthorizationTools/Private"),
            Path.Combine(ModulePath, "WebSocketTools/Private"),
            Path.Combine(ModulePath, "EditorConfiguration/Private"),
        });
    }
}

