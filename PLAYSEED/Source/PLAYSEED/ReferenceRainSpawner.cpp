#include "ReferenceRainSpawner.h"
#include "ReferenceBattlefieldTerrain.h"
#include "WindseedCharacter.h"
#include "PlayseedVitalityComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AReferenceRainSpawner::AReferenceRainSpawner()
{
    PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickGroup=TG_PostPhysics;
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("RainSource"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    Drop=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterDrop"));Drop->SetupAttachment(RootComponent);Drop->SetStaticMesh(Sphere.Object);Drop->SetRelativeScale3D(FVector(1.6f));
    Glint=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterHighlight"));Glint->SetupAttachment(Drop);Glint->SetStaticMesh(Sphere.Object);Glint->SetRelativeLocation(FVector(27,-20,29));Glint->SetRelativeScale3D(FVector(.16f,.22f,.1f));
    Shadow=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterShadow"));Shadow->SetupAttachment(RootComponent);Shadow->SetStaticMesh(Cylinder.Object);Shadow->SetRelativeScale3D(FVector(1.6f,1.6f,.02f));
    for(int32 I=0;I<5;++I)
    {
        auto* Bubble=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("SplashBubble%d"),I));Bubble->SetupAttachment(RootComponent);Bubble->SetStaticMesh(Sphere.Object);Bubble->SetRelativeScale3D(FVector(.3f));Bubbles.Add(Bubble);
    }
    for(UStaticMeshComponent* Part:{Drop.Get(),Glint.Get(),Shadow.Get()}){Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetCastShadow(false);}
    for(UStaticMeshComponent* Part:Bubbles){Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetCastShadow(false);}
}
void AReferenceRainSpawner::BeginPlay()
{
    Super::BeginPlay();
    if(auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor")))
    {
        auto Paint=[&](UStaticMeshComponent* Part,FLinearColor Color){auto* Tint=UMaterialInstanceDynamic::Create(Palette,this);Tint->SetVectorParameterValue(TEXT("Tint"),Color);Part->SetMaterial(0,Tint);};
        Paint(Drop,FLinearColor(.02f,.18f,.65f));Paint(Glint,FLinearColor(.65f,.8f,1));Paint(Shadow,FLinearColor(.002f,.003f,.004f));
        for(UStaticMeshComponent* Bubble:Bubbles)Paint(Bubble,FLinearColor(.02f,.25f,.8f));
    }
    ResetRain();
}
void AReferenceRainSpawner::ResetRain()
{
    ++Revision;Phase=EReferenceRainPhase::Idle;PhaseAge=0;WaitRemaining=.7f;Bounces=0;Velocity=FVector::ZeroVector;
    Drop->SetVisibility(false,true);Shadow->SetVisibility(false);
    for(UStaticMeshComponent* Bubble:Bubbles)Bubble->SetVisibility(false);
}
bool AReferenceRainSpawner::StartRain(AWindseedCharacter* Hero)
{
    auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(this,AReferenceBattlefieldTerrain::StaticClass()));
    if(!Terrain)return false;
    const FVector HeroFeet=Hero->GetActorLocation()-FVector(0,0,Hero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    FVector Ahead=HeroFeet+FVector(Hero->GetVelocity().X,Hero->GetVelocity().Y,0)*(.933333f)+Hero->GetActorForwardVector()*100;
    // 実際の速さから少し先を狙います。足場の外なら、今いる実床を使います。
    if(!Terrain->FindFloor(Ahead,Target)&&!Terrain->FindFloor(HeroFeet,Target))return false;
    Position=FVector(Target.X,Target.Y,GetActorLocation().Z+2000);Velocity=FVector(0,0,-1200);Bounces=0;BouncePeak=0;
    Phase=EReferenceRainPhase::Falling;PhaseAge=0;++SpawnCount;
    Drop->SetWorldLocation(Position);Drop->SetRelativeScale3D(FVector(1.6f));Drop->SetVisibility(true,true);
    Shadow->SetWorldLocation(Target+FVector(0,0,2));Shadow->SetVisibility(true);RefreshShadow();
    UE_LOG(LogTemp,Display,TEXT("REFERENCE RAIN: source%s real spawn%d target%s"),*GetActorLocation().ToString(),SpawnCount,*Target.ToString());return true;
}
void AReferenceRainSpawner::RefreshShadow()
{
    // 跳ねて横へ動く間も、実際の水玉の下へ影を投影します。傾いた床にも合わせます。
    FCollisionQueryParams Query(SCENE_QUERY_STAT(ReferenceRainShadow),false,this);
    if(auto* Hero=UGameplayStatics::GetPlayerPawn(this,0))Query.AddIgnoredActor(Hero);
    FHitResult Floor;
    const bool bFloor=GetWorld()->LineTraceSingleByChannel(Floor,Position+FVector(0,0,80),Position-FVector(0,0,12000),ECC_Visibility,Query);
    Shadow->SetVisibility(bFloor);
    if(bFloor){Target=Floor.ImpactPoint;Shadow->SetWorldLocation(Target+FVector(0,0,2));Shadow->SetWorldRotation(FRotationMatrix::MakeFromZ(Floor.ImpactNormal).Rotator());}
    const float Height=FMath::Max(0.f,static_cast<float>(Position.Z-Target.Z));
    const float Size=FMath::Clamp(1.6f-Height/12000,.8f,1.6f);Shadow->SetWorldScale3D(FVector(Size,Size,.02f));
}
void AReferenceRainSpawner::Burst(FVector At)
{
    Phase=EReferenceRainPhase::Splash;PhaseAge=0;Position=At;++BurstCount;
    Drop->SetVisibility(false,true);Shadow->SetVisibility(false);
    for(UStaticMeshComponent* Bubble:Bubbles){Bubble->SetWorldLocation(At);Bubble->SetVisibility(true);}
}
bool AReferenceRainSpawner::CheckHeroContact(FVector Start,FVector End,AWindseedCharacter* Hero)
{
    const auto* Capsule=Hero->GetCapsuleComponent();const float Radius=Capsule->GetScaledCapsuleRadius();
    const float AxisHalf=Capsule->GetScaledCapsuleHalfHeight()-Radius;const FVector Center=Hero->GetActorLocation();
    FVector WaterPoint,HeroPoint;
    FMath::SegmentDistToSegmentSafe(Start,End,Center-FVector(0,0,AxisHalf),Center+FVector(0,0,AxisHalf),WaterPoint,HeroPoint);
    if(FVector::DistSquared(WaterPoint,HeroPoint)>FMath::Square(80+Radius))return false;
    // 状態を先に終え、実被弾から死亡・R初期化へ入っても古い水玉を再表示しません。
    Burst(WaterPoint);
    if(Hero->GetVitality()->TryTakeImpact(1,Center-WaterPoint))++ConfirmedHits;
    UE_LOG(LogTemp,Display,TEXT("REFERENCE RAIN: actual contact source%s hit%d health%d"),*GetActorLocation().ToString(),ConfirmedHits,Hero->GetVitality()->GetHealth());return true;
}
void AReferenceRainSpawner::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Hero=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0));if(!Hero)return;
    const int32 TickRevision=Revision;PhaseAge+=DeltaSeconds;
    if(Phase==EReferenceRainPhase::Idle)
    {
        if(FVector::Dist2D(Hero->GetActorLocation(),GetActorLocation())<Range&&Hero->GetActorLocation().Z-GetActorLocation().Z<1000)
        {WaitRemaining-=DeltaSeconds;if(WaitRemaining<=0&&!StartRain(Hero))WaitRemaining=.5f;}
        return;
    }
    if(Phase==EReferenceRainPhase::Splash)
    {
        for(int32 I=0;I<Bubbles.Num();++I)
        {
            const float Angle=2*PI*I/Bubbles.Num();
            Bubbles[I]->SetWorldLocation(Position+FVector(FMath::Cos(Angle)*330*PhaseAge,FMath::Sin(Angle)*330*PhaseAge,230*PhaseAge-500*PhaseAge*PhaseAge));
        }
        if(PhaseAge>.45f){for(UStaticMeshComponent* Bubble:Bubbles)Bubble->SetVisibility(false);Phase=EReferenceRainPhase::Idle;WaitRemaining=.8f;PhaseAge=0;}
        return;
    }
    if(Phase==EReferenceRainPhase::Squash)
    {
        if(CheckHeroContact(Position,Position,Hero)||Revision!=TickRevision)return;
        if(PhaseAge>.12f){Phase=EReferenceRainPhase::Falling;PhaseAge=0;Drop->SetRelativeScale3D(FVector(1.6f));Drop->SetWorldLocation(Position);}
        return;
    }
    Velocity.Z-=2400*DeltaSeconds;const FVector Start=Position;FVector End=Position+Velocity*DeltaSeconds;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(ReferenceWaterDrop),false,this);Query.AddIgnoredActor(Hero);
    FHitResult Hit;const bool bHit=GetWorld()->SweepSingleByChannel(Hit,Start,End,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(80),Query);
    if(bHit)End=Hit.Location;
    if(CheckHeroContact(Start,End,Hero)||Revision!=TickRevision)return;
    Position=End;Drop->SetWorldLocation(Position);RefreshShadow();
    if(Bounces>0)BouncePeak=FMath::Max(BouncePeak,static_cast<float>(Position.Z-Target.Z-80));
    if(bHit)
    {
        if(Hit.ImpactNormal.Z>.55f&&++Bounces<3)
        {
            Phase=EReferenceRainPhase::Squash;PhaseAge=0;Target=Hit.ImpactPoint;
            // 球スイープが次の上向き移動でも同じ床を即座に拾わないよう、実接触面から3cm離します。
            Position+=Hit.ImpactNormal*3;
            Drop->SetRelativeScale3D(FVector(1.92f,1.92f,.88f));Drop->SetWorldLocation(Position-FVector(0,0,36));
            Velocity=(Hero->GetActorLocation()-Position).GetSafeNormal2D()*300;Velocity.Z=Bounces==1?1200.f:900.f;
            Shadow->SetWorldLocation(Target+FVector(0,0,2));
            UE_LOG(LogTemp,Display,TEXT("REFERENCE RAIN BOUNCE: actual%d center%s floor%s upward%.1f"),Bounces,*Position.ToString(),*Target.ToString(),Velocity.Z);
        }
        else Burst(Position);
    }
    else if(Position.Z<-2500||PhaseAge>12)Burst(Position);
}
