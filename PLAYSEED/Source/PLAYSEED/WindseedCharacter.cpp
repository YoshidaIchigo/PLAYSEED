#include "WindseedCharacter.h"
#include "PlayseedGameModeBase.h"
#include "PlayseedVitalityComponent.h"
#include "PlayseedActionComponent.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedCombatComponent.h"
#include "PlayseedRideComponent.h"
#include "PlayseedInteractionComponent.h"
#include "PlayseedLaunchComponent.h"
#include "PlayseedFlightComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AWindseedCharacter::AWindseedCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    Vitality = CreateDefaultSubobject<UPlayseedVitalityComponent>(TEXT("Vitality"));
    Actions = CreateDefaultSubobject<UPlayseedActionComponent>(TEXT("Actions"));
    Carry = CreateDefaultSubobject<UPlayseedCarryComponent>(TEXT("Carry"));
    Climb = CreateDefaultSubobject<UPlayseedClimbComponent>(TEXT("Climb"));
    Combat = CreateDefaultSubobject<UPlayseedCombatComponent>(TEXT("Combat"));
    Ride = CreateDefaultSubobject<UPlayseedRideComponent>(TEXT("GaleleafRide"));
    Interaction = CreateDefaultSubobject<UPlayseedInteractionComponent>(TEXT("Interaction"));
    Launch = CreateDefaultSubobject<UPlayseedLaunchComponent>(TEXT("SkybloomLaunch"));
    Flight = CreateDefaultSubobject<UPlayseedFlightComponent>(TEXT("LeafwingFlight"));
    GetCapsuleComponent()->InitCapsuleSize(34.f, 82.f);
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0, 650, 0);
    GetCharacterMovement()->MaxWalkSpeed = 470;
    GetCharacterMovement()->JumpZVelocity = 740;
    GetCharacterMovement()->GravityScale = 1.45f;
    GetCharacterMovement()->AirControl = .65f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2200;
    GetCharacterMovement()->MaxStepHeight = 35;
    JumpMaxHoldTime = .14f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 900;
    CameraBoom->SocketOffset = FVector(0, 0, 75);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 10;
    CameraBoom->ProbeSize = 14;
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom);
    FollowCamera->FieldOfView = 60;
    FollowCamera->SetAspectRatio(4.f/3.f);
    FollowCamera->bConstrainAspectRatio=true;

    SeedBody = CreateDefaultSubobject<USceneComponent>(TEXT("SeedBody"));
    SeedBody->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    auto Part = [&](const TCHAR* Name, FVector Position, FVector Scale)
    {
        auto* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Mesh->SetupAttachment(SeedBody);
        Mesh->SetStaticMesh(Sphere.Object);
        Mesh->SetRelativeLocation(Position);
        Mesh->SetRelativeScale3D(Scale);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        return Mesh;
    };
    Part(TEXT("AmberSeed"), FVector(0,0,0), FVector(.64f,.58f,1.05f));
    Part(TEXT("LeftBoot"), FVector(9,-21,-61), FVector(.42f,.31f,.25f));
    RightBoot = Part(TEXT("RightBoot"), FVector(9,21,-61), FVector(.42f,.31f,.25f));
    LeftHand = Part(TEXT("LeftHand"), FVector(3,-36,-3), FVector(.22f));
    RightHand = Part(TEXT("RightHand"), FVector(3,36,-3), FVector(.22f));
    Part(TEXT("LeftEye"), FVector(29,-12,17), FVector(.1f,.105f,.14f));
    Part(TEXT("RightEye"), FVector(29,12,17), FVector(.1f,.105f,.14f));
    auto* Leaf = Part(TEXT("Leaf"), FVector(-7,0,65), FVector(.6f,.19f,.18f));
    Leaf->SetRelativeRotation(FRotator(-28,0,0));
}

void AWindseedCharacter::BeginPlay()
{
    Super::BeginPlay();
    UMaterialInterface* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette"));
    TArray<UStaticMeshComponent*> Parts;
    GetComponents(Parts);
    for (UStaticMeshComponent* Part : Parts)
    {
        if (!Palette) continue;
        auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
        const FString Name = Part->GetName();
        const FLinearColor Color = Name.Contains(TEXT("Eye")) ? FLinearColor(.015f,.025f,.035f) :
            (Name.Contains(TEXT("Boot")) || Name.Contains(TEXT("Hand"))) ? FLinearColor(.025f,.22f,.27f) :
            Name.Contains(TEXT("Leaf")) ? FLinearColor(.12f,.65f,.3f) : FLinearColor(1,.58f,.14f);
        Material->SetVectorParameterValue(TEXT("Tint"), Color);
        Part->SetMaterial(0, Material);
    }
    if (auto* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetControlRotation(FRotator(-18, 0, 0));
        PC->PlayerCameraManager->ViewPitchMin = -65;
        PC->PlayerCameraManager->ViewPitchMax = 30;
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }
}

void AWindseedCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (GetActorLocation().Z < -650)
    {
        if (auto* Mode = Cast<APlayseedGameModeBase>(GetWorld()->GetAuthGameMode())) Mode->HandlePlayerFall(this);
        else ReturnToTrail();
    }
    const float Speed = GetVelocity().Size2D();
    SeedBody->SetRelativeLocation(FVector(0,0, FMath::Sin(GetWorld()->GetTimeSeconds()*12)*FMath::Min(Speed/180, 3.f)));
}

void AWindseedCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Actions->SetupInput(Input);
    Carry->SetupInput(Input);
    Climb->SetupInput(Input);
    Combat->SetupInput(Input);
    Ride->SetupInput(Input);
    Interaction->SetupInput(Input);
    Launch->SetupInput(Input);
    Input->BindAxis(TEXT("MoveForward"), this, &AWindseedCharacter::MoveForward);
    Input->BindAxis(TEXT("MoveRight"), this, &AWindseedCharacter::MoveRight);
    Input->BindAxis(TEXT("LookYaw"), this, &AWindseedCharacter::LookYaw);
    Input->BindAxis(TEXT("LookPitch"), this, &AWindseedCharacter::LookPitch);
    Input->BindAxis(TEXT("LookYawRate"), this, &AWindseedCharacter::LookYawRate);
    Input->BindAxis(TEXT("LookPitchRate"), this, &AWindseedCharacter::LookPitchRate);
    Input->BindAxis(TEXT("CameraZoom"), this, &AWindseedCharacter::Zoom);
    Input->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
    Input->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
    Input->BindAction(TEXT("Sprint"), IE_Pressed, this, &AWindseedCharacter::StartSprint);
    Input->BindAction(TEXT("Sprint"), IE_Released, this, &AWindseedCharacter::StopSprint);
    Input->BindAction(TEXT("RestartTrail"), IE_Pressed, this, &AWindseedCharacter::RestartTrail);
    Input->BindAction(TEXT("ReleasePointer"), IE_Pressed, this, &AWindseedCharacter::ReleasePointer);
}
void AWindseedCharacter::MoveForward(float Value)
{
    if (Launch->IsAiming()) return;
    if (Flight->IsFlying()) { Flight->SetSpeedInput(Value); return; }
    if (Climb->IsClimbing()) { Climb->SetClimbInput(Value); return; }
    if (Controller) AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X), Value);
}
void AWindseedCharacter::MoveRight(float Value)
{
    if (Launch->IsAiming()) return;
    if (Flight->IsFlying()) { Flight->SetTurnInput(Value); return; }
    if (Climb->IsClimbing()) return;
    if (Controller) AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y), Value);
}
void AWindseedCharacter::LookYaw(float Value) { if (Launch->IsAiming()) Launch->AdjustAim(Value * 1.2f, 0); else if (Flight->IsFlying()) Flight->AdjustDirection(Value * 1.2f, 0); else AddControllerYawInput(Value); }
void AWindseedCharacter::LookPitch(float Value) { if (Launch->IsAiming()) Launch->AdjustAim(0, Value * 1.2f); else if (Flight->IsFlying()) Flight->AdjustDirection(0, Value * 1.2f); else AddControllerPitchInput(Value); }
void AWindseedCharacter::LookYawRate(float Value) { if (Launch->IsAiming()) Launch->AdjustAim(Value * 100 * GetWorld()->GetDeltaSeconds(), 0); else if (Flight->IsFlying()) Flight->AdjustDirection(Value * 100 * GetWorld()->GetDeltaSeconds(), 0); else AddControllerYawInput(Value*120.f*GetWorld()->GetDeltaSeconds()); }
void AWindseedCharacter::LookPitchRate(float Value) { if (Launch->IsAiming()) Launch->AdjustAim(0, Value * 80 * GetWorld()->GetDeltaSeconds()); else if (Flight->IsFlying()) Flight->AdjustDirection(0, Value * 80 * GetWorld()->GetDeltaSeconds()); else AddControllerPitchInput(Value*90.f*GetWorld()->GetDeltaSeconds()); }
void AWindseedCharacter::Zoom(float Value) { if (!Launch->IsAiming()) CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength-Value*40, 450.f, 1400.f); }
void AWindseedCharacter::SetSprinting(bool bEnabled)
{
    // 乗っている間のShiftも記憶し、降りたときに現在の入力へ戻します。
    bSprintRequested = bEnabled;
    if (!Ride->IsRiding()) GetCharacterMovement()->MaxWalkSpeed = bEnabled ? 670 : 470;
}
void AWindseedCharacter::StartSprint() { SetSprinting(true); }
void AWindseedCharacter::StopSprint() { SetSprinting(false); }
void AWindseedCharacter::ReturnToTrail()
{
    Launch->CancelAim();
    Flight->ResetPower();
    Interaction->ClearMessage();
    Ride->CancelRide();
    Actions->CancelGroundPound();
    Carry->DropHeld();
    Climb->CancelClimb();
    Combat->CancelMelee();
    // 直前の落下速度やジャンプの長押し状態を残すと、復帰直後にまた飛び出してしまいます。
    // CharacterMovementComponent が管理する速度を止めてから、安全な開始位置へ移します。
    StopJumping();
    GetCharacterMovement()->StopMovementImmediately();
    // 物理処理後に受けたLaunchは次の移動Tickまで保留されるため、現在の速度と別に消します。
    GetCharacterMovement()->ClearAccumulatedForces();
    // 動く床の座標差を復帰後へ持ち越さないよう、CharacterMovement の立っている床を解除します。
    // UE5.8 の新しい SetBase オーバーロードを選びます（Scene の Attach とは別の仕組みです）。
    SetBase(static_cast<FMovementBaseInterfaceData*>(nullptr));
    // TeleportPhysics は復帰用の瞬間移動です。通常の探索やジャンプでは使いません。
    // 座標を Character 内へ固定しないことで、別のステージでも同じ移動コードを再利用できます。
    SetActorLocation(TrailStartLocation, false, nullptr, ETeleportType::TeleportPhysics);
    SetSprinting(false);
    Vitality->Restore();
    ++RecoveryCount;
}
void AWindseedCharacter::RestartTrail()
{
    // 入力処理はステージ内の Actor を直接探しません。現在のルール担当へ依頼します。
    // GetAuthGameMode はシングルプレイの World から GameMode を取得する UE の API です。
    if (auto* Mode = Cast<APlayseedGameModeBase>(GetWorld()->GetAuthGameMode()))
        Mode->RestartStage(this);
    else
        ReturnToTrail(); // 専用 GameMode がない確認用マップでも、プレイヤーの復帰は使えます。
}
void AWindseedCharacter::ReleasePointer()
{
    if (auto* PC = Cast<APlayerController>(Controller))
    {
        PC->bShowMouseCursor = !PC->bShowMouseCursor;
        if (PC->bShowMouseCursor) PC->SetInputMode(FInputModeGameAndUI());
        else PC->SetInputMode(FInputModeGameOnly());
    }
}

void AWindseedCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    Actions->HandleLanded(Hit);
}

void AWindseedCharacter::SetPoundPose(bool bEnabled)
{
    // 見た目だけを縮めて急降下の姿勢を表現します。カプセル衝突の大きさは変えません。
    SeedBody->SetRelativeScale3D(bEnabled ? FVector(1.2f, 1.2f, .65f) : FVector::OneVector);
}

void AWindseedCharacter::Jump()
{
    if (Launch->IsAiming()) return;
    if (Flight->IsFlying()) { Flight->CancelFlight(); return; }
    if (Flight->TryBeginFlight()) return;
    Combat->CancelMelee();
    if (!Climb->JumpAway()) Super::Jump();
}

void AWindseedCharacter::SetClimbPose(bool bEnabled)
{
    SeedBody->SetRelativeRotation(bEnabled ? FRotator(-12, 0, 0) : FRotator::ZeroRotator);
}

void AWindseedCharacter::SetMeleePose(int32 Stage, float Progress)
{
    // 描画用の手・靴だけを動かします。命中範囲はCombat Componentで別に管理します。
    const float Swing = Stage ? FMath::Sin(FMath::Clamp(Progress, 0.f, 1.f) * PI) : 0;
    LeftHand->SetRelativeLocation(FVector(3 + (Stage == 1 ? Swing * 55 : 0), -36, -3));
    RightHand->SetRelativeLocation(FVector(3 + (Stage == 2 ? Swing * 55 : 0), 36, -3));
    RightBoot->SetRelativeLocation(FVector(9 + (Stage == 3 ? Swing * 70 : 0), 21, -61 + (Stage == 3 ? Swing * 35 : 0)));
    SeedBody->SetRelativeRotation(FRotator(Stage == 3 ? -Swing * 12 : 0, 0, 0));
}
