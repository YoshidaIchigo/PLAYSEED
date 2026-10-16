#include "ReferenceBattlefieldTerrain.h"
#include "ReferenceBattlefieldData.h"
#include "ProceduralMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

AReferenceBattlefieldTerrain::AReferenceBattlefieldTerrain()
{
    Surface=CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ReferenceSurface"));
    RootComponent=Surface;
    Surface->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Surface->SetCastShadow(false);
    Collision=CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ReferenceWalkableGeometry"));
    Collision->SetupAttachment(RootComponent);
    Collision->bUseComplexAsSimpleCollision=true;
    Collision->bUseAsyncCooking=false;
    Collision->SetCollisionObjectType(ECC_WorldStatic);
    Collision->SetCollisionResponseToAllChannels(ECR_Block);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

bool AReferenceBattlefieldTerrain::IsReferenceCourse()
{
    return FParse::Param(FCommandLine::Get(),TEXT("ReferenceCourse")) || FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceTest"));
}

void AReferenceBattlefieldTerrain::BeginPlay()
{
    Super::BeginPlay();
    using namespace ReferenceBattlefieldData;
    TArray<FVector> Vertices,Normals;TArray<int32> Indices;TArray<FVector2D> UV;
    TArray<FLinearColor> Colors;TArray<FProcMeshTangent> Tangents;
    const TCHAR* Names[]={TEXT("Grass"),TEXT("GrassEdge"),TEXT("Dirt"),TEXT("Stone"),TEXT("Wood"),TEXT("Cave"),TEXT("Fence"),TEXT("Rock"),TEXT("Shadow")};
    for(int32 Section=0;Section<UE_ARRAY_COUNT(Sections);++Section)
    {
        const auto& Data=Sections[Section];Vertices.Reset();Normals.Reset();Indices.Reset();UV.Reset();Colors.Reset();Tangents.Reset();
        for(int32 I=0;I<Data.Count;++I)
        {
            const auto& V=RenderVertices[Data.First+I];Vertices.Add(V.Position);UV.Add(V.UV);Normals.Add(V.Normal.GetSafeNormal());Indices.Add(I);
            // N64風の面の明暗を頂点色に固定し、UEの自動露出で草が白くなるのを避けます。
            const float Light=Data.Material==6 || Data.Material==8 ? 1.f : .48f+.52f*FMath::Max(0.f,FVector::DotProduct(V.Normal.GetSafeNormal(),FVector(-.4,-.3,.86).GetSafeNormal()));
            Colors.Add(FLinearColor(Light,Light,Light,1));
        }
        Surface->CreateMeshSection_LinearColor(Section,Vertices,Indices,Normals,UV,Colors,Tangents,false);
        const FString Asset=FString::Printf(TEXT("/Game/Battlefield/M_%s.M_%s"),Names[Data.Material],Names[Data.Material]);
        if(auto* Material=LoadObject<UMaterialInterface>(nullptr,*Asset))Surface->SetMaterial(Section,Material);
        else UE_LOG(LogTemp,Error,TEXT("REFERENCE TERRAIN: missing material %s"),*Asset);
    }
    Vertices.Reset();Vertices.Append(CollisionVertices,UE_ARRAY_COUNT(CollisionVertices));
    Indices.Reset();Indices.Append(CollisionIndices,UE_ARRAY_COUNT(CollisionIndices));
    Collision->CreateMeshSection(0,Vertices,Indices,TArray<FVector>(),TArray<FVector2D>(),TArray<FColor>(),Tangents,true);
    Collision->SetMeshSectionVisible(0,false);
    UE_LOG(LogTemp,Display,TEXT("REFERENCE TERRAIN: 1101 render triangles, 1060 collision triangles; start(-6558,-6464), summit4294, island3072."));
    // 17本の木の位置も地形資料と同じ座標系です。形は同梱メッシュから組みます。
    auto* Sphere=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    auto* Cylinder=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor"));
    for(const FVector& Position:Trees)
    {
        for(int32 I=0;I<3;++I)
        {
            auto* Part=NewObject<UStaticMeshComponent>(this);Part->SetupAttachment(RootComponent);
            Part->SetStaticMesh(I==0?Cylinder:Sphere);Part->SetRelativeLocation(Position+FVector(I==2?35:0,0,I==0?220:I==1?520:680));
            Part->SetRelativeScale3D(I==0?FVector(.6,.6,4.4):I==1?FVector(4.5,3.8,5.2):FVector(3.6,3.2,3.5));
            Part->SetCollisionEnabled(I==0?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);Part->SetCollisionResponseToAllChannels(ECR_Block);
            if(Palette){auto* MID=UMaterialInstanceDynamic::Create(Palette,this);MID->SetVectorParameterValue(TEXT("Tint"),I==0?FLinearColor(.32,.14,.035):FLinearColor(.15,.6,.015));Part->SetMaterial(0,MID);}
            Part->RegisterComponent();
        }
    }
}

bool AReferenceBattlefieldTerrain::FindFloor(FVector Near,FVector& Floor) const
{
    FHitResult Hit;
    if(Collision->LineTraceComponent(Hit,Near+FVector(0,0,180),Near-FVector(0,0,650),FCollisionQueryParams(SCENE_QUERY_STAT(ReferenceFloor),true)))
    {Floor=Hit.ImpactPoint;return Hit.ImpactNormal.Z>.65;}
    return false;
}
