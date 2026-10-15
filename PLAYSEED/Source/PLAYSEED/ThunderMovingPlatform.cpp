#include "ThunderMovingPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderMovingPlatform::AThunderMovingPlatform()
{
    PrimaryActorTick.bCanEverTick = true;
    Surface = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MovingSurface"));
    RootComponent = Surface;
    Surface->SetMobility(EComponentMobility::Movable);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Surface->SetStaticMesh(Cube.Object);
    Surface->SetRelativeScale3D(FVector(4.5f, 4.5f, .6f));
    Surface->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Surface->SetCollisionResponseToAllChannels(ECR_Block);
    Surface->CanCharacterStepUpOn = ECB_Yes;
}

void AThunderMovingPlatform::BeginPlay()
{
    Super::BeginPlay();
    Origin = GetActorLocation();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
        Material->SetVectorParameterValue(TEXT("Tint"), FLinearColor(.85f, .45f, .1f));
        Surface->SetMaterial(0, Material);
    }
}

void AThunderMovingPlatform::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    MotionTime += DeltaSeconds;
    // 秒で進むためフレームレートに依存しません。左右へ200cmずつ、端で滑らかに折り返します。
    SetActorLocation(Origin + FVector(FMath::Sin(MotionTime * .7f) * 200, 0, 0), false, nullptr, ETeleportType::None);
}

void AThunderMovingPlatform::ResetMotion()
{
    MotionTime = 0;
    SetActorLocation(Origin, false, nullptr, ETeleportType::None);
}
