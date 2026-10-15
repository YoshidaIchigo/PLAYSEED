# PR #32 実ゲーム検証

対象コード: a71e14dcc5bbeffd1272669a55b092af09e6a3e5

## セルフレビュー

Draft作成後に、未解放や他アクション中の入場禁止、座席の衝突確認、MOVE_Noneの使用範囲、G・R・被弾・World終了時の解除、カメラの長さ・オフセット・追従・角度制限の保存と復元、攻撃入力の排他、発射の重複防止、発射後の通常重力と衝突、Actor弱参照を確認しました。

検証は新しいValidation_スロットで、実際の会話による解放から始めます。通常歩行で発射台へ到達し、照準中の移動抑止、マウスによるYaw変更とPitch上限、Gでの退出と再入場、照準中R、再入場後の一回の左クリック発射を確認しました。飛行中は移動入力も座標変更も行わず、連続した飛行距離・最高点・実際の浮島の床への着地を検査します。最後にRと新しいWorldへの再入場、Garden全ルートを回帰します。通常セーブを前後で照合します。

## 実動作結果

```text
[2026.10.15-21.37.03:483][548]LogTemp: Display: GUIDE TEST PASS: unlocked through dialogue into a fresh isolated slot Validation_PR32_Launcher_A; normal walking, distant interaction rejection, actual G/D-pad dialogue, idempotence, launcher visual and R persistence with zero quest rewards passed.
[2026.10.15-21.37.27:315][756]LogTemp: Display: SKYBLOOM: actual launch 1 from X=-2400.000 Y=1800.000 Z=140.000 with velocity X=874.909 Y=159.074 Z=2443.201
[2026.10.15-21.37.30:082][238]LogTemp: Display: LAUNCH LANDING: X=17.785 Y=2239.597 Z=1484.150; peak feet 2158.4; continuous flight distance 3964.7; velocity X=874.909 Y=159.074 Z=2443.201
[2026.10.15-21.37.31:085][407]LogTemp: Display: LAUNCH TEST PASS: actual fresh guide unlock and walking, G enter/exit, mouse angle/clamp and movement freeze, R during aim, one normal gravity launch, continuous flight and actual island landing, camera and R restoration passed.
[2026.10.15-21.37.39:117][397]LogTemp: Display: GUIDE REENTRY TEST PASS: saved launcher is open in a newly created World; quest rewards remain zero.
[2026.10.15-21.37.43:122][428]LogTemp: Display: SELECTION TEST PASS: mission 6, R, re-entry with mission 2, invalid numbers and Garden entry passed.
[2026.10.15-21.38.08:941][205]LogTemp: Display: WINDSEED TEST PASS: Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
```

通常セーブ照合:
```json
{
  "normal_before": "ABSENT",
  "normal_after": "ABSENT",
  "slot": "Validation_PR32_Launcher_A",
  "game_exit": 0
}
```


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 1283
UTC: 2026-10-15T21:38:08.940Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 85.333, "frames": 1281, "bytes": 16304518}

SHA-256: 3011b8afd5b247e5ea5d8690e7710c74a215a74b2601b6dd8c1006c15395eabe

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。