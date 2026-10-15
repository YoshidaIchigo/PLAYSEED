#include "ThunderCarryTestSubsystem.h"
#include "PlayseedCarryable.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

namespace
{
void CarryTap(APlayerController* PC, FKey Key)
{
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1));
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0));
}
}

void UThunderCarryTestSubsystem::BeginTest(AWindseedCharacter* Player, APlayseedCarryable* InProp)
{
    Prop = InProp;
    Phase = 0;
    PhaseTime = 0;
    RecoveryBaseline = Player->RecoveryCount;
    PickupBaseline = Prop->GetPickupCount();
    ThrowBaseline = Prop->GetThrowCount();
    LandingBaseline = Prop->GetThrownLandingCount();
    HitBaseline = Player->GetVitality()->GetSuccessfulImpacts();
    Error.Reset();
    bSnapshotSaved = false;
    if (auto* PC = Cast<APlayerController>(Player->GetController())) CarryTap(PC, EKeys::E);
}

bool UThunderCarryTestSubsystem::WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds)
{
    const FVector Offset = Goal - Player->GetActorLocation();
    if (Offset.Size2D() < 25 && Player->GetCharacterMovement()->IsMovingOnGround())
    { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-18, Offset.Rotation().Yaw, 0), DeltaSeconds, 2));
    return false;
}

bool UThunderCarryTestSubsystem::UpdateTest(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (!Error.IsEmpty()) return false;
    if (!Prop.IsValid()) { Error = TEXT("Carry prop is missing"); return false; }
    PhaseTime += DeltaSeconds;
    if (PhaseTime > 20) { Error = FString::Printf(TEXT("Carry phase %d timed out at %s"), Phase, *Player->GetActorLocation().ToString()); return false; }
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (!PC) { Error = TEXT("Carry controller is missing"); return false; }
    auto Advance = [this](int32 Next) { Phase = Next; PhaseTime = 0; };
    auto HeldCorrectly = [&]() { return Prop->IsHeldBy(Player) && Player->GetCarry()->GetHeldObject() == Prop.Get() && !Prop->GetBody()->IsSimulatingPhysics() && FVector::Dist(Prop->GetActorLocation(), Player->GetActorLocation() + Player->GetActorRotation().RotateVector(FVector(100, 0, 30))) < 5; };
    if (Phase == 0 && PhaseTime > .3f)
    {
        if (Prop->IsHeld() || Player->GetCarry()->GetHeldObject()) { Error = TEXT("E picked up an out-of-range prop"); return false; }
        Advance(1);
    }
    else if (Phase == 1 || Phase == 7 || Phase == 11)
    {
        if (WalkToward(Player, FVector(-580, -1100, 82), DeltaSeconds))
        {
            CarryTap(PC, Phase == 7 ? EKeys::Gamepad_FaceButton_Left : EKeys::E);
            Advance(Phase == 1 ? 2 : (Phase == 7 ? 8 : 12));
        }
    }
    else if (Phase == 2 && PhaseTime > .2f)
    {
        if (!HeldCorrectly()) { Error = TEXT("E did not attach the prop while disabling its physics"); return false; }
        Advance(3);
    }
    else if (Phase == 3)
    {
        if (!HeldCorrectly()) { Error = TEXT("Prop did not follow ordinary movement"); return false; }
        if (WalkToward(Player, FVector(-150, -900, 82), DeltaSeconds)) Advance(4);
    }
    else if (Phase == 4 && PhaseTime > .5f)
    {
        FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderCarrying.png"), false, false);
        ThrowStart = Prop->GetActorLocation();
        CarryTap(PC, EKeys::E);
        Advance(5);
    }
    else if (Phase == 5 && PhaseTime > .3f)
    {
        if (Prop->IsHeld() || Player->GetCarry()->GetHeldObject() || Prop->GetAttachParentActor() || !Prop->GetBody()->IsSimulatingPhysics() || Prop->GetThrowCount() != ThrowBaseline + 1)
        {
            Error = FString::Printf(TEXT("Throw state: held=%d component=%d attached=%d physics=%d count=%d distance=%.1f velocity=%s"), Prop->IsHeld(), Player->GetCarry()->GetHeldObject() != nullptr, Prop->GetAttachParentActor() != nullptr, Prop->GetBody()->IsSimulatingPhysics(), Prop->GetThrowCount(), FVector::Dist2D(Prop->GetActorLocation(), ThrowStart), *Prop->GetBody()->GetPhysicsLinearVelocity().ToString());
            return false;
        }
        if (Prop->GetThrownLandingCount() == LandingBaseline + 1)
        {
            // Screenshotの読出しで長いフレームが発生しても、物理時間は上限で短く刻まれます。
            // 壁時計の0.3秒だけで飛距離を判断せず、実際に飛んで着地した結果を確かめます。
            if (FVector::Dist2D(Prop->GetActorLocation(), ThrowStart) < 200) { Error = TEXT("Physics throw landed without travelling 200 cm"); return false; }
            UE_LOG(LogTemp, Display, TEXT("CARRY TEST: real pickup, transport and physics throw landed once after %.1f cm."), FVector::Dist2D(Prop->GetActorLocation(), ThrowStart));
            CarryTap(PC, EKeys::R);
            Advance(6);
        }
    }
    else if (Phase == 6 && PhaseTime > .4f)
    {
        if (Prop->IsHeld() || FVector::Dist2D(Prop->GetActorLocation(), FVector(-450, -1100, 60)) > 5 || Player->RecoveryCount != RecoveryBaseline + 1)
        { Error = TEXT("R did not reset the thrown object"); return false; }
        Advance(7);
    }
    else if (Phase == 8 && PhaseTime > .2f)
    {
        if (!HeldCorrectly()) { Error = TEXT("Gamepad X did not pick up the prop"); return false; }
        Advance(9);
    }
    else if (Phase == 9)
    {
        if (Player->GetVitality()->GetSuccessfulImpacts() == HitBaseline + 1)
        {
            if (Prop->IsHeld() || Player->GetCarry()->GetHeldObject()) { Error = TEXT("Real damage did not drop the carried prop"); return false; }
            CarryTap(PC, EKeys::R);
            Advance(10);
        }
        else WalkToward(Player, FVector(0, -1200, 82), DeltaSeconds);
    }
    else if (Phase == 10 && PhaseTime > .4f)
    {
        if (Prop->IsHeld() || Player->RecoveryCount != RecoveryBaseline + 2) { Error = TEXT("R after damage did not reset carry state"); return false; }
        Advance(11);
    }
    else if (Phase == 12 && PhaseTime > .2f)
    {
        if (!HeldCorrectly()) { Error = TEXT("Third pickup failed"); return false; }
        // 場外の座標置換は復帰時の保持解除だけを確認します。持ち上げ・運搬・投擲には使いません。
        Player->SetActorLocation(FVector(0, -1200, -900), false, nullptr, ETeleportType::TeleportPhysics);
        Advance(13);
    }
    else if (Phase == 13 && PhaseTime > .4f)
    {
        if (Prop->IsHeld() || Player->GetCarry()->GetHeldObject() || Prop->IsInThrownFlight() || !Prop->GetBody()->IsSimulatingPhysics() || FVector::Dist2D(Prop->GetActorLocation(), FVector(-450, -1100, 60)) > 5 || Player->RecoveryCount != RecoveryBaseline + 3 || Prop->GetPickupCount() != PickupBaseline + 3)
        { Error = TEXT("Fall did not release and reset the held object safely"); return false; }
        Advance(14);
    }
    else if (Phase == 14)
    {
        if (PC) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-20, -80, 0), DeltaSeconds, 3));
        if (PhaseTime > 1 && !bSnapshotSaved)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Validation/ThunderCarryRecovery.png"), false, false);
            bSnapshotSaved = true;
            UE_LOG(LogTemp, Display, TEXT("CARRY TEST PASS: range rejection, E/X pickup, normal transport, physics throw/landing, R reset, real damage drop and fall release passed."));
        }
        return PhaseTime > 2;
    }
    return false;
}
