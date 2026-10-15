#include "ThunderTraversalTestSubsystem.h"
#include "WindseedCharacter.h"
#include "ThunderMeadowStage.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void UThunderTraversalTestSubsystem::BeginTerraceRoute(AWindseedCharacter* Player)
{
    Route = {FVector(-650, 350, 82), FVector(-1150, 350, 182), FVector(-1550, 700, 182),
        FVector(-1650, 1200, 282), FVector(-1450, 1650, 282), FVector(-900, 1650, 382)};
    bJumpForSteps = true;
    MinimumJumps = 3;
    CaptureName = TEXT("ThunderTerraces");
    bInspectSkyIsland = false;
    ResetProgress(Player);
}

void UThunderTraversalTestSubsystem::BeginMountainRoute(AWindseedCharacter* Player, const TArray<FVector>& FootPath, bool bInspectIsland)
{
    Route = {FVector(0, 1100, 82), FVector(600, 1850, 82)};
    // 地形の順路は足元、Character の座標はカプセルの中心なので高さを加えます。
    const float CapsuleHeight = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    for (const FVector& FootPosition : FootPath) Route.Add(FootPosition + FVector(0, 0, CapsuleHeight));
    // 山頂中央では手前の床が浮島を隠すため、西の縁まで普通に歩いてから見渡します。
    if (bInspectIsland && FootPath.Num() > 1)
        Route.Add(FootPath[FootPath.Num() - 2] + FVector(0, 0, CapsuleHeight));
    bJumpForSteps = false; // 山道はジャンプ入力を使わず、斜面を歩いて登れることを検証します。
    MinimumJumps = 0;
    bInspectSkyIsland = bInspectIsland;
    CaptureName = bInspectIsland ? TEXT("ThunderIsland") : TEXT("ThunderSummit");
    ResetProgress(Player);
}

void UThunderTraversalTestSubsystem::ResetProgress(AWindseedCharacter* Player)
{
    Waypoint = 0;
    JumpCount = 0;
    LegTime = 0;
    FinishTime = 0;
    Error.Reset();
    bScreenshotRequested = false;
    bIslandCollisionVerified = false;
    RecoveryBaseline = Player->RecoveryCount;
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseWheelAxis, IE_Axis, -6));
}

bool UThunderTraversalTestSubsystem::UpdateRoute(AWindseedCharacter* Player, float DeltaSeconds)
{
    if (HasFailed()) return false;
    if (Player->RecoveryCount != RecoveryBaseline)
    { Error = TEXT("Unexpected fall recovery during terrain traversal"); return false; }
    auto* PC = Cast<APlayerController>(Player->GetController());
    if (Waypoint == Route.Num())
    {
        if (bInspectSkyIsland && !bIslandCollisionVerified)
        {
            if (!VerifyIslandCollision(Player)) return false;
            bIslandCollisionVerified = true;
        }
        if (JumpCount < MinimumJumps) { Error = TEXT("The terrain route did not use the required jumps"); return false; }
        FinishTime += DeltaSeconds;
        const float ViewYaw = bInspectSkyIsland || bJumpForSteps ? 180 : 225;
        const float ViewPitch = bInspectSkyIsland ? -22 : -25;
        if (PC) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(ViewPitch, ViewYaw, 0), DeltaSeconds, 3));
        if (FinishTime > 1 && !bScreenshotRequested)
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / (TEXT("Validation/") + CaptureName + TEXT(".png")), false, false);
            bScreenshotRequested = true;
            UE_LOG(LogTemp, Display, TEXT("TRAVERSAL TEST PASS: %s, all %d waypoints, %d ordinary jumps, no recovery."), *CaptureName, Route.Num(), JumpCount);
        }
        return FinishTime > 2;
    }
    LegTime += DeltaSeconds;
    if (LegTime > 15)
    {
        Error = FString::Printf(TEXT("%s waypoint %d timed out at %s"), *CaptureName, Waypoint, *Player->GetActorLocation().ToString());
        return false;
    }
    const FVector Offset = Route[Waypoint] - Player->GetActorLocation();
    auto* Movement = Player->GetCharacterMovement();
    if (Offset.Size2D() < 55 && FMath::Abs(Offset.Z) < 65 && Movement->IsMovingOnGround())
    {
        ++Waypoint;
        LegTime = 0;
        Player->StopJumping();
        Movement->StopMovementImmediately();
        UE_LOG(LogTemp, Display, TEXT("TRAVERSAL TEST: %s reached waypoint %d."), *CaptureName, Waypoint);
        return false;
    }
    Player->AddMovementInput(Offset.GetSafeNormal2D());
    if (bJumpForSteps && Offset.Z > 50 && Offset.Size2D() < 480 && Movement->IsMovingOnGround())
    { Player->Jump(); ++JumpCount; }
    else if (Player->GetVelocity().Z < 0) Player->StopJumping();
    if (PC) PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), FRotator(-23, Offset.Rotation().Yaw, 0), DeltaSeconds, 2));
    return false;
}

bool UThunderTraversalTestSubsystem::VerifyIslandCollision(AWindseedCharacter* Player)
{
    auto* Meadow = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(Player, AThunderMeadowStage::StaticClass()));
    if (!Meadow) { Error = TEXT("Sky island stage is missing"); return false; }
    const FVector Top = Meadow->GetSkyIslandTop();
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Player);
    const auto* Capsule = Player->GetCapsuleComponent();
    // プレイヤーを移動させず、同じ寸法のカプセルを物理問い合わせで下へ掃引します。
    // これは床の衝突検証です。発射台からの実際の着地は、その機能のPRで確認します。
    const bool bHit = Player->GetWorld()->SweepSingleByChannel(Hit, Top + FVector(0, 0, 400), Top - FVector(0, 0, 100), FQuat::Identity,
        ECC_Visibility, FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight()), Params);
    if (!bHit || Hit.GetActor() != Meadow || FMath::Abs(Hit.ImpactPoint.Z - Top.Z) > 5 || Hit.ImpactNormal.Z < .95f)
    { Error = TEXT("The sky island has no flat blocking landing surface"); return false; }
    UE_LOG(LogTemp, Display, TEXT("ISLAND COLLISION TEST PASS: top %s, capsule floor impact %s."), *Top.ToString(), *Hit.ImpactPoint.ToString());
    return true;
}
