// Fill out your copyright notice in the Description page of Project Settings.

#include "VRCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AVRCharacter::AVRCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GlobalEchoFadeTime = 0.5f;

	SmoothedEchoAlpha = 0.0f;
	SmoothedEchoRadius = 0.0f;
	SmoothedEchoInnerRadius = 0.0f;
	SmoothedEchoOrigin = FVector::ZeroVector;
}

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	SmoothedEchoOrigin = GetActorLocation();

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
	FEcho NewEcho(Location, Radius, EchoSpeed, GlobalEchoFadeTime);
	ActiveEchoes.Add(NewEcho);

	if (EchoNiagara)
	{
		UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			EchoNiagara,
			Location
		);

		if (Comp)
		{
			ActiveEchoComponents.Add(Comp);
			if (ActiveEchoes.Num() > 0)
			{
				ActiveEchoes.Last().NiagaraComp = Comp;
			}
		}
	}
}

void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 歩行エコー
	WalkEchoTimer += DeltaTime;
	const float Speed = GetVelocity().Size();
	if (Speed > WalkEchoVelocityThreshold && WalkEchoTimer >= WalkEchoInterval)
	{
		TriggerEchoAt(GetActorLocation(), WalkEchoRadius);
		WalkEchoTimer = 0.f;
	}

	// 各エコー更新 + 合成（内側フェード半径を計算）
	float MaxAlpha = 0.0f;
	float MaxRadius = 0.0f;
	float MaxInnerRadius = 0.0f; // これをポストプロセスへ渡す（中心から透明になる量）
	FVector WeightedOrigin = FVector::ZeroVector;
	float TotalWeight = 0.0f;

	for (int32 i = ActiveEchoes.Num() - 1; i >= 0; --i)
	{
		FEcho& Echo = ActiveEchoes[i];

		Echo.Age += DeltaTime;

		if (Echo.LifeTime > KINDA_SMALL_NUMBER)
		{
			float Progress = FMath::Clamp(Echo.Age / Echo.LifeTime, 0.0f, 1.0f);
			Echo.CurrentRadius = FMath::Lerp(0.0f, Echo.MaxRadius, Progress);
		}
		else
		{
			Echo.CurrentRadius += Echo.Speed * DeltaTime;
		}

		// フェード係数（寿命後に 1->0）
		float Alpha = 1.0f;
		if (Echo.Age > Echo.LifeTime)
		{
			if (Echo.FadeTime > KINDA_SMALL_NUMBER)
			{
				Alpha = 1.0f - FMath::Clamp((Echo.Age - Echo.LifeTime) / Echo.FadeTime, 0.0f, 1.0f);
			}
			else
			{
				Alpha = 0.0f;
			}
		}

		// 内側半径をフラグで決定:
		// bInnerRadiusIsTransparent == true  -> InnerRadius は「透明化される半径（中心から透明化が広がる）」: 0 -> MaxRadius
		// bInnerRadiusIsTransparent == false -> InnerRadius は「不透明な内側半径（中心が残り外側から消える）」: MaxRadius -> 0
		float InnerRadius = 0.0f;
		if (Echo.Age > Echo.LifeTime && Echo.FadeTime > KINDA_SMALL_NUMBER)
		{
			float FadeProgress = FMath::Clamp((Echo.Age - Echo.LifeTime) / Echo.FadeTime, 0.0f, 1.0f);
			if (bInnerRadiusIsTransparent)
			{
				// 中心から透明化が広がる（中心が先に消える）
				InnerRadius = FadeProgress * Echo.MaxRadius;
			}
			else
			{
				// 外から中心へ消える（中心が最後まで残る）
				InnerRadius = (1.0f - FadeProgress) * Echo.MaxRadius;
			}
		}

		MaxAlpha = FMath::Max(MaxAlpha, Alpha);
		MaxRadius = FMath::Max(MaxRadius, Echo.CurrentRadius);
		MaxInnerRadius = FMath::Max(MaxInnerRadius, InnerRadius);

		float Weight = Alpha * Echo.CurrentRadius;
		if (Weight > 0.0f)
		{
			WeightedOrigin += Echo.Origin * Weight;
			TotalWeight += Weight;
		}

		if (Echo.Age >= (Echo.LifeTime + Echo.FadeTime))
		{
			ActiveEchoes.RemoveAt(i);
		}
	}

	// Niagara コンポーネント群のクリーンアップ
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

	FVector FinalOrigin = GetActorLocation();
	if (TotalWeight > KINDA_SMALL_NUMBER)
	{
		FinalOrigin = WeightedOrigin / TotalWeight;
	}
	else if (ActiveEchoes.Num() > 0)
	{
		FinalOrigin = ActiveEchoes.Last().Origin;
	}

	// スムージング（徐々に薄く/変化させる）
	SmoothedEchoAlpha = FMath::FInterpTo(SmoothedEchoAlpha, MaxAlpha, DeltaTime, EchoAlphaInterpSpeed);
	SmoothedEchoRadius = FMath::FInterpTo(SmoothedEchoRadius, MaxRadius, DeltaTime, EchoRadiusInterpSpeed);
	SmoothedEchoInnerRadius = FMath::FInterpTo(SmoothedEchoInnerRadius, MaxInnerRadius, DeltaTime, EchoInnerInterpSpeed);
	SmoothedEchoOrigin = FMath::VInterpTo(SmoothedEchoOrigin, FinalOrigin, DeltaTime, EchoRadiusInterpSpeed);

	// ポストプロセス用にセット（スムースされた値）
	if (EchoMPC)
	{
		UMaterialParameterCollectionInstance* MPC = GetWorld()->GetParameterCollectionInstance(EchoMPC);
		if (MPC)
		{
			MPC->SetVectorParameterValue(TEXT("EchoOrigin"), FLinearColor(SmoothedEchoOrigin));
			MPC->SetScalarParameterValue(TEXT("EchoRadius"), SmoothedEchoRadius);         // 外側の表現
			MPC->SetScalarParameterValue(TEXT("EchoInnerRadius"), SmoothedEchoInnerRadius); // 中心から透明化する内側半径
			MPC->SetScalarParameterValue(TEXT("EchoAlpha"), SmoothedEchoAlpha);
		}
	}
}

void AVRCharacter::EmitEcho(const FInputActionValue& Value)
{
	TriggerEchoAt(GetActorLocation(), MaxEchoRadius);
}
