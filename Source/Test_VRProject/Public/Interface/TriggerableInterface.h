// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TriggerableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTriggerableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TEST_VRPROJECT_API ITriggerableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// Blueprintで実装可能な関数。アクティベーション状態を設定する。
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Activation")
	void OnPlayerOverlap(AActor* PlayerPawn);
};
