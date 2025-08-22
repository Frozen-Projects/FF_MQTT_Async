namespace UnrealBuildTool.Rules
{
    using System.IO;

    public class paho_c_async : ModuleRules
    {
        public paho_c_async(ReadOnlyTargetRules Target) : base(Target)
        {
    		Type = ModuleType.External;
            CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;
            bEnableExceptions = true;

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                bUseRTTI = true;

                PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Win64", "include"));
                PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Win64", "lib", "paho-mqtt3as-static.lib"));
            }
        }
    }
}
