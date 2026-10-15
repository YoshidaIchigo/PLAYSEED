#include "ThunderCloudflagRace.h"
#include "PlayseedLantern.h"
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
    if (auto* Palette = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Windseed/M_Palette.M_Palette")))
        for (int32 Index = 0; Index < Parts.Num(); ++Index) { auto* Material = UMaterialInstanceDynamic::Create(Palette, this); Material->SetVectorParameterValue(TEXT("Tint"), Colors[Index]); Parts[Index]->SetMaterial(0, Material); }
}
void AThunderCloudflagRace::Configure(const TArray<FVector>& MountainPath)
{
    Route = {FVector(0, 1100, 0), FVector(600, 1850, 0)}; Route.Append(MountainPath);
    if (!MountainPath.IsEmpty()) FlagFloor = MountainPath.Last();
}
const TCHAR* AThunderCloudflagRace::GetStateLabel() const
{
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
        Player->GetInteraction()->ShowMessage(TEXT("BRISK: Reach the crown flag, then wait for me! Caves allowed; Skybloom is not."));
        UE_LOG(LogTemp, Display, TEXT("RACE: started through actual dialogue.")); return;
    }
    if (State == ECloudflagRaceState::Won && !Reward.IsValid())
    {
        Reward = GetWorld()->SpawnActor<APlayseedLantern>(FlagFloor + FVector(-180, 0, 100), FRotator::ZeroRotator);
        if (Reward.IsValid()) Reward->ConfigureReward(2);
        Player->GetInteraction()->ShowMessage(TEXT("BRISK: You won fairly! This lantern belongs to your trail."));
        UE_LOG(LogTemp, Display, TEXT("RACE: winner talked after both arrived; reward 2 appeared.")); return;
    }
    Player->GetInteraction()->ShowMessage(FString(TEXT("BRISK: ")) + GetStateLabel());
}
void AThunderCloudflagRace::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (State != ECloudflagRaceState::Running && State != ECloudflagRaceState::AwaitingRival) return;
    auto* Player = Cast<AWindseedCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)); if (!Player) return;
    // 発射した累計を開始時と比べます。照準を覗くだけでは反則にしません。
    if (Player->GetLaunch()->GetLaunchCount() != LaunchBaseline)
    {
        State = ECloudflagRaceState::Disqualified; Player->GetInteraction()->ShowMessage(TEXT("BRISK: Skybloom launches are outside this footrace. Press R to retry."));
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
