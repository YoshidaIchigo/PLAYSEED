# PR #70 実ゲーム検証

対象コード: 7353cb0d325165108ccfa81fb98a1fa1e99c5c11

ソース7353cb0d325165108ccfa81fb98a1fa1e99c5c11、Editor7453/Game Developmentの両ビルド成功。最終Bは990フレーム/UTC2026-10-16T19:47:30.878ZでPASSしました。

参照配置の11体が実床に存在し、通常歩行で実追跡と1ダメージ/押し戻しを確認。普通のRで11体/HP8/入口を再開し、LMB1回で1体を撃破/1枚だけの実コインを生成して普通の移動で取得。2体目へ実床に沿って接近してから普通のSpaceで踏み、実ストンプ1回/実コイン1枚を取得しました。2回目のRで11体を生存状態、HP8/入口/実集計0、自分の落下コインを削除したことを確認し、Mで絵前7へ帰還しています。

主人公・敵の位置/体力/進行をQAから直接設定していません。実7スターの専用セーブSHA-256は前後で6691511A3066D8C2DBE962B5CD0CB5A727F93A009FC3B81F71D28E57F329B683のまま、通常セーブはABSENTです。

Aは1150フレームで2体目の踏みつけ操作に失敗。記録と地形を確認し、実上面へ続く48点の細かな経路、実床の高さ、同じ高さからの通常Spaceと必要な再試行へ修正しました。ゲームの衝突を弱めたり、主人公や敵を移したりしていません。

最終コード/資産の新規同一セーブ全7通しは、残る爆弾・素材/モデル・飛行姿勢・必要な経路/100枚集計の修正後です。


## 実行結果

```text
PASS
Eleven reference walkers at two single/three triplet source positions. Ordinary approach caused one real contact damage and knockback, R restored all eleven. Ordinary LMB defeated one and collected its one actual gold; normal Space jump stomped another and collected its one actual gold. Second R restored health/entrance/score0, alive enemies and removed own drops. M returned to painting with legitimate earned7 unchanged. No injected hero/enemy position, health or progress.
CaptureFrames: 990
UTC: 2026-10-16T19:47:30.878Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 66.0, "frames": 990, "bytes": 12096157}

SHA-256: 5eb1a728a76b0f9e4d5e0f9f3d63d5b661339ed716f9fb8f10f19560e803075f

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。