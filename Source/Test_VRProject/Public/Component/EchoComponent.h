#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "EchoComponent.generated.h"

class UMaterialParameterCollection;
class UNiagaraSystem;

/// <summary>
/// 音波の情報を保持する構造体
/// </summary>
USTRUCT(BlueprintType)
struct FEcho
{
	GENERATED_BODY()

	// 音波の発生位置
	UPROPERTY()
	FVector Origin = FVector::ZeroVector;

	// 現在の音波半径
	UPROPERTY()
	float CurrentRadius = 0.f;

	// 音波が広がる最大半径
	UPROPERTY()
	float MaxRadius = 1000.f;

	// 音波が広がる速度
	UPROPERTY()
	float Speed = 2000.f;

	// 音波が発生してからの経過時間
	UPROPERTY()
	float Age = 0.f;

	// 音波が広がり切るまでの時間
	UPROPERTY()
	float LifeTime = 1.f;

	// 音波が消えるまでのフェード時間
	UPROPERTY()
	float FadeTime = 0.5f;

	// この音波に対応するNiagaraコンポーネント　※今のところは使用していない
	TWeakObjectPtr<UNiagaraComponent> NiagaraComp;

	// 右クリック・杖の音波なら true
	bool bIsPriority = false;
};

/// <summary>
/// 音波の生成・更新・削除を管理するコンポーネント
/// ポストプロセスへ値を渡し、音波を制御する
/// </summary>
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TEST_VRPROJECT_API UEchoComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEchoComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;

	// 右クリックまたは杖が表示中かを返す関数
	bool IsPriorityEchoActive() const;

	// 右クリック・杖の音波の表示開始を登録する関数
	void StartPriorityEcho(float Radius, float Speed, float FadeTime);
	
	// 右クリックの音波発生アクションで呼ばれる関数
	UFUNCTION(BlueprintCallable)
	void EmitEcho(const FVector& Location, float Radius);

	// 杖の音波発生アクションで呼ばれる関数
	UFUNCTION(BlueprintCallable)
	void CaneEmitEcho(const FVector& Location, float Radius, float Speed, float FadeTime);

	// 歩行用の音波を発生させる関数
	UFUNCTION(BlueprintCallable)
	void WalkEmitEcho(const FVector& Location, float Radius);

protected:
	// ポストプロセスへ値を渡すマテリアルのパラメータコレクション
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	UMaterialParameterCollection* EchoMPC;

	// 音波のNiagaraエフェクト
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo")
	UNiagaraSystem* EchoNiagara;

	// 音波の広がる速度
	UPROPERTY(EditAnywhere)
	float EchoSpeed = 2000.f;

	// 音波が消えるまでの時間
	UPROPERTY(EditAnywhere)
	float GlobalEchoFadeTime = 0.5f;
private:
	// 現在存在している音波
	TArray<FEcho> ActiveEchoes;

	// 再生中のNiagaraコンポーネント
	TArray<TObjectPtr<UNiagaraComponent>> ActiveEchoComponents;

	// 現在、右クリックまたは杖の音波を表示しているか
	bool bPriorityEchoActive = false;

	// 音波が完全に消える時間
	float PriorityEchoEndTime = 0.f;
	// ポストプロセスのAlpha値を滑らかに補間する速度
	UPROPERTY(EditAnywhere)
	float EchoAlphaInterpSpeed = 6.0f;

	// ポストプロセスの半径を滑らかに補間する速度
	UPROPERTY(EditAnywhere)
	float EchoRadiusInterpSpeed = 6.0f;

	// ポストプロセスの内側半径を滑らかに補間する速度
	UPROPERTY(EditAnywhere)
	float EchoInnerInterpSpeed = 6.0f;

	// trueなら中心から透明になる
	// falseなら外側から透明になる
	UPROPERTY(EditAnywhere)
	bool bInnerRadiusIsTransparent = true;

	// ポストプロセスの補間後のAlpha値
	float SmoothedEchoAlpha = 0.0f;

	// ポストプロセスの補間後の音波の半径
	float SmoothedEchoRadius = 0.0f;

	// ポストプロセスの補間後の音波の内側半径
	float SmoothedEchoInnerRadius = 0.0f;

	// ポストプロセスの補間後の音波の中心座標
	FVector SmoothedEchoOrigin = FVector::ZeroVector;

};