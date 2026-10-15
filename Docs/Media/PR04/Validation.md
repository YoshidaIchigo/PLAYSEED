# PR #4 実ゲーム検証

対象コード: 9c2ed4d721d300af5ca892362c08123548570811

変更した録画処理をセルフレビューしました。GameMode の EndPlay による終了処理を外し、GameInstanceSubsystem が Viewport の描画コールバックと AVI writer を所有します。二重開始では既存ファイルを維持し、終了とゲーム終了時のフォールバックでコールバック除去・Finalize・解放を行います。レベルごとにリセットされる World 時刻を録画時計に使わないようにしました。不要な末尾空行を修正しました。

Editor と Game の Win64 Development ビルドを実行しました。実ゲームで3秒録画した後に OpenLevel で同じマップを再読込し、新しい GameMode が既存セッションと録画済みフレームを引き継いだことを確認します。その後、既存の落下復帰、入力割当、通常移動と6回のジャンプによる全6収集、ゴール、Rによる再開の回帰を実行します。

入力は Unreal の自動入力・CharacterMovement を使用します。人によるキーボード・ゲームパッド操作は未検証です。レベル再読込は録画寿命の確認用で、収集ルートは地形を歩き、ジャンプして進みます。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 428
UTC: 2026-10-15T14:10:03.297Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 28.333, "frames": 426, "bytes": 5175381}

SHA-256: c8448ca184abf22f5d25eaa7e303812f744ab6b73c7c007a2469968b0ba20ef8

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。