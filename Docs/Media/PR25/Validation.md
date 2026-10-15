# PR #25 実ゲーム検証

対象コード: c6ff38aacebc9df3e4d41ec37f93ac5c17385250

- World所有の岩球Actorが山道の足元19→3の経路を受け取り、半径70cm分だけ上に置いて450cm/秒で下る。移動距離に合わせて描画用の石と琥珀色の筋を回転する。描画メッシュでプレイヤーを物理的に押さず、接触だけを体力Componentへ依頼する。
- 移動前後の線分とカプセルの軸の最短距離で接触を判定する。短い区間が同じフレームで終わっても残り距離を次の区間へ渡す。終点で消え、1.5秒後に戻る。出現点からカプセルまで240cm以内の間は待ち続ける。
- Rで進行・待機・位置・描画回転を戻す。致命傷で同じTick中に再開したときは、初期化の世代を照合して古い位置を上書きしない。
- 検証は三度の本当のトゲ接触で体力2にし、徒歩で山道へ行って岩球の致命傷・一度だけの死亡・再開後の岩と開始位置を確認する。その後もう一度下側で実接触し、2点の被弾と押し戻し、Rを確認する。
- 道の端を通常歩行で登り、出現点を占有したまま待機期限後に二秒待つ。岩が出現しないことを確認し、さらに上側へ歩いて離れると一度だけ再出現し、無被弾であることを確認する。座標・体力・岩の進行・被弾結果を検証側から書かない。
- ミッション再入場とGardenの全6種収集・ゴール・Rも回帰確認する。山道の中央の100コイン経路には障害が加わるため、以降の統合収集では通常ジャンプなどで回避する。
- 実ゲームの自動操作であり、人による入力機器の手動検証ではない。
- 岩球単体の確認後、Rで最初から草原80個と山道20個を続けて回収する。近づく岩球には通常Jumpを入力し、垂直速度を書かずに避ける。全100個・100点、予期せぬ復帰なし、Rと再入場まで回帰確認する。

- 最初の実動作では致命傷・通常接触・RはPASSしたが、外側を一直線に歩く検証が下側の洞窟側壁に当たった。ゲームの地形を削らず、洞窟区間は既存の中央の道、その先は外側を通る道順へ修正し、近づく岩には通常ジャンプを使う。無被弾・安全な再出現の条件は維持する。失敗時の生録画も作業フォルダへ保存する。

実行結果（100個の回収では通常ジャンプ4回、体力8/8を確認）:
```text
[2026.10.15-19.41.44:359][182]LogTemp: Display: BOULDER FATAL RESET TEST PASS: three real thistle hits, fatal rolling contact, one defeat and player/boulder positions remained reset.
[2026.10.15-19.41.55:469][ 67]LogTemp: Display: BOULDER TEST: real contact dealt two points and knockback; R restored health, start and path.
[2026.10.15-19.42.23:031][  8]LogTemp: Display: BOULDER TEST PASS: real rolling contact, two damage and knockback, R reset, normal walk up the edge, source occupied for two seconds without respawn, leave and one safe respawn.
[2026.10.15-19.43.27:809][673]LogTemp: Display: TRAVERSAL TEST PASS: ThunderCoinTrail, all 109 waypoints, 4 ordinary jumps, no recovery.
[2026.10.15-19.43.29:289][933]LogTemp: Display: COIN TRAIL RESET TEST PASS: R restored all 100 trail coins and total zero.
[2026.10.15-19.43.37:297][982]LogTemp: Display: COIN TEST: fresh re-entry cleared totals and restored all placed coin actors.
```


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 2527
UTC: 2026-10-15T19:44:07.012Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 168.267, "frames": 2525, "bytes": 30808379}

SHA-256: ff3e078c625b9edf297fb04f23b5edd2b5fb1eb0f78c44ad362f5b8ce8e747b1

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。