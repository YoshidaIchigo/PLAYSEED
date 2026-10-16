#include "ReferenceWalkerField.h"
#include "ReferenceWalker.h"
#include "ReferenceBattlefieldTerrain.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void AReferenceWalkerField::BeginPlay()
{
    Super::BeginPlay();auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(this,AReferenceBattlefieldTerrain::StaticClass()));
    auto Spawn=[&](FVector Position)
    {
        FVector Floor;if(!Terrain||!Terrain->FindFloor(Position,Floor)){++FloorFailures;return;}
        Walkers.Add(GetWorld()->SpawnActor<AReferenceWalker>(Floor,FRotator(0,-90,0)));
    };
    Spawn(FVector(-2713,-5778,152));Spawn(FVector(-342,-5433,400));
    for(FVector Centre:{FVector(3640,-6280,768),FVector(6060,-2000,877),FVector(-6050,-1250,768)})
        for(int32 N=0;N<3;++N)
        {
            const float Angle=(N*21845)*2*PI/65536;
            // 元の配置と同じ整数オフセットです。動きや描画のモデルは自作です。
            const int32 X=static_cast<int32>(500*FMath::Cos(Angle)),Y=static_cast<int32>(-500*FMath::Sin(Angle));
            Spawn(Centre+FVector(X,Y,0));
        }
}
void AReferenceWalkerField::ResetWalkers(){for(AReferenceWalker* Walker:Walkers)if(Walker)Walker->ResetEnemy();}
