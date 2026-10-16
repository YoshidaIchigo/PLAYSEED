# PR #67 実ゲーム検証

対象コード: 92a8b8fabe1794610a724be4bb5a5d47a1821d5f

最終ソース92a8b8fabe1794610a724be4bb5a5d47a1821d5f、Editor7444/Game Development両ビルド成功。実Cは2406フレーム、UTC2026-10-16T17:32:44.360ZでPASS。通常歩行で地点2の実水玉に接触してHP8→7/押し戻し、Rで全3地点を初期化/HP8/入口へ復帰。同地点の次の水玉を普通の移動で避けながらカメラで振り返り、実スイープ接地からの2回の跳ね返り/実床に追従する影/破裂を確認。床から120cm以上上がることを確認し、その後も実被弾合計1/HP8を維持、R2回目で全3地点を初期化しMで絵前へ帰還。QAから位置・速度・体力・コイン・進行を設定していません。実7スター専用セーブhashは前後で。通常セーブはABSENTで作成/変更していません。

## 実行結果

```text
PASS
Three reference rain spawners at original positions/ranges: ordinary walking into a real falling drop caused one damage and physical knockback, normal R restored health and cleared all three, ordinary movement left the next real shadow before the actual burst with no extra hit, second R cleared all rain and M returned to painting with actual earned7 unchanged. No injected position, velocity, health or progress.
CaptureFrames: 2406
UTC: 2026-10-16T17:32:44.360Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 160.4, "frames": 2406, "bytes": 26395269}

SHA-256: 88d3de6283815b619944c61e0efc26c0e18dcf7a158455beda1f4e0275ab6550

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。