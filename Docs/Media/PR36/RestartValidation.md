# PR #36 実ゲーム検証

対象コード: 3328761f66259c29e5eceda7be533fec4cc7dd0d

実際の競走に勝ち、出現したランタン2へ歩いて触れた Validation_PR36_Race_A を、別プロセスからそのまま読み直します。報酬fixtureは付与せず、獲得済み2だけ・新しい競走状態・R保持・新規World再入場とGardenを確認します。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 706
UTC: 2026-10-15T23:13:26.864Z

```

[実ゲーム動画](ReadRestart.mp4) / [完了画面](RestartComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 46.867, "frames": 704, "bytes": 9251835}

SHA-256: 96fd222edeff04c8d9ec49ab4030cb1e8f2fa76aa7b90efe9a091fd0fe9b4a19

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。