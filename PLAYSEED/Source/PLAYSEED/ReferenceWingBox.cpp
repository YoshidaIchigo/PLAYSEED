#include "ReferenceWingBox.h"
#include "ReferenceWingCap.h"
#include "ThunderExpeditionSubsystem.h"
#include "PlayseedActionComponent.h"
#include "WindseedCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AReferenceWingBox::AReferenceWingBox()
{
    PrimaryActorTick.bCanEverTick=true;RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("WingBoxRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Body=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RedBox"));Body->SetupAttachment(RootComponent);Body->SetStaticMesh(Cube.Object);Body->SetRelativeLocation(FVector(0,0,50));
    Body->SetCollisionResponseToAllChannels(ECR_Block);Body->SetNotifyRigidBodyCollision(true);
    auto Add=[&](FName Name,FVector Position,FVector Scale)
    {auto* Part=CreateDefaultSubobject<UStaticMeshComponent>(Name);Part->SetupAttachment(RootComponent);Part->SetStaticMesh(Cube.Object);Part->SetRelativeLocation(Position);Part->SetRelativeScale3D(Scale);Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);return Part;};
    for(int32 Side=0;Side<4;++Side)for(int32 Piece=0;Piece<2;++Piece)
    {
        const FVector Direction=FRotator(0,Side*90,0).Vector();
        Markers.Add(Add(*FString::Printf(TEXT("Mark%d_%d"),Side,Piece),Direction*51+FVector(0,0,Piece==0?65:28),Side%2==0?FVector(.03f,.14f,Piece==0?.4f:.14f):FVector(.14f,.03f,Piece==0?.4f:.14f)));
    }
    for(int32 Axis=0;Axis<3;++Axis)for(int32 A:{-1,1})for(int32 B:{-1,1})
    {
        FVector Position(0,0,50),Scale(.025f,.025f,.025f);Scale[Axis]=1.025;
        Position[(Axis+1)%3]+=A*50;Position[(Axis+2)%3]+=B*50;
        Outline.Add(Add(*FString::Printf(TEXT("Edge%d_%d_%d"),Axis,A,B),Position,Scale));
    }
}
void AReferenceWingBox::BeginPlay()
{
    Super::BeginPlay();Body->OnComponentHit.AddDynamic(this,&AReferenceWingBox::HandleHeadBump);
    if(auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor")))
    {
        auto* Red=UMaterialInstanceDynamic::Create(Palette,this);Red->SetVectorParameterValue(TEXT("Tint"),FLinearColor(.85f,.015f,.008f));Body->SetMaterial(0,Red);
        for(UStaticMeshComponent* Edge:Outline)Edge->SetMaterial(0,Red);
        auto* White=UMaterialInstanceDynamic::Create(Palette,this);White->SetVectorParameterValue(TEXT("Tint"),FLinearColor::White);for(UStaticMeshComponent* Marker:Markers)Marker->SetMaterial(0,White);
    }
    Refresh();
}
void AReferenceWingBox::Refresh()
{
    bUnlocked=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->IsFlightUnlocked();
    Body->SetHiddenInGame(!bUnlocked||bBroken);Body->SetCollisionEnabled(bUnlocked&&!bBroken?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);
    for(UStaticMeshComponent* Marker:Markers)Marker->SetHiddenInGame(!bUnlocked||bBroken);
    for(UStaticMeshComponent* Edge:Outline)Edge->SetHiddenInGame(bUnlocked||bBroken);
}
bool AReferenceWingBox::ReceiveStrike(AWindseedCharacter* Hero,int32 Damage)
{
    if(!bUnlocked||bBroken||!Hero||Damage<=0)return false;
    bBroken=true;RespawnRemaining=301.f/30.f;Refresh();
    Cap=GetWorld()->SpawnActor<AReferenceWingCap>(GetActorLocation()+FVector(0,0,100),FRotator::ZeroRotator);
    if(Cap.IsValid())++SpawnCount;
    UE_LOG(LogTemp,Display,TEXT("REFERENCE WING BOX: normal impact spawned cap%d"),SpawnCount);return true;
}
void AReferenceWingBox::HandleHeadBump(UPrimitiveComponent*,AActor* OtherActor,UPrimitiveComponent*,FVector,const FHitResult& Hit)
{
    auto* Hero=Cast<AWindseedCharacter>(OtherActor);
    if(!bUnlocked||bBroken||!Hero||!Hero->GetCharacterMovement()->IsFalling()||Hero->GetActorLocation().Z>=GetActorLocation().Z-25||Hero->GetVelocity().Z<-.1f||FMath::Abs(Hit.ImpactPoint.Z-GetActorLocation().Z)>18)return;
    ++HeadBumps;ReceiveStrike(Hero,1);
}
void AReferenceWingBox::HandlePound(FVector Impact)
{
    const FVector Local=GetActorTransform().InverseTransformPosition(Impact);
    if(!bUnlocked||bBroken||FMath::Abs(Local.X)>65||FMath::Abs(Local.Y)>65||FMath::Abs(Local.Z-100)>15)return;
    ReceiveStrike(Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0)),1);
}
void AReferenceWingBox::ResetBox()
{
    if(Cap.IsValid())Cap->Destroy();Cap.Reset();bBroken=false;RespawnRemaining=0;Refresh();
}
void AReferenceWingBox::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if(!Actions.IsValid())if(auto* Hero=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0)))
    {Actions=Hero->GetActions();Actions->OnPoundImpact.AddUObject(this,&AReferenceWingBox::HandlePound);}
    if(bUnlocked!=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->IsFlightUnlocked())Refresh();
    if(bBroken){RespawnRemaining-=DeltaSeconds;if(RespawnRemaining<=0)ResetBox();}
}
void AReferenceWingBox::EndPlay(const EEndPlayReason::Type Reason)
{if(Actions.IsValid())Actions->OnPoundImpact.RemoveAll(this);Super::EndPlay(Reason);}
