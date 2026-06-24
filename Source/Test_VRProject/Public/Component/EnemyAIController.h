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

	//playerまでの判定距離
	float DistanceToPlayer = 100.0f;

	//playerを見つけたかどうかのフラグ
	bool bMovinToPlayerPoint = false;

	//playerを捕まえたかどうかのフラグ
	bool bAttacking=false;

	//距離を取ったかどうかの処理
	bool bCanDetectPlayer = true;

	//次のポイントに移動する関数
	void MoveToNextPoint();

	//Playerのポイントを探し、見つけたら移動する関数
	void ChackPlayer();

	//攻撃処理
	void EnemyAttack();

	//パトロールに戻る処理
	void ReturnToPatrol();

private:

	FTimerHandle PatrolTimerHandle;
	FTimerHandle DetectTimerHandle;
};
