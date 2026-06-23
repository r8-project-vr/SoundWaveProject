// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NiagaraFunctionLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "NiagaraSystem.h"
#include "VRCharacter.generated.h"

class UMaterialInterface;
class UNiagaraSystem;
class UNiagaraComponent;
class UInputAction;

struct FInputActionValue;

USTRUCT(BlueprintType)
struct FEcho
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Origin = FVector::ZeroVector;

	UPROPERTY()
	float CurrentRadius = 0.f;

	UPROPERTY()
	float MaxRadius = 1000.f;

	UPROPERTY()
	float Speed = 2000.f;

	UPROPERTY()
	float Age = 0.f;

	UPROPERTY()
	float LifeTime = 1.f;

	UPROPERTY()
	float FadeTime = 0.5f;

	TWeakObjectPtr<UNiagaraComponent> NiagaraComp;

	FEcho() = default;

	FEcho(const FVector& InOrigin, float InMaxRadius, float InSpeed, float InFadeTime = 0.5f)
		: Origin(InOrigin)
		, CurrentRadius(0.f)
		, MaxRadius(InMaxRadius)
		, Speed(InSpeed)
		, Age(0.f)
		, LifeTime((InSpeed > 0.f) ? (InMaxRadius / InSpeed) : 1.f)
		, FadeTime(InFadeTime)
	{}
};


UCLASS()
class TEST_VRPROJECT_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVRCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CaneAction;

	UPROPERTY(EditAnywhere, Category = "Echo")
	UMaterialParameterCollection* EchoMPC;

	UPROPERTY(EditAnywhere, Category = "Echo")
	UNiagaraSystem* EchoNiagara;

	UPROPERTY(EditAnywhere, Category = "Echo", meta = (ClampMin = "0.0"))
	float GlobalEchoFadeTime = 0.5f;

	void EmitEcho(const FInputActionValue& Value);

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float MaxEchoRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float EchoSpeed = 2000.f;

	// 歩行エコー設定
	UPROPERTY(EditAnywhere, Category = "Echo")
	float WalkEchoInterval = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float WalkEchoVelocityThreshold = 30.f;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float WalkEchoRadius = 400.f;

private:
	float WalkEchoTimer = 0.f;

	UPROPERTY()
	TArray<FEcho> ActiveEchoes;

	UPROPERTY()
	TArray<TObjectPtr<UNiagaraComponent>> ActiveEchoComponents;

	// ポストプロセス用スムージング
	UPROPERTY(EditAnywhere, Category = "Echo", meta = (ClampMin = "0.0"))
	float EchoAlphaInterpSpeed = 6.0f;

	UPROPERTY(EditAnywhere, Category = "Echo", meta = (ClampMin = "0.0"))
	float EchoRadiusInterpSpeed = 6.0f;

	// 中心から消えていくための内側半径スムース速度
	UPROPERTY(EditAnywhere, Category = "Echo", meta = (ClampMin = "0.0"))
	float EchoInnerInterpSpeed = 6.0f;

	// true: EchoInnerRadius は「透明化される内側半径」（中心から透明化が広がる）
	// false: EchoInnerRadius は「不透明な内側半径」（中心が残り、外側から消える）
	UPROPERTY(EditAnywhere, Category = "Echo")
	bool bInnerRadiusIsTransparent = true;

	// 内部スムース状態
	float SmoothedEchoAlpha = 0.0f;
	float SmoothedEchoRadius = 0.0f;
	float SmoothedEchoInnerRadius = 0.0f;
	FVector SmoothedEchoOrigin = FVector::ZeroVector;

	void TriggerEchoAt(const FVector& Location, float Radius);
};
