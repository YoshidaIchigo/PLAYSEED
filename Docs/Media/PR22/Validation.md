# PR #22 実ゲーム検証

対象コード: 3812dc7b591a6633027b8b91533973f0c6a013d0

- World所有のCoin Actorは描画円盤と収集球を所有し、接触をGameMode所有のLedgerへ渡す。GameModeは入場で作り直されるため、永続SaveGameへコインを混ぜない。
- 金1、赤2、青5。空ID・未知の種類・取得済みIDを拒否し、値を更新した後で将来のミッションへ通知する。Actor側も先に取得ガードを立て、通知中の重複を防ぐ。
- 非衝突の見た目だけ回転・上下する。収集は実際のPawnとのOverlapで受け取り、生成時に既に重なる場合も扱う。取得済み球は通過可能なOverlapのまま維持し、Rで再有効化した瞬間の再計上を避ける。
- 通常歩行で三個を取得し合計1→3→8・赤1個を観察する。同じ場所を歩き直しても増えず、金の位置でRを押すと0へ戻り全Actorが再表示される。その後青だけを取り直して5点を確認する。
- 再入場で集計0・三種類の復活、保存済みランタンが不変、Garden全6種収集・ゴール・Rを確認する。検証側から集計や取得結果を直接書かない。
- 実ゲームの自動操作であり、人による入力機器の手動検証ではない。

## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 806
UTC: 2026-10-15T19:03:26.076Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 53.533, "frames": 804, "bytes": 10729490}

SHA-256: eee4edd475b10d79d9a8daef86ab8a5cb0ea49a1346c4c2d9615cd8ec225175a

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。