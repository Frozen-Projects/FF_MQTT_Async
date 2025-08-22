// Some copyright should be here...

using System;
using System.IO;
using UnrealBuildTool;

public class FF_MQTT_Async : ModuleRules
{
	public FF_MQTT_Async(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;
        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "OpenSSL",
                "FF_MQTT_Sync",
            }
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Json",
                "JsonUtilities",
                "JsonBlueprintUtilities",
                "paho_c_async",
            }
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{

			}
			);
	}
}
