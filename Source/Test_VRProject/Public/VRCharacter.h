#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "VRCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UEchoComponent;
class ACane;

struct FInputActionValue;

UCLASS()
class TEST_VRPROJECT_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVRCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	void EmitEcho(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CaneAction;

	UPROPERTY(VisibleAnywhere, Category = "Echo")
	UEchoComponent* EchoComponent;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACane> CaneClass;

	UPROPERTY()
	ACane* Cane;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float MaxEchoRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float WalkEchoInterval = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float WalkEchoVelocityThreshold = 30.f;

	UPROPERTY(EditAnywhere, Category = "Echo")
	float WalkEchoRadius = 400.f;

	float WalkEchoTimer = 0.f;
};