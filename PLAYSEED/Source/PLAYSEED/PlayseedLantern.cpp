#include "PlayseedLantern.h"
#include "ThunderExpeditionSubsystem.h"
#include "ReferenceBattlefieldTerrain.h"
#include "ReferenceTravelSubsystem.h"
#include "WindseedCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APlayseedLantern::APlayseedLantern()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("LanternRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
    auto Part = [this](FName Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color)
    {
        auto* Shape = CreateDefaultSubobject<UStaticMeshComponent>(Name); Shape->SetupAttachment(RootComponent); Shape->SetStaticMesh(Mesh);
        Shape->SetRelativeLocation(Position); Shape->SetRelativeScale3D(Scale); Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Parts.Add(Shape); Colors.Add(Color);
    };
    // 5本の先端を持つ金色のスター。素材は同梱基本形状から組み、保存IDは以前と同じです。
    Part(TEXT("StarCore"),Sphere.Object,FVector::ZeroVector,FVector(.3f,.58f,.58f),FLinearColor(1,.7f,.015f));
    for(int32 I=0;I<5;++I)
    {
        const float A=I*2*PI/5;const FVector Direction(0,FMath::Sin(A),FMath::Cos(A));
        Part(*FString::Printf(TEXT("StarPoint%d"),I),Cone.Object,Direction*38,FVector(.36f,.36f,.8f),FLinearColor(1,.7f,.015f));
        Parts.Last()->SetRelativeRotation(FQuat::FindBetweenNormals(FVector::UpVector,Direction).Rotator());
    }
    for(float Side:{-1.f,1.f})Part(Side<0?TEXT("StarEyeLeft"):TEXT("StarEyeRight"),Sphere.Object,FVector(19,Side*11,4),FVector(.055f,.065f,.18f),FLinearColor(.015f,.015f,.02f));
}
void APlayseedLantern::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr,AReferenceBattlefieldTerrain::IsReferenceCourse()?TEXT("/Game/Battlefield/M_Actor.M_Actor"):TEXT("/Game/Windseed/M_Palette.M_Palette")))
        for (int32 Index = 0; Index < Parts.Num(); ++Index)
        {
            auto* Tint = UMaterialInstanceDynamic::Create(Palette, this); Tint->SetVectorParameterValue(TEXT("Tint"), Colors[Index]); for(int32 Slot=0;Slot<Parts[Index]->GetNumMaterials();++Slot)Parts[Index]->SetMaterial(Slot,Tint);
        }
}
void APlayseedLantern::ConfigureReward(int32 Number)
{
    RestLocation = GetActorLocation();
    if (Number < 1 || Number > UThunderExpeditionSubsystem::TotalRewardCount) { SetActorHiddenInGame(true); return; }
    RewardNumber = Number;
    if(GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->HasEarnedReward(Number))
    {
        // 獲得済みでも青いスターとして取り直せます。保存済みの個数は増やしません。
        for(UStaticMeshComponent* Part:Parts)if(!Part->GetName().Contains(TEXT("Eye")))
            for(int32 Slot=0;Slot<Part->GetNumMaterials();++Slot)if(auto* Tint=Cast<UMaterialInstanceDynamic>(Part->GetMaterial(Slot)))Tint->SetVectorParameterValue(TEXT("Tint"),FLinearColor(.08f,.24f,.85f));
    }
}
void APlayseedLantern::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bCollected || RewardNumber == 0) return;
    RetryRemaining = FMath::Max(0.f, RetryRemaining - DeltaSeconds);
    SetActorLocation(RestLocation + FVector(0, 0, FMath::Sin(GetWorld()->GetTimeSeconds() * 3) * 8));
    AddActorLocalRotation(FRotator(0, DeltaSeconds * 65, 0));
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if(!Player)return;
    const bool bTouching=FVector::Dist2D(Player->GetActorLocation(),GetActorLocation())<=75&&FMath::Abs(Player->GetActorLocation().Z-GetActorLocation().Z)<=100;
    if(!bTouching){bNeedsExitAfterFailure=false;return;}
    auto* Flow=GetGameInstance()->GetSubsystem<UReferenceTravelSubsystem>();
    if(RetryRemaining>0||bNeedsExitAfterFailure||Flow->IsTransitionLocked())return;
    auto* Progress = GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>();
    // 保存成功後にだけ演出へ移り、全スターで絵の前へ帰還します。演出中の二重取得も状態で防ぎます。
    bCollected = true;
    const bool bNewReward = Progress->TryAwardReward(RewardNumber);
    if (bNewReward || Progress->HasEarnedReward(RewardNumber))
    {
        SetActorHiddenInGame(true);
        if (bNewReward) { ++SuccessfulCollections; UE_LOG(LogTemp, Display, TEXT("STAR COLLECTED: actual reward %d, saved total %d."), RewardNumber, Progress->GetEarnedRewardCount()); }
        if(!Flow->Celebrate(Player,RewardNumber))Flow->BeginReturn();
    }
    else { bCollected=false;RetryRemaining=1;bNeedsExitAfterFailure=true; } // 保存失敗なら消さず、離れて再接触して試せます。
}
