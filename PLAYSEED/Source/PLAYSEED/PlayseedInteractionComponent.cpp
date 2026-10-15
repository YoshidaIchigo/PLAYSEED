#include "PlayseedInteractionComponent.h"
#include "PlayseedInteractionTarget.h"
#include "PlayseedRideComponent.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedCombatComponent.h"
#include "PlayseedActionComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"

UPlayseedInteractionComponent::UPlayseedInteractionComponent() { PrimaryComponentTick.bCanEverTick = true; }
void UPlayseedInteractionComponent::SetupInput(UInputComponent* Input)
{
    Input->BindKey(EKeys::G, IE_Pressed, this, &UPlayseedInteractionComponent::InteractNearest);
    Input->BindKey(EKeys::Gamepad_DPad_Down, IE_Pressed, this, &UPlayseedInteractionComponent::InteractNearest);
}
void UPlayseedInteractionComponent::InteractNearest()
{
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player || Cooldown > 0 || Player->GetRide()->IsRiding() || Player->GetClimb()->IsClimbing() || Player->GetCombat()->IsAttacking() || Player->GetActions()->IsGroundPounding() || Player->GetVitality()->IsInvulnerable()) return;
    AActor* Closest = nullptr;
    double BestDistance = 240;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        if (!Cast<IPlayseedInteractionTarget>(*It)) continue;
        const double Distance = FVector::Dist(Player->GetActorLocation(), It->GetActorLocation());
        if (Distance >= BestDistance) continue;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(Player); Params.AddIgnoredActor(*It);
        FHitResult Wall;
        // 壁越しには話しかけません。専用の装飾メッシュは衝突を持ちません。
        if (GetWorld()->LineTraceSingleByChannel(Wall, Player->GetActorLocation(), It->GetActorLocation() + FVector(0, 0, 60), ECC_Visibility, Params)) continue;
        Closest = *It; BestDistance = Distance;
    }
    if (Closest) { Cooldown = .25f; Cast<IPlayseedInteractionTarget>(Closest)->Interact(Player); }
}
void UPlayseedInteractionComponent::ShowMessage(const FString& Text) { Message = Text; MessageRemaining = 9; }
void UPlayseedInteractionComponent::ClearMessage() { Message.Reset(); MessageRemaining = 0; }
void UPlayseedInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    Cooldown = FMath::Max(0.f, Cooldown - DeltaTime);
    MessageRemaining -= DeltaTime;
    if (MessageRemaining <= 0) Message.Reset();
}
