// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "NiagaraFunctionLibrary.h"

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

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bEchoActive)
	{
		return;
	}

	CurrentEchoRadius += EchoSpeed * DeltaTime;

	if (CurrentEchoRadius >= MaxEchoRadius)
	{
		bEchoActive = false;

		CurrentEchoRadius = 0.f;

		if (EchoMPC)
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

		return;
	}

	if (EchoMPC)
	{
		UMaterialParameterCollectionInstance* MPC =
			GetWorld()->GetParameterCollectionInstance(EchoMPC);

		if (MPC)
		{
			MPC->SetVectorParameterValue(
				TEXT("EchoOrigin"),
				FLinearColor(EchoOrigin)
			);

			MPC->SetScalarParameterValue(
				TEXT("EchoRadius"),
				CurrentEchoRadius
			);
		}
	}
}
void AVRCharacter::EmitEcho(const FInputActionValue& Value)
{
	EchoOrigin = GetActorLocation();

	CurrentEchoRadius = 0.f;

	bEchoActive = true;
	const float EchoRadius = 1000.f;
	const float WaveSpeed = 2000.f;

	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		EchoRadius,
		32,
		FColor::Green,
		false,
		2.f
	);

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
				GetActorLocation(),
				Actor->GetActorLocation()
			);

		if (Distance > EchoRadius)
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
