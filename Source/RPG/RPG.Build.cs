// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RPG : ModuleRules
{
	public RPG(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				
				// UE 基础模块
				"Core",
				"CoreUObject",
				"Engine",
				
				// GAS 技能系统
				"GameplayTags",
				"GameplayTasks",
				"GameplayAbilities",
				
				// AI 系统
				"AIModule",
				"NavigationSystem",
				
				// 模块化游戏框架
				"ModularGameplay",
				"GameFeatures",
				
				// 粒子特效
				"Niagara",
				
				// UI 框架
				"UMG",
				"CommonGame",
				"CommonUI",
				
				// 动画
				"MotionWarping",
			}
			);

			PublicIncludePaths.AddRange(
				new string[] {
					"Public/AbilitySystem",
					"Public/UI",
					"Public/Character"
				}
			);

			PrivateDependencyModuleNames.AddRange(
			new string[] {
				
				// 输入系统
				"InputCore",
				"EnhancedInput",
				
				// Slate UI 基础
				"Slate",
				"SlateCore",
				
				// 跨平台输入
				"CommonInput",
				
				// 音频
				"AudioMixer",
				
				// 数据序列化
				"Json",
				
				// 配置系统
				"DeveloperSettings",
				"EngineSettings",
				}
			);

		

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}