#include "Actor/Cane.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Component/EchoComponent.h"
#include "DrawDebugHelpers.h"
#include "VRCharacter.h"

ACane::ACane()
{
	PrimaryActorTick.bCanEverTick = true;

	CaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaneMesh"));

	RootComponent = CaneMesh;

	TipPoint = CreateDefaultSubobject<USphereComponent>(TEXT("TipPoint"));

	TipPoint->SetupAttachment(CaneMesh);

	TipPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TipPoint->SetSphereRadius(5.f);

	// 杖の長さに応じて調整
	TipPoint->SetRelativeLocation(FVector(0.f, 0.f, -90.f));

}

void ACane::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Cane BeginPlay : %s"),
		*GetName()
	);
}

void ACane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Start = TipPoint->GetComponentLocation();

	FHitResult Hit;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(10.f);

	bool bHit =
		GetWorld()->SweepSingleByChannel(
			Hit,
			Start,
			Start,
			FQuat::Identity,
			ECC_Visibility,
			Sphere
		);

	DrawDebugSphere(
		GetWorld(),
		Start,
		10.f,
		12,
		bHit ? FColor::Green : FColor::Red,
		false,
		0.f
	);

	if (LastHighlighted)
	{
		LastHighlighted->SetRenderCustomDepth(false);
		LastHighlighted = nullptr;
	}

	if (bHit && Hit.GetComponent())
	{
		Hit.GetComponent()->SetRenderCustomDepth(true);
		Hit.GetComponent()->SetCustomDepthStencilValue(1);

		LastHighlighted = Hit.GetComponent();

		if (!bWasHitLastFrame)
		{

			UE_LOG(LogTemp, Warning, TEXT("CANE ECHO"));

			AVRCharacter* Character = Cast<AVRCharacter>(GetOwner());

			if (Character)
			{
				if (UEchoComponent* Echo = Character->FindComponentByClass<UEchoComponent>())
				{
					Echo->EmitEcho(
						Hit.ImpactPoint,
						300.f);
				}
			}
			else
			{
				UE_LOG(
					LogTemp,
					Error,
					TEXT("Owner NULL")
				);
			}
		}

		bWasHitLastFrame = true;
	}
	else
	{
		bWasHitLastFrame = false;
	}
}