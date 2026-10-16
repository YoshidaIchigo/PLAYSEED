# PR #68 実ゲーム検証

対象コード: 724e6e21d766a7e25e04c95dadcbde4aac797d5e

ソース724e6e21d766a7e25e04c95dadcbde4aac797d5e、Editor7447/Game Developmentの両ビルド成功。最終Cは2677フレーム/UTC2026-10-16T18:13:18.113ZでPASSしました。

普通の歩行で野原側へ接近し、目的1の実台座・砲身と3秒以上の非発射を確認。M→絵→目的6→1000cmの物理落下で再入場し、山道の歩行と実ジャンプで山側へ到達。通常のカメラを横60度/上5度へ向け、砲口を1000cm以上離れた実水玉を撮影しました。自動照準と2発以上の実発射、観察中の実体力不変、Rで水玉初期化/HP8/入口、Mで絵前7まで確認しています。

A/Bの全編実検証もPASSしましたが、中央の体力表示と水玉が重なったため、通常カメラのみを調整してCを最終証跡としました。A/Bの動画は作業領域へ保管しています。ゲームの発射・当たり判定の挙動は同じです。

位置・速度・体力・進行はQAから直接設定していません。実7スターの専用セーブSHA-256は前後で6691511A3066D8C2DBE962B5CD0CB5A727F93A009FC3B81F71D28E57F329B683のまま、通常セーブはABSENTです。最終コード/資産による新しい同一セーブの全7通しは後続です。


## 実行結果

```text
PASS
Actual first-mission field water cannon visible without firing; M/painting/mission6 physical reentry, ordinary mountain walk and real gap jump, active mountain cannon aimed and fired at least two intangible shots with unchanged observation health, R cleared its shot and returned to entrance, M returned to painting with legitimate earned7 unchanged. No injected hero position, velocity, health or progress.
CaptureFrames: 2677
UTC: 2026-10-16T18:13:18.113Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 178.467, "frames": 2677, "bytes": 31155766}

SHA-256: beb81cccf8fced3c9f357a38c48972a2825c021d97ecec8516851c45c2e15d62

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。