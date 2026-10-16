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
    return FParse::Param(FCommandLine::Get(),TEXT("ReferenceCourse")) || FParse::Param(FCommandLine::Get(),TEXT("ThunderReferenceTest")) || FParse::Param(FCommandLine::Get(),TEXT("ThunderDropTest")) || FParse::Param(FCommandLine::Get(),TEXT("ThunderChainTest")) || FParse::Param(FCommandLine::Get(),TEXT("ThunderStarTest")) || FParse::Param(FCommandLine::Get(),TEXT("ThunderFadeTest"));
}

void AReferenceBattlefieldTerrain::BeginPlay()
{
    Super::BeginPlay();
    using namespace ReferenceBattlefieldData;
    TArray<FVector> Vertices,Normals;TArray<int32> Indices;TArray<FVector2D> UV;
    TArray<FLinearColor> Colors;TArray<FProcMeshTangent> Tangents;
    const TCHAR* Names[]={TEXT("Grass"),TEXT("GrassEdge"),TEXT("Dirt"),TEXT("Stone"),TEXT("Wood"),TEXT("Cave"),TEXT("Fence"),TEXT("Rock"),TEXT("Shadow"),TEXT("WireFence")};
    for(int32 Section=0;Section<UE_ARRAY_COUNT(Sections);++Section)
    {
        const auto& Data=Sections[Section];Vertices.Reset();Normals.Reset();Indices.Reset();UV.Reset();Colors.Reset();Tangents.Reset();
        for(int32 I=0;I<Data.Count;++I)
        {
            const auto& V=RenderVertices[Data.First+I];const FVector Normal=FVector(V.NX,V.NY,V.NZ).GetSafeNormal();Vertices.Add(FVector(V.X,V.Y,V.Z));UV.Add(FVector2D(V.U,V.V));Normals.Add(Normal);Indices.Add(I);
            // N64風の面の明暗を頂点色に固定し、UEの自動露出で草が白くなるのを避けます。
            const float Light=Data.Material==6 || Data.Material==8 || Data.Material==9 ? 1.f : .48f+.52f*FMath::Max(0.f,FVector::DotProduct(Normal,FVector(-.4,-.3,.86).GetSafeNormal()));
            Colors.Add(FLinearColor(Light,Light,Light,1));
        }
        Surface->CreateMeshSection_LinearColor(Section,Vertices,Indices,Normals,UV,Colors,Tangents,false);
        const FString Asset=FString::Printf(TEXT("/Game/Battlefield/M_%s.M_%s"),Names[Data.Material],Names[Data.Material]);
        if(auto* Material=LoadObject<UMaterialInterface>(nullptr,*Asset))Surface->SetMaterial(Section,Material);
        else UE_LOG(LogTemp,Error,TEXT("REFERENCE TERRAIN: missing material %s"),*Asset);
    }
    Vertices.Reset();for(const auto& P:CollisionVertices)Vertices.Add(FVector(P.X,P.Y,P.Z));
    Indices.Reset();Indices.Append(CollisionIndices,UE_ARRAY_COUNT(CollisionIndices));
    // 参照資料は反時計回り、UEの衝突面は時計回りです。逆のままだと床で落下状態が続きます。
    for(int32 I=0;I<Indices.Num();I+=3)Swap(Indices[I+1],Indices[I+2]);
    Collision->CreateMeshSection(0,Vertices,Indices,TArray<FVector>(),TArray<FVector2D>(),TArray<FColor>(),Tangents,true);
    Collision->SetMeshSectionVisible(0,false);
    UE_LOG(LogTemp,Display,TEXT("REFERENCE TERRAIN: 1101 render triangles, 1060 collision triangles; start(-6558,-6464), summit4294, island3072."));
    // 17本の木の位置も地形資料と同じ座標系です。形は同梱メッシュから組みます。
    auto* Sphere=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    auto* Cylinder=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor"));
    for(const auto& P:Trees)
    {
        const FVector Position(P.X,P.Y,P.Z);
        for(int32 I=0;I<3;++I)
        {
            auto* Part=NewObject<UStaticMeshComponent>(this);Part->SetupAttachment(RootComponent);
            Part->SetStaticMesh(I==0?Cylinder:Sphere);Part->SetRelativeLocation(Position+FVector(I==2?35:0,0,I==0?220:I==1?520:680));
            Part->SetRelativeScale3D(I==0?FVector(.6,.6,4.4):I==1?FVector(4.5,3.8,5.2):FVector(3.6,3.2,3.5));
            Part->SetCollisionEnabled(I==0?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);Part->SetCollisionResponseToAllChannels(ECR_Block);
            if(Palette){auto* MID=UMaterialInstanceDynamic::Create(Palette,this);MID->SetVectorParameterValue(TEXT("Tint"),I==0?FLinearColor(.32f,.14f,.035f):FLinearColor(.15f,.6f,.015f));Part->SetMaterial(0,MID);}
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
