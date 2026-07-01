// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/EnemyAIController.h"
#include "EnemeyCharacter.h"                    //敵キャラクター
#include "Actor/EnemyPatrolPoint.h"
#include "TimerManager.h"                       //時間関係
#include "Math/RandomStream.h"                  //ランダム関係
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	AEnemeyCharacter* Enemy =
		Cast<AEnemeyCharacter>(GetPawn());

	CurrentIndex;

	Enemy->GetCharacterMovement()->MaxWalkSpeed = PatrolMoveSpeed;
	MoveToNextPoint();

	//デバック用--------------------------------------------------------------------
	UE_LOG(
		LogTemp,
		Display,
		TEXT("-初期設定----------------------------")
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("ポイントのインデックス(初期化)=%d"),
		CurrentIndex
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("playerまでの判定距離=%f"),
		DistanceToPlayer
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("待機時間=%f"),
		WaitTime
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("離れるまでの時間=%f"),
		ToLeaveTime
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("次の検知までの時間=%f"),
		DetectionTime
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("巡回時の移動速度=%f"),
		PatrolMoveSpeed
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("追跡時の移動速度=%f"),
		TrackingMoveSpeed
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("---------------------------------")
	);
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
	//現在のインデックスを表示
	UE_LOG(
		LogTemp,
		Display,
		TEXT("NowCurrentIndex=%d"),
		CurrentIndex
	);

	if (!GetPawn())
	{
		UE_LOG(LogTemp, Error, TEXT("Pawn is null"));
		return;
	}

	UE_LOG(LogTemp, Display,
		TEXT("Pawn Location = %s"),
		*GetPawn()->GetActorLocation().ToString());

	AEnemyPatrolPoint* Point = Enemy->Patrolpoints[CurrentIndex];

	UE_LOG(LogTemp, Display,
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
	//次移動するインデックスを表示
	UE_LOG(
		LogTemp,
		Display,
		TEXT("NextCurrentIndex = %d"),
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
		WaitTime,
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
	if (Distance < DetectionRange && !bMovinToPlayerPoint)
	{
		bMovinToPlayerPoint = true;

		Enemy->GetCharacterMovement()->MaxWalkSpeed = PatrolMoveSpeed;

		MoveToActor(
			Enemy->PlayerPoint,
			DistanceToPlayer);

		//デバック用--------------------------------------------------------------------
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Playerを発見")
		);
		//_______________________________________________________________________________
	}

	//追跡時のみ処理を行う
	if (!bMovinToPlayerPoint) { return; }

	//移動速度の設定
	if(Distance < 300.0f)
	{
		Enemy->GetCharacterMovement()->MaxWalkSpeed = TrackingMoveSpeed;
	}
	else {
		Enemy->GetCharacterMovement()->MaxWalkSpeed = PatrolMoveSpeed;
	}

	//見失った際にパトロールに戻る処理
	if (Distance >= DetectionRange)
	{
		//デバック用--------------------------------------------------------------------
		UE_LOG(
			LogTemp,
			Display,
			TEXT("プレイヤーから離れました")
		);
		//_______________________________________________________________________________

		StopMovement();

		bMovinToPlayerPoint = false;

		bCanDetectPlayer = false;

		Enemy->GetCharacterMovement()->MaxWalkSpeed = PatrolMoveSpeed;
		MoveToNextPoint();
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
		ToLeaveTime,
		false
	);
}

void AEnemyAIController::ReturnToPatrol()
{
	bMovinToPlayerPoint = false;
	bAttacking = false;

	AEnemeyCharacter* Enemy =
		Cast<AEnemeyCharacter>(GetPawn());

	Enemy->GetCharacterMovement()->MaxWalkSpeed = PatrolMoveSpeed;
	MoveToNextPoint();
	
	GetWorld()->GetTimerManager().SetTimer(
		DetectTimerHandle,
		[this]()
		{
			UE_LOG(LogTemp, 
				Warning,
				TEXT("Playerを捕まえられるようになりました"));
			bCanDetectPlayer = true;
		},
		DetectionTime,
		false
	);
}
