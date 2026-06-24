#include "VRCharacter.h"
#include "Component/EchoComponent.h"
#include "Actor/Cane.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

AVRCharacter::AVRCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	EchoComponent =
		CreateDefaultSubobject<UEchoComponent>(
			TEXT("EchoComponent"));
}

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	if (CaneClass)
	{
		Cane = GetWorld()->SpawnActor<ACane>(CaneClass);

		if (Cane)
		{
			Cane->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetIncludingScale,
				TEXT("RightHandSocket"));
		}
	}
}

void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	WalkEchoTimer += DeltaTime;

	if (!EchoComponent)
	{
		return;
	}

	const float Speed = GetVelocity().Size();

	if (Speed > WalkEchoVelocityThreshold &&
		WalkEchoTimer >= WalkEchoInterval)
	{
		EchoComponent->EmitEcho(
			GetActorLocation(),
			WalkEchoRadius);

		WalkEchoTimer = 0.f;
	}
}

void AVRCharacter::SetupPlayerInputComponent(
	UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(
			CaneAction,
			ETriggerEvent::Started,
			this,
			&AVRCharacter::EmitEcho);
	}
}

void AVRCharacter::EmitEcho(
	const FInputActionValue& Value)
{
	if (EchoComponent)
	{
		EchoComponent->EmitEcho(
			GetActorLocation(),
			MaxEchoRadius);
	}
}