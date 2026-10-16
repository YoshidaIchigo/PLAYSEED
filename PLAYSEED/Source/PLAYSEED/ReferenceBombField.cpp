#include "ReferenceBombField.h"
#include "ReferenceBomb.h"
#include "ReferenceBattlefieldTerrain.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
void AReferenceBombField::BeginPlay()
{
    Super::BeginPlay();auto* Terrain=Cast<AReferenceBattlefieldTerrain>(UGameplayStatics::GetActorOfClass(this,AReferenceBattlefieldTerrain::StaticClass()));
    const FVector Positions[]={FVector(-3080,5200,0),FVector(-3688,3813,885),FVector(-4629,1772,1024),FVector(-3480,2120,891),FVector(-3800,460,1024),FVector(6888,5608,2002),FVector(2350,-3700,757),FVector(-1750,2800,0),FVector(-1400,950,0),FVector(-2650,-1750,0),FVector(-1900,-3450,0),FVector(1127,2495,2897)};
    for(int32 I=0;I<UE_ARRAY_COUNT(Positions);++I)
    {
        FVector Floor;if(!Terrain||!Terrain->FindFloor(Positions[I],Floor)){++FloorFailures;continue;}
        auto* Bomb=GetWorld()->SpawnActor<AReferenceBomb>(Floor+FVector(0,0,65),FRotator(0,-90,0));
        if(Bomb){Bomb->ConfigureStationary(I==11);Bombs.Add(Bomb);}
    }
}
void AReferenceBombField::ResetBombs() { for(AReferenceBomb* Bomb:Bombs)if(Bomb)Bomb->ResetToSpawn(); }
