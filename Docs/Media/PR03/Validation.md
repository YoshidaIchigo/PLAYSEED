# PR #3 実ゲーム検証

対象コード: d24a1a4d7bf42ddda5491893f6b2df28e708c904

Editor（別名DLL 7317）とGameのWin64 Developmentビルドが成功しました。

セルフレビューでは、CharacterがWindseed GardenのActorを探索しないこと、現在のGameModeが自分のステージだけを初期化すること、共通クラスが復帰以外のステージ固有処理を持たないことを確認しました。直接使うWorld APIのincludeも明示しました。

実ゲームの回帰検証で、別の設定位置への落下復帰、W・Shift・Space・マウス・ホイールの入力、通常の移動とジャンプによる全6個の収集・ゴールを確認しました。最後はRキーのシミュレーション入力イベントを送り、Character→現在のGameMode→ステージ初期化→開始位置への復帰という経路で、収集数とゴールが初期化されることを確認しました。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 355
UTC: 2026-10-15T13:55:48.839Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 23.467, "frames": 353, "bytes": 4124901}

SHA-256: e22f3c4ddfd0e849a16c89535d504ff098c3c99430a541436df29a2a7c1dae19

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。