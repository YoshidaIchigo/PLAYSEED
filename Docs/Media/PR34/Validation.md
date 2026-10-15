# PR #34 実ゲーム検証

対象コード: a2a244c32a614fbc02cde0def5990b087911bea0

## セルフレビュー

Draft作成後に、8段の高さと間隔・塔と既存の経路の衝突、スイッチ上面だけの急降下判定、弱参照による着地通知の購読と解除、保存失敗時のロールバック、既存Version1での追加フラグ初期値false、Rと再入場での永続状態の復元、解放前の花の非表示と取得拒否、実際の前提攻略による飛行検証を確認しました。

新しいValidation_スロットで「案内役→未解放の花へ実際に歩く→R→通常ジャンプで塔を攻略→パンチ・普通のジャンプ着地では未解放→急降下で解放→重複急降下→R→花を実際に取る→飛行・着地→R」を実行します。続けて飛行能力の45秒失効・再取得・発射・着地まで回帰します。別プロセスで同じ保存を読み直し、実際の取得と飛行を確認しました。通常セーブを前後照合し、報酬fixtureを付与しません。

塔が従来の水弾テストの徒歩経路に重なるため、塔の北・東を通る5地点へ調整しました。水弾の警告・着弾・被弾・退避・Rの条件を変えずに回帰します。全実行でGardenも全ルートを回帰します。

## 塔と飛行の実動作

```text
[2026.10.15-22.18.29:501][950]LogTemp: Display: TRAVERSAL TEST PASS: ThunderWingTower, all 10 waypoints, 8 ordinary jumps, no recovery.
[2026.10.15-22.18.32:491][487]LogTemp: Display: WING SWITCH: actual ground pound saved leafwing unlock.
[2026.10.15-22.18.51:408][758]LogTemp: Display: TOWER TEST PASS: locked pickup rejected, ordinary jump/punch/landing excluded, tower climbed with normal jumps, top ground pound saved unlock, duplicate pound ignored; actual flower pickup, powered flight, Space cancel, gravity landing and R persistence passed with zero rewards.
[2026.10.15-22.19.07:287][498]LogTemp: Display: FLIGHT SPEED: W peak 1222.8 current 1222.0 at X=-2561.881 Y=865.442 Z=1213.821
[2026.10.15-22.20.13:266][780]LogTemp: Display: FLIGHT TEST PASS: actual bloom pickup, ordinary jump and air Space, mouse steering loop, W/S speed, actual45s expiry and gravity landing, reacquisition, cannon-powered flight, real floor landing and R cleanup passed; orbit waypoints 16.
[2026.10.15-22.20.21:272][828]LogTemp: Display: TOWER REENTRY TEST PASS: saved switch and flower restored in a new World, rewards zero.
[2026.10.15-22.20.21:272][828]LogTemp: Display: GUIDE REENTRY TEST PASS: saved launcher is open in a newly created World; quest rewards remain zero.
[2026.10.15-22.20.51:059][626]LogTemp: Display: WINDSEED TEST PASS: Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
```

通常セーブ照合:
```json
{
  "game_exit": 0,
  "slot": "Validation_PR34_Tower_A",
  "normal_after": "ABSENT",
  "normal_before": "ABSENT"
}
```


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 2916
UTC: 2026-10-15T22:20:51.059Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 194.2, "frames": 2914, "bytes": 35822054}

SHA-256: 1f3a0316773f7bcdbbbad43893f4666aa2a877a33a8d4761ab27fe5d3f1759a1

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。