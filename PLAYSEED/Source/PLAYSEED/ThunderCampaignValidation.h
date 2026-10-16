#pragma once
#include "CoreMinimal.h"
#include "ThunderExpeditionSubsystem.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

// 実攻略の通し確認専用です。既存の保存を読むだけで、報酬や能力を付与する処理を持ちません。
inline bool ThunderCampaignEnabled() { return FParse::Param(FCommandLine::Get(), TEXT("ThunderCampaignTest")) || FParse::Param(FCommandLine::Get(), TEXT("ThunderCampaignReadTest")); }
inline TArray<int32> ThunderCampaignPrefix(int32 Target)
{
    switch (Target) { case 1: return {}; case 2: return {1}; case 3: return {1, 2}; case 5: return {1, 2, 3}; case 6: return {1, 2, 3, 5}; case 4: return {1, 2, 3, 5, 6}; default: return {}; }
}
inline bool ThunderHasExactRewards(const UThunderExpeditionSubsystem* Progress, const TArray<int32>& Expected)
{
    if (Progress->GetEarnedRewardCount() != Expected.Num()) return false;
    for (int32 Id : Expected) if (!Progress->HasEarnedReward(Id)) return false;
    return true;
}
struct FThunderCampaignBaseline
{
    bool Begin(const UThunderExpeditionSubsystem* Progress, int32 Target, bool bRead)
    {
        Prior = ThunderCampaignEnabled() ? ThunderCampaignPrefix(Target) : TArray<int32>{};
        if (ThunderCampaignEnabled() && bRead && Target == 4) Prior.Add(7);
        TArray<int32> Expected = Prior; if (bRead) Expected.Add(Target);
        return ThunderHasExactRewards(Progress, Expected);
    }
    bool HasPrior(const UThunderExpeditionSubsystem* Progress) const { for (int32 Id : Prior) if (!Progress->HasEarnedReward(Id)) return false; return true; }
    int32 BeforeCount() const { return Prior.Num(); }
    int32 AfterCount() const { return Prior.Num() + 1; }
    TArray<int32> Prior;
};
