#include "ThunderFlightBloom.h"
#include "PlayseedFlightComponent.h"
#include "ThunderExpeditionSubsystem.h"
#include "Engine/GameInstance.h"
#include "WindseedCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderFlightBloom::AThunderFlightBloom()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("FlightBloomRoot"));
    Bud = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingBud")); Bud->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
    Bud->SetStaticMesh(Mesh.Object); Bud->SetRelativeLocation(FVector(0, 0, 80)); Bud->SetRelativeScale3D(FVector(.8f, 1.3f, .9f));
    Bud->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AThunderFlightBloom::BeginPlay()
{
    Super::BeginPlay();
    ResetBloom();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto* Tint = UMaterialInstanceDynamic::Create(Palette, this); Tint->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.1f, .8f, .6f)); Bud->SetMaterial(0, Tint);
    }
}
void AThunderFlightBloom::ResetBloom()
{
    bCollected = false; RespawnRemaining = 0;
    bUnlocked = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->IsFlightUnlocked();
    SetActorHiddenInGame(!bUnlocked);
}
void AThunderFlightBloom::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    bUnlocked = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->IsFlightUnlocked();
    if (!bUnlocked) { SetActorHiddenInGame(true); return; }
    if (!bCollected) SetActorHiddenInGame(false);
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player) return;
    const float Distance = FVector::Dist2D(Player->GetActorLocation(), GetActorLocation());
    if (bCollected)
    {
        RespawnRemaining -= DeltaSeconds;
        // 真上に居続けても自動で取り直さず、いったん離れてから再出現します。
        if (RespawnRemaining <= 0 && Distance > 120) ResetBloom();
        return;
    }
    Bud->AddLocalRotation(FRotator(0, DeltaSeconds * 55, 0));
    if (Distance < 80 && FMath::Abs(Player->GetActorLocation().Z - 82 - GetActorLocation().Z) < 100)
    {
        bCollected = true; RespawnRemaining = 8; ++Collections; SetActorHiddenInGame(true);
        Player->GetFlight()->GrantPower();
    }
}
