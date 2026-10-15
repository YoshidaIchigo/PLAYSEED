# PR #23 実ゲーム検証

対象コード: ee3a478faba56cf202e59a3445e3b7f69509eef3

- 箱Actorが衝突用の本体と描画用の帯を所有する。共通の攻撃インターフェースでパンチを受け、CharacterのAction Componentから急降下の着地通知を弱参照で購読する。BeginPlay順に依存せずPawnの準備後に購読し、EndPlayで解除する。
- 急降下は箱の上面の範囲と高さを照合し、近くの地面への着地では壊さない。破壊フラグを先に立ててから衝突・表示を消し、同じ箱の生成を一度だけにする。
- 箱固有IDと内容物の番号でコインIDを作る。Rではこの箱自身が生成したActorだけを片付け、ほかの配置物を削除しない。集計は既存GameModeが初期化する。
- 通常歩行とLMBで破壊し、連続攻撃でも一回の生成・三個のコインだけであることを確認する。実際に歩いて3点を取得する。Rで箱と生成物・集計が戻った後、通常ジャンプとCtrl急降下で上面へ着地し、再度三個・3点を確認する。
- 座標・垂直速度・破壊結果・コイン合計を検証側から書き換えない。ミッション再入場とGardenの全6種収集・ゴール・Rも回帰確認する。
- 実ゲームの自動操作であり、人による入力機器の手動検証ではない。

## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 836
UTC: 2026-10-15T19:12:02.739Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 55.533, "frames": 834, "bytes": 11128576}

SHA-256: 456782bf0ebd2c993a03a7eda6843158455d5117bac094b5f13f16abf6325126

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。