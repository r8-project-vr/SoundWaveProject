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

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	// 杖本体のメッシュ
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CaneMesh;

	// 杖先端の当たり判定
	UPROPERTY(VisibleAnywhere)
	USphereComponent* TipPoint;

	// 前フレームでアウトライン表示していたオブジェクト
	UPrimitiveComponent* LastHighlighted = nullptr;

	// 前フレームで何かに当たっていたか
	bool bWasHitLastFrame = false;
};