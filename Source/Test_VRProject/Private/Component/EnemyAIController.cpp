// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/EnemyAIController.h"
#include "EnemeyCharacter.h"                    //敵キャラクター
#include "Actor/EnemyPatrolPoint.h"
#include "TimerManager.h"                       //時間関係
#include "Math/RandomStream.h"                  //ランダム関係
#include "Navigation/PathFollowingComponent.h"

void AEnemyAIController::BeginPlay()
{
	//デバック用----------------------------------------------
	AEnemeyCharacter* Enemy =
		Cast<AEnemeyCharacter>(GetPawn());

	if (!Enemy)
	{
		UE_LOG(LogTemp, Error, TEXT("Enemy nullptr"));
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("Enemy=%s"),
		*Enemy->GetName());

	UE_LOG(LogTemp, Warning,
		TEXT("Pawn=%s"),
		*GetPawn()->GetName());
	//_______________________________________________________

	Super::BeginPlay();

	//次移動する場所
	CurrentIndex = 0;

	MoveToNextPoint();
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	if (Distance < 500.f)
	{
		MoveToActor(
			Enemy->PlayerPoint,
			100.f);

		bMovinToPlayerPoint = true;
	}
	else
	{
		//Playerが範囲外に出たらパトロール再開
		if (GetMoveStatus() != EPathFollowingStatus::Moving)
		{
			bMovinToPlayerPoint = true;
			MoveToNextPoint();
		}
	}
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

	//デバック用--------------------------------------------------------------------

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Enemy Pos = %s"),
		*GetPawn()->GetActorLocation().ToString()
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Target Pos = %s"),
		*Point->GetActorLocation().ToString()
	);
	//______________________________________________________________________________

	//移動先のポイントに近づける距離
	const EPathFollowingRequestResult::Type Result =
		MoveToActor(
			Enemy->Patrolpoints[CurrentIndex],
			100.f //受け入れ半径
		);

	//デバック用--------------------------------------------------------------------
	UE_LOG(LogTemp, Warning,
		TEXT("Move Result = %d"),
		(int32)Result);
	//_____________________________________________________________________________
}

void AEnemyAIController::OnMoveCompleted(
	FAIRequestID RequestID,
	const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	//デバック用-------------------------------------------------------------------
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("OnMoveCompleted Result=%d"),
		(int32)Result.Code
	);
	//_____________________________________________________________________________

	// 成功時だけ次へ
	if (Result.Code != EPathFollowingResult::Success)
	{
		return;
	}

	AEnemeyCharacter* Enemy =
		Cast<AEnemeyCharacter>(GetPawn());

	if (!Enemy)
		return;

	if (Enemy->Patrolpoints.Num() == 0)
		return;

	if (bMovinToPlayerPoint)
		return;

	//ランダムシード値の決定
	FRandomStream RandomStream;
	RandomStream.GenerateNewSeed();

	//次のポイントの決定
	CurrentIndex = RandomStream.RandRange(0, Enemy->Patrolpoints.Num() - 1);

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

	GetWorld()->GetTimerManager().SetTimer(
		PatrolTimerHandle,
		this,
		&AEnemyAIController::MoveToNextPoint,
		0.5f,      //待機時間
		false
	);
}

