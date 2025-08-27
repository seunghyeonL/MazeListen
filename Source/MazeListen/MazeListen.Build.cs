// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MazeListen : ModuleRules
{
	public MazeListen(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"OnlineSubsystem", 
			"OnlineSubsystemUtils", 
			"Sockets", 
			"Networking",
			"UMG",
			"Slate",
			"SlateCore"
		});
		
		PrivateIncludePaths.Add("MazeListen");
	}
}
