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
	//---変数-----------------------------------------------------------------------

	//次に移動するポイントのインデックス
	int32 CurrentIndex = 0;

	//巡回時の移動速度
	const float PatrolMoveSpeed = 400.0f;

	//追跡時の移動速度
	const float TrackingMoveSpeed = 200.0f;

	//playerまでの判定距離
	const float DistanceToPlayer = 100.0f;

	//発見する範囲の距離
	const float DetectionRange = 700.0f;

	//待機時間
	const float WaitTime = 1.0f;

	//離れるまでの時間
	const float ToLeaveTime = 2.0f;

	//次の検知までの時間
	const float DetectionTime = 5.0f;

	//playerを見つけたかどうかのフラグ
	bool bMovinToPlayerPoint = false;

	//playerを捕まえたかどうかのフラグ
	bool bAttacking=false;

	//距離を取ったかどうかの処理
	bool bCanDetectPlayer = true;



	//---関数-----------------------------------------------------------------------

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
