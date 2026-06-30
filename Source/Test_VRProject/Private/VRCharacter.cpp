#include "VRCharacter.h"
#include "Component/EchoComponent.h"
#include "Actor/Cane.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

AVRCharacter::AVRCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	EchoComponent = CreateDefaultSubobject<UEchoComponent>(TEXT("EchoComponent"));
}

/// <summary>
/// ゲーム開始時の初期化処理
/// 入力設定と杖の生成・アタッチを行う
/// </summary>
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 入力の登録
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// 杖を生成して右手ソケットへ装着
	if (CaneClass)
	{
		Cane = GetWorld()->SpawnActor<ACane>(CaneClass);

		if (Cane)
		{
			UE_LOG(LogTemp,Warning,TEXT("Spawned Cane = %s"), *Cane->GetName());
			Cane->SetOwner(this);

			UE_LOG(LogTemp,Warning,TEXT("After SetOwner = %s"), Cane->GetOwner() ? *Cane->GetOwner()->GetName() : TEXT("NULL"));

			Cane->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("RightHandSocket"));
		}
	}
}

/// <summary>
/// 毎フレーム呼ばれる更新処理
/// プレイヤーが歩いている場合は一定間隔でエコーを発生させる
/// </summary>
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 歩行エコー用タイマーを更新
	WalkEchoTimer += DeltaTime;

	if (!EchoComponent)
	{
		return;
	}

	// 現在の移動速度を取得
	const float Speed = GetVelocity().Size();

	// 一定速度以上で歩いていればエコーを発生
	if (Speed > WalkEchoVelocityThreshold && WalkEchoTimer >= WalkEchoInterval)
	{
		//音波の呼び出し
		EchoComponent->EmitEcho(GetActorLocation(),WalkEchoRadius);

		WalkEchoTimer = 0.f;
	}
}

void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent =Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(
			CaneAction,
			ETriggerEvent::Started,
			this,
			&AVRCharacter::EmitEcho
		);
	}
}

/// <summary>
/// プレイヤーが右クリックした時に呼ばれる
/// 最大半径のエコーを発生させる
/// </summary>
void AVRCharacter::EmitEcho(const FInputActionValue& Value)
{
	if (EchoComponent)
	{
		EchoComponent->EmitEcho(
			GetActorLocation(),
			MaxEchoRadius);
	}
}