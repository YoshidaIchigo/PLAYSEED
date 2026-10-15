#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderGaleleafTestSubsystem.generated.h"

class AWindseedCharacter;
class APlayseedGaleleafEnemy;

// 初回のパンチと踏みつけを比べ、葉と青コインの発生条件を実際の移動で確認します。
UCLASS()
class PLAYSEED_API UThunderGaleleafTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, APlayseedGaleleafEnemy* Enemy);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool WalkToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds);
    bool Approach(AWindseedCharacter* Player, float DeltaSeconds);
    TWeakObjectPtr<APlayseedGaleleafEnemy> Enemy;
    FString Error;
    int32 Phase = 0;
    int32 Loop = 0;
    int32 Waypoint = 0;
    int32 LeafBaseline = 0;
    int32 AzureBaseline = 0;
    int32 StompBaseline = 0;
    int32 RecoveryBaseline = 0;
    float PhaseTime = 0;
    bool bAzureSaved = false;
};
