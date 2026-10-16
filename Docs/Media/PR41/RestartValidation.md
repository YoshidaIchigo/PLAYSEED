# PR #41 実ゲーム検証

対象コード: 1e28a685af0ef79e9d74359c73b402624e8a7d1a

通常のジャンプと急降下3回、番獣の実衝突による門破壊、奥のランタン6への通常歩行による取得を行った隔離スロット Validation_PR41_Post_G を、別のゲームプロセスからそのまま読みます。報酬fixtureやセーブの配列操作は使いません。保存済み6だけが一個あること、新しい杭と門が訪問の初期状態であること、Rで保存済み報酬を失わないこと、新Worldへの再入場とGardenを確認します。通常セーブの前後指紋も保持します。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 703
UTC: 2026-10-16T01:33:07.589Z

```

[実ゲーム動画](ReadRestart.mp4) / [完了画面](RestartComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 46.667, "frames": 701, "bytes": 9161338}

SHA-256: f2f6beedfa6883398bacb40680c9ce8bfe604f349c1591dadd4d1846a6fabe64

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。