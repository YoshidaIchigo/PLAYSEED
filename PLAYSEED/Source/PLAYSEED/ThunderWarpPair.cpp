#include "ThunderWarpPair.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AThunderWarpPair::AThunderWarpPair()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("WarpPairRoot"));
}

void AThunderWarpPair::Configure(FVector LowerFloor, FVector UpperFloor, float LowerExitYaw, float UpperExitYaw)
{
    Floors[0] = LowerFloor;
    Floors[1] = UpperFloor;
    ExitYaws[0] = LowerExitYaw;
    ExitYaws[1] = UpperExitYaw;
    bConfigured = true;
    ResetWarps();
}

void AThunderWarpPair::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bConfigured) return;
    Cooldown = FMath::Max(0.f, Cooldown - DeltaSeconds);
    auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!Player) return;
    const FVector Position = Player->GetActorLocation();
    if (bNeedsExit)
    {
        // 時間切れだけでは再作動しません。到着先で立ち止まっていても戻されない条件です。
        if (FVector::Dist2D(Position, Floors[0]) > 210 && FVector::Dist2D(Position, Floors[1]) > 210) bNeedsExit = false;
        return;
    }
    if (Cooldown > 0 || !Player->GetCharacterMovement()->IsMovingOnGround()) return;
    const float FeetZ = Position.Z - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    for (int32 Index = 0; Index < 2; ++Index)
    {
        if (FVector::Dist2D(Position, Floors[Index]) < 75 && FMath::Abs(FeetZ - Floors[Index].Z) < 20)
        { WarpPlayer(Player, 1 - Index); break; }
    }
}

void AThunderWarpPair::WarpPlayer(ACharacter* Player, int32 DestinationIndex)
{
    auto* Movement = Player->GetCharacterMovement();
    Player->StopJumping();
    Movement->StopMovementImmediately();
    Player->SetBase(static_cast<FMovementBaseInterfaceData*>(nullptr));
    const FVector Destination = Floors[DestinationIndex] + FVector(0, 0, Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 4);
    const FRotator Facing(0, ExitYaws[DestinationIndex], 0);
    // これは仕掛けとしての瞬間移動です。TeleportTo は到着先の衝突を確認し、配置できなければ失敗します。
    // 無条件の座標置換で壁へ埋め込まず、成功したときだけ作動回数と出口待ちを更新します。
    Cooldown = 1;
    bFailedWarp = !Player->TeleportTo(Destination, Facing);
    if (bFailedWarp) return;
    Movement->SetMovementMode(MOVE_Walking);
    if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FRotator(-15, Facing.Yaw, 0));
    ++WarpCount;
    bNeedsExit = true;
}

void AThunderWarpPair::ResetWarps()
{
    WarpCount = 0;
    Cooldown = 0;
    bNeedsExit = false;
    bFailedWarp = false;
}
