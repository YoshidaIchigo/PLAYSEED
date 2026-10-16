# PR #66 実ゲーム検証

対象コード: 43390debc78f953c11feceade6a7ffc51d952e93

最終ソース43390debc78f953c11feceade6a7ffc51d952e93、Editor7441/Game Development両ビルド成功。最終Dは1568フレーム、UTC2026-10-16T16:43:04.724ZでPASS。実くぼ地と山道の鉄球接触でそれぞれHP8→6/押し戻し、Rで全5個・HP8・入口を2回再開してからMで絵前へ帰還。既存の実7スター専用セーブは保持しています。QAから位置・速度・体力・コイン・進行を注入していません。

動画レビューの灰色を黒へ修正した後、R直後にカメラが旧位置から長距離を追う不具合も見つけました。復帰時にSpringArmの前フレーム位置と向きの履歴を更新し、通常の追従を維持したまま復帰後の入口を映すように修正しました。実Rの後も0.35秒待ち、HP8・実入口を維持した状態を確認して録画しています。

くぼ地は実床に沿う自作の連続周回、山道は公開された18/27点を実床へ合わせる自作の下り加速です。形・素材は自作です。

## 実行結果

```text
PASS
Reference metal balls: two source mountain paths and three actual-floor pit loops; normal walking into a moving pit ball and actual mountain ball caused real damage/knockback, R restored hero and all five mechanisms twice, M returned to painting with legitimate earned1..7 unchanged. No injected hero position, velocity, health or progress.
CaptureFrames: 1568
UTC: 2026-10-16T16:43:04.724Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 104.534, "frames": 1568, "bytes": 17907728}

SHA-256: 591ff36bd95eebcb4e653fea155ba7b3af052ca3f00a6d0de6b74b65921d5534

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。