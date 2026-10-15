#include "ThunderDamageThistle.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderDamageThistle::AThunderDamageThistle()
{
    PrimaryActorTick.bCanEverTick = true;
    Thorn = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SparkingThistle"));
    RootComponent = Thorn;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
    Thorn->SetStaticMesh(Cone.Object);
    Thorn->SetRelativeScale3D(FVector(1.6f, 1.6f, 1.2f));
    // 見た目でキャラクターを引っ掛けず、接触範囲の判定とノックバックで危険を表します。
    Thorn->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AThunderDamageThistle::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
        Material->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.9f, .15f, .27f));
        Thorn->SetMaterial(0, Material);
    }
}

void AThunderDamageThistle::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player) return;
    const FVector Offset = Player->GetActorLocation() - GetActorLocation();
    const float FeetZ = Player->GetActorLocation().Z - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    // 高く跳び越えた場合は当たりません。接触が続いても、体力Componentの無敵時間が連続被弾を防ぎます。
    if (Offset.Size2D() < 120 && FeetZ > GetActorLocation().Z - 70 && FeetZ < GetActorLocation().Z + 70)
        Player->GetVitality()->TryTakeImpact(2, Offset);
}
