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

	SmoothedEchoOrigin =
		GetOwner()->GetActorLocation();
}

void UEchoComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(
		DeltaTime,
		TickType,
		ThisTickFunction);

	float MaxAlpha = 0.0f;
	float MaxRadius = 0.0f;
	float MaxInnerRadius = 0.0f;

	FVector WeightedOrigin = FVector::ZeroVector;
	float TotalWeight = 0.0f;

	for (int32 i = ActiveEchoes.Num() - 1; i >= 0; --i)
	{
		FEcho& Echo = ActiveEchoes[i];

		Echo.Age += DeltaTime;

		if (Echo.LifeTime > KINDA_SMALL_NUMBER)
		{
			float Progress =
				FMath::Clamp(
					Echo.Age / Echo.LifeTime,
					0.0f,
					1.0f);

			Echo.CurrentRadius =
				FMath::Lerp(
					0.0f,
					Echo.MaxRadius,
					Progress);
		}
		else
		{
			Echo.CurrentRadius +=
				Echo.Speed * DeltaTime;
		}

		float Alpha = 1.0f;

		if (Echo.Age > Echo.LifeTime)
		{
			if (Echo.FadeTime > KINDA_SMALL_NUMBER)
			{
				Alpha =
					1.0f -
					FMath::Clamp(
						(Echo.Age - Echo.LifeTime)
						/ Echo.FadeTime,
						0.0f,
						1.0f);
			}
			else
			{
				Alpha = 0.0f;
			}
		}
		float InnerRadius = 0.0f;

		if (Echo.Age > Echo.LifeTime &&
			Echo.FadeTime > KINDA_SMALL_NUMBER)
		{
			float FadeProgress =
				FMath::Clamp(
					(Echo.Age - Echo.LifeTime)
					/ Echo.FadeTime,
					0.0f,
					1.0f);

			if (bInnerRadiusIsTransparent)
			{
				InnerRadius =
					FadeProgress *
					Echo.MaxRadius;
			}
			else
			{
				InnerRadius =
					(1.0f - FadeProgress)
					* Echo.MaxRadius;
			}
		}

		MaxAlpha =
			FMath::Max(MaxAlpha, Alpha);

		MaxRadius =
			FMath::Max(MaxRadius,
				Echo.CurrentRadius);

		MaxInnerRadius =
			FMath::Max(MaxInnerRadius,
				InnerRadius);

		float Weight =
			Alpha *
			Echo.CurrentRadius;

		if (Weight > 0.0f)
		{
			WeightedOrigin +=
				Echo.Origin * Weight;

			TotalWeight += Weight;
		}

		if (Echo.Age >=
			(Echo.LifeTime + Echo.FadeTime))
		{
			ActiveEchoes.RemoveAt(i);
		}
	}

	FVector FinalOrigin =
		GetOwner()->GetActorLocation();

	if (TotalWeight > KINDA_SMALL_NUMBER)
	{
		FinalOrigin =
			WeightedOrigin /
			TotalWeight;
	}
	else if (ActiveEchoes.Num() > 0)
	{
		FinalOrigin =
			ActiveEchoes.Last().Origin;
	}

	SmoothedEchoAlpha =
		FMath::FInterpTo(
			SmoothedEchoAlpha,
			MaxAlpha,
			DeltaTime,
			EchoAlphaInterpSpeed);

	SmoothedEchoRadius =
		FMath::FInterpTo(
			SmoothedEchoRadius,
			MaxRadius,
			DeltaTime,
			EchoRadiusInterpSpeed);

	SmoothedEchoInnerRadius =
		FMath::FInterpTo(
			SmoothedEchoInnerRadius,
			MaxInnerRadius,
			DeltaTime,
			EchoInnerInterpSpeed);

	SmoothedEchoOrigin =
		FMath::VInterpTo(
			SmoothedEchoOrigin,
			FinalOrigin,
			DeltaTime,
			EchoRadiusInterpSpeed);

	if (EchoMPC)
	{
		UMaterialParameterCollectionInstance* MPC =
			GetWorld()->
			GetParameterCollectionInstance(
				EchoMPC);

		if (MPC)
		{
			MPC->SetVectorParameterValue(
				TEXT("EchoOrigin"),
				FLinearColor(
					SmoothedEchoOrigin));

			MPC->SetScalarParameterValue(
				TEXT("EchoRadius"),
				SmoothedEchoRadius);

			MPC->SetScalarParameterValue(
				TEXT("EchoInnerRadius"),
				SmoothedEchoInnerRadius);

			MPC->SetScalarParameterValue(
				TEXT("EchoAlpha"),
				SmoothedEchoAlpha);
		}
	}
}

void UEchoComponent::EmitEcho(
	const FVector& Location,
	float Radius)
{
	FEcho NewEcho;

	NewEcho.Origin = Location;
	NewEcho.MaxRadius = Radius;
	NewEcho.Speed = EchoSpeed;
	NewEcho.CurrentRadius = 0.f;
	NewEcho.Age = 0.f;

	NewEcho.LifeTime =
		Radius / EchoSpeed;

	NewEcho.FadeTime =
		GlobalEchoFadeTime;

	ActiveEchoes.Add(NewEcho);

	if (EchoNiagara)
	{
		UNiagaraComponent* Comp =
			UNiagaraFunctionLibrary::
			SpawnSystemAtLocation(
				GetWorld(),
				EchoNiagara,
				Location);

		if (Comp)
		{
			ActiveEchoComponents.Add(
				Comp);

			ActiveEchoes.Last()
				.NiagaraComp = Comp;
		}
	}
}