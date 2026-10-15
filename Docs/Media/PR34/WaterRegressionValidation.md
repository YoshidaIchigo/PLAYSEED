# PR #34 実ゲーム検証

対象コード: a2a244c32a614fbc02cde0def5990b087911bea0

# 塔追加後の水弾回帰

新しい塔の北と東を通る5地点の通常歩行で接近しました。警告・落下・一度の着弾・実際の2点被弾と押し戻し・落下中Rによる中断・警告後の徒歩退避の条件を変更せず確認し、Garden全ルートもPASSしました。

```text
[2026.10.15-22.27.00:951][343]LogTemp: Display: WATER TEST PASS: visible warning, falling, one landing, real two-damage knockback, R while falling, enter then retreat during warning without damage.
[2026.10.15-22.27.13:987][539]LogTemp: Display: SELECTION TEST PASS: mission 6, R, re-entry with mission 2, invalid numbers and Garden entry passed.
[2026.10.15-22.27.39:870][392]LogTemp: Display: WINDSEED TEST PASS: Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
```

通常セーブ照合:
```json
{
  "slot": "Validation_PR34_Water_A",
  "normal_after": "ABSENT",
  "game_exit": 0,
  "normal_before": "ABSENT"
}
```

## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 1459
UTC: 2026-10-15T22:27:39.869Z

```

[実ゲーム動画](WaterRegression.mp4) / [完了画面](WaterRegressionComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 97.067, "frames": 1457, "bytes": 18547893}

SHA-256: 7e89bae3060e9b4eaf021a85c6b826520fc83cf1740e1f36f2621c5c885b9595

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。