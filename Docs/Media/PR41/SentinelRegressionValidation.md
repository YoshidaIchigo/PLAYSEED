# PR #41 実ゲーム検証

対象コード: 1e28a685af0ef79e9d74359c73b402624e8a7d1a

杭と格子の門がある最終ソースで、前の番獣AIの実ゲーム検証を繰り返します。新しい隔離スロットから通常歩行で近づき、二回の実突進が300cmに制限されること、外側は無傷、内側の実接触は体力2、横移動での回避、予告中Rからの遅延攻撃なしとGardenの全6個・ゴール・Rを確認します。杭の攻略結果や体力を検証から直接設定しません。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 1256
UTC: 2026-10-16T01:35:51.622Z

```

[実ゲーム動画](SentinelRegression.mp4) / [完了画面](SentinelRegressionComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 83.533, "frames": 1254, "bytes": 15746047}

SHA-256: 43ad5f8abb81339a5c08611b80a08c53e4f1823c2d68ec3ae6f22f506b0fc6b4

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。