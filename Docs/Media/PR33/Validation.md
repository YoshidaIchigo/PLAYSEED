# PR #33 実ゲーム検証

対象コード: 96b063de6ff047fdcd622d6f3a9f0f648ced5057

## セルフレビュー

Draft作成後に、能力の残り時間と一時状態の所有、翼のComponentとGC、通常のカプセル衝突、飛行方向とカメラの分離、全解除経路での移動・カメラ復元、発射のPendingLaunch反映後にFlyingへ移る順序、床への実際の接触、再取得の重複防止を確認しました。

検証は新しいValidation_スロットで会話解放から始め、通常歩行で花を取ります。通常ジャンプと空中Spaceで飛行し、実際のマウス軸で周回・上昇下降し、W/Sの速度差を確認しました。タイマーを直接変更せず45秒の失効まで飛行を続け、その後の重力落下と着地・設定復元を確認しました。歩いて再取得し、発射台からの飛行と床への着地、Rでの能力消去と花の復元、Garden全ルートを回帰します。通常セーブを前後で照合します。

初回の検証で2秒後の単一点速度の判定が失敗しました。目標に到達した後の旋回と重なる可能性を経路から除くため、十分遠い上昇目標へ変更し、実測ピーク1100cm/秒以上を記録して通常速度1000との差を確認する手順へ修正しました。失敗録画と専用セーブは保存し、新しい専用スロットで再実行します。

## 実動作結果

```text
[2026.10.15-21.58.43:926][446]LogTemp: Display: GUIDE TEST PASS: unlocked through dialogue into a fresh isolated slot Validation_PR33_Flight_B; normal walking, distant interaction rejection, actual G/D-pad dialogue, idempotence, launcher visual and R persistence with zero quest rewards passed.
[2026.10.15-21.58.59:836][223]LogTemp: Display: FLIGHT SPEED: W peak 1222.7 current 1221.4 at X=-2556.121 Y=857.959 Z=1213.387
[2026.10.15-21.59.42:242][396]LogTemp: Display: FLIGHT TEST: phase 7
[2026.10.15-21.59.43:467][588]LogTemp: Display: FLIGHT TEST: phase 8
[2026.10.15-22.00.06:046][467]LogTemp: Display: FLIGHT TEST PASS: actual bloom pickup, ordinary jump and air Space, mouse steering loop, W/S speed, actual45s expiry and gravity landing, reacquisition, cannon-powered flight, real floor landing and R cleanup passed; orbit waypoints 16.
[2026.10.15-22.00.18:074][430]LogTemp: Display: SELECTION TEST PASS: mission 6, R, re-entry with mission 2, invalid numbers and Garden entry passed.
[2026.10.15-22.00.43:946][122]LogTemp: Display: WINDSEED TEST PASS: Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
```

通常セーブ照合:
```json
{
  "slot": "Validation_PR33_Flight_B",
  "game_exit": 0,
  "normal_before": "ABSENT",
  "normal_after": "ABSENT"
}
```


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 2071
UTC: 2026-10-15T22:00:43.946Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 137.867, "frames": 2069, "bytes": 25244376}

SHA-256: a43c8304094326f9e4c940568e159c275226bed2b7d68ab89ea183fa5278feec

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。