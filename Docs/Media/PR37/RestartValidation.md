# PR #37 実ゲーム検証

対象コード: 110ea9dfb6ba078622774d5dfb652da6bb009736

地上から通常歩行で発射台へ行き、実際に飛んで浮島へ着地し、箱を攻撃して出たランタン3へ歩いて触れた Validation_PR37_SkyCache_A を、別プロセスからそのまま読みます。保存APIのfixtureは使いません。獲得済み3だけ・未破壊の新しい箱・R・新規World再入場とGardenを確認します。通常セーブの前後指紋を保存します。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 701
UTC: 2026-10-15T23:24:07.276Z

```

[実ゲーム動画](ReadRestart.mp4) / [完了画面](RestartComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 46.533, "frames": 699, "bytes": 9111093}

SHA-256: 280f4e1e749e89884a94d4f38263adcbb55e499dfbdfed2e6bec0a83fbf512a5

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。