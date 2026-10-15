#include "PlayseedClimbComponent.h"
#include "PlayseedClimbTree.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedActionComponent.h"
#include "PlayseedVitalityComponent.h"
#include "PlayseedCombatComponent.h"
#include "PlayseedRideComponent.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"

UPlayseedClimbComponent::UPlayseedClimbComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UPlayseedClimbComponent::SetupInput(UInputComponent* Input)
{
    Input->BindKey(EKeys::F, IE_Pressed, this, &UPlayseedClimbComponent::ToggleClimb);
    Input->BindKey(EKeys::Gamepad_FaceButton_Top, IE_Pressed, this, &UPlayseedClimbComponent::ToggleClimb);
}

void UPlayseedClimbComponent::ToggleClimb()
{
    if (bClimbing) { CancelClimb(); return; }
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player || Player->GetVitality()->IsInvulnerable() || Player->GetActions()->IsGroundPounding()) return;
    APlayseedClimbTree* Closest = nullptr;
    double Distance = 170;
    const float HalfHeight = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    for (TActorIterator<APlayseedClimbTree> It(GetWorld()); It; ++It)
    {
        const double Candidate = FVector::Dist2D(Player->GetActorLocation(), It->GetActorLocation());
        const float Feet = Player->GetActorLocation().Z - HalfHeight;
        if (Candidate < Distance && Feet >= It->GetActorLocation().Z - 10 && Feet <= It->GetTopZ() + 20) { Closest = *It; Distance = Candidate; }
    }
    if (!Closest) return;
    Player->GetRide()->CancelRide();
    Player->GetCarry()->DropHeld();
    Player->GetCombat()->CancelMelee();
    Tree = Closest;
    Outward = (Player->GetActorLocation() - Tree->GetActorLocation()).GetSafeNormal2D();
    if (Outward.IsNearlyZero()) Outward = FVector::ForwardVector;
    bClimbing = true;
    ClimbInput = 0;
    Player->StopJumping();
    Player->GetCharacterMovement()->StopMovementImmediately();
    Player->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    // この木だけを登っている間の移動衝突から外します。枝の裏へ頭をぶつけて止まらないためです。
    // 他の地形は通常どおり衝突し、木から離れる全経路でこの設定を戻します。
    Player->GetCapsuleComponent()->IgnoreActorWhenMoving(Tree.Get(), true);
    Player->SetClimbPose(true);
}

void UPlayseedClimbComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bClimbing) return;
    if (!Tree.IsValid()) { CancelClimb(); return; }
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player) return;
    const FVector Position = Player->GetActorLocation();
    const float HalfHeight = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const float Bottom = Tree->GetActorLocation().Z + HalfHeight + 4;
    const float Top = Tree->GetTopZ() + HalfHeight + 4;
    if ((ClimbInput > 0 && Position.Z >= Top - 2) || (ClimbInput < 0 && Position.Z <= Bottom + 2)) { CancelClimb(); return; }
    const float Radius = Tree->GetTrunkRadius() + Player->GetCapsuleComponent()->GetScaledCapsuleRadius() + 5;
    const FVector XYError = Tree->GetActorLocation() + Outward * Radius - FVector(Position.X, Position.Y, Tree->GetActorLocation().Z);
    const FVector Correction = XYError * 8;
    // Flyingは登り中だけ重力を止めるために使います。速度と衝突による移動で、座標を置き換えません。
    const float CurrentZ = Position.Z;
    const float NextZ = FMath::Clamp(CurrentZ + ClimbInput * 220 * DeltaTime, Bottom, Top);
    Player->GetCharacterMovement()->Velocity = FVector(Correction.X, Correction.Y, (NextZ - Position.Z) / FMath::Max(DeltaTime, .001f));
}

void UPlayseedClimbComponent::CancelClimb()
{
    if (!bClimbing) return;
    bClimbing = false;
    if (auto* Player = Cast<AWindseedCharacter>(GetOwner()))
    {
        if (Tree.IsValid()) Player->GetCapsuleComponent()->IgnoreActorWhenMoving(Tree.Get(), false);
        Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->SetClimbPose(false);
    }
    Tree.Reset();
    ClimbInput = 0;
}

bool UPlayseedClimbComponent::JumpAway()
{
    if (!bClimbing) return false;
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    const FVector Impulse = Outward * 380 + FVector(0, 0, 580);
    CancelClimb();
    if (Player) Player->LaunchCharacter(Impulse, true, true);
    return true;
}
