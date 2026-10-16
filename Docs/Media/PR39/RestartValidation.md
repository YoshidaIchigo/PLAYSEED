# PR #39 実ゲーム検証

対象コード: 1e46ba37e14bb007a9a1640196a831ffae6a3e4b

実際の中央5枚を飛行で取得し、浮島へ着地してランタン5へ歩いて触れた Validation_PR39_Chime_B を別プロセスからそのまま読みます。報酬fixtureは使いません。保存済み5と実解放、訪問初期化された5枚と輪、R、新しいWorld再入場、Gardenを確認します。通常セーブの前後指紋を保存します。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 705
UTC: 2026-10-16T00:45:19.713Z

```

[実ゲーム動画](ReadRestart.mp4) / [完了画面](RestartComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 46.8, "frames": 703, "bytes": 9279947}

SHA-256: bf3b4dccd9de6edb2d882e18e08026a603c22c191921210929baf97663deeade

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。