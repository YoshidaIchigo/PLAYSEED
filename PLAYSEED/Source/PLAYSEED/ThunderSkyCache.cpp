#include "ThunderSkyCache.h"
#include "PlayseedLantern.h"
#include "Engine/World.h"
#include "ReferenceBattlefieldTerrain.h"
#include "WindseedCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderSkyCache::AThunderSkyCache()
{
    bReferenceBox=AReferenceBattlefieldTerrain::IsReferenceCourse();
    if(!bReferenceBox)return;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    for(int32 Side=0;Side<4;++Side)
        for(int32 Piece=0;Piece<2;++Piece)
        {
            auto* Marker=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Exclamation%d_%d"),Side,Piece));
            Marker->SetupAttachment(RootComponent);Marker->SetStaticMesh(Cube.Object);Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            const FVector Direction=FRotator(0,Side*90,0).Vector();
            Marker->SetRelativeLocation(Direction*51+FVector(0,0,Piece==0?65:28));
            Marker->SetRelativeScale3D(Side%2==0?FVector(.03f,.14f,Piece==0?.4f:.14f):FVector(.14f,.03f,Piece==0?.4f:.14f));
            Markers.Add(Marker);
        }
}
void AThunderSkyCache::BeginPlay()
{
    Super::BeginPlay();if(!bReferenceBox)return;
    if(auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor")))
    {
        auto* Yellow=UMaterialInstanceDynamic::Create(Palette,this);Yellow->SetVectorParameterValue(TEXT("Tint"),FLinearColor(1,.65f,.025f));GetBoxBody()->SetMaterial(0,Yellow);
        for(UStaticMeshComponent* Marker:Markers)
        {auto* White=UMaterialInstanceDynamic::Create(Palette,this);White->SetVectorParameterValue(TEXT("Tint"),FLinearColor::White);Marker->SetMaterial(0,White);}
    }
    for(UStaticMeshComponent* Band:GetBoxBands())Band->SetHiddenInGame(true);
    GetBoxBody()->SetNotifyRigidBodyCollision(true);
    GetBoxBody()->OnComponentHit.AddDynamic(this,&AThunderSkyCache::HandleReferenceHeadBump);
}
void AThunderSkyCache::HandleReferenceHeadBump(UPrimitiveComponent*,AActor* OtherActor,UPrimitiveComponent*,FVector,const FHitResult& Hit)
{
    auto* Player=Cast<AWindseedCharacter>(OtherActor);
    // 浮いた箱の底へ実際の上昇ジャンプで触れた時だけ壊します。検証から直接壊しません。
    if(!bReferenceBox||IsBroken()||!Player||!Player->GetCharacterMovement()->IsFalling()
        ||Player->GetActorLocation().Z>=GetActorLocation().Z-25||Player->GetVelocity().Z<-.1f
        ||FMath::Abs(Hit.ImpactPoint.Z-GetActorLocation().Z)>18)return;
    ++HeadBumpCount;ReceiveStrike(Player,1);
    UE_LOG(LogTemp,Display,TEXT("REFERENCE BOX: actual upward head bump broke star3 box once"));
}


void AThunderSkyCache::SpawnContents(AWindseedCharacter*)
{
    // 基底側で破壊済みを先に立てているため、重複パンチでも一度だけ呼ばれます。
    // コイン用のSuperは呼ばず、World所有のランタン3を一個だけ出します。
    Reward = GetWorld()->SpawnActor<APlayseedLantern>(GetActorLocation() + FVector(0, 0, 100), FRotator::ZeroRotator);
    if (Reward.IsValid()) { Reward->ConfigureReward(3); ++RewardGenerationCount; }
}
void AThunderSkyCache::ResetBox()
{
    if (Reward.IsValid()) Reward->Destroy(); Reward.Reset(); Super::ResetBox();
}
