#include "ThunderLifeseed.h"
#include "ThunderLivesComponent.h"
#include "ThunderMeadowGameMode.h"
#include "WindseedCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
AThunderLifeseed::AThunderLifeseed()
{
    PrimaryActorTick.bCanEverTick=true;
    Trigger=CreateDefaultSubobject<USphereComponent>(TEXT("LifeseedTrigger")); RootComponent=Trigger;
    Trigger->InitSphereRadius(52); Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly); Trigger->SetCollisionObjectType(ECC_WorldDynamic);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore); Trigger->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap); Trigger->SetGenerateOverlapEvents(true);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Seed=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LifeseedHeart")); Seed->SetupAttachment(Trigger); Seed->SetStaticMesh(Sphere.Object);
    Seed->SetRelativeScale3D(FVector(.65f,.5f,.85f)); Seed->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    for (int32 I=0;I<2;++I)
    {
        auto* Part=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("LifeseedLeaf%d"),I)); Part->SetupAttachment(Seed); Part->SetStaticMesh(Sphere.Object);
        Part->SetRelativeLocation(FVector(I==0 ? -35 : 35,0,45)); Part->SetRelativeScale3D(FVector(.8f,.35f,.18f)); Part->SetRelativeRotation(FRotator(I==0 ? -30 : 30,0,0)); Part->SetCollisionEnabled(ECollisionEnabled::NoCollision); Leaves.Add(Part);
    }
}
void AThunderLifeseed::BeginPlay()
{
    Super::BeginPlay(); Trigger->OnComponentBeginOverlap.AddDynamic(this,&AThunderLifeseed::HandleOverlap);
    if (auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Windseed/M_Palette.M_Palette")))
    {
        auto* Amber=UMaterialInstanceDynamic::Create(Palette,this); Amber->SetVectorParameterValue(TEXT("Tint"),FLinearColor(1,.42f,.18f)); Seed->SetMaterial(0,Amber);
        auto* Teal=UMaterialInstanceDynamic::Create(Palette,this); Teal->SetVectorParameterValue(TEXT("Tint"),FLinearColor(.1f,.85f,.65f)); for(UStaticMeshComponent* Part:Leaves) Part->SetMaterial(0,Teal);
    }
}
void AThunderLifeseed::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds); if(bCollected) return; Age+=DeltaSeconds;
    Seed->SetRelativeRotation(FRotator(0,Age*75,0)); Seed->SetRelativeLocation(FVector(0,0,FMath::Sin(Age*3)*6));
    auto* Player=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0)); if(Player && Trigger->IsOverlappingActor(Player)) TryCollect(Player);
}
void AThunderLifeseed::HandleOverlap(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,int32,bool,const FHitResult&) { TryCollect(Cast<AWindseedCharacter>(Other)); }
void AThunderLifeseed::TryCollect(AWindseedCharacter* Player)
{
    if(bCollected || !Player) return;
    auto* Mode=Cast<AThunderMeadowGameMode>(GetWorld()->GetAuthGameMode()); if(!Mode) return;
    // Overlap通知とTickの両方が来ても、一度の取得だけにします。Rでは復活させません。
    bCollected=true;
    if(!Mode->GetLives()->TryCollectLife()) { bCollected=false; return; }
    ++Collections; SetActorHiddenInGame(true);
}
