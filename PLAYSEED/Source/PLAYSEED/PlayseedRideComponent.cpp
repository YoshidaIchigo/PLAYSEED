#include "PlayseedRideComponent.h"
#include "PlayseedGlideBoard.h"
#include "PlayseedActionComponent.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedCombatComponent.h"
#include "PlayseedVitalityComponent.h"
#include "PlayseedLaunchComponent.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"

UPlayseedRideComponent::UPlayseedRideComponent() { PrimaryComponentTick.bCanEverTick = true; }

void UPlayseedRideComponent::SetupInput(UInputComponent* Input)
{
    Input->BindKey(EKeys::Q, IE_Pressed, this, &UPlayseedRideComponent::ToggleRide);
    Input->BindKey(EKeys::Gamepad_DPad_Up, IE_Pressed, this, &UPlayseedRideComponent::ToggleRide);
}

void UPlayseedRideComponent::ToggleRide()
{
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player) return;
    if (Player->GetLaunch()->IsAiming()) return;
    auto* Movement = Player->GetCharacterMovement();
    // 空中のQでは降りません。被弾や急降下などの中断はCancelRideを直接呼びます。
    if (!Movement->IsMovingOnGround()) return;
    if (bRiding) { CancelRide(); return; }
    if (Player->GetVitality()->IsInvulnerable() || Player->GetCarry()->GetHeldObject() || Player->GetClimb()->IsClimbing() || Player->GetActions()->IsGroundPounding() || Player->GetCombat()->IsAttacking()) return;
    APlayseedGlideBoard* Closest = nullptr;
    double BestDistance = 180;
    for (TActorIterator<APlayseedGlideBoard> It(GetWorld()); It; ++It)
    {
        const double Distance = FVector::Dist2D(Player->GetActorLocation(), It->GetActorLocation());
        if (!It->GetAttachParentActor() && Distance < BestDistance && FMath::Abs(Player->GetActorLocation().Z - It->GetActorLocation().Z) < 150)
        { Closest = *It; BestDistance = Distance; }
    }
    if (!Closest) return;
    PreviousSpeed = Movement->MaxWalkSpeed;
    PreviousFriction = Movement->GroundFriction;
    PreviousBraking = Movement->BrakingDecelerationWalking;
    PreviousAcceleration = Movement->MaxAcceleration;
    PreviousAirControl = Movement->AirControl;
    Board = Closest;
    bRiding = true;
    Movement->MaxWalkSpeed = 900;
    Movement->GroundFriction = .4f;
    Movement->BrakingDecelerationWalking = 120;
    Movement->MaxAcceleration = 850;
    Movement->AirControl = .35f;
    // 見た目の葉だけを足元へ付けます。移動ベースやCapsuleの寸法は変えません。
    Closest->AttachToComponent(Player->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    Closest->SetActorRelativeLocation(FVector(0, 0, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
}

void UPlayseedRideComponent::CancelRide()
{
    if (!bRiding) return;
    bRiding = false;
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (Player)
    {
        auto* Movement = Player->GetCharacterMovement();
        Movement->MaxWalkSpeed = PreviousSpeed;
        Movement->GroundFriction = PreviousFriction;
        Movement->BrakingDecelerationWalking = PreviousBraking;
        Movement->MaxAcceleration = PreviousAcceleration;
        Movement->AirControl = PreviousAirControl;
        Player->SetSprinting(Player->WantsToSprint());
        if (Board.IsValid())
        {
            Board->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
            const FVector Candidate = Player->GetActorLocation() + Player->GetActorRightVector() * 130;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(Player);
            Params.AddIgnoredActor(Board.Get());
            FHitResult Floor;
            // 空中で中断しても葉は実際の床へ戻します。床のない場外ではWorldから除きます。
            if (GetWorld()->LineTraceSingleByChannel(Floor, Candidate + FVector(0, 0, 100), Candidate - FVector(0, 0, 3000), ECC_Visibility, Params) && Floor.ImpactNormal.Z > .6f)
            {
                Board->SetActorLocation(Floor.ImpactPoint);
                Board->SetActorRotation(FRotator(0, Player->GetActorRotation().Yaw, 0));
            }
            else Board->Destroy();
        }
    }
    Board.Reset();
}

void UPlayseedRideComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (bRiding && !Board.IsValid()) CancelRide();
}

void UPlayseedRideComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CancelRide();
    Super::EndPlay(EndPlayReason);
}
