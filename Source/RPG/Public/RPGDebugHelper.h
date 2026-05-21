#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

// 声明全局 RPG 日志分类
DECLARE_LOG_CATEGORY_EXTERN(LogRPG, Log, All);

namespace Debug
{
	/**
	 * 屏幕 + 日志输出（Log 级别）
	 * @param Msg 消息内容
	 * @param InKey 屏幕消息 Key（相同 Key 会覆盖，-1 表示新增）
	 */
	static void Print(const FString& Msg, int32 InKey = -1)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, 7.0f, FColor::White, Msg);
			UE_LOG(LogRPG, Log, TEXT("%s"), *Msg);
		}
	}

	/**
	 * 屏幕 + 日志输出（Warning 级别）
	 * @param Msg 消息内容
	 * @param InKey 屏幕消息 Key
	 */
	static void PrintWarning(const FString& Msg, int32 InKey = -1)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, 7.0f, FColor::Yellow, Msg);
			UE_LOG(LogRPG, Warning, TEXT("%s"), *Msg);
		}
	}

	/**
	 * 屏幕 + 日志输出（Error 级别）
	 * @param Msg 消息内容
	 * @param InKey 屏幕消息 Key
	 */
	static void PrintError(const FString& Msg, int32 InKey = -1)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, 7.0f, FColor::Red, Msg);
			UE_LOG(LogRPG, Error, TEXT("%s"), *Msg);
		}
	}

	/**
	 * 浮点数屏幕输出
	 * @param FloatTitle 标题
	 * @param FloatValuePrint 浮点数值
	 * @param InKey 屏幕消息 Key
	 */
	static void PrintFloat(const FString& FloatTitle, float FloatValuePrint, int32 InKey = -1)
	{
		if(GEngine)
		{
			const FString FinalMsg = FloatTitle + TEXT(": ") + FString::SanitizeFloat(FloatValuePrint);
			GEngine->AddOnScreenDebugMessage(InKey, 7.0f, FColor::Cyan, FinalMsg);
			UE_LOG(LogRPG, Log, TEXT("%s"), *FinalMsg);
		}
	}

	/**
	 * 仅日志输出（不显示屏幕）
	 * @param Msg 消息内容
	 */
	static void Log(const FString& Msg)
	{
		UE_LOG(LogRPG, Log, TEXT("%s"), *Msg);
	}

	/**
	 * 仅日志输出（Warning 级别）
	 * @param Msg 消息内容
	 */
	static void LogWarning(const FString& Msg)
	{
		UE_LOG(LogRPG, Warning, TEXT("%s"), *Msg);
	}

	/**
	 * 仅日志输出（Error 级别）
	 * @param Msg 消息内容
	 */
	static void LogError(const FString& Msg)
	{
		UE_LOG(LogRPG, Error, TEXT("%s"), *Msg);
	}
}
