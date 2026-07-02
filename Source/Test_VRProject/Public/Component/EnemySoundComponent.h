// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemySoundComponent.generated.h"


UENUM(BlueprintType)
enum class EFootSoundType : uint8
{
	patrol,		//巡回
	tracking,	//追跡
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TEST_VRPROJECT_API UEnemySoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UFUNCTION(BlueprintImplementableEvent)
	void EnemyFootSound(EFootSoundType SoundTypeEnemy);

	UFUNCTION(BlueprintImplementableEvent)
	void EnemyAttackSound();
};
