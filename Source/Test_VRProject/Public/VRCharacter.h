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

struct FInputActionValue;
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

	void EmitEcho(const FInputActionValue& Value);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

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

};
