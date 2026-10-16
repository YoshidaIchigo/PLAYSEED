#include "ReferenceWingCap.h"
#include "WindseedCharacter.h"
#include "PlayseedFlightComponent.h"
#include "ReferenceTravelSubsystem.h"
#include "ThunderExpeditionSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AReferenceWingCap::AReferenceWingCap()
{
    PrimaryActorTick.bCanEverTick=true;
    Trigger=CreateDefaultSubobject<USphereComponent>(TEXT("CapPickup"));RootComponent=Trigger;Trigger->InitSphereRadius(32);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);Trigger->SetCollisionObjectType(ECC_WorldDynamic);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);Trigger->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);Trigger->SetGenerateOverlapEvents(true);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    auto Part=[&](FName Name,FVector Position,FVector Scale,FLinearColor Color)
    {
        auto* Mesh=CreateDefaultSubobject<UStaticMeshComponent>(Name);Mesh->SetupAttachment(RootComponent);Mesh->SetStaticMesh(Sphere.Object);
        Mesh->SetRelativeLocation(Position);Mesh->SetRelativeScale3D(Scale);Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);Parts.Add(Mesh);Colors.Add(Color);return Mesh;
    };
    Part(TEXT("RedCap"),FVector(0,0,3),FVector(.72f,.6f,.32f),FLinearColor(.85f,.015f,.008f));
    Part(TEXT("CapBrim"),FVector(30,0,-7),FVector(.65f,.58f,.06f),FLinearColor(.85f,.015f,.008f));
    for(int32 Side:{-1,1})for(int32 I=0;I<3;++I)
    {
        auto* Feather=Part(*FString::Printf(TEXT("WhiteWing%d_%d"),Side,I),FVector(-8-I*6,Side*(35+I*11),15+I*7),FVector(.16f,.55f,.08f),FLinearColor(.9f,.92f,.95f));
        Feather->SetRelativeRotation(FRotator(0,Side*(20+I*8),Side*15));
    }
}
void AReferenceWingCap::BeginPlay()
{
    Super::BeginPlay();Trigger->OnComponentBeginOverlap.AddDynamic(this,&AReferenceWingCap::HandleOverlap);
    if(auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor")))for(int32 I=0;I<Parts.Num();++I)
    {auto* Material=UMaterialInstanceDynamic::Create(Palette,this);Material->SetVectorParameterValue(TEXT("Tint"),Colors[I]);Parts[I]->SetMaterial(0,Material);}
}
void AReferenceWingCap::TryCollect(AWindseedCharacter* Hero)
{
    if(bCollected||!Hero||!GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->IsFlightUnlocked()||GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>()->IsTransitionLocked())return;
    bCollected=true;++Collections;Trigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);SetActorHiddenInGame(true);
    Hero->GetFlight()->GrantPower();UE_LOG(LogTemp,Display,TEXT("REFERENCE WING CAP: actual capsule overlap granted timed wings once"));
}
void AReferenceWingCap::HandleOverlap(UPrimitiveComponent*,AActor* OtherActor,UPrimitiveComponent*,int32,bool,const FHitResult&)
{TryCollect(Cast<AWindseedCharacter>(OtherActor));}
void AReferenceWingCap::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);if(bCollected)return;Age+=DeltaSeconds;
    SetActorRotation(FRotator(0,Age*55,0));
    if(auto* Hero=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0)))if(Trigger->IsOverlappingActor(Hero))TryCollect(Hero);
}
