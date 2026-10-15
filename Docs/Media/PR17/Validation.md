# PR #17 実ゲーム検証

対象コード: bdb2f10f082d1bcd28aebae8b7725f5ad4ef54ae

- Character所有のAction Componentへ空中アクションを分離し、Ctrl/RBの入力を登録する。地上・被弾後の無敵中は開始しない。
- 急降下はCharacterMovementの速度・重力・空中操作を変更する。座標を置き換えず、見た目だけを縮めて姿勢を示し、カプセル衝突は変えない。
- Characterの実際のLandedコールバックで一度だけ衝撃位置を通知する。状態を先に解除してから通知し、後続Tickで重複させない。後続の杭などが購読できるC++通知を用意した。
- R・死亡・被弾で解除し、開始前の重力と空中操作へ戻す。被弾のノックバックを急降下Tickで上書きしないよう、成功した被弾で先に解除する。
- 検証は地上のCtrl拒否、普通のジャンプ→Ctrl急降下、実着地と通知一回、RB急降下中のR中断を確認する。イバラの上へ普通に跳び、急降下から実接触したときの被弾中断も確認する。
- 中断後は着地通知を出さず、重力・空中操作を復元する。最後に既存Gardenの全6種収集・ゴール・Rを回帰確認し、通常ジャンプへの影響を確認する。
- UPROPERTY、Componentの所有、入力、物理移動、着地通知、保存した設定の復元を日本語コメントで説明した。
- 動画は実ゲームの自動操作で、人が入力機器を操作した手動検証とは区別する。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 789
UTC: 2026-10-15T17:36:10.152Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 52.4, "frames": 787, "bytes": 9902461}

SHA-256: b42c9c90bee87ade9440bad1b895ca8b85421a77905d0f45f48ad4990a6d35ef

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。