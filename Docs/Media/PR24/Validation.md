# PR #24 実ゲーム検証

対象コード: 7e45090c3efa5f309724e272c26dfb0c1e59c042

- 草原の四つの花輪に各20個、山道の足元の順路の最初の20点に各1個を置く。金コイン100個はそれぞれ固有IDを持つ。元の訓練用三種類や箱の内容物とは別のActor配列で管理する。
- 現在のPLAYSEED地形に合わせた独自配置であり、参照映像に見える全コイン位置を復元したと主張しない。100点報酬は別の作業単位で追加する。
- Stageが実際に配置した草原の足元と山道の順路を検証へ渡す。検証は西側から北の草原へ歩き、四つの花輪を通り、山道を歩いて山頂へ到達する。ジャンプ・ワープ・座標変更・集計変更を使わず、100個すべてのActorが取得済み、合計100・赤0、予期せぬ復帰なしを確かめる。
- 山頂からRを押して合計0・全100個復活を確認する。再入場でも合計0・全配置復活を確認する。既存Gardenの全6種収集・ゴール・Rも回帰確認する。
- Actorの生成とUPROPERTYによる参照、足元とカプセル中心の高さの違い、各コインのID、GameMode側の入場集計と再開の役割を既存設計に沿わせた。
- 実ゲームの自動操作であり、人による入力機器の手動検証ではない。

実行ログの確認結果:
```text
[2026.10.15-19.20.32:425][144]LogTemp: Display: COIN TRAIL TEST PASS: all 80 meadow and 20 mountain coins collected by continuous normal walking, total 100, no recovery.
[2026.10.15-19.20.35:112][538]LogTemp: Display: COIN TRAIL RESET TEST PASS: R restored all 100 trail coins and total zero.
[2026.10.15-19.20.43:163][536]LogTemp: Display: COIN TEST: fresh re-entry cleared totals and restored all placed coin actors.
```


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 1602
UTC: 2026-10-15T19:21:13.151Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 106.6, "frames": 1600, "bytes": 19765909}

SHA-256: 37ac64aa26f405c59dac223be7b88f1c2e8eb04a318eb04112858958d9ea75f8

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。