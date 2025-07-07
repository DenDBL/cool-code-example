// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ESNetwork : ModuleRules
{
	public ESNetwork(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine",  "OnlineSubsystem", "OnlineSubsystemUtils" });

		// Mass modules
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"MassEntity", "MassCommon"
        });
		
		PublicDependencyModuleNames.AddRange(new string[] {});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
		
		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("EventSimEditor");
			PublicDefinitions.Add("ESNETWORK_WITH_EDITOR=1");
		}
		else
		{
			PublicDefinitions.Add("ESNETWORK_WITH_EDITOR=0");
		}
	}
}
