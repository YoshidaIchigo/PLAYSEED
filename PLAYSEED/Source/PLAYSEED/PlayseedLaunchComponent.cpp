#include "PlayseedLaunchComponent.h"
#include "ThunderLauncher.h"
#include "PlayseedActionComponent.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedCombatComponent.h"
#include "PlayseedRideComponent.h"
#include "PlayseedVitalityComponent.h"
#include "PlayseedInteractionComponent.h"
#include "PlayseedFlightComponent.h"
#include "WindseedCharacter.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"

UPlayseedLaunchComponent::UPlayseedLaunchComponent() { PrimaryComponentTick.bCanEverTick = true; }
void UPlayseedLaunchComponent::SetupInput(UInputComponent* Input)
{
    Input->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &UPlayseedLaunchComponent::Fire);
    Input->BindKey(EKeys::Gamepad_RightTrigger, IE_Pressed, this, &UPlayseedLaunchComponent::Fire);
}
bool UPlayseedLaunchComponent::TryBeginAim(AThunderLauncher* Launcher)
{
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    auto* PC = Player ? Cast<APlayerController>(Player->GetController()) : nullptr;
    if (Player && Player->GetFlight()->IsFlying()) return false;
    if (!Player || !PC || !PC->PlayerCameraManager || !Launcher || !Launcher->IsOpen() || bAiming || !Player->GetCharacterMovement()->IsMovingOnGround() || Player->GetVitality()->IsInvulnerable() || Player->GetRide()->IsRiding() || Player->GetClimb()->IsClimbing() || Player->GetCarry()->GetHeldObject() || Player->GetCombat()->IsAttacking() || Player->GetActions()->IsGroundPounding()) return false;
    const FVector Entry = Player->GetActorLocation();
    const float Yaw = Launcher->GetDefaultAimYaw();
    // 台の座席への移動は、この仕掛けの入場操作です。配置先の衝突を確認してから始めます。
    if (!Player->TeleportTo(Launcher->GetSeatLocation(), FRotator(0, Yaw, 0))) return false;
    EntryLocation = Entry; EntryView = PC->GetControlRotation();
    Target = Launcher; AimYaw = Yaw; AimPitch = 70; bAiming = true;
    auto* Move = Player->GetCharacterMovement();
    Player->StopJumping(); Player->ConsumeMovementInputVector();
    Move->StopMovementImmediately(); Move->ClearAccumulatedForces();
    Player->SetBase(static_cast<FMovementBaseInterfaceData*>(nullptr));
    Move->SetMovementMode(MOVE_None);
    auto* Boom = Player->GetCameraBoom();
    PreviousArmLength = Boom->TargetArmLength; PreviousSocket = Boom->SocketOffset; bPreviousCameraLag = Boom->bEnableCameraLag;
    Boom->TargetArmLength = 0; Boom->SocketOffset = FVector(0, 0, 80); Boom->bEnableCameraLag = false;
    PreviousPitchMin = PC->PlayerCameraManager->ViewPitchMin; PreviousPitchMax = PC->PlayerCameraManager->ViewPitchMax;
    PC->PlayerCameraManager->ViewPitchMin = -30; PC->PlayerCameraManager->ViewPitchMax = 85;
    PC->SetControlRotation(FRotator(AimPitch, AimYaw, 0));
    Player->GetInteraction()->ClearMessage();
    return true;
}
void UPlayseedLaunchComponent::AdjustAim(float YawDelta, float PitchDelta)
{
    if (!bAiming) return;
    AimYaw = FRotator::NormalizeAxis(AimYaw + YawDelta);
    AimPitch = FMath::Clamp(AimPitch + PitchDelta, 20.f, 80.f);
}
void UPlayseedLaunchComponent::CancelAim(bool bReturnToEntry)
{
    if (!bAiming) return;
    bAiming = false;
    if (Target.IsValid()) Target->SetAimPose(70, Target->GetDefaultAimYaw());
    if (auto* Player = Cast<AWindseedCharacter>(GetOwner()))
    {
        auto* Boom = Player->GetCameraBoom();
        Boom->TargetArmLength = PreviousArmLength; Boom->SocketOffset = PreviousSocket; Boom->bEnableCameraLag = bPreviousCameraLag;
        if (auto* PC = Cast<APlayerController>(Player->GetController()))
        {
            if (PC->PlayerCameraManager) { PC->PlayerCameraManager->ViewPitchMin = PreviousPitchMin; PC->PlayerCameraManager->ViewPitchMax = PreviousPitchMax; }
            PC->SetControlRotation(bReturnToEntry ? EntryView : FRotator(-10, AimYaw, 0));
        }
        auto* Move = Player->GetCharacterMovement();
        Move->StopMovementImmediately(); Move->ClearAccumulatedForces(); Player->ConsumeMovementInputVector();
        if (bReturnToEntry) Player->TeleportTo(EntryLocation, Player->GetActorRotation());
        Move->SetMovementMode(MOVE_Falling);
    }
    Target.Reset();
}
void UPlayseedLaunchComponent::Fire()
{
    if (!bAiming || !Target.IsValid()) return;
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player) return;
    LastLaunchVelocity = FRotator(AimPitch, AimYaw, 0).Vector() * 2600;
    CancelAim(false);
    LastLaunchOrigin = Player->GetActorLocation();
    ++LaunchCount;
    // 座標で浮島へ移さず、初速だけを与えます。以後の重力・壁・着地は通常の物理移動です。
    Player->LaunchCharacter(LastLaunchVelocity, true, true);
    Player->GetFlight()->RequestFlightAfterLaunch();
    UE_LOG(LogTemp, Display, TEXT("SKYBLOOM: actual launch %d from %s with velocity %s"), LaunchCount, *LastLaunchOrigin.ToString(), *LastLaunchVelocity.ToString());
}
void UPlayseedLaunchComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bAiming) return;
    if (!Target.IsValid()) { CancelAim(); return; }
    if (auto* Player = Cast<AWindseedCharacter>(GetOwner()))
    {
        Target->SetAimPose(AimPitch, AimYaw);
        Player->SetActorRotation(FRotator(0, AimYaw, 0));
        if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FRotator(AimPitch, AimYaw, 0));
    }
}
void UPlayseedLaunchComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Worldを出るときは旧Worldの座席から移動し直さず、カメラと状態だけを戻します。
    CancelAim(false);
    Super::EndPlay(EndPlayReason);
}
