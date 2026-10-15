#include "PlayseedActionComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"

UPlayseedActionComponent::UPlayseedActionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UPlayseedActionComponent::SetupInput(UInputComponent* Input)
{
    Input->BindKey(EKeys::LeftControl, IE_Pressed, this, &UPlayseedActionComponent::BeginGroundPound);
    Input->BindKey(EKeys::Gamepad_RightShoulder, IE_Pressed, this, &UPlayseedActionComponent::BeginGroundPound);
}

void UPlayseedActionComponent::BeginGroundPound()
{
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player || bGroundPounding || Player->GetVitality()->IsInvulnerable()) return;
    auto* Movement = Player->GetCharacterMovement();
    if (!Movement->IsFalling()) return; // 地上では開始しません。ジャンプ中か落下中だけ使えます。
    PreviousGravity = Movement->GravityScale;
    PreviousAirControl = Movement->AirControl;
    bGroundPounding = true;
    Player->StopJumping();
    Movement->Velocity = FVector(0, 0, -1300);
    Movement->GravityScale = 3;
    Movement->AirControl = 0;
    Player->SetPoundPose(true);
}

void UPlayseedActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bGroundPounding) return;
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player) return;
    auto* Movement = Player->GetCharacterMovement();
    if (!Movement->IsFalling()) { CancelGroundPound(); return; }
    // 急降下中は横へ滑らせません。座標の置換ではなくCharacterMovementの落下速度を変えます。
    Movement->Velocity.X = 0;
    Movement->Velocity.Y = 0;
    Movement->Velocity.Z = FMath::Min(Movement->Velocity.Z, -1300.0);
}

void UPlayseedActionComponent::CancelGroundPound()
{
    if (!bGroundPounding) return;
    bGroundPounding = false;
    if (auto* Player = Cast<AWindseedCharacter>(GetOwner()))
    {
        auto* Movement = Player->GetCharacterMovement();
        // 保存した値へ戻し、R・被弾・死亡後の通常ジャンプへ急降下設定を残しません。
        Movement->GravityScale = PreviousGravity;
        Movement->AirControl = PreviousAirControl;
        Player->SetPoundPose(false);
    }
}

void UPlayseedActionComponent::HandleLanded(const FHitResult& Hit)
{
    if (!bGroundPounding) return;
    CancelGroundPound();
    ++CompletedPounds;
    // 着地コールバック一回につき一度だけ通知し、次のTickで重複発生させません。
    OnPoundImpact.Broadcast(Hit.ImpactPoint);
}
