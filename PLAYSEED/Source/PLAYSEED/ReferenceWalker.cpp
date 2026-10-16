#include "ReferenceWalker.h"
#include "ReferenceTravelSubsystem.h"
#include "Engine/GameInstance.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AReferenceWalker::AReferenceWalker()
{
    WalkerVisual=CreateDefaultSubobject<USceneComponent>(TEXT("ReferenceWalkerVisual"));WalkerVisual->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    auto Part=[&](UStaticMesh* Mesh,FVector Position,FVector Size,FLinearColor Color,FRotator Rotation=FRotator::ZeroRotator)
    {
        auto* P=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("ReferenceWalkerPart%d"),WalkerParts.Num()));P->SetupAttachment(WalkerVisual);P->SetStaticMesh(Mesh);P->SetRelativeLocation(Position);P->SetRelativeScale3D(Size/100);P->SetRelativeRotation(Rotation);
        P->SetCollisionEnabled(ECollisionEnabled::NoCollision);P->SetCastShadow(false);WalkerParts.Add(P);WalkerColors.Add(Color);return P;
    };
    const FLinearColor Brown(.36f,.12f,.025f),Cream(.72f,.48f,.24f),Dark(.07f,.022f,.008f),White(.95f,.92f,.82f),Black(.003f,.004f,.006f);
    Part(Cylinder.Object,FVector(0,0,2),FVector(160,160,2),FLinearColor(.009f,.01f,.006f));
    Part(Sphere.Object,FVector(0,0,32),FVector(75,75,45),Cream);
    Part(Sphere.Object,FVector(0,0,55),FVector(180,155,60),Brown);
    for(int32 Side:{-1,1})
    {
        Feet.Add(Part(Sphere.Object,FVector(-15,Side*45,13),FVector(90,62,26),Dark));
        Part(Sphere.Object,FVector(65,Side*29,65),FVector(20,34,30),White);
        Part(Sphere.Object,FVector(75,Side*27,64),FVector(9,14,21),Black);
        Part(Cube.Object,FVector(74,Side*30,78),FVector(7,39,6),Dark,FRotator(0,0,Side*-18));
    }
}
void AReferenceWalker::HideOriginalParts()
{
    TArray<UStaticMeshComponent*> All;GetComponents<UStaticMeshComponent>(All);
    for(UStaticMeshComponent* Part:All)if(!WalkerParts.Contains(Part))Part->SetHiddenInGame(true);
}
void AReferenceWalker::BeginPlay()
{
    Super::BeginPlay();HideOriginalParts();
    if(auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor")))
        for(int32 I=0;I<WalkerParts.Num();++I){auto* Tint=UMaterialInstanceDynamic::Create(Palette,this);Tint->SetVectorParameterValue(TEXT("Tint"),WalkerColors[I]);WalkerParts[I]->SetMaterial(0,Tint);}
}
void AReferenceWalker::ResetEnemy()
{
    Super::ResetEnemy();WalkerVisual->SetRelativeScale3D(FVector::OneVector);HideOriginalParts();
}
void AReferenceWalker::Tick(float DeltaSeconds)
{
    if(auto* Flow=GetGameInstance()?GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>():nullptr)
        if(Flow->IsCelebrating()||Flow->IsFading())
        {
            // 取得と白フェード中は追跡・接触を止め、近い敵がポーズへ重ならないようにします。
            auto* Hero=UGameplayStatics::GetPlayerPawn(this,0);
            const float Distance=Hero?FVector::Dist(Hero->GetActorLocation(),GetActorLocation()):5000;
            SetActorHiddenInGame(Distance<700||Distance>4000);return;
        }
    Super::Tick(DeltaSeconds);
    if(IsDefeated()){WalkerVisual->SetRelativeScale3D(FVector(1.2f,1.2f,.22f));return;}
    if(auto* Hero=UGameplayStatics::GetPlayerPawn(this,0))SetActorHiddenInGame(FVector::Dist(Hero->GetActorLocation(),GetActorLocation())>4000);
    WalkClock+=DeltaSeconds*(IsChasing()?14:3);
    for(int32 I=0;I<Feet.Num();++I){const float Step=FMath::Sin(WalkClock+I*PI);Feet[I]->SetRelativeLocation(FVector(-15+Step*(IsChasing()?12:2),I==0?-45:45,13+FMath::Max(0.f,Step)*(IsChasing()?8:1)));}
}
