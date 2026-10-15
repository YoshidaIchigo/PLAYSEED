# PR #34 実ゲーム検証

対象コード: a2a244c32a614fbc02cde0def5990b087911bea0

# 別プロセスでの飛行解放復元

初回の実際の塔攻略と急降下で作成したValidation_PR34_Tower_Aを、新しいゲームプロセスから読みました。報酬fixtureは付与していません。実際の花取得・飛行・解除・重力着地・R・再入場とGarden全ルートを確認しました。

```text
[2026.10.15-22.23.44:725][169]LogTemp: Display: TOWER TEST PASS: loaded actual saved wing unlock after process restart; actual flower pickup, powered flight, Space cancel, gravity landing and R persistence passed with zero rewards.
[2026.10.15-22.23.52:759][201]LogTemp: Display: TOWER REENTRY TEST PASS: saved switch and flower restored in a new World, rewards zero.
[2026.10.15-22.23.56:763][183]LogTemp: Display: SELECTION TEST PASS: mission 6, R, re-entry with mission 2, invalid numbers and Garden entry passed.
[2026.10.15-22.24.22:542][953]LogTemp: Display: WINDSEED TEST PASS: Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
```

通常セーブ照合:
```json
{
  "normal_after": "ABSENT",
  "normal_before": "ABSENT",
  "game_exit": 0,
  "slot": "Validation_PR34_Tower_A"
}
```

## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 933
UTC: 2026-10-15T22:24:22.541Z

```

[実ゲーム動画](ReadRestart.mp4) / [完了画面](RestartComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 62.0, "frames": 931, "bytes": 12244266}

SHA-256: f8282508358be23719c6c4eaba08694e8d41df95b00563b020c82cf0368075ec

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。