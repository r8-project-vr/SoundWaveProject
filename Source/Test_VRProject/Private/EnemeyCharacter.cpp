// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemeyCharacter.h"
#include "Component/EnemyAIController.h"

// Sets default values
AEnemeyCharacter::AEnemeyCharacter()
{
	AIControllerClass = AEnemyAIController::StaticClass();

	AutoPossessAI =
		EAutoPossessAI::PlacedInWorldOrSpawned;

	SoundComponent =
		CreateDefaultSubobject<UEnemySoundComponent>( TEXT("EnemySoundComponent"));

}

// Called when the game starts or when spawned
void AEnemeyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemeyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemeyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

