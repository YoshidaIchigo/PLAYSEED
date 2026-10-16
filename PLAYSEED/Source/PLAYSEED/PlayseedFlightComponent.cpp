#include "PlayseedFlightComponent.h"
#include "PlayseedLaunchComponent.h"
#include "PlayseedRideComponent.h"
#include "PlayseedClimbComponent.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedActionComponent.h"
#include "PlayseedCombatComponent.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UPlayseedFlightComponent::UPlayseedFlightComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    // 発射のPendingLaunchが移動へ反映された後にFlyingへ移ります。
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
}
void UPlayseedFlightComponent::BeginPlay()
{
    Super::BeginPlay();
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    auto* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette"));
    if (!Player || !Mesh || !Palette) return;
    for (int32 Side : {-1, 1})
    {
        auto* Wing = NewObject<UStaticMeshComponent>(Player);
        Wing->SetupAttachment(Player->GetRootComponent()); Wing->SetStaticMesh(Mesh);
        Wing->SetRelativeLocation(FVector(0, Side * 75, 10)); Wing->SetRelativeScale3D(FVector(.38f, 1.6f, .16f));
        Wing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        auto* Tint = UMaterialInstanceDynamic::Create(Palette, Player);
        Tint->SetVectorParameterValue(TEXT("Tint"), Side < 0 ? FLinearColor(.03f, .58f, .45f) : FLinearColor(.95f, .75f, .18f));
        Wing->SetMaterial(0, Tint); Wing->RegisterComponent(); Wings.Add(Wing);
    }
    RefreshWings();
}
void UPlayseedFlightComponent::RefreshWings()
{
    for (int32 Index = 0; Index < Wings.Num(); ++Index)
    {
        const float Side = Index ? 1 : -1;
        // 人型では帽子の羽を描くため、旧式の背中の羽は表示しません。
        Wings[Index]->SetVisibility(HasPower() && !GetOwner()->ActorHasTag(TEXT("ReferenceHero")));
        Wings[Index]->SetRelativeRotation(FRotator(0, Side * 20, bFlying ? Side * FMath::Sin(GetWorld()->GetTimeSeconds() * 9) * 18 : Side * 45));
    }
}
void UPlayseedFlightComponent::GrantPower()
{
    Remaining = 45; ++PowerGrants; RefreshWings();
}
void UPlayseedFlightComponent::RequestFlightAfterLaunch() { bAwaitingLaunch = HasPower(); }
bool UPlayseedFlightComponent::TryBeginFlight()
{
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player || !HasPower() || bFlying || !Player->GetCharacterMovement()->IsFalling() || Player->GetLaunch()->IsAiming() || Player->GetRide()->IsRiding() || Player->GetClimb()->IsClimbing() || Player->GetCarry()->GetHeldObject() || Player->GetActions()->IsGroundPounding() || Player->GetVitality()->IsInvulnerable()) return false;
    Player->GetCombat()->CancelMelee(); Player->StopJumping(); Player->ConsumeMovementInputVector();
    auto* Move = Player->GetCharacterMovement();
    PreviousFlySpeed = Move->MaxFlySpeed; PreviousAcceleration = Move->MaxAcceleration; PreviousBraking = Move->BrakingDecelerationFlying;
    const FRotator Heading = Player->GetVelocity().IsNearlyZero() ? Player->GetActorRotation() : Player->GetVelocity().Rotation();
    FlightYaw = Heading.Yaw; FlightPitch = FMath::Clamp(Heading.Pitch, -60.f, 70.f);
    bFlying = true; bAwaitingLaunch = false; SpeedInput = TurnInput = 0; ++FlightStarts;
    Move->MaxFlySpeed = 1400; Move->MaxAcceleration = 1800; Move->BrakingDecelerationFlying = 0;
    Move->SetMovementMode(MOVE_Flying);
    auto* Boom = Player->GetCameraBoom();
    PreviousArm = Boom->TargetArmLength; PreviousSocket = Boom->SocketOffset; bPreviousCameraLag = Boom->bEnableCameraLag;
    Boom->TargetArmLength = 650; Boom->SocketOffset = FVector(0, 0, 100); Boom->bEnableCameraLag = false;
    return true;
}
void UPlayseedFlightComponent::CancelFlight()
{
    bAwaitingLaunch = false;
    if (!bFlying) return;
    bFlying = false; SpeedInput = TurnInput = 0;
    if (auto* Player = Cast<AWindseedCharacter>(GetOwner()))
    {
        auto* Move = Player->GetCharacterMovement();
        Move->MaxFlySpeed = PreviousFlySpeed; Move->MaxAcceleration = PreviousAcceleration; Move->BrakingDecelerationFlying = PreviousBraking;
        if (Move->MovementMode == MOVE_Flying) Move->SetMovementMode(MOVE_Falling);
        Player->ConsumeMovementInputVector();
        auto* Boom = Player->GetCameraBoom();
        Boom->TargetArmLength = PreviousArm; Boom->SocketOffset = PreviousSocket; Boom->bEnableCameraLag = bPreviousCameraLag;
        if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FRotator(-18, FlightYaw, 0));
    }
}
void UPlayseedFlightComponent::ResetPower() { CancelFlight(); Remaining = 0; RefreshWings(); }
void UPlayseedFlightComponent::AdjustDirection(float YawDelta, float PitchDelta)
{
    if (!bFlying) return;
    FlightYaw = FRotator::NormalizeAxis(FlightYaw + YawDelta);
    FlightPitch = FMath::Clamp(FlightPitch + PitchDelta, -60.f, 70.f);
}
bool UPlayseedFlightComponent::IsTouchingFloor() const
{
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player || Player->GetVelocity().Z > 5) return false;
    const FVector Feet = Player->GetActorLocation() - FVector(0, 0, Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    FCollisionQueryParams Params; Params.AddIgnoredActor(Player);
    FHitResult Floor;
    return GetWorld()->LineTraceSingleByChannel(Floor, Feet + FVector(0, 0, 8), Feet - FVector(0, 0, 14), ECC_Visibility, Params)
        && Floor.ImpactNormal.Z > .65f && FMath::Abs(Feet.Z - Floor.ImpactPoint.Z) < 6;
}
void UPlayseedFlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (Remaining > 0)
    {
        Remaining = FMath::Max(0.f, Remaining - DeltaTime);
        if (Remaining == 0) { ++Expirations; CancelFlight(); }
    }
    auto* Player = Cast<AWindseedCharacter>(GetOwner());
    if (!Player) return;
    auto* Move = Player->GetCharacterMovement();
    if (bAwaitingLaunch && Move->IsFalling() && Player->GetVelocity().Size() > 1000) TryBeginFlight();
    if (bFlying)
    {
        if (IsTouchingFloor()) { ++FlightLandings; CancelFlight(); }
        else
        {
            FlightYaw = FRotator::NormalizeAxis(FlightYaw + TurnInput * 100 * DeltaTime);
            const float Speed = FMath::Clamp(1000 + SpeedInput * 400, 600.f, 1400.f);
            Move->Velocity = FMath::VInterpTo(Move->Velocity, FRotator(FlightPitch, FlightYaw, 0).Vector() * Speed, DeltaTime, 3);
            Player->SetActorRotation(FRotator(0, FlightYaw, 0));
            // 飛行の傾きとカメラの上下を分け、上昇中もキャラクターを画面内へ保ちます。
            if (auto* PC = Cast<APlayerController>(Player->GetController())) PC->SetControlRotation(FRotator(FMath::Clamp(FlightPitch * .25f - 15, -30.f, 0.f), FlightYaw, 0));
        }
    }
    RefreshWings();
}
void UPlayseedFlightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) { CancelFlight(); Super::EndPlay(EndPlayReason); }
