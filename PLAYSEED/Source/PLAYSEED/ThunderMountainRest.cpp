#include "ThunderMountainRest.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderMountainRest::AThunderMountainRest()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RestRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Bud = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DewBud")); Bud->SetupAttachment(RootComponent);
    Bud->SetStaticMesh(Mesh.Object); Bud->SetRelativeLocation(FVector(0, 0, 48)); Bud->SetRelativeScale3D(FVector(.65f, .65f, .8f)); Bud->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    for (int32 I = 0; I < 6; ++I)
    {
        auto* Part = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("DewPetal%d"), I)); Part->SetupAttachment(RootComponent);
        const float A = I * PI / 3; Part->SetStaticMesh(Mesh.Object); Part->SetRelativeLocation(FVector(FMath::Cos(A) * 44, FMath::Sin(A) * 44, 12));
        Part->SetRelativeScale3D(FVector(.65f, .35f, .16f)); Part->SetRelativeRotation(FRotator(0, FMath::RadiansToDegrees(A), 0)); Part->SetCollisionEnabled(ECollisionEnabled::NoCollision); Petals.Add(Part);
    }
}
void AThunderMountainRest::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        Tint = UMaterialInstanceDynamic::Create(Palette, this); Bud->SetMaterial(0, Tint);
        auto* Leaf = UMaterialInstanceDynamic::Create(Palette, this); Leaf->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.05f, .4f, .25f));
        for (UStaticMeshComponent* Part : Petals) Part->SetMaterial(0, Leaf);
    }
}
void AThunderMountainRest::ResetRest() { Cooldown = 0; bNeedsLeave = false; }
void AThunderMountainRest::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds); Cooldown = FMath::Max(0.f, Cooldown - DeltaSeconds);
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)); if (!Player) return;
    const float Distance = FVector::Dist2D(Player->GetActorLocation(), GetActorLocation());
    if (Distance > 140) bNeedsLeave = false;
    if (Tint) Tint->SetVectorParameterValue(TEXT("Tint"), IsReady() ? FLinearColor(.25f, 1, .85f) : FLinearColor(.8f, .48f, .12f));
    const float Feet = Player->GetActorLocation().Z - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    // 地上で触れた時だけ2回復。満タンなら何も消費せず、無敵時間や移動モードも変えません。
    if (IsReady() && Distance < 80 && FMath::Abs(Feet - GetActorLocation().Z) < 90 && Player->GetCharacterMovement()->IsMovingOnGround() && Player->GetVitality()->TryHeal(2))
    {
        Cooldown = 5; bNeedsLeave = true; ++SuccessfulHeals;
        UE_LOG(LogTemp, Display, TEXT("MOUNTAIN REST: actual ground contact healed2, health%d, count%d."), Player->GetVitality()->GetHealth(), SuccessfulHeals);
    }
}
