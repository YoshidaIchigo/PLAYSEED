# PR #42 実ゲーム検証

対象コード: d822eaf27fc395f17845dccaac7dd78a26e59857

実際に金100枚を収集して最後のコイン付近のランタン7へ歩いて触れた Validation_PR42_Hundred_C を、別ゲームプロセスからそのまま読みます。報酬7だけが一個あること、合計0と一時報酬なしの新しい訪問、Rでの獲得保持、再入場とGardenを確認します。保存fixtureや複数セーブの合成は使いません。通常セーブの前後指紋も記録します。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 702
UTC: 2026-10-16T02:21:04.331Z

```

[実ゲーム動画](ReadRestart.mp4) / [完了画面](RestartComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 46.6, "frames": 700, "bytes": 9116174}

SHA-256: 01839ac18c5246e4d863d31a7fa6d8ea15c30ed04d3244a328ba95529684ab84

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。