# PR #35 実ゲーム検証

対象コード: ba4a66d5f5eef858c85f14b86ac2a7c3f88085b7

新しい専用スロット Validation_PR35_Carry_A を使用して、通常の持てる箱の回帰を実行します。番人専用の仮想関数を追加した後も、従来の持ち上げ・物理投擲・実着地・R・持っている時の被弾と手放しが同じ条件で通るかを確認します。通常の箱の持ち位置100/0/30と初速1100+450は変更していません。保存の報酬fixtureは使いません。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 849
UTC: 2026-10-15T22:58:34.233Z

```

[実ゲーム動画](CarryRegression.mp4) / [完了画面](CarryRegressionComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 56.4, "frames": 847, "bytes": 11243129}

SHA-256: ba8820f55148c71a7d00997a439e003f016c9fd3bc15b94b3a9591987c41c7ba

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。