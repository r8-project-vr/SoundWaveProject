#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Component/EnemySoundComponent.h"
#include "EnemeyCharacter.generated.h"

class AEnemyPatrolPoint;

UCLASS()
class TEST_VRPROJECT_API AEnemeyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemeyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//移動用Pointの宣言
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	TArray<AEnemyPatrolPoint*> Patrolpoints;

	//Player用のPointの宣言
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	AEnemyPatrolPoint* PlayerPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UEnemySoundComponent* SoundComponent;
};
