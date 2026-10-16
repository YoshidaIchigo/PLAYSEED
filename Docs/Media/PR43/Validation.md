# PR #43 実ゲーム検証

対象コード: f1f7aa35f0e926cc3a974aeda8fa4a35764ba390

Draft後セルフレビューを行い、f1f7aa35f0e926cc3a974aeda8fa4a35764ba390のEditor/Game7374が両方成功しました。
実取得済み全7個の同じValidation_PR42_Campaign_Aを継続。実イバラ2接触で体力4、通常歩行で山道中腹の花へ到達し、02:58:13.613 UTCの地上接触で6へ回復しました。6秒間立ったままでは追加回復なし。離れて戻ると02:58:20.905に8へ回復し、もう一度6秒待って往復しても満タンでは回復回数2のまま・待ち時間を消費しません。回復で無敵時間や移動モードを与えず、山道中に余分な被弾や復帰はありません。Rで花を準備状態と体力8に戻し、新しいWorldでは花の累計0・準備状態・実取得全7個の保持を照合しました。
Gardenの移動・カメラ・ジャンプ・落下復帰・全6個・ゴール・RまでPASS1363frames、UTC2026-10-16T02:59:07.726Z。通常セーブは前後ABSENT。全7個の実セーブも前後SHA256 41AA55C7B783F8F53B85306D29BC38003F71200C8DB8EDB546E2957DBEDB1D56で同一です。報酬fixtureや体力・座標の注入は使いません。自動入力による実ゲームで、人の入力機器テストとは区別しています。

## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 1363
UTC: 2026-10-16T02:59:07.726Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 90.667, "frames": 1361, "bytes": 17105610}

SHA-256: bc5aef675469686df1ec424c0ee71e30e3de74d2258792f4166a6fa68f72e833

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。