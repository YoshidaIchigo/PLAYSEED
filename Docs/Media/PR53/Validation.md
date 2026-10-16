# PR #53 実ゲーム検証

対象コード: db28b1c2a40f6700527a9abb6c9dd76d2b6df20d

# PR53 Draft後のセルフレビュー

対象ソース: db28b1c2a40f6700527a9abb6c9dd76d2b6df20d

- 保存成功または既に保存済みのIDだけが演出へ進む。保存失敗は報酬をロールバックし、Actorを表示したまま離脱・再接触で再試行する。
- 取得Actorは演出開始前に収集済みとなり、同フレームの別Actorも共通の遷移ロックで止まる。表示用スターはID0で報酬を付与しない。
- 全ID1〜7が同じ演出と入口帰還を通る。100枚ボーナスを例外にする旧経路を除去した。
- GameInstanceはWorld Actorを弱参照し、入口到着で破棄済み参照を解放する。入口からの再入場で前回表示を消す。
- 保存を先に完了し、入力・ダメージ・死亡をロックする。落下中は通常の重力を継続してからポーズへ進む。通常プレイの物理値は変えない。
- カメラの壁検査はHeroを除外し、6方向から選ぶ。参照動画03:47.750を確認し、HUDを残す・赤いカメラ×・左手の小さな星・右手のピースに修正した。
- QAは新規Validationスロットを要求し、実際のM帰還と再入場・落下を2回行う。新地形を歩き、Space/Ctrlで杭3回、実際のChomp Sweep、実スター接触と保存を使う。座標・速度・体力・報酬の注入はしない。
- QAは演出中の入力連打、姿勢・位置・体力・実カメラ・一度だけの取得、入口位置、再入場した選択画面の獲得表示まで確認する。
- Editor/Game build7404成功。主検証1511フレーム、別プロセス読み取り222フレームでPASS。実セーブのSHA-256不変と通常セーブABSENTを確認。詳細は[セルフレビュー](SelfReview.md)と[読み取り検証](ReadValidation.md)に記録。

残る範囲: 新地形の他目的物と通常起動切替、全7取得、実保存失敗/再試行、実死亡を起こす検証。参照画像との比較で初期選択UI・芝/壁・Chompの外観差分も残る。PR53を全体完成とは扱わない。


## 実行結果

```text
PASS
Actual M return and painting reentry, second physical drop, reference post3 and gate collision, actual star6 save, pose/camera/held star, transition input spam and duplicate guard, automatic painting return, earned6 menu display persisted.
CaptureFrames: 1511
UTC: 2026-10-16T07:59:48.179Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 100.734, "frames": 1511, "bytes": 22024750}

SHA-256: 8cbd72d00041d015f8c143289f62a65952bae17c629da7207d1205d9a1540a9a

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。