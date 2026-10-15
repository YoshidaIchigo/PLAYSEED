#include "PlayseedCarryable.h"
#include "WindseedCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APlayseedCarryable::APlayseedCarryable()
{
    PrimaryActorTick.bCanEverTick = true;
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarryBody"));
    RootComponent = Body;
    Body->SetMobility(EComponentMobility::Movable);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Body->SetStaticMesh(Cube.Object);
    Body->SetRelativeScale3D(FVector(.8f));
    Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Body->SetCollisionResponseToAllChannels(ECR_Block);
    Body->SetNotifyRigidBodyCollision(true);
    Body->BodyInstance.bUseCCD = true; // 速い投擲でも薄い床を抜けにくくします。
    Body->SetLinearDamping(.1f);
}

void APlayseedCarryable::BeginPlay()
{
    Super::BeginPlay();
    SpawnTransform = GetActorTransform();
    Body->SetSimulatePhysics(true);
    Body->SetMassOverrideInKg(NAME_None, 35, true);
    Body->OnComponentHit.AddDynamic(this, &APlayseedCarryable::HandleBodyHit);
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
        Material->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.1f, .6f, .62f));
        Body->SetMaterial(0, Material);
    }
}

void APlayseedCarryable::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bHeld && !Holder.IsValid()) Drop();
    if (PawnIgnoreTime > 0)
    {
        PawnIgnoreTime -= DeltaSeconds;
        if (PawnIgnoreTime <= 0 && !bHeld) Body->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    }
}

bool APlayseedCarryable::CanBePickedUp(AWindseedCharacter* Player) const
{
    return Player && !bHeld && !bThrown && Player->GetCharacterMovement()->IsMovingOnGround() && FVector::Dist(Player->GetActorLocation(), GetActorLocation()) <= 170;
}

bool APlayseedCarryable::TryPickup(AWindseedCharacter* Player)
{
    if (!CanBePickedUp(Player)) return false;
    Body->SetSimulatePhysics(false);
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // Attachするのは持たれる物だけです。プレイヤー自身の移動は従来のCharacterMovementのままです。
    if (!AttachToComponent(Player->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform)) { ResetToSpawn(); return false; }
    SetActorRelativeLocation(FVector(100, 0, 30));
    SetActorRelativeRotation(FRotator::ZeroRotator);
    Holder = Player;
    bHeld = true;
    ++PickupCount;
    return true;
}

void APlayseedCarryable::Release(FVector Velocity, bool bWasThrown)
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Holder.Reset();
    bHeld = false;
    bThrown = bWasThrown;
    Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    // 放した直後の自分への衝突を避け、少し離れてから通常のPawn衝突へ戻します。
    Body->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    PawnIgnoreTime = .35f;
    Body->SetSimulatePhysics(true);
    Body->SetPhysicsLinearVelocity(Velocity);
    Body->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
}

void APlayseedCarryable::Throw()
{
    auto* Player = Holder.Get();
    if (!bHeld || !Player) return;
    const FVector Velocity = Player->GetActorForwardVector() * 1100 + FVector(0, 0, 450);
    ++ThrowCount;
    Release(Velocity, true);
}

void APlayseedCarryable::Drop()
{
    if (!bHeld) return;
    const FVector Forward = Holder.IsValid() ? Holder->GetActorForwardVector() : FVector::ForwardVector;
    Release(Forward * 180 + FVector(0, 0, 50), false);
}

void APlayseedCarryable::HandleBodyHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!bThrown || Hit.ImpactNormal.Z < .55f) return;
    bThrown = false;
    ++ThrownLandingCount;
    OnThrownLanding(Hit);
}

void APlayseedCarryable::ResetToSpawn()
{
    Body->SetSimulatePhysics(false);
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Holder.Reset();
    bHeld = false;
    bThrown = false;
    PawnIgnoreTime = 0;
    SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
    Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Body->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    Body->SetSimulatePhysics(true);
    Body->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Body->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
}
