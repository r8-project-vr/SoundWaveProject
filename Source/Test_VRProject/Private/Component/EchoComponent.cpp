#include "Component/EchoComponent.h"

#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UEchoComponent::UEchoComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEchoComponent::BeginPlay()
{
	Super::BeginPlay();

	SmoothedEchoOrigin = GetOwner()->GetActorLocation();
}

bool UEchoComponent::IsPriorityEchoActive() const
{
	return bPriorityEchoActive;
}

void UEchoComponent::StartPriorityEcho(float Radius, float Speed, float FadeTime)
{
	bPriorityEchoActive = true;

	const float LifeTime = Radius / Speed;

	PriorityEchoEndTime =
		GetWorld()->GetTimeSeconds()
		+ LifeTime
		+ FadeTime;
}

/// <summary>
/// 音波の更新処理
/// 半径・透明度・ポストプロセスのパラメータを毎フレーム更新する
/// </summary>
void UEchoComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime,TickType,ThisTickFunction);

	bool bHasPriorityEcho = false;

	// 現在表示する音波の情報を保持するための変数
	float MaxAlpha = 0.0f;
	float MaxRadius = 0.0f;
	float MaxInnerRadius = 0.0f;

	// 音波の中心位置を計算するための変数
	FVector WeightedOrigin = FVector::ZeroVector;
	float TotalWeight = 0.0f;


    //=================================================
    // 全ての音波を更新
    //=================================================
	for (int32 i = ActiveEchoes.Num() - 1; i >= 0; --i)
	{
		FEcho& Echo = ActiveEchoes[i];

		if (Echo.bIsPriority){ bHasPriorityEcho = true; }

		// 音波の経過時間を更新
		Echo.Age += DeltaTime;

		// 音波半径を時間経過に応じて拡大
		if (Echo.LifeTime > KINDA_SMALL_NUMBER)
		{
			float Progress = FMath::Clamp(Echo.Age / Echo.LifeTime, 0.0f, 1.0f);

			Echo.CurrentRadius = FMath::Lerp(0.0f, Echo.MaxRadius, Progress);
		}
		else
		{
			Echo.CurrentRadius += Echo.Speed * DeltaTime;
		}

		// 音波の透明度を計算
		float Alpha = 1.0f;
		if (Echo.Age > Echo.LifeTime)
		{
			if (Echo.FadeTime > KINDA_SMALL_NUMBER)
			{
				Alpha = 1.0f - FMath::Clamp((Echo.Age - Echo.LifeTime) / Echo.FadeTime, 0.0f, 1.0f);
			}
			else
			{
				Alpha = 0.0f;
			}
		}

		// フェード中に内側から消える範囲を計算
		float InnerRadius = 0.0f;

		// 現在表示する音波情報を更新
		MaxAlpha 	   = FMath::Max(MaxAlpha, Alpha);
		MaxRadius 	   = FMath::Max(MaxRadius, Echo.CurrentRadius);
		MaxInnerRadius = FMath::Max(MaxInnerRadius,InnerRadius);

		// 音波中心位置の重み付き平均を計算
		float Weight = Alpha * Echo.CurrentRadius;
		if (Weight > 0.0f)
		{
			WeightedOrigin += Echo.Origin * Weight;

			TotalWeight += Weight;
		}

		// 寿命を過ぎた音波を削除
		if (Echo.Age >= (Echo.LifeTime + Echo.FadeTime + 1.0f))
		{
			ActiveEchoes.RemoveAt(i);
		}
	}
	bPriorityEchoActive = bHasPriorityEcho;

	//=================================================
    // 表示するエコーの中心位置を決定
    //=================================================

    // デフォルトは音波発生源の位置
	FVector FinalOrigin = GetOwner()->GetActorLocation();

	// 複数音波がある場合は重み付き平均を使用
	if (TotalWeight > KINDA_SMALL_NUMBER)
	{
		FinalOrigin = WeightedOrigin / TotalWeight;
	}
	else if (ActiveEchoes.Num() > 0)
	{
		FinalOrigin = ActiveEchoes.Last().Origin;
	}

	//=================================================
	// 音波の変化を滑らかに補間
	//=================================================

	// 透明度を補間
	SmoothedEchoAlpha =
		FMath::FInterpTo(
			SmoothedEchoAlpha,
			MaxAlpha,
			DeltaTime,
			EchoAlphaInterpSpeed
		);

	// 半径を補間
	SmoothedEchoRadius =
		FMath::FInterpTo(
			SmoothedEchoRadius,
			MaxRadius,
			DeltaTime,
			EchoRadiusInterpSpeed
		);

	// 内側半径を補間
	SmoothedEchoInnerRadius =
		FMath::FInterpTo(
			SmoothedEchoInnerRadius,
			MaxInnerRadius,
			DeltaTime,
			EchoInnerInterpSpeed
		);

	// 音波の中心位置を補間
	SmoothedEchoOrigin =
		FMath::VInterpTo(
			SmoothedEchoOrigin,
			FinalOrigin,
			DeltaTime,
			EchoRadiusInterpSpeed
		);

    //=================================================
    // マテリアルパラメータへ反映
    //=================================================

	if (EchoMPC)
	{
		UMaterialParameterCollectionInstance* MPC = GetWorld()->GetParameterCollectionInstance(EchoMPC);

		if (MPC)
		{
			// 音波の中心
			MPC->SetVectorParameterValue(TEXT("EchoOrigin"),FLinearColor(SmoothedEchoOrigin));

			// 音波の半径
			MPC->SetScalarParameterValue(TEXT("EchoRadius"),SmoothedEchoRadius);

			// 内側フェード半径
			MPC->SetScalarParameterValue(TEXT("EchoInnerRadius"),0.0f);

			// 音波の透明度
			MPC->SetScalarParameterValue(TEXT("EchoAlpha"),SmoothedEchoAlpha);
		}
	}
}

/// <summary>
/// 右クリックで指定した位置から音波を発生させる
/// 音波の情報を登録し、Niagaraエフェクトを生成する ※今は未使用
/// </summary>
void UEchoComponent::EmitEcho(const FVector& Location,float Radius)
{
	// 新しい音波の情報を作成
	FEcho NewEcho;

	// 音波の発生位置
	NewEcho.Origin = Location;

	// 音波の最大到達半径
	NewEcho.MaxRadius = Radius;

	// 音波が広がる速度
	NewEcho.Speed = EchoSpeed;

	// 発生直後なので半径は0
	NewEcho.CurrentRadius = 0.f;

	// 発生直後なので経過時間は0秒
	NewEcho.Age = 0.f;

	// 最大半径まで到達する時間を計算
	NewEcho.LifeTime = Radius / EchoSpeed;

	// 音波が消えるまでのフェード時間
	NewEcho.FadeTime = GlobalEchoFadeTime;

	// 右クリックの音波中にする
	NewEcho.bIsPriority = true;
	bPriorityEchoActive = true;

	// 音波を管理リストへ追加
	ActiveEchoes.Add(NewEcho);

	//=================================================
	// Niagaraエフェクトを生成 ※今は未使用
	//=================================================

	if (EchoNiagara)
	{
		// 指定位置にNiagaraエフェクトを生成
		UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),EchoNiagara,Location);

		// 生成に成功した場合
		if (Comp)
		{
			UE_LOG(LogTemp, Log, TEXT("Niagaraコンポーネントを生成しました"));
			// Niagaraコンポーネントを管理リストへ追加
			ActiveEchoComponents.Add(Comp);

			// 作成した音波にNiagaraコンポーネントを関連付ける
			ActiveEchoes.Last().NiagaraComp = Comp;
		}
	}
}

/// <summary>
/// 杖で指定した位置から音波を発生させる
/// 音波の情報を登録し、Niagaraエフェクトを生成する ※今は未使用
/// </summary>
void UEchoComponent::CaneEmitEcho(const FVector& Location,float Radius,float Speed,float FadeTime)
{
	// 新しい音波の情報を作成
	FEcho NewEcho;

	// 音波の発生位置
	NewEcho.Origin = Location;

	// 音波の最大到達半径
	NewEcho.MaxRadius = Radius;

	// 音波が広がる速度
	NewEcho.Speed = Speed;

	// 発生直後なので半径は0
	NewEcho.CurrentRadius = 0.f;

	// 発生直後なので経過時間は0秒
	NewEcho.Age = 0.f;

	// 最大半径まで到達する時間を計算
	NewEcho.LifeTime = Radius / Speed;

	// 音波が消えるまでのフェード時間
	NewEcho.FadeTime = FadeTime;

	// 右クリックの音波中にする
	NewEcho.bIsPriority = true;
	bPriorityEchoActive = true;

	// 音波を管理リストへ追加
	ActiveEchoes.Add(NewEcho);
	
	//=================================================
	// Niagaraエフェクトを生成 ※今は未使用
	//=================================================

	if (EchoNiagara)
	{
		// 指定位置にNiagaraエフェクトを生成
		UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),EchoNiagara,Location);

		// 生成に成功した場合
		if (Comp)
		{
			// Niagaraコンポーネントを管理リストへ追加
			ActiveEchoComponents.Add(Comp);

			// 作成した音波にNiagaraコンポーネントを関連付ける
			ActiveEchoes.Last().NiagaraComp = Comp;
		}
	}
}

/// <summary>
/// 歩行音波を発生させる
/// 音波の情報を登録し、Niagaraエフェクトを生成する ※今は未使用
/// </summary>
void UEchoComponent::WalkEmitEcho(const FVector& Location, float Radius)
{
	if (bPriorityEchoActive){ return; }
	// 新しい音波の情報を作成
	FEcho NewEcho;

	// 音波の発生位置
	NewEcho.Origin = Location;

	// 音波の最大到達半径
	NewEcho.MaxRadius = Radius;

	// 音波が広がる速度
	NewEcho.Speed = EchoSpeed;

	// 発生直後なので半径は0
	NewEcho.CurrentRadius = 0.f;

	// 発生直後なので経過時間は0秒
	NewEcho.Age = 0.f;

	// 最大半径まで到達する時間を計算
	NewEcho.LifeTime = Radius / EchoSpeed;

	// 音波が消えるまでのフェード時間
	NewEcho.FadeTime = GlobalEchoFadeTime;

	// 音波を管理リストへ追加
	ActiveEchoes.Add(NewEcho);
}