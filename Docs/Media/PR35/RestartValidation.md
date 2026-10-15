# PR #35 実ゲーム検証

対象コード: ba4a66d5f5eef858c85f14b86ac2a7c3f88085b7

最初の検証で山頂ボスを実際に攻略してランタン1を取った Validation_PR35_Boss_B を、そのまま別のゲームプロセスから読み直しました。ProgressWriteTest等のfixtureは使いません。新しい戦闘状態・取得済み1だけ・Rでの保持・ミッション2への新規World再入場を確認し、Gardenの通常収集とゴールを続けます。通常セーブの前後指紋は別途記録します。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 707
UTC: 2026-10-15T22:57:00.235Z

```

[実ゲーム動画](ReadRestart.mp4) / [完了画面](RestartComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 46.933, "frames": 705, "bytes": 9266686}

SHA-256: 711b74f67ec8e4e9be45ff066bddbaf7db594021731196d3fe85952c9817e5c1

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。