#include "ThunderCoinLedger.h"
#include "PlayseedCoin.h"

bool UThunderCoinLedger::TryCollect(FName CoinId, EPlayseedCoinKind Kind)
{
    const int32 Value = APlayseedCoin::ValueOf(Kind);
    if (CoinId.IsNone() || Value == 0 || CollectedIds.Contains(CoinId)) return false;
    CollectedIds.Add(CoinId);
    Total += Value;
    if (Kind == EPlayseedCoinKind::Scarlet) RedIds.Add(CoinId);
    // 集計を終えてから通知します。後続のミッションも同じ取得結果を観察できます。
    OnCoinCollected.Broadcast(CoinId, Kind);
    return true;
}

void UThunderCoinLedger::ResetVisit()
{
    CollectedIds.Reset();
    RedIds.Reset();
    Total = 0;
}
