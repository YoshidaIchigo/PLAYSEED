# PR #26 実ゲーム検証

対象コード: 8013004042e7b491e7a351887e1b9892aa800485

- World所有の水弾Actorが警告円と描画用の球を所有する。警告1.6秒、加速する落下、着弾後2秒の待機を別の状態で扱う。状態を先に着弾済みへ変えてから体力Componentへ通知し、同じ着弾で何度も被弾させない。
- 着弾位置からプレイヤーのカプセルの軸までの距離を照合する。範囲は半径200cmと身体の半径を合わせる。描画の球で身体を押さず、既存の被弾処理で2点と押し戻しを与える。
- Rで落下速度・高さ700cm・警告の時間・見た目を初期化する。外部の死亡・再開を呼ぶ可能性がある被弾通知を最後にし、古い着弾状態を後から戻さない。
- 検証は西側の地上から歩いて南へ行き、警告を撮影する。撮影の停止時間を退避の有効時間へ混ぜず、次の本当の周期から円内へ歩く。実際の着弾で一度だけ2点被弾・押し戻しを確認する。
- 次の落下中にRを押し、古い着弾を中断し警告・体力が戻ることを確認する。もう一度歩いて戻り、警告中に円へ入ってから外へ退避し、同じ着弾でも無被弾であることを確認する。
- 座標・垂直速度・周期・被弾結果を検証側から書かない。既存の100個ルートは北側、今回の水弾は南側で経路を交差させない。ミッション再入場とGardenの全6種収集・ゴール・Rも確認する。
- 実ゲームの自動操作であり、人による入力機器の手動検証ではない。
実行ログの確認結果:
```text
[2026.10.15-19.55.58:451][612]LogTemp: Display: WATER TEST: one real landing dealt two damage and knockback; R during next falling cancelled impact and restored warning and health.
[2026.10.15-19.56.20:353][582]LogTemp: Display: WATER TEST PASS: visible warning, falling, one landing, real two-damage knockback, R while falling, enter then retreat during warning without damage.
```


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 1455
UTC: 2026-10-15T19:56:59.109Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 96.8, "frames": 1453, "bytes": 18990760}

SHA-256: e0dd71d57647d187457498a2671175ca6a1a0251eae0a39a285ac9dd76c71e5b

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。