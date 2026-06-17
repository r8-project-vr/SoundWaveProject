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
class UInputAction;
class UNiagaraComponent;

struct FInputActionValue;

USTRUCT(BlueprintType)
struct FEcho
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Origin = FVector::ZeroVector;

	UPROPERTY()
	float CurrentRadius = 0.f;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float MaxRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float Speed = 2000.f;

	FEcho() = default;

	FEcho(const FVector& InOrigin, float InMaxRadius, float InSpeed)
		: Origin(InOrigin), CurrentRadius(0.f), MaxRadius(InMaxRadius), Speed(InSpeed)
	{}
};


UCLASS()
class TEST_VRPROJECT_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CaneAction;

	UPROPERTY(EditAnywhere, Category = "Echo")
	UMaterialParameterCollection* EchoMPC;

	// Niagara エフェクト（各エコーごとにスポーンする）
	UPROPERTY(EditAnywhere, Category = "Echo")
	UNiagaraSystem* EchoNiagara;

	void EmitEcho(const FInputActionValue& Value);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 旧来のフラグは残すが、実際の拡張は ActiveEchoes が担当
	bool bEchoActive = false;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float CurrentEchoRadius = 0.f;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float MaxEchoRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float EchoSpeed = 2000.f;

	UPROPERTY()
	TSet<UStaticMeshComponent*> RevealedMeshes;

	FVector EchoOrigin;

	// --- 歩行エコー設定 ---
	UPROPERTY(EditAnywhere, Category = "Echo")
	float WalkEchoInterval = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float WalkEchoVelocityThreshold = 30.f;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float WalkEchoRadius = 400.f;

private:
	// 歩行エコー用タイマー
	float WalkEchoTimer = 0.f;

	// 現在アクティブなエコー群
	UPROPERTY()
	TArray<FEcho> ActiveEchoes;

	// 各エコー用の Niagara コンポーネント群（視覚的に重ねるため保持）
	UPROPERTY()
	TArray<UNiagaraComponent*> ActiveEchoComponents;

	// エコーを新規追加するユーティリティ
	void TriggerEchoAt(const FVector& Location, float Radius);
};
