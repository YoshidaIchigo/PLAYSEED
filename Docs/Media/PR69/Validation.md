# PR #69 実ゲーム検証

対象コード: e01921e5923a316e6739862559dd259be46e22c1

ソースe01921e5923a316e6739862559dd259be46e22c1、Editor7451/Game Developmentの両ビルドに成功。最終Dの実ゲーム検証は5236フレーム/UTC2026-10-16T19:07:26.220ZでPASSしました。

普通の歩行で閉じた実格子に止められ、実紫スイッチに立つと2枚が1回だけ横へ開放。普通に通過し、Rで再閉鎖/再作動とHP8/入口を確認しました。再閉鎖した柵を避けて西側へ普通に歩き、西→野原/野原→西を実転送。2.2秒立ち止まっても戻されず、実退出/再接触で逆方向が作動しています。

M→絵→目的6→物理落下で再入場し、普通に山道を登って山下→山上へ転送。実被弾後に自作の回転ハートへ接触し、通常の回復処理が4回成功して最後の実体力が8になりました。ハートから普通に離れて撮影し、普通の歩行/Spaceで洞窟の段差を越えて山上→山下、到着待ち、Mで絵前7まで帰還しました。

実転送ログ:
- 回数1/出口1: X=1980.000 Y=-6618.000 Z=768.000
- 回数2/出口0: X=-6612.000 Y=3351.000 Z=1024.000
- 回数1/出口1: X=1680.000 Y=5523.000 Z=3835.000
- 回数2/出口0: X=583.000 Y=5387.000 Z=2683.000

位置・速度・体力・進行をQAから直接設定していません。実7スター専用セーブSHA-256は前後で6691511A3066D8C2DBE962B5CD0CB5A727F93A009FC3B81F71D28E57F329B683のまま、通常セーブは前後ともABSENTです。A/Bは閉じた柵を横切る旧検証経路、Cは戻りの洞窟入口で止まったため、通常の迂回とSpaceへ修正しました。ゲーム側の衝突を弱めたり、主人公を移したりしていません。

最終コード/資産による新しい同一セーブの全7通しは、残る敵と素材・モデル・飛行姿勢の調整後です。


## 実行結果

```text
PASS
Reference double grill physically blocked normal walking, actual floor switch opened both halves once, ordinary crossing and R closed/rearmed it. Ordinary walking triggered both directions of both actual warp pairs with stable arrivals and physical exit/reentry; actual damaged health recovered through rotating-heart contact/spins. M/painting/mission6 physical reentry and final painting-front earned7 preserved. No injected hero position, velocity, health or progress.
CaptureFrames: 5236
UTC: 2026-10-16T19:07:26.220Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 349.067, "frames": 5236, "bytes": 46970715}

SHA-256: 3f92b16398b0786c41fa4c3ed3bf3710522e4aee8abd2d31c19826bac8062236

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。