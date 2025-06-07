



// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MultiplayerTPP : ModuleRules
{
	public MultiplayerTPP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Multiplayer", "OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemEOS", "EOS_Login", "Niagara", "SlateCore"});

		PrivateDependencyModuleNames.AddRange(new string[] { });

        PrivateDefinitions.Add("P2PMODE=1");

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
