#include "PlayseedClimbTree.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APlayseedClimbTree::APlayseedClimbTree()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TreeRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    AddPart(TEXT("Trunk"), Cylinder.Object, FVector(0, 0, 325), FVector(1.1f, 1.1f, 6.5f), FLinearColor(.32f, .2f, .11f), true);
    AddPart(TEXT("CrownLanding"), Cylinder.Object, FVector(0, 0, 640), FVector(3.6f, 3.6f, .2f), FLinearColor(.18f, .4f, .24f), true);
    AddPart(TEXT("HighLeaves"), Sphere.Object, FVector(0, 0, 980), FVector(5, 4.5f, 3), FLinearColor(.08f, .32f, .18f), false);
    AddPart(TEXT("LeafTip"), Sphere.Object, FVector(40, 0, 1120), FVector(3.2f, 3.2f, 2.2f), FLinearColor(.16f, .45f, .23f), false);
}

void APlayseedClimbTree::AddPart(FName Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color, bool bSolid)
{
    auto* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
    Part->SetupAttachment(RootComponent);
    Part->SetStaticMesh(Mesh);
    Part->SetRelativeLocation(Position);
    Part->SetRelativeScale3D(Scale);
    Part->SetCollisionEnabled(bSolid ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    Part->SetCollisionResponseToAllChannels(ECR_Block);
    Parts.Add(Part);
    Colors.Add(Color);
}

void APlayseedClimbTree::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        for (int32 Index = 0; Index < Parts.Num(); ++Index)
        {
            auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
            Material->SetVectorParameterValue(TEXT("Tint"), Colors[Index]);
            Parts[Index]->SetMaterial(0, Material);
        }
    }
}
