#include "Actor/Cane.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Component/EchoComponent.h"
#include "DrawDebugHelpers.h"
#include "VRCharacter.h"

/// <summary>
/// 杖の各コンポーネントを生成し初期設定を行う
/// </summary>
ACane::ACane()
{
	// Tickを有効化
	PrimaryActorTick.bCanEverTick = true;

	//==============================
	// 杖メッシュの生成
	//==============================

	CaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaneMesh"));

	RootComponent = CaneMesh;

	//==============================
	// 杖先端の判定用コンポーネント
	//==============================

	TipPoint = CreateDefaultSubobject<USphereComponent>(TEXT("TipPoint"));

	TipPoint->SetupAttachment(CaneMesh);

	// 位置取得
	TipPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 判定位置の目印となる球の大きさ(デバッグ用)
	TipPoint->SetSphereRadius(5.f);

	// 杖先端の位置へ移動
	TipPoint->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
}

void ACane::BeginPlay()
{
	Super::BeginPlay();
}

/// <summary>
/// 毎フレーム杖先端の当たり判定を行う
/// 先端にオブジェクトが接触したら音波を発生させる
/// </summary>
void ACane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//==============================
	// 杖先端の位置を取得
	//==============================

	FVector Start = TipPoint->GetComponentLocation();

	FHitResult Hit;

	// 球状の判定を作成
	FCollisionShape Sphere = FCollisionShape::MakeSphere(10.f);

	//==============================
	// 杖先端で球判定を実行
	//==============================

	bool bHit =
		GetWorld()->SweepSingleByChannel(
			Hit,
			Start,
			Start,
			FQuat::Identity,
			ECC_Visibility,
			Sphere
		);

	// 判定位置をデバッグ表示
	DrawDebugSphere(
		GetWorld(),
		Start,
		10.f,
		12,
		bHit ? FColor::Green : FColor::Red,
		false,
		0.f
	);

	//==============================
	// 先端にオブジェクトが接触した場合
	//==============================

	if (bHit && Hit.GetComponent())
	{
		// アウトライン表示を有効化(ポストプロセス)
		Hit.GetComponent()->SetRenderCustomDepth(true);

		// 描画値を設定
		Hit.GetComponent()->SetCustomDepthStencilValue(1);

		//==============================
		// 接触時のみ音波を発生
		//==============================

		if (!bWasHitLastFrame)
		{
			UE_LOG(LogTemp,Error,TEXT("CANE ECHO"));

			// プレイヤーを取得
			AVRCharacter* Character = Cast<AVRCharacter>(GetOwner());

			if (Character)
			{
				// プレイヤーが持つEchoComponentを取得
				if (UEchoComponent* Echo = Character->FindComponentByClass<UEchoComponent>())
				{
					// 杖が接触した位置を中心に音波を発生させる
					Echo->CaneEmitEcho(Hit.ImpactPoint,EchoRadius,EchoSpeed,EchoFadeTime);
				}
			}
		}

		// オブジェクトから離れるまで接触している判定
		bWasHitLastFrame = true;
	}
	else
	{
		// オブジェクトから離れたら接触状態をリセット
		bWasHitLastFrame = false;
	}
}