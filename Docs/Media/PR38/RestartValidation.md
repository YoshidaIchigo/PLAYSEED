# PR #38 実ゲーム検証

対象コード: 42bee322d8e0147051bac19f017ea22694824523

同じ入場で8個の対象を実際に集め、広場へ出たランタン4へ歩いて触れた Validation_PR38_Scarlet_D を別プロセスからそのまま読みます。報酬fixtureは使わず、実獲得4だけ、初期化された8枚と訪問中集計、R、新しいWorld再入場、Gardenの実収集とゴールを確認します。通常セーブの前後指紋を保存します。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 700
UTC: 2026-10-15T23:59:19.210Z

```

[実ゲーム動画](ReadRestart.mp4) / [完了画面](RestartComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 46.467, "frames": 698, "bytes": 9134712}

SHA-256: f9a4d9f775ad71dbac6543c749584e5076abfd6cfb3785e92b24268d497dd025

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。