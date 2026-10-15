# PR #12 実ゲーム検証

対象コード: 885998ff46ab11267b74ccd6ff8780c9db2e298c

- 左右へ ±200 cm、周期約9秒で往復する足場を Actor として追加。Mesh Component を衝突と床に使用し、CharacterMovement の通常の床追従で運搬する。
- プレイヤーの Attach や乗車時の座標置換は使用しない。R ではステージが所有する足場だけを初期化し、共通の復帰処理で床参照・速度・ジャンプを解除する。
- 地上から通常移動とジャンプで乗車し、無入力のまま120 cm以上運ばれること、乗車中の R 後に開始位置が引きずられないこと、再乗車と高台へのジャンプを検証する。
- M からの退出、ミッション再選択、Windseed Garden の6種収集・ゴール・R は既存の一連のゲーム内検証で回帰確認する。
- テスト Subsystem はレベル間で残るため、足場を弱参照で観察する。通常セーブへの書き込みは追加しない。
- 初心者向け日本語コメントで Actor/Component の役割、動く床と Attach の違い、時間に基づく移動、床参照解除の意図を説明した。
- 動画は実ゲームを自動入力で操作した記録で、人によるキーボード・ゲームパッドの手動検証ではない。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 990
UTC: 2026-10-15T16:44:10.927Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 65.8, "frames": 988, "bytes": 12936471}

SHA-256: 29d28b0d20d1fd26ef2fc62c233077036940100bd9d7222e0c731576f19a230f

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。