#include "ReferenceBoulderField.h"
#include "ReferenceBoulderPaths.h"
#include "ReferenceBattlefieldTerrain.h"
#include "ThunderRollingBoulder.h"
#include "ThunderExpeditionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"

AReferenceBoulderField::AReferenceBoulderField()
{RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("ReferenceBoulderRoot"));}

void AReferenceBoulderField::BeginPlay()
{
    Super::BeginPlay();
    auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(this,AReferenceBattlefieldTerrain::StaticClass()));
    if(!Terrain){++FloorFailures;return;}
    auto ActualFloor=[&](FVector Near)
    {
        FVector Floor;
        if(Terrain->FindFloor(Near,Floor))return Floor;
        ++FloorFailures;UE_LOG(LogTemp,Warning,TEXT("REFERENCE BOULDER: missing floor near%s"),*Near.ToString());return Near;
    };
    for(int32 Index=0;Index<2;++Index)
    {
        const auto* Points=Index==0?ReferenceBoulderPath0:ReferenceBoulderPath1;
        const int32 Count=Index==0?UE_ARRAY_COUNT(ReferenceBoulderPath0):UE_ARRAY_COUNT(ReferenceBoulderPath1);
        TArray<FVector> Feet;
        for(int32 I=0;I<Count;++I)Feet.Add(ActualFloor(FVector(Points[I].X,Points[I].Y,Points[I].Z)));
        auto* Ball=GetWorld()->SpawnActor<AThunderRollingBoulder>();Ball->ConfigureReferencePath(Feet);Balls.Add(Ball);
    }
    const FVector Pits[]={FVector(-993,3565,886),FVector(-785,4301,886),FVector(-93,3414,886)};
    const int32 PitCount=GetGameInstance()->GetSubsystem<UThunderExpeditionSubsystem>()->GetSelectedMissionNumber()>1?3:2;
    for(int32 Index=0;Index<PitCount;++Index)
    {
        TArray<FVector> Feet;
        // くぼ地の実床に沿って周回します。途切れる出現・消滅はなく、接触は実カプセルで判定します。
        for(int32 I=0;I<=32;++I){const float Angle=2*PI*I/32;Feet.Add(ActualFloor(Pits[Index]+FVector(180*FMath::Cos(Angle),180*FMath::Sin(Angle),0)));}
        auto* Ball=GetWorld()->SpawnActor<AThunderRollingBoulder>();Ball->ConfigureReferencePath(Feet,true);Balls.Add(Ball);
    }
    UE_LOG(LogTemp,Display,TEXT("REFERENCE BOULDERS: actual paths18/27, pits%d, floorFailures%d"),PitCount,FloorFailures);
}
void AReferenceBoulderField::ResetBoulders()
{for(AThunderRollingBoulder* Ball:Balls)if(Ball)Ball->ResetMotion();}
