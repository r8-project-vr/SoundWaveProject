// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class TEST_VRPROJECT_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)override;
	virtual void Tick(float DeltaTime) override;

public:

	//次に移動するポイントのインデックス
	int32 CurrentIndex = 0;

	bool bMovinToPlayerPoint = false;

	//次のポイントに移動する関数
	void MoveToNextPoint();

private:

	FTimerHandle PatrolTimerHandle;

};
