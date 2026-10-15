#include "PlayseedCarryComponent.h"
#include "PlayseedCarryable.h"
#include "PlayseedActionComponent.h"
#include "PlayseedVitalityComponent.h"
#include "PlayseedCombatComponent.h"
#include "PlayseedRideComponent.h"
#include "WindseedCharacter.h"
#include "Components/InputComponent.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"

UPlayseedCarryComponent::UPlayseedCarryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UPlayseedCarryComponent::SetupInput(UInputComponent* Input)
{
    Input->BindKey(EKeys::E, IE_Pressed, this, &UPlayseedCarryComponent::Interact);
    Input->BindKey(EKeys::Gamepad_FaceButton_Left, IE_Pressed, this, &UPlayseedCarryComponent::Interact);
}

void UPlayseedCarryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (Held.IsValid() && !Held->IsHeldBy(Cast<AWindseedCharacter>(GetOwner()))) Held.Reset();
}

void UPlayseedCarryComponent::Interact()
{
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player || Player->GetRide()->IsRiding() || Player->GetVitality()->IsInvulnerable() || Player->GetActions()->IsGroundPounding() || Player->GetCombat()->IsAttacking()) return;
    if (Held.IsValid() && Held->IsHeldBy(Player)) { Held->Throw(); Held.Reset(); return; }
    Held.Reset();
    APlayseedCarryable* Closest = nullptr;
    double BestDistance = 170;
    // 探索対象は専用Actorだけです。無関係な地形や他ステージの収集物を持ち上げません。
    for (TActorIterator<APlayseedCarryable> It(GetWorld()); It; ++It)
    {
        const double Distance = FVector::Dist(Player->GetActorLocation(), It->GetActorLocation());
        if (Distance < BestDistance && It->CanBePickedUp(Player)) { Closest = *It; BestDistance = Distance; }
    }
    if (Closest && Closest->TryPickup(Player)) Held = Closest;
}

void UPlayseedCarryComponent::DropHeld()
{
    if (Held.IsValid() && Held->IsHeldBy(Cast<AWindseedCharacter>(GetOwner()))) Held->Drop();
    Held.Reset();
}
