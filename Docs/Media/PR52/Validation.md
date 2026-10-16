# PR #52 実ゲーム検証

対象コード: a597cfe1404b02c810ff7e9f5d54950d4a4d34f1

Draft PR52後のセルフレビュー。座標は元の配置から(X,-Z,Y)へ変換し、格子の自作メッシュの原点が中央である分だけ200cm補正した。スター6は最初から奥に表示し、格子破壊で新たな報酬を重複生成しない。Rでは格子とその入場中の一つのスターを作り直す。参照版だけ900cm鎖・4金属球・3ダメージ・黒い球モデル・4跳ねを使い、従来設定は残す。身体の壁衝突と攻撃範囲を別に扱う。影は床を読むだけでプレイヤーを動かさない。鎖の終点が本体へ埋まりすぎていた点を直し、4球が見える位置へ調整した。QA順路は実衝突形状から計画した57点と12点で、通常のAddMovementInputとSpace/Ctrlだけを使う。3回の実ヒップドロップ、敵のSweepによる格子破壊1回、元位置のスター表示を確認し、取得前にMで戻る。QAから座標・速度・体力・報酬を設定しない。これから両ビルドと新規Chain_Aを実行する。Editor/Game7403で全nativeモジュールを再コンパイルし両成功。新規Validation_PR52_Chain_Cで絵への実ジャンプと落下、参照地形の57点の実歩行、Space/Ctrlで杭を3回踏み込む、解放後4跳ね、敵の実Sweepで格子破壊1回、12点の実歩行で元位置のスター6を表示、取得前のM帰還を確認。PASS1079フレーム UTC2026-10-16T07:15:11.693Z。通常セーブ前後ABSENT、進行0、死亡やR復帰なし。Draft後の実画像レビューで鎖終点、参照コースのモーションブラー、スター素材、確認カメラを修正し再実行した。星はHUDと重ならず金色で見える。今回スターは取得しない。


## 実行結果

```text
PASS
Painting entry, physical drop, ordinary walking to reference post, three real ground pounds, released Chomp physical sweep breaks gate once, visible original-position star6, stop before collection and M returns to painting. No recovery, no injected state, zero progress unchanged.
CaptureFrames: 1079
UTC: 2026-10-16T07:15:11.693Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 71.934, "frames": 1079, "bytes": 19206154}

SHA-256: 22fef9a6566158d4fec41f826abcf5a4b98dd9e63c5681c2a866142a1ce99b37

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。