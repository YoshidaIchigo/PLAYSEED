#include "PlayseedClimbTree.h"
#include "ReferenceBattlefieldTerrain.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APlayseedClimbTree::APlayseedClimbTree()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TreeRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    bReferenceTree=AReferenceBattlefieldTerrain::IsReferenceCourse();
    PrimaryActorTick.bCanEverTick=bReferenceTree;
    if(bReferenceTree)
    {
        // 元の17本の位置を使い、枝の足場は追加しません。Fで幹につかまって登れます。
        AddPart(TEXT("Trunk"),Cylinder.Object,FVector(0,0,325),FVector(.6f,.6f,6.5f),FLinearColor(.32f,.14f,.035f),true);
        AddPart(TEXT("Leaves"),Sphere.Object,FVector(0,0,520),FVector(4.5f,.1f,5.2f),FLinearColor(.15f,.6f,.015f),false);
        AddPart(TEXT("Tip"),Sphere.Object,FVector(35,0,680),FVector(3.6f,.1f,3.5f),FLinearColor(.15f,.6f,.015f),false);
        return;
    }
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
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, bReferenceTree?TEXT("/Game/Battlefield/M_Actor.M_Actor"):TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        for (int32 Index = 0; Index < Parts.Num(); ++Index)
        {
            auto* Material = UMaterialInstanceDynamic::Create(Palette, this);
            Material->SetVectorParameterValue(TEXT("Tint"), Colors[Index]);
            Parts[Index]->SetMaterial(0, Material);
        }
    }
}

void APlayseedClimbTree::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);if(!bReferenceTree)return;
    if(auto* Camera=UGameplayStatics::GetPlayerCameraManager(this,0))
    {
        // 薄い葉をカメラへ向けます。幹の手前を登る人を大きな立体の葉で包みません。
        const float Yaw=(Camera->GetCameraLocation()-GetActorLocation()).Rotation().Yaw-90;
        for(int32 I=1;I<Parts.Num();++I)Parts[I]->SetRelativeRotation(FRotator(0,Yaw,0));
    }
}
