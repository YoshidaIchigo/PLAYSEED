# PR #28 実ゲーム検証

対象コード: 631a8e5256b0a5483885b641fe7e1a1418c6eb2c

- 共通の敵Actorから、赤い芽・導火線表示・爆発表示を持つActorを派生する。450cm以内で2秒の導火線を開始し、半径300cmとカプセルの軸の距離で一度だけ爆発を判定する。
- 基底Actorは見た目の色を仮想関数で受け取り、描画部品そのものを派生へ露出しない。共通の撃破ガードを使い、攻撃や踏みつけで先に倒されると導火線を止める。
- 爆発済み・撃破・表示を先に確定し、体力Componentへ被弾を最後に依頼する。死亡から再開した場合に古い状態を後から書き戻さない。Rでは導火線、爆発表示、姿勢、体力を戻す。
- 検証は通常歩行で範囲外から近づき、導火線と実際の爆発による2点被弾・押し戻し・一度だけの判定を確認する。R後は同じ接近から通常歩行で退避し、無被弾を確認する。
- さらにR後に先制のLMBで倒し、2秒を過ぎても爆発しないことを確認する。もう一度R後に導火線中のRを試し、2秒を過ぎても古い爆発が残らず体力8/8であることを確認する。
- 検証側から座標・体力・導火線・爆発結果を書き換えない。ミッション再入場とGardenの全6種収集・ゴール・Rも回帰確認する。
- 実ゲームの自動操作であり、人による入力機器の手動検証ではない。
- 最初の検証は、範囲外の550cm地点へ向かう途中で経路が敵から400cmの位置を横切り、導火線を先に起動して失敗した。道中も600cm以上離れる側へ接近経路を修正し、待機・被弾・退避の条件を維持した。失敗時の生録画を保存する。

実行ログの確認結果:
```text
[2026.10.15-20.26.34:995][487]LogTemp: Display: BURST TEST PASS: proximity two-second fuse, one two-damage blast, normal retreat avoids radius, melee cancels fuse, R during fuse prevents delayed blast, four resets and recovery passed.
```


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 1530
UTC: 2026-10-15T20:27:13.516Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 101.8, "frames": 1528, "bytes": 19716322}

SHA-256: 36f3345464dc78ad13783a3a40bd797b62ecb1138a1e2e2bb3f8a030352c87b9

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。