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
/// <summary>
/// プレイヤーキャラクター
/// 入力・歩行音波・杖の管理を行う
/// </summary>
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

	// 右クリックの音波発生アクション
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* EchoAction;

	// 音波を管理するコンポーネント
	UPROPERTY(VisibleAnywhere, Category = "Echo")
	UEchoComponent* EchoComponent;

	// プレイヤーが持つ杖のBP
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACane> CaneClass;

	// スポーンした杖
	UPROPERTY()
	ACane* Cane;

	// 右クリック時の音波最大半径
	UPROPERTY(EditAnywhere, Category = "Echo")
	float MaxEchoRadius = 1000.f;

	// 歩行音波の発生間隔
	UPROPERTY(EditAnywhere, Category = "Echo")
	float WalkEchoInterval = 0.5f;

	// 歩行音波を発生させる最低速度
	UPROPERTY(EditAnywhere, Category = "Echo")
	float WalkEchoVelocityThreshold = 30.f;

	// 歩行音波の半径
	UPROPERTY(EditAnywhere, Category = "Echo")
	float WalkEchoRadius = 400.f;

	// 歩行音波のタイマー
	float WalkEchoTimer = 0.f;
};