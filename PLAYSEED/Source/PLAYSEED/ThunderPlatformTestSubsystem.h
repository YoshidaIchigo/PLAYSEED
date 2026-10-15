#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ThunderPlatformTestSubsystem.generated.h"

class AWindseedCharacter;
class AThunderMovingPlatform;

// 実際に飛び乗り、CharacterMovement に運ばれ、R と飛び降りを確認する検証専用クラスです。
UCLASS()
class PLAYSEED_API UThunderPlatformTestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void BeginTest(AWindseedCharacter* Player, AThunderMovingPlatform* Platform);
    bool UpdateTest(AWindseedCharacter* Player, float DeltaSeconds);
    const FString& GetError() const { return Error; }
private:
    bool MoveToward(AWindseedCharacter* Player, FVector Goal, float DeltaSeconds, bool bJump);
    bool IsRiding(AWindseedCharacter* Player) const;
    void StartRide(AWindseedCharacter* Player);
    // World が所有する Actor を観察するだけなので、レベルの寿命を延ばさない弱参照にします。
    TWeakObjectPtr<AThunderMovingPlatform> Platform;
    int32 Phase = 0;
    int32 RecoveryBaseline = 0;
    float PhaseTime = 0;
    float RideStartX = 0;
    FVector RideRelativeStart = FVector::ZeroVector;
    FString Error;
    bool bRestartVerified = false;
    bool bSnapshotSaved = false;
};
