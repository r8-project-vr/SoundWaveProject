#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cane.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UPrimitiveComponent;

/// <summary>
/// プレイヤーが持つ杖
/// 杖先端の衝突判定と音波発生を管理する
/// </summary>
UCLASS(Blueprintable)
class TEST_VRPROJECT_API ACane : public AActor
{
	GENERATED_BODY()

public:
	ACane();
	virtual void Tick(float DeltaTime) override;

	// 音波の最大半径
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	float EchoRadius = 300.f;

	// 音波の広がる速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	float EchoSpeed = 1000.f;

	// 音波のフェード時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	float EchoFadeTime = 1.0f;

protected:
	virtual void BeginPlay() override;

private:
	// 杖本体のメッシュ
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CaneMesh;

	// 杖先端の当たり判定
	UPROPERTY(VisibleAnywhere)
	USphereComponent* TipPoint;

	// 前フレームで何かに当たっていたか
	bool bWasHitLastFrame = false;
};