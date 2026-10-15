# PR #5 実ゲーム検証

対象コード: 1e99fc920dc31c5829fb95a5b107d9d2420f52c7

セルフレビューでは、各マップに専用のGameModeを保存していること、入場要求の連打を防いでいること、雷鳴の丘でWindseedStageを作らないこと、共通キーMがプレイヤーの移動入力と重複しないことを確認しました。新マップの生成スクリプトは既存アセットの存在を確認してから開き、GameMode指定だけを更新します。起動マップ変更に合わせ、既存Gardenの検証コマンドには対象マップを明示しました。

2→雷鳴の丘でW/Space/R→M→1を、Unrealの入力イベントを使い実ゲームで実行します。追加ステージの専用開始位置、移動・ジャンプ・再開、GardenのActorが混在しないことを確認し、続いて既存ステージの全6収集・ゴール・R再開と移動・カメラ・落下復帰を回帰検証します。人による入力機器の確認は未実施です。

この単位の追加ステージは床と照明のみです。ゲーム完成や7ミッション完成を示すものではなく、後続の個別PRの入口を整える変更です。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 522
UTC: 2026-10-15T14:20:06.095Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 34.6, "frames": 520, "bytes": 6893688}

SHA-256: a93c36a835888b0dadb730c90e9e0824b1a862ece67125e3873bb2fd514622a9

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。