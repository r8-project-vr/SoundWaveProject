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

	// 杖を生成
	if (CaneClass)
	{
		Cane = GetWorld()->SpawnActor<ACane>(CaneClass);

		if (Cane)
		{
			Cane->SetOwner(this);

			Cane->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("MotionControllerRightGrip"));
		}
	}
}

/// <summary>
/// 毎フレーム呼ばれる更新処理
/// プレイヤーが歩いている場合は一定間隔で音波を発生させる
/// </summary>
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 歩行音波用タイマーを更新
	WalkEchoTimer += DeltaTime;

	if (!EchoComponent){ return; }

	// 現在の移動速度を取得
	const float Speed = GetVelocity().Size();

	// 一定速度以上で歩いていれば音波を発生
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
			EchoAction,
			ETriggerEvent::Started,
			this,
			&AVRCharacter::EmitEcho
		);
	}
}

/// <summary>
/// プレイヤーが右クリックした時に呼ばれる
/// 最大半径の音波を発生させる
/// </summary>
void AVRCharacter::EmitEcho(const FInputActionValue& Value)
{
	if (EchoComponent){ EchoComponent->EmitEcho(GetActorLocation(), MaxEchoRadius); }
}