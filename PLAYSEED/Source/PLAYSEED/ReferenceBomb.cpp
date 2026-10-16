#include "ReferenceBomb.h"
#include "ReferenceBattlefieldTerrain.h"
#include "PlayseedCarryComponent.h"
#include "PlayseedCoin.h"
#include "PlayseedVitalityComponent.h"
#include "WindseedCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AReferenceBomb::AReferenceBomb()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    GetBody()->SetStaticMesh(Sphere.Object);GetBody()->SetRelativeScale3D(FVector(1.3f));
    GetBody()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
    Visual=CreateDefaultSubobject<USceneComponent>(TEXT("BombVisual"));Visual->SetupAttachment(RootComponent);Visual->SetRelativeScale3D(FVector(1/1.3f));
    auto Part=[&](UStaticMesh* Mesh,FVector Pos,FVector Size,FLinearColor Color)
    {
        auto* P=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("BombPart%d"),Parts.Num()));P->SetupAttachment(Visual);P->SetStaticMesh(Mesh);P->SetRelativeLocation(Pos);P->SetRelativeScale3D(Size/100);P->SetCollisionEnabled(ECollisionEnabled::NoCollision);P->SetCastShadow(false);Parts.Add(P);Colors.Add(Color);return P;
    };
    for(int32 Side:{-1,1})
    {
        Feet.Add(Part(Sphere.Object,FVector(3,Side*37,-51),FVector(72,49,28),FLinearColor(.9f,.46f,.008f)));
        Part(Sphere.Object,FVector(58,Side*22,15),FVector(16,18,42),FLinearColor(.92f,.94f,.85f));
    }
    Part(Cylinder.Object,FVector(0,0,66),FVector(35,35,17),FLinearColor(.35f,.22f,.09f));
    Part(Cylinder.Object,FVector(0,0,89),FVector(7,7,38),FLinearColor(.62f,.5f,.25f));
    Spark=Part(Sphere.Object,FVector(0,0,109),FVector(15),FLinearColor(1,.18f,.003f));Spark->SetVisibility(false);
    Part(Sphere.Object,FVector(36,-34,40),FVector(7,21,21),FLinearColor(.16f,.18f,.22f));
    Part(Cylinder.Object,FVector(0,0,-64),FVector(145,145,2),FLinearColor(.008f,.009f,.006f));
    Flash=Part(Sphere.Object,FVector(0,0,35),FVector(300),FLinearColor(1,.38f,.005f));Flash->SetVisibility(false);
}
void AReferenceBomb::BeginPlay()
{
    Super::BeginPlay();Home=GetActorLocation();HomeRotation=GetActorRotation();
    GetBody()->SetSimulatePhysics(false);GetBody()->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
    GetBody()->OnComponentHit.AddDynamic(this,&AReferenceBomb::HandleKickedHit);
    if(auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor")))
    {
        auto* Black=UMaterialInstanceDynamic::Create(Palette,this);Black->SetVectorParameterValue(TEXT("Tint"),FLinearColor(.003f,.004f,.007f));GetBody()->SetMaterial(0,Black);
        for(int32 I=0;I<Parts.Num();++I){auto* M=UMaterialInstanceDynamic::Create(Palette,this);M->SetVectorParameterValue(TEXT("Tint"),Colors[I]);Parts[I]->SetMaterial(0,M);}
    }
}
bool AReferenceBomb::CanBePickedUp(AWindseedCharacter* Player) const { return State<2&&!bKicked&&Super::CanBePickedUp(Player); }
bool AReferenceBomb::TryPickup(AWindseedCharacter* Player)
{
    if(!Super::TryPickup(Player))return false;
    bFuseLit=true;Spark->SetVisibility(true);return true;
}
FVector AReferenceBomb::GetThrowVelocity(const AWindseedCharacter* Player) const { return Player->GetActorForwardVector()*750+FVector(0,0,600); }
void AReferenceBomb::OnThrownLanding(const FHitResult&) { BeginGrow(); }
void AReferenceBomb::HandleKickedHit(UPrimitiveComponent*,AActor*,UPrimitiveComponent*,FVector,const FHitResult& Hit)
{if(bKicked&&Hit.ImpactNormal.Z>.55f)BeginGrow();}
bool AReferenceBomb::ReceiveStrike(AWindseedCharacter* Player,int32 Damage)
{
    if(!Player||Damage<=0||State>=2||IsHeld()||IsInThrownFlight()||bKicked)return false;
    bFuseLit=true;bKicked=true;Spark->SetVisibility(true);GetBody()->SetSimulatePhysics(true);
    GetBody()->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);GetBody()->SetPhysicsLinearVelocity(Player->GetActorForwardVector()*750+FVector(0,0,900));return true;
}
void AReferenceBomb::MoveGround(FVector Target,float Speed,float DeltaSeconds)
{
    const FVector Start=GetActorLocation(),D=(Target-Start).GetSafeNormal2D();FVector Candidate=Start+D*FMath::Min(Speed*DeltaSeconds,FVector::Dist2D(Start,Target));
    FCollisionQueryParams Q;Q.AddIgnoredActor(this);Q.AddIgnoredActor(UGameplayStatics::GetPlayerPawn(this,0));FHitResult Wall,Floor;
    if(GetWorld()->SweepSingleByChannel(Wall,Start+FVector(0,0,55),Candidate+FVector(0,0,55),FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(58),Q))return;
    if(!GetWorld()->LineTraceSingleByChannel(Floor,Candidate+FVector(0,0,115),Candidate-FVector(0,0,230),ECC_Visibility,Q)||Floor.ImpactNormal.Z<.65f||FMath::Abs(Floor.ImpactPoint.Z+65-Start.Z)>35)return;
    Candidate.Z=Floor.ImpactPoint.Z+65;SetActorLocation(Candidate);if(State==1)ChaseTravel+=FVector::Dist2D(Start,Candidate);if(!D.IsNearlyZero())SetActorRotation(FRotator(0,D.Rotation().Yaw,0));
}
void AReferenceBomb::BeginGrow()
{
    if(State>=2)return;
    State=2;StateAge=0;bKicked=false;GetBody()->SetSimulatePhysics(false);GetBody()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AReferenceBomb::Burst()
{
    State=3;StateAge=0;bExplosionHit=false;++ExplosionCount;
    GetBody()->SetRelativeScale3D(FVector(1.3f));GetBody()->SetVisibility(false);for(UStaticMeshComponent* P:Parts)P->SetVisibility(false);Flash->SetVisibility(true);
    // 遠方で再出現しても同じ個体のコインを増やしません。通常Rは新しい挑戦です。
    if(!bCoinIssued)
    {
        bCoinIssued=true;++DropGenerationCount;
        auto* Coin=GetWorld()->SpawnActor<APlayseedCoin>(GetActorLocation()+FVector(0,0,-3),FRotator::ZeroRotator);
        if(Coin){Coin->ConfigureCoin(*FString::Printf(TEXT("%s_BombGold"),*GetFName().ToString()),EPlayseedCoinKind::Gold);DropCoin=Coin;}
    }
}
void AReferenceBomb::Restore(bool bNewAttempt)
{
    ++Revision;Super::ResetToSpawn();GetBody()->SetSimulatePhysics(false);GetBody()->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
    GetBody()->SetVisibility(true);GetBody()->SetRelativeScale3D(FVector(1.3f));for(UStaticMeshComponent* P:Parts)P->SetVisibility(true);
    Visual->SetRelativeScale3D(FVector(1/1.3f));Flash->SetVisibility(false);Spark->SetVisibility(false);SetActorRotation(HomeRotation);SetActorHiddenInGame(false);
    State=0;StateAge=FuseAge=WalkAge=PatrolAge=0;bFuseLit=bExplosionHit=bKicked=false;
    if(bNewAttempt){if(DropCoin.IsValid())DropCoin->Destroy();DropCoin.Reset();bCoinIssued=false;}
}
void AReferenceBomb::ResetToSpawn() { Restore(true); }
void AReferenceBomb::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);auto* Hero=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0));if(!Hero)return;
    StateAge+=DeltaSeconds;const float Distance=FVector::Dist2D(Hero->GetActorLocation(),GetActorLocation());
    if(State==4){if(FVector::Dist2D(Hero->GetActorLocation(),Home)>3000)Restore(false);return;}
    if(State==3)
    {
        Flash->SetRelativeScale3D(FVector(3*(1+StateAge/.3f)));
        const float FeetZ=Hero->GetActorLocation().Z-Hero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        if(!bExplosionHit&&Distance<184&&FeetZ<GetActorLocation().Z+85&&FeetZ+164>GetActorLocation().Z-215)
        {
            FCollisionQueryParams Q;Q.AddIgnoredActor(this);Q.AddIgnoredActor(Hero);
            if(!GetWorld()->LineTraceTestByChannel(GetActorLocation(),Hero->GetActorLocation(),ECC_Visibility,Q))
            {
                bExplosionHit=true;const int32 Before=Revision;
                if(Hero->GetVitality()->TryTakeImpact(2,Hero->GetActorLocation()-GetActorLocation()))++ExplosionDamageCount;
                if(Revision!=Before)return; // 実ダメージから再開されたTickの続きを実行しません。
            }
        }
        if(StateAge>=.3f){Flash->SetVisibility(false);State=4;SetActorHiddenInGame(true);}return;
    }
    if(State==2){GetBody()->SetRelativeScale3D(FVector(1.3f*(1+StateAge/(5.f/30))));if(StateAge>=5.f/30)Burst();return;}
    if(bFuseLit){FuseAge+=DeltaSeconds;Spark->SetVisibility(true);Spark->SetRelativeScale3D(FVector(.15f*(1+.35f*FMath::Sin(FuseAge*60))));}
    if(bFuseLit&&FuseAge>5)
    {
        if(IsHeld())Hero->GetCarry()->DropHeld();
        BeginGrow();return;
    }
    if(IsHeld()||IsInThrownFlight()||bKicked)return;
    // 通常のDrop後も重力で落ち着くまでは物理へ任せます。
    if(GetBody()->IsSimulatingPhysics())return;
    if(Distance>4000)return;
    if(!bStationary)
    {
        if(State==0)
        {
            const FVector ToHero=Hero->GetActorLocation()-GetActorLocation();
            if(Distance<400&&FMath::Abs(ToHero.Z-17)<180&&FVector::DotProduct(GetActorForwardVector(),ToHero.GetSafeNormal2D())>.7f)
            {
                FCollisionQueryParams Q;Q.AddIgnoredActor(this);Q.AddIgnoredActor(Hero);
                if(!GetWorld()->LineTraceTestByChannel(GetActorLocation(),Hero->GetActorLocation(),ECC_Visibility,Q)){State=1;bFuseLit=true;++ChaseStarts;}
            }
            if(State==0){PatrolAge+=DeltaSeconds;MoveGround(Home+FVector(FMath::Cos(PatrolAge*.75f)*200,FMath::Sin(PatrolAge*.75f)*200,0),150,DeltaSeconds);}
        }
        if(State==1)MoveGround(Hero->GetActorLocation(),600,DeltaSeconds);
        WalkAge+=DeltaSeconds*12;for(int32 I=0;I<Feet.Num();++I){const float S=FMath::Sin(WalkAge+I*PI);Feet[I]->SetRelativeLocation(FVector(3+S*10,I==0?-37:37,-51+FMath::Max(0.f,S)*8));}
    }
}
