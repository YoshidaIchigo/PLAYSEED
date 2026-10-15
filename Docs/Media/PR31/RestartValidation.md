# PR #31 実ゲーム検証

対象コード: ac321667d4bcec2371861fccb4306e28f6b7e9ec

# 別プロセスでの復元

初回の実際の会話で作成したValidation_PR31_Guide_Aを、終了後の新しいゲームプロセスで読みました。fixture報酬は付与していません。報酬0のまま解放済みの外観・会話・R・再入場を確認しました。

```text
[2026.10.15-21.23.12:731][284]LogTemp: Display: GUIDE TEST PASS: loaded after process restart isolated slot Validation_PR31_Guide_A; normal walking, distant interaction rejection, actual G/D-pad dialogue, idempotence, launcher visual and R persistence with zero quest rewards passed.
[2026.10.15-21.23.20:737][305]LogTemp: Display: GUIDE REENTRY TEST PASS: saved launcher is open in a newly created World; quest rewards remain zero.
[2026.10.15-21.23.24:745][329]LogTemp: Display: SELECTION TEST PASS: mission 6, R, re-entry with mission 2, invalid numbers and Garden entry passed.
[2026.10.15-21.23.50:665][143]LogTemp: Display: WINDSEED TEST PASS: Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
```

通常セーブ照合:
```json
{
  "slot": "Validation_PR31_Guide_A",
  "game_exit": 0,
  "normal_after": "ABSENT",
  "normal_before": "ABSENT"
}
```

## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 863
UTC: 2026-10-15T21:23:50.664Z

```

[実ゲーム動画](ReadRestart.mp4) / [完了画面](RestartComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 57.333, "frames": 861, "bytes": 11519922}

SHA-256: e80a1b23bc60182d5a2739e93ca8bf01c8dfa105a16642ed195ac165d71accf3

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。