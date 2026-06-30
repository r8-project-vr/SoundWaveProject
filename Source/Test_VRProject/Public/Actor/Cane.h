#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cane.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UPrimitiveComponent;

UCLASS(Blueprintable)
class TEST_VRPROJECT_API ACane : public AActor
{
	GENERATED_BODY()

public:
	ACane();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CaneMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* TipPoint;

	UPrimitiveComponent* LastHighlighted = nullptr;
	bool bWasHitLastFrame = false;
};