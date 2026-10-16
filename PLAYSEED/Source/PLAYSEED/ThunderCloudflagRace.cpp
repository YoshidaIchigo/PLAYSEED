#include "ThunderCloudflagRace.h"
#include "PlayseedLantern.h"
#include "ReferenceBattlefieldTerrain.h"
#include "PlayseedInteractionComponent.h"
#include "PlayseedLaunchComponent.h"
#include "ThunderMeadowStage.h"
#include "ThunderWarpPair.h"
#include "WindseedCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AThunderCloudflagRace::AThunderCloudflagRace()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("BriskRoot"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
    auto Part = [this](FName Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color)
    {
        auto* Shape = CreateDefaultSubobject<UStaticMeshComponent>(Name); Shape->SetupAttachment(RootComponent); Shape->SetStaticMesh(Mesh);
        Shape->SetRelativeLocation(Position); Shape->SetRelativeScale3D(Scale); Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision); Parts.Add(Shape); Colors.Add(Color);
    };
    bReferenceRace=AReferenceBattlefieldTerrain::IsReferenceCourse();
    if(bReferenceRace)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
        const FLinearColor Yellow(.95f,.66f,.07f),Green(.07f,.46f,.08f),White(.92f,.92f,.8f),Orange(.9f,.26f,.025f);
        Part(TEXT("ShellRim"),Sphere.Object,FVector(-25,0,139),FVector(.7f,1.45f,1.65f),White);
        Part(TEXT("GreenShell"),Sphere.Object,FVector(-39,0,139),FVector(.67f,1.32f,1.52f),Green);
        Part(TEXT("YellowBody"),Sphere.Object,FVector(8,0,122),FVector(.8f,.85f,1.15f),Yellow);
        Part(TEXT("LongNeck"),Cylinder.Object,FVector(22,0,190),FVector(.35f,.35f,.75f),Yellow);
        Part(TEXT("TurtleHead"),Sphere.Object,FVector(35,0,231),FVector(.9f,.74f,.68f),Yellow);
        for(int32 Side:{-1,1})
        {
            Part(*FString::Printf(TEXT("OrangeBoot%d"),Side),Sphere.Object,FVector(25,Side*42,28),FVector(.85f,.5f,.45f),Orange);
            Part(*FString::Printf(TEXT("YellowHand%d"),Side),Sphere.Object,FVector(15,Side*56,139),FVector(.32f,.38f,.55f),Yellow);
            Part(*FString::Printf(TEXT("WhiteEye%d"),Side),Sphere.Object,FVector(72,Side*18,244),FVector(.16f,.16f,.27f),White);
            Part(*FString::Printf(TEXT("BlackPupil%d"),Side),Sphere.Object,FVector(80,Side*18,244),FVector(.05f,.08f,.17f),FLinearColor(.015f,.02f,.025f));
        }
        return;
    }
    Part(TEXT("IndigoBody"), Sphere.Object, FVector(0, 0, 65), FVector(.8f, .7f, 1), FLinearColor(.19f, .26f, .7f));
    Part(TEXT("GoldenFeather"), Cone.Object, FVector(0, 0, 135), FVector(.3f, .45f, .6f), FLinearColor(1, .7f, .14f));
    for (int32 Side : {-1, 1})
    {
        Part(*FString::Printf(TEXT("BriskBoot%d"), Side), Sphere.Object, FVector(18, Side * 22, 15), FVector(.6f, .3f, .25f), FLinearColor(.85f, .5f, .16f));
        Part(*FString::Printf(TEXT("BriskEye%d"), Side), Sphere.Object, FVector(37, Side * 17, 83), FVector(.12f, .12f, .19f), FLinearColor(.9f, 1, .8f));
    }
}
void AThunderCloudflagRace::BeginPlay()
{
    Super::BeginPlay(); StartFloor = GetActorLocation();
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr,bReferenceRace?TEXT("/Game/Battlefield/M_Actor.M_Actor"):TEXT("/Game/Windseed/M_Palette.M_Palette")))
        for (int32 Index = 0; Index < Parts.Num(); ++Index) { auto* Material = UMaterialInstanceDynamic::Create(Palette, this); Material->SetVectorParameterValue(TEXT("Tint"), Colors[Index]); Parts[Index]->SetMaterial(0, Material); }
}
void AThunderCloudflagRace::Configure(const TArray<FVector>& MountainPath)
{
    if(bReferenceRace)
    {
        // 相手は参照の経路を走ります。元の座標付近にある実際の床へ足元を合わせます。
        Route.Reset();
        auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(this,AReferenceBattlefieldTerrain::StaticClass()));
        for(const FVector& Point:MountainPath)
        {
            FVector Floor;
            Route.Add(Terrain&&Terrain->FindFloor(Point,Floor)?Floor:Point);
        }
        FlagFloor=FVector(3304,4603,4293);return;
    }
    Route = {FVector(0, 1100, 0), FVector(600, 1850, 0)}; Route.Append(MountainPath);
    if (!MountainPath.IsEmpty()) FlagFloor = MountainPath.Last();
}
const TCHAR* AThunderCloudflagRace::GetStateLabel() const
{
    if(bReferenceRace)
    {
        switch(State)
        {
            case ECloudflagRaceState::Running:return TEXT("山頂の旗まで競走！");
            case ECloudflagRaceState::AwaitingRival:return TEXT("相手が着くまで旗で待とう");
            case ECloudflagRaceState::Won:return TEXT("君の勝ち！ Gで話しかけよう");
            case ECloudflagRaceState::Lost:return TEXT("相手の勝ち。Rでやりなおそう");
            case ECloudflagRaceState::Disqualified:return TEXT("大砲は禁止。Rでやりなおそう");
            default:return TEXT("Gで競走を始めよう");
        }
    }
    switch (State)
    {
        case ECloudflagRaceState::Running: return bRivalFinished ? TEXT("RIVAL AT FLAG - reach the flag") : TEXT("RACE TO THE CLOUDFLAG");
        case ECloudflagRaceState::AwaitingRival: return TEXT("FLAG REACHED - wait for Brisk");
        case ECloudflagRaceState::Won: return TEXT("YOU WON - talk to Brisk (G / DOWN)");
        case ECloudflagRaceState::Lost: return TEXT("BRISK WON - R to retry");
        case ECloudflagRaceState::Disqualified: return TEXT("SKYBLOOM USED - R to retry");
        default: return TEXT("Talk to Brisk to start (G / DOWN)");
    }
}
void AThunderCloudflagRace::Interact(AWindseedCharacter* Player)
{
    if (!Player || Route.IsEmpty()) return;
    if (State == ECloudflagRaceState::Idle)
    {
        State = ECloudflagRaceState::Running; Elapsed = 0; LaunchBaseline = Player->GetLaunch()->GetLaunchCount();
        const auto* Stage = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(this, AThunderMeadowStage::StaticClass()));
        WarpBaseline = Stage && Stage->GetWarpPair() ? Stage->GetWarpPair()->GetWarpCount() : 0;
        Player->GetInteraction()->ShowMessage(bReferenceRace?TEXT("山頂の旗まで競走だ！\n先に着いたら、そこで待っていてね。\nShiftで走る。大砲は使わないでね。"):TEXT("BRISK: Reach the crown flag, then wait for me! Caves allowed; Skybloom is not."));
        UE_LOG(LogTemp, Display, TEXT("RACE: started through actual dialogue.")); return;
    }
    if (State == ECloudflagRaceState::Won && !Reward.IsValid())
    {
        Reward = GetWorld()->SpawnActor<APlayseedLantern>(bReferenceRace?FVector(3030,4600,4500):FlagFloor + FVector(-180, 0, 100), FRotator::ZeroRotator);
        if (Reward.IsValid()) Reward->ConfigureReward(2);
        Player->GetInteraction()->ShowMessage(bReferenceRace?TEXT("参った！ 君の勝ちだ！\nスターを受け取ってね。"):TEXT("BRISK: You won fairly! This lantern belongs to your trail."));
        UE_LOG(LogTemp, Display, TEXT("RACE: winner talked after both arrived; reward 2 appeared.")); return;
    }
    Player->GetInteraction()->ShowMessage(bReferenceRace?FString(GetStateLabel()):FString(TEXT("BRISK: ")) + GetStateLabel());
}
void AThunderCloudflagRace::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (State != ECloudflagRaceState::Running && State != ECloudflagRaceState::AwaitingRival) return;
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)); if (!Player) return;
    // 発射した累計を開始時と比べます。照準を覗くだけでは反則にしません。
    if (Player->GetLaunch()->GetLaunchCount() != LaunchBaseline)
    {
        State = ECloudflagRaceState::Disqualified; Player->GetInteraction()->ShowMessage(bReferenceRace?TEXT("この競走で大砲は使えないよ。\nRでやりなおそう。"):TEXT("BRISK: Skybloom launches are outside this footrace. Press R to retry."));
        UE_LOG(LogTemp, Display, TEXT("RACE: disqualified after a real Skybloom launch.")); return;
    }
    const auto* Stage = Cast<AThunderMeadowStage>(UGameplayStatics::GetActorOfClass(this, AThunderMeadowStage::StaticClass()));
    if (Stage && Stage->GetWarpPair() && Stage->GetWarpPair()->GetWarpCount() > WarpBaseline) bUsedWarp = true;
    Elapsed += DeltaSeconds;
    // NPCだけを既知の徒歩経路へ進めます。低フレームでも余った距離を次区間へ渡します。
    float Distance = 280 * DeltaSeconds;
    while (!bRivalFinished && Waypoint < Route.Num())
    {
        const FVector Offset = Route[Waypoint] - GetActorLocation(); const float Length = Offset.Size();
        if (Length <= Distance) { SetActorLocation(Route[Waypoint]); Distance -= Length; ++Waypoint; }
        else { SetActorLocation(GetActorLocation() + Offset.GetSafeNormal() * Distance); SetActorRotation(FRotator(0, Offset.Rotation().Yaw, 0)); break; }
    }
    if (!bRivalFinished && Waypoint == Route.Num())
    {
        bRivalFinished = true; RivalFinishTime = Elapsed;
        UE_LOG(LogTemp, Display, TEXT("RACE: Brisk reached flag in %.2f seconds."), Elapsed);
    }
    if(bReferenceRace)
        for(UStaticMeshComponent* Part:Parts)
            if(Part->GetName().StartsWith(TEXT("OrangeBoot")))
            {
                FVector Position=Part->GetRelativeLocation();
                const float PhaseOffset=Part->GetName().EndsWith(TEXT("-1"))?PI:0;
                Position.Z=28+(bRivalFinished?0:FMath::Max(0.f,FMath::Sin(Elapsed*12+PhaseOffset))*14);
                Part->SetRelativeLocation(Position);
            }
    const float Feet = Player->GetActorLocation().Z - 82;
    if (PlayerFinishTime < 0 && Player->GetCharacterMovement()->IsMovingOnGround() && FVector::Dist2D(Player->GetActorLocation(), FlagFloor) < 100 && FMath::Abs(Feet - FlagFloor.Z) < 40)
    {
        PlayerFinishTime = Elapsed; State = bRivalFinished ? ECloudflagRaceState::Lost : ECloudflagRaceState::AwaitingRival;
        UE_LOG(LogTemp, Display, TEXT("RACE: player actually reached flag in %.2f seconds, warp allowed %d."), Elapsed, bUsedWarp);
    }
    if (bRivalFinished && State == ECloudflagRaceState::AwaitingRival) State = ECloudflagRaceState::Won;
}
void AThunderCloudflagRace::ResetRace()
{
    if (Reward.IsValid()) Reward->Destroy(); Reward.Reset(); SetActorLocation(StartFloor); SetActorRotation(FRotator::ZeroRotator);
    State = ECloudflagRaceState::Idle; Waypoint = 0; Elapsed = 0; PlayerFinishTime = RivalFinishTime = -1; bRivalFinished = bUsedWarp = false;
}
