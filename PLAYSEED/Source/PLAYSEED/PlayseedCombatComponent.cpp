#include "PlayseedCombatComponent.h"
#include "PlayseedStrikeTarget.h"
#include "PlayseedActionComponent.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/InputComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"

UPlayseedCombatComponent::UPlayseedCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UPlayseedCombatComponent::SetupInput(UInputComponent* Input)
{
    Input->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &UPlayseedCombatComponent::BeginMelee);
    Input->BindKey(EKeys::Gamepad_RightTrigger, IE_Pressed, this, &UPlayseedCombatComponent::BeginMelee);
}

void UPlayseedCombatComponent::BeginMelee()
{
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player || Cooldown > 0 || !Player->GetCharacterMovement()->IsMovingOnGround() || Player->GetVitality()->IsInvulnerable() || Player->GetClimb()->IsClimbing() || Player->GetActions()->IsGroundPounding() || Player->GetCarry()->GetHeldObject()) return;
    ComboStage = SinceLastAttack <= .75f ? ComboStage % 3 + 1 : 1;
    SinceLastAttack = 0;
    AttackAge = 0;
    Cooldown = .26f;
    bAttacking = true;
    HitTargets.Reset();
    ++AttackSerial;
}

void UPlayseedCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    SinceLastAttack += DeltaTime;
    Cooldown = FMath::Max(0.f, Cooldown - DeltaTime);
    if (!bAttacking) return;
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player) return;
    const float PreviousAge = AttackAge;
    AttackAge += DeltaTime;
    // 長いフレームで命中時間を飛び越しても一度は検査します。相手ごとの重複は別に防ぎます。
    if (PreviousAge <= .16f && AttackAge >= .04f) CheckTargets();
    Player->SetMeleePose(ComboStage, FMath::Clamp(AttackAge / .22f, 0.f, 1.f));
    if (AttackAge >= .22f) { bAttacking = false; Player->SetMeleePose(0, 0); }
}

void UPlayseedCombatComponent::CheckTargets()
{
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player) return;
    const float Reach = ComboStage == 3 ? 200 : 170;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        if (*It == Player || HitTargets.Contains(TWeakObjectPtr<AActor>(*It))) continue;
        auto* Target = Cast<IPlayseedStrikeTarget>(*It);
        if (!Target) continue;
        const FVector Offset = It->GetActorLocation() - Player->GetActorLocation();
        const double Distance = Offset.Size2D();
        if (Distance > Reach || FMath::Abs(Offset.Z) > 120 || (Distance > 30 && FVector::DotProduct(Offset.GetSafeNormal2D(), Player->GetActorForwardVector()) < .35)) continue;
        // 受け手が攻撃を拒否しても、この一振りでは同じ相手へ何度も依頼しません。
        HitTargets.Add(*It);
        if (Target->ReceiveStrike(Player, ComboStage == 3 ? 2 : 1)) ++SuccessfulHits;
    }
}

void UPlayseedCombatComponent::CancelMelee()
{
    bAttacking = false;
    ComboStage = 0;
    Cooldown = 0;
    SinceLastAttack = 2;
    HitTargets.Reset();
    if (auto* Player = Cast<AWindseedCharacter>(GetOwner())) Player->SetMeleePose(0, 0);
}
