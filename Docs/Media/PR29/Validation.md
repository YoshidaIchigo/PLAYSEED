# PR #29 実ゲーム検証

対象コード: 26bf41822b5b946e40b4c204218087c5f01f232a

## セルフレビュー

Draft PR作成後、初回攻撃と撃破の分岐、葉と青コインそれぞれの一回限りの生成、World所有Actorの弱参照、Rでの自己生成物の破棄と復元、既存の共通踏みつけへの影響を確認しました。

実ゲームでは通常歩行、LMB、通常ジャンプだけで初回のパンチと踏みつけを別の周回で試し、各周回で追いかけて倒し、実際の青コイン回収5点と重複防止を確認しました。数値や座標の注入でクリア扱いにはしません。

実動作で見つかった初回踏みつけ後の接触被弾を修正しました。LaunchCharacterが次の移動Tickで反映されるまで0.3秒の側面接触猶予を設けます。猶予はRで消去されます。近距離追跡では通常アナログ入力の強さを0.4にし、長い描画フレームで敵へ突っ込まないよう検証経路も修正しました。失敗時の生録画は作業フォルダに保管しています。

## 実動作結果

```text
[2026.10.15-20.50.47:080][693]LogTemp: Display: GALELEAF TEST PASS: first punch and ordinary stomp each release one leaf while alive, flee and bounce, second attack drops one azure five, R cleanup and re-collection without duplicates passed.
[2026.10.15-20.50.59:119][683]LogTemp: Display: SELECTION TEST PASS: mission 6, R, re-entry with mission 2, invalid numbers and Garden entry passed.
[2026.10.15-20.51.24:984][500]LogTemp: Display: WINDSEED TEST PASS: Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
```


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 1140
UTC: 2026-10-15T20:51:24.983Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 75.8, "frames": 1138, "bytes": 15037239}

SHA-256: 886391fbeffa7599dcaf891e1605708c4e8c493cc687b634fc878fd57e35a5b3

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。