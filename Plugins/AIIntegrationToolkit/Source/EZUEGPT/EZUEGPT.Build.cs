using UnrealBuildTool;
using System.IO;

public class EZUEGPT : ModuleRules
{
    public EZUEGPT(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                //"Runtime/Core/Public/Microsoft",
                Path.Combine(ModuleDirectory, "ThirdParty"),
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
                // Add other private include paths required here...
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "HTTP",
                "Json",
                "JsonUtilities",
                "AudioCapture",
                "AudioMixerCore",
                "AudioExtensions",
                "Synthesis",
                "AudioMixer",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "Voice",
                "InputCore",
                "DeveloperSettings",
                "AudioPlatformConfiguration",
                "ProceduralMeshComponent",
                "WebSockets",
                "AIITK_NTK_NetworkingToolkit",

                // Add other public dependencies that you statically link with here...
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "CoreUObject",      // Also add it here to ensure linking
                // Add private dependencies that you statically link with here...
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // Add any modules that your module loads dynamically here...
            }
        );
    }
}
