// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CICO_Simulation : ModuleRules
{
	public CICO_Simulation(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			// HTTP + JSON para conexion con backend StealthVision
			"HTTP",
			"Json",
			"JsonUtilities",
			// Encoding de frames (JPEG/PNG) para enviar al backend
			"ImageWrapper",
			// IA y navegacion NavMesh para Pedestrian y Vehicle
			"AIModule",
			"NavigationSystem",
			// UMG para widgets (RiskHUD)
			"UMG",
			"Slate",
			"SlateCore",
			// Media Player para video de fondo en el menu
			"MediaAssets"
		});
	}
}
