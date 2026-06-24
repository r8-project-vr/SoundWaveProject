// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/EnemyAIController.h"
#include "EnemeyCharacter.h"                    //敵キャラクター
#include "Actor/EnemyPatrolPoint.h"
#include "TimerManager.h"                       //時間関係
#include "Math/RandomStream.h"                  //ランダム関係
#include "Navigation/PathFollowingComponent.h"

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	//次移動する場所
	CurrentIndex = 0;

	MoveToNextPoint();
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ChackPlayer();
}

void AEnemyAIController::MoveToNextPoint()
{
	AEnemeyCharacter* Enemy =
		Cast<AEnemeyCharacter>(GetPawn());

	//デバック用--------------------------------------------------------------------
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("CurrentIndex=%d"),
		CurrentIndex
	);

	if (!GetPawn())
	{
		UE_LOG(LogTemp, Error, TEXT("Pawn is null"));
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("Pawn Location = %s"),
		*GetPawn()->GetActorLocation().ToString());

	AEnemyPatrolPoint* Point = Enemy->Patrolpoints[CurrentIndex];

	UE_LOG(LogTemp, Warning,
		TEXT("Target Location = %s"),
		*Point->GetActorLocation().ToString());
	//______________________________________________________________________________

	if (!Enemy)
		return;

	if (Enemy->Patrolpoints.Num() == 0)
		return;

	if (bMovinToPlayerPoint)
		return;

	//移動先のポイントに近づける距離
	const EPathFollowingRequestResult::Type Result =
		MoveToActor(
			Enemy->Patrolpoints[CurrentIndex],
			DistanceToPlayer //受け入れ半径
		);
}

void AEnemyAIController::OnMoveCompleted(
	FAIRequestID RequestID,
	const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	// 成功時だけ次へ
	if (Result.Code != EPathFollowingResult::Success)
	{
		return;
	}

	if (bMovinToPlayerPoint)
	{
		EnemyAttack();
		return;
	}

	AEnemeyCharacter* Enemy =
		Cast<AEnemeyCharacter>(GetPawn());

	if (!Enemy)
		return;

	if (Enemy->Patrolpoints.Num() == 0)
		return;

	//ランダムシード値の決定
	FRandomStream RandomStream;
	RandomStream.GenerateNewSeed();

	int32 NewIndex;

	do {
		//次のポイントの決定
		NewIndex = RandomStream.RandRange(0, Enemy->Patrolpoints.Num() - 1);

	} while (
		//もし移動先が変わらなかったら、もう一度繰り返す
		Enemy->Patrolpoints.Num() > 1 &&
		NewIndex == CurrentIndex
		);

	CurrentIndex = NewIndex;

	//デバック用--------------------------------------------------------------------

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("CurrentIndex = %d"),
		CurrentIndex
	);
	//_______________________________________________________________________________

	//ポイント(敵の動ける位置)より高い数字の場合は0に戻す
	if (CurrentIndex >= Enemy->Patrolpoints.Num())
	{
		CurrentIndex = 0;
	}

	if (bMovinToPlayerPoint)
	{
		bMovinToPlayerPoint = false;
	}

	GetWorld()->GetTimerManager().SetTimer(
		PatrolTimerHandle,
		this,
		&AEnemyAIController::MoveToNextPoint,
		1.0f,      //待機時間
		false
	);
}


void AEnemyAIController::ChackPlayer()
{
	if(bAttacking){ return; }
	if(!bCanDetectPlayer) { return; }

	AEnemeyCharacter* Enemy =
		Cast<AEnemeyCharacter>(GetPawn());

	if (!Enemy) { return; }
	if (!Enemy->PlayerPoint) { return; }

	//Playerのポイントまでの距離を取得
	float Distance =
		FVector::Dist(
			GetPawn()->GetActorLocation(),
			Enemy->PlayerPoint->GetActorLocation()
		);

	//Playerが特定の範囲に入ったら追跡
	if (Distance < 500.f&&!bMovinToPlayerPoint)
	{
		bMovinToPlayerPoint = true;

		MoveToActor(
			Enemy->PlayerPoint,
			DistanceToPlayer);
	}
	else if (Distance >700.f && bMovinToPlayerPoint)
	{
		//デバック用--------------------------------------------------------------------
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("プレイヤーから離れました")
		);
		//_______________________________________________________________________________

		StopMovement();

		bMovinToPlayerPoint = false;

		bCanDetectPlayer = false;

		MoveToNextPoint();
	}
	else
	{
		//Playerが範囲外に出たらパトロール再開
		if (GetMoveStatus() != EPathFollowingStatus::Moving)
		{
			bMovinToPlayerPoint = false;
		}
	}
}

void AEnemyAIController::EnemyAttack()
{
	if (bAttacking)
	{
		return;
	}

	bAttacking = true;
	bCanDetectPlayer = false;

	//デバック用--------------------------------------------------------------------

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("プレイヤーを捕まえました")
	);
	//_______________________________________________________________________________

	GetWorld()->GetTimerManager().SetTimer(
		PatrolTimerHandle,
		this,
		&AEnemyAIController::ReturnToPatrol,
		2.0f,//離れるまでの時間
		false
	);
}

void AEnemyAIController::ReturnToPatrol()
{
	bMovinToPlayerPoint = false;
	bAttacking = false;

	MoveToNextPoint();
	
	GetWorld()->GetTimerManager().SetTimer(
		DetectTimerHandle,
		[this]()
		{
			UE_LOG(LogTemp, Warning, TEXT("Playerを捕まえられるようになりました"));
			bCanDetectPlayer = true;
		},
		5.0f,//次の検知までの時間
		false
	);
}
