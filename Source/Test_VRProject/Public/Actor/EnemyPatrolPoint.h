// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyPatrolPoint.generated.h"

UCLASS()
class TEST_VRPROJECT_API AEnemyPatrolPoint : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;
public:	
	// Sets default values for this actor's properties
	AEnemyPatrolPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
