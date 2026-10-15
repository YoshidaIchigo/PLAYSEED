# PR #2 実ゲーム検証

対象コード: 2dda0424541b48fb52a70b6d00e9483bbc6f8fd2

Editor（別名DLL 7316）とGameのWin64 Developmentビルドが成功しました。

セルフレビューでは、開始位置の責務がGameModeにあり、Characterは渡された位置を使うこと、既定の座標が変わっていないこと、復帰前に移動速度とジャンプ状態を止めていることを確認しました。

実ゲームでは、開始位置を一時的に`(-650,-350,100)`へ変更して意図的に落下させ、その場所へ復帰することを確認しました。その後に既存の開始位置を戻し、W・Shift・Space・マウス・ホイールの入力割り当て、通常の移動・ジャンプによる6個の収集、ゴール、リスタートを回帰確認しました。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 352
UTC: 2026-10-15T13:45:59.353Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 23.267, "frames": 350, "bytes": 4080556}

SHA-256: 937755e32abc2efc68e8f5b1293a71f24e58bcbae2ed440b685b953f49735f7c

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。