#include "PlayseedGlideBoard.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APlayseedGlideBoard::APlayseedGlideBoard()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GaleleafRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> LeafMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Leaf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Galeleaf"));
    Leaf->SetupAttachment(RootComponent);
    Leaf->SetStaticMesh(LeafMesh.Object);
    Leaf->SetRelativeLocation(FVector(0, 0, 9));
    Leaf->SetRelativeScale3D(FVector(1.5f, .9f, .18f));
    Leaf->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Ridge = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GoldenRidge"));
    Ridge->SetupAttachment(RootComponent);
    Ridge->SetStaticMesh(LeafMesh.Object);
    Ridge->SetRelativeLocation(FVector(0, 0, 14));
    Ridge->SetRelativeScale3D(FVector(1.2f, .12f, .1f));
    Ridge->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayseedGlideBoard::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto Paint = [this, Palette](UStaticMeshComponent* Part, FLinearColor Color)
        {
            auto* Tint = UMaterialInstanceDynamic::Create(Palette, this);
            Tint->SetVectorParameterValue(TEXT("Tint"), Color);
            Part->SetMaterial(0, Tint);
        };
        Paint(Leaf, FLinearColor(.04f, .5f, .43f));
        Paint(Ridge, FLinearColor(1, .72f, .12f));
    }
}
