#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ReferenceTravelSubsystem.generated.h"

UENUM()
enum class EReferenceTravelState : uint8 { InCourse, AtPainting, EnteringPainting, SelectingStar, Dropping, Celebrating, Returning };

// GameInstanceに残る状態です。マップのActorは保存せず、遷移の二重実行だけを防ぎます。
UCLASS()
class PLAYSEED_API UReferenceTravelSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    void ArriveAtPainting() { bEntranceFlow=true; State=EReferenceTravelState::AtPainting; }
    bool BeginPainting();
    bool FinishPainting();
    bool BeginCourse();
    bool FinishDrop();
    bool BeginReturn();
    bool IsTransitionLocked() const;
    bool UsesEntrance() const { return bEntranceFlow; }
    EReferenceTravelState GetState() const { return State; }
private:
    EReferenceTravelState State=EReferenceTravelState::InCourse;
    bool bEntranceFlow=false;
};
