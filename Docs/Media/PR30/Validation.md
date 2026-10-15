# PR #30 実ゲーム検証

対象コード: 996f73e7cd8cb30cbc5cf47e23db9b36b88b35de

## セルフレビュー

Draft作成後に、乗車条件と他アクションとの排他、移動値の保存と全解除経路での復元、Shift入力の記憶、破棄済み弱参照への対応、葉だけのAttach、World終了時の参照、降車先の実際の床判定を確認しました。

検証では敵を攻撃して葉を落とすところから通常の入力で実行します。加速・入力を離した惰性・Qと方向パッドでの乗降・実際の山道の上昇下降・乗車中R・棘への接触被弾・通常ジャンプからの急降下・木への登り・場外へ滑走した落下を確認しました。結果や座標を注入しません。既存Gardenも全ルートを回帰します。

最初の検証はInputKeyを渡した同じフレームで乗車を判定していました。PlayerControllerが入力を処理した後のフレームで乗降を確認する状態遷移へ修正し、取得し直す経路でもQを一回だけ送るようにしました。失敗録画を保管し、判定条件は維持します。

## 実動作結果

```text
[2026.10.15-21.06.38:522][700]LogTemp: Display: GALELEAF TEST PASS: first punch and ordinary stomp each release one leaf while alive, flee and bounce, second attack drops one azure five, R cleanup and re-collection without duplicates passed.
[2026.10.15-21.07.59:070][644]LogTemp: Display: RIDE TEST PASS: actual carrier drop, Q/D-pad mount/dismount/remount, speed and coasting, slope ascent/descent, mounted R, real hazard, ordinary jump/pound, tree climb and actual offstage fall cleanup passed.
[2026.10.15-21.08.11:109][689]LogTemp: Display: SELECTION TEST PASS: mission 6, R, re-entry with mission 2, invalid numbers and Garden entry passed.
[2026.10.15-21.08.37:110][535]LogTemp: Display: WINDSEED TEST PASS: Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
```


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 2320
UTC: 2026-10-15T21:08:37.109Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 154.467, "frames": 2318, "bytes": 29248795}

SHA-256: f0ff0c27547ec73b0182a2508fda8d1d3f5cc54b771eff5bf831b991a21cd693

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。