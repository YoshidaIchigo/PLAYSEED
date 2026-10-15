#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderTowerTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderMeadowStage;

// 通常ジャンプで塔を登り、急降下で解放し、実際の花取得と再起動を確認します。
UCLASS()
class PLAYSEED_API UThunderTowerTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMeadowStage* InStage, bool bRestoredProcess);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    void SetPhase(int32 Number);
    void BeginBloomRoute();
    bool WalkBloomRoute(AWindseedCharacter* Player, float DeltaSeconds);
    TWeakObjectPtr<AThunderMeadowStage> Stage;
    TArray<FVector> Route;
    FString Error;
    int32 Phase = 0;
    int32 Waypoint = 0;
    int32 RecoveryBefore = 0;
    int32 GrantsBefore = 0;
    int32 ActivationBefore = 0;
    int32 PoundsBefore = 0;
    float PhaseTime = 0;
    bool bReadProcess = false;
};
