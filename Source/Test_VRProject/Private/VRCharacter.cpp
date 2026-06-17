// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h" // ← 追加

// Sets default values
AVRCharacter::AVRCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(
			CaneAction,
			ETriggerEvent::Started,
			this,
			&AVRCharacter::EmitEcho
		);
	}
}

void AVRCharacter::TriggerEchoAt(const FVector& Location, float Radius)
{
	// 新しいエコーを配列に追加（既存は上書きされない）
	ActiveEchoes.Add(FEcho(Location, Radius, EchoSpeed));

	// 各エコーごとに視覚的エフェクトをスポーン（重なって表示される）
	if (EchoNiagara)
	{
		UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			EchoNiagara,
			Location
		);

		if (Comp)
		{
			// オプション：Niagara にパラメータを渡すならここで行う
			// Comp->SetVariableFloat("User.EchoMaxRadius", Radius); // Niagara 側で User.EchoMaxRadius を作る場合など

			ActiveEchoComponents.Add(Comp);
		}
	}

	// 追補的にデバッグ球を描画（寿命を波の到達時間と合わせる）
	const float LifeTime = (EchoSpeed > 0.f) ? (Radius / EchoSpeed) : 1.f;
	DrawDebugSphere(
		GetWorld(),
		Location,
		Radius,
		32,
		FColor::Green,
		false,
		LifeTime
	);

	// 既存のログ/当たり判定は発生時点で実行（従来の EmitEcho と同様）
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsWithTag(
		GetWorld(),
		FName("EchoObject"),
		Actors
	);

	for (AActor* Actor : Actors)
	{
		if (!Actor)
		{
			continue;
		}

		float Distance =
			FVector::Distance(
				Location,
				Actor->GetActorLocation()
			);

		if (Distance > Radius)
		{
			continue;
		}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Detected : %s"),
			*Actor->GetName()
		);

		UStaticMeshComponent* StaticMesh =
			Actor->FindComponentByClass<UStaticMeshComponent>();

		if (!StaticMesh)
		{
			continue;
		}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Echo!")
		);
	}
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// --- 歩行検出（常にチェック） ---
	WalkEchoTimer += DeltaTime;
	const float Speed = GetVelocity().Size();
	if (Speed > WalkEchoVelocityThreshold && WalkEchoTimer >= WalkEchoInterval)
	{
		TriggerEchoAt(GetActorLocation(), WalkEchoRadius);
		WalkEchoTimer = 0.f;
	}

	// アクティブな全エコーを更新（後ろから削除して安全に配列を編集）
	for (int32 i = ActiveEchoes.Num() - 1; i >= 0; --i)
	{
		FEcho& Echo = ActiveEchoes[i];
		Echo.CurrentRadius += Echo.Speed * DeltaTime;

		// 各エコーの視覚化（毎フレーム短時間描画）。Niagara があればそれで見た目は出るため補助的。
		DrawDebugSphere(
			GetWorld(),
			Echo.Origin,
			Echo.CurrentRadius,
			24,
			FColor::Green,
			false,
			0.f,
			0,
			1.f
		);

		if (Echo.CurrentRadius >= Echo.MaxRadius)
		{
			ActiveEchoes.RemoveAt(i);
		}
	}

	// ActiveEchoComponents の後始末（Niagara が再生終了したら破棄）
	for (int32 i = ActiveEchoComponents.Num() - 1; i >= 0; --i)
	{
		UNiagaraComponent* Comp = ActiveEchoComponents[i];
		if (!Comp || !Comp->IsValidLowLevel() || !Comp->IsActive())
		{
			if (Comp)
			{
				Comp->DestroyComponent();
			}
			ActiveEchoComponents.RemoveAt(i);
		}
	}

	if (EchoMPC && ActiveEchoes.Num() > 0)
	{
		UMaterialParameterCollectionInstance* MPC =
			GetWorld()->GetParameterCollectionInstance(EchoMPC);

		if (MPC)
		{
			const FEcho& Latest = ActiveEchoes.Last();

			MPC->SetVectorParameterValue(
				TEXT("EchoOrigin"),
				FLinearColor(Latest.Origin)
			);

			MPC->SetScalarParameterValue(
				TEXT("EchoRadius"),
				Latest.CurrentRadius
			);
		}
	}
	else if (EchoMPC && ActiveEchoes.Num() == 0)
	{
		UMaterialParameterCollectionInstance* MPC =
			GetWorld()->GetParameterCollectionInstance(EchoMPC);

		if (MPC)
		{
			MPC->SetScalarParameterValue(
				TEXT("EchoRadius"),
				0.f
			);
		}
	}
}
void AVRCharacter::EmitEcho(const FInputActionValue& Value)
{
	TriggerEchoAt(GetActorLocation(), MaxEchoRadius);
}
