# PR #27 実ゲーム検証

対象コード: e7d96cbdfd7660efb51cb7bebc3bc434ae54f50f

- 共通の敵Actorが接触・踏みつけ・撃破の一度だけの通知を担当し、派生Actorが待機・追跡・帰還の状態だけを追加する。近距離650cm・高さ・遮蔽物のない視線で発見し、離れすぎると追跡をやめて元の場所へ戻る。
- 地面の高さ・法線・35cm以内の段差と、身体の幅の球による壁の掃引を確認して地上を移動する。高い段差や崖へ歩き込まない。これは開けた草原用の簡単な地上AIで、経路探索全体を実装したものではない。
- 撃破通知の先頭でドロップ済みを立て、一体から一個の金コインだけを生成する。敵の名前から入場中の固有IDを作る。Rではその敵自身が生成したコインだけを片付け、共通の復活処理と状態を戻す。
- 検証は通常歩行で範囲外の待機を確認し、近づいて発見・180cm以上の実移動を観察する。走り去らず通常歩行で距離を離し、実際に元の場所へ戻って待機することを確認する。
- もう一度近づいてLMBで先に倒し、追加攻撃でも一個の生成に留まり、歩いて拾って1点になることを確認する。Rで敵・生成物・集計が戻った後に再び近づいて倒し、同じIDの新しい一個を1点として取得できることを確認する。
- 検証側から敵やプレイヤーの座標・AI状態・撃破結果・集計を書き換えない。ミッション再入場とGardenの全6種収集・ゴール・Rも確認する。
- 実ゲームの自動操作であり、人による入力機器の手動検証ではない。
実行ログの確認結果:
```text
[2026.10.15-20.09.56:989][664]LogTemp: Display: CHASE TEST: out-of-range idle, actual discovery and movement, ordinary retreat and completed return home passed.
[2026.10.15-20.10.11:376][276]LogTemp: Display: CHASE TEST PASS: discovery, chase, escape, return home, melee defeat and exactly one collectible gold drop, R cleanup, second chase/defeat/fresh one point passed.
```


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 1098
UTC: 2026-10-15T20:10:49.295Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 73.0, "frames": 1096, "bytes": 14414206}

SHA-256: 29f78a41d09f96eb7c81783b267d769351f53827d1a9b43cc36ddcd0edfbd3fd

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。