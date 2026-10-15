# PR #6 実ゲーム検証

対象コード: 13a84583466908536cfe5fd70e2f63f5e57e2dfb

セルフレビューで選択番号の範囲、6つの名称・目的文の対応、入場の連打防止、選択と達成の区別、Rで選択目的が消えないこと、再選択が次のWorldへ渡ることを確認しました。新規C++にはGameInstanceSubsystemを使う理由、Actorを所有しないこと、1始まりの番号、HUDの役割を日本語で説明しています。

実ゲームで1〜6の上端である6番を選び、移動・ジャンプ・R再開を確認し、退出後に2番を選び直して再入場しました。APIの0/7の拒否も確認しました。既存Gardenでは落下復帰、移動・走行・カメラ・ズームの入力、通常移動と6回のジャンプでの全6収集・ゴール・Rを回帰検証しました。人によるキーボード・ゲームパッド操作は未実施です。

この変更はミッション選択の入口です。選択しただけで報酬は加算せず、6主ミッションと100コイン報酬の達成条件は後続PRで個別に実装します。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 685
UTC: 2026-10-15T14:32:04.858Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 45.467, "frames": 683, "bytes": 7895861}

SHA-256: bf134cc51ed5d548edb4895916ec4fb628a59d644a849f82cdb43ffb0fd2d728

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。