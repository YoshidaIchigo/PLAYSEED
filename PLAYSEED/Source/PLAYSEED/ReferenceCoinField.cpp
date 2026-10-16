#include "ReferenceCoinField.h"
#include "ReferenceBattlefieldTerrain.h"
#include "PlayseedActionComponent.h"
#include "PlayseedCoin.h"
#include "WindseedCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AReferenceCoinPost::AReferenceCoinPost()
{
    PrimaryActorTick.bCanEverTick=true;
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("PostRoot"));
    Body=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WoodenPost"));Body->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    Body->SetStaticMesh(Cylinder.Object);Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);Body->SetCollisionResponseToAllChannels(ECR_Block);
    RefreshShape();
}
void AReferenceCoinPost::BeginPlay()
{
    Super::BeginPlay();
    if(auto* Wood=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Wood.M_Wood")))Body->SetMaterial(0,Wood);
}
void AReferenceCoinPost::RefreshShape()
{
    const float Sink=FMath::Min(Pounds*65.f,190.f);
    Body->SetRelativeLocation(FVector(0,0,112.5f-Sink));Body->SetRelativeScale3D(FVector(1.6f,1.6f,2.25f));
}
void AReferenceCoinPost::HandlePound(FVector ImpactPoint)
{
    const FVector Local=GetActorTransform().InverseTransformPosition(ImpactPoint);
    const float Top=225-FMath::Min(Pounds*65.f,190.f);
    if(Pounds>=3||Local.Size2D()>80||FMath::Abs(Local.Z-Top)>15)return;
    ++Pounds;RefreshShape();
}
void AReferenceCoinPost::SpawnCoins()
{
    // 生成済みを先に確定し、同じ周回で何度もコインを増やさないようにします。
    if(GenerationCount!=0)return;++GenerationCount;
    for(int32 I=0;I<5;++I)
    {
        const float Angle=I*2*PI/5;
        auto* Coin=GetWorld()->SpawnActor<APlayseedCoin>(GetActorLocation()+FVector(FMath::Cos(Angle)*140,FMath::Sin(Angle)*140,62),FRotator::ZeroRotator);
        if(!Coin)continue;
        Coin->ConfigureCoin(*FString::Printf(TEXT("%s_coin%d"),*PostId.ToString(),I),EPlayseedCoinKind::Gold);Drops.Add(Coin);
    }
    UE_LOG(LogTemp,Display,TEXT("REFERENCE POST: actual 3 circles spawned5 once: %s degrees%.1f seconds%.2f"),*PostId.ToString(),TotalAngle,CircleSeconds);
}
void AReferenceCoinPost::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Hero=Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this,0));if(!Hero)return;
    if(!Actions.IsValid()){Actions=Hero->GetActions();Actions->OnPoundImpact.AddUObject(this,&AReferenceCoinPost::HandlePound);}
    if(Pounds>0||GenerationCount>0)return;
    const FVector Feet=Hero->GetActorLocation()-FVector(0,0,Hero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    const FVector Delta=Feet-GetActorLocation();const float Angle=FMath::RadiansToDegrees(FMath::Atan2(Delta.Y,Delta.X));
    if(Delta.Size()>400)
    {CircleSeconds=TotalAngle=0;bHaveAngle=false;return;}
    CircleSeconds+=DeltaSeconds;
    if(bHaveAngle)TotalAngle+=FRotator::NormalizeAxis(Angle-PreviousAngle);
    PreviousAngle=Angle;bHaveAngle=true;
    // 一次資料の200フレームを30Hzから秒へ直します。近くで時間を過ごしたら、一度離れてやり直せます。
    if(FMath::Abs(TotalAngle)>1080&&CircleSeconds<200.f/30)SpawnCoins();
}
void AReferenceCoinPost::ResetPost()
{
    for(const auto& Coin:Drops)if(Coin.IsValid())Coin->Destroy();Drops.Reset();
    Pounds=GenerationCount=0;PreviousAngle=TotalAngle=CircleSeconds=0;bHaveAngle=false;RefreshShape();
}
void AReferenceCoinPost::EndPlay(const EEndPlayReason::Type Reason)
{
    if(Actions.IsValid())Actions->OnPoundImpact.RemoveAll(this);Super::EndPlay(Reason);
}

void AReferenceCoinBox::BeginPlay()
{
    Super::BeginPlay();
    if(auto* Wood=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Wood.M_Wood")))GetBoxBody()->SetMaterial(0,Wood);
    if(auto* Palette=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Battlefield/M_Actor.M_Actor")))for(UStaticMeshComponent* Band:GetBoxBands())
    {auto* Tint=UMaterialInstanceDynamic::Create(Palette,this);Tint->SetVectorParameterValue(TEXT("Tint"),FLinearColor(.26f,.12f,.035f));Band->SetMaterial(0,Tint);}
}
AReferenceCoinField::AReferenceCoinField()
{RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("CoinFieldRoot"));}
void AReferenceCoinField::AddGroundCoin(FName Id,FVector Near)
{
    auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(GetWorld(),AReferenceBattlefieldTerrain::StaticClass()));
    FVector Floor;
    if(!Terrain||!Terrain->FindFloor(Near+FVector(0,0,120),Floor))
    {UE_LOG(LogTemp,Warning,TEXT("REFERENCE COIN: no actual ground for%s at%s"),*Id.ToString(),*Near.ToString());return;}
    auto* Coin=GetWorld()->SpawnActor<APlayseedCoin>(Floor+FVector(0,0,30),FRotator::ZeroRotator);
    if(Coin){Coin->ConfigureCoin(Id,EPlayseedCoinKind::Gold);Coins.Add(Coin);}
}
void AReferenceCoinField::BeginPlay()
{
    Super::BeginPlay();
    const FVector Lines[]={FVector(-129,-5668,0),FVector(1851,2234,2917),FVector(4836,2707,3079)};
    const float Yaws[]={90,75,120};
    // N64のローカルZをyawで回し、(X,Y,Z)からUEの(X,-Z,Y)へ変換します。
    for(int32 Group=0;Group<3;++Group)
    {
        if(Group==1)for(int32 I=0;I<8;++I)
        {
            const float Angle=I*2*PI/8;
            AddGroundCoin(*FString::Printf(TEXT("ReferenceGroundRing_%d"),I),FVector(-6060+FMath::Sin(Angle)*300,5340-FMath::Cos(Angle)*300,1060));
        }
        const float Angle=FMath::DegreesToRadians(Yaws[Group]);
        for(int32 I=0;I<5;++I)
        {
            const float Offset=160*(I-2);
            AddGroundCoin(*FString::Printf(TEXT("ReferenceGroundLine%d_%d"),Group,I),Lines[Group]+FVector(FMath::Sin(Angle)*Offset,-FMath::Cos(Angle)*Offset,0));
        }
    }
    const FVector Positions[]={FVector(-6500,-1800,768),FVector(-5500,-1800,768),FVector(-6500,-3000,768),FVector(-5500,-3000,768)};
    for(int32 I=0;I<4;++I)
    {
        auto* Post=GetWorld()->SpawnActor<AReferenceCoinPost>(Positions[I],FRotator::ZeroRotator);
        if(Post){Post->ConfigurePost(*FString::Printf(TEXT("ReferenceWoodPost%d"),I));Posts.Add(Post);}
    }
    CoinBox=GetWorld()->SpawnActor<AReferenceCoinBox>(FVector(-3979,-6627,0),FRotator::ZeroRotator);
    if(CoinBox)CoinBox->ConfigureBox(TEXT("ReferenceThreeCoinBox"));
    UE_LOG(LogTemp,Display,TEXT("REFERENCE GROUND COINS: actual terrain placed%d static coins, %d wooden posts and one three-coin box"),Coins.Num(),Posts.Num());
}
void AReferenceCoinField::ResetCoins()
{
    for(APlayseedCoin* Coin:Coins)if(Coin)Coin->ResetCoin();
    for(AReferenceCoinPost* Post:Posts)if(Post)Post->ResetPost();
    if(CoinBox)CoinBox->ResetBox();
}
