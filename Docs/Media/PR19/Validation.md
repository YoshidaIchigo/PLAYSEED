# PR #19 実ゲーム検証

対象コード: 8549f796a9d6eb3e4feb74ba9da98effb11b46c7

- 木Actorが幹・頂上の枝の足場・葉のComponentを所有し、CharacterのClimb Componentが取り付きと移動を担当する。
- F/Yで170cm以内の木へ取り付き、W/Sで上下する。Flyingは登っている間だけ重力を止めるために使い、CharacterMovementの速度で移動する。プレイヤーのAttachや座標置換は使わない。
- 登っている木だけを移動衝突から一時的に除外する。頂上の足元が枝の上へ達してから戻し、実際の床の衝突で立たせる。ジャンプ・F解除・R・被弾・死亡でも衝突と移動モードを戻す。
- 検証は届かない木の拒否、通常歩行での接近、F/Y、Wの上昇、Sで140cm以上下降、頂上での床接触を確認する。頂上からの通常ジャンプと、取り付き中のSpaceによる外向きの飛び離れを両方確認する。
- 登り直してRで解除し、再び歩いて幹へぶつかり、木の衝突が無視されたままでないことを確認する。最後に既存Gardenの全6種収集・ゴール・Rで通常移動も回帰確認する。
- セルフレビューで通常ジャンプの入力を移動処理まで保持するよう調整し、頂上からの空中移動を明示した。
- Actor/Component/UPROPERTYの所有、入力、速度による移動、一時的な衝突除外と全解除経路を日本語で説明した。保存形式は変更していない。
- 動画は実ゲームの自動操作で、人が入力機器を操作した手動検証ではない。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 1005
UTC: 2026-10-15T18:12:11.924Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 66.8, "frames": 1003, "bytes": 12783818}

SHA-256: 885d826955ba656e1562a9dd5f6b3bb5c7b5dda5812f9765115e07f1d30ecc69

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。