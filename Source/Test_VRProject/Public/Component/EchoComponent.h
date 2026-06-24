#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "EchoComponent.generated.h"

class UMaterialParameterCollection;
class UNiagaraSystem;

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
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TEST_VRPROJECT_API UEchoComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	UFUNCTION(BlueprintCallable)
	void EmitEcho(
		const FVector& Location,
		float Radius);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	UMaterialParameterCollection* EchoMPC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	UNiagaraSystem* EchoNiagara;

	UPROPERTY(EditAnywhere)
	float EchoSpeed = 2000.f;

	UPROPERTY(EditAnywhere)
	float GlobalEchoFadeTime = 0.5f;

private:

	TArray<FEcho> ActiveEchoes;

	TArray<TObjectPtr<UNiagaraComponent>> ActiveEchoComponents;

	// ポストプロセス用スムージング
	UPROPERTY(EditAnywhere)
	float EchoAlphaInterpSpeed = 6.0f;

	UPROPERTY(EditAnywhere)
	float EchoRadiusInterpSpeed = 6.0f;

	UPROPERTY(EditAnywhere)
	float EchoInnerInterpSpeed = 6.0f;

	UPROPERTY(EditAnywhere)
	bool bInnerRadiusIsTransparent = true;

	// スムージング状態
	float SmoothedEchoAlpha = 0.0f;

	float SmoothedEchoRadius = 0.0f;

	float SmoothedEchoInnerRadius = 0.0f;

	FVector SmoothedEchoOrigin = FVector::ZeroVector;

};