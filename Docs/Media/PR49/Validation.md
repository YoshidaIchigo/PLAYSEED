# PR #49 実ゲーム検証

対象コード: 92911532e6a16e37770b3ad4a8c6893ea600683b

Draft PR49後のセルフレビュー。入場判定は絵の範囲と実際のFalling状態で行い、製品側の演出開始を一度だけ確定する。移動無効化とPawn入力停止、共通MのCanLeaveStage確認、R/死亡の上書きで入場中の中断を防ぐ。演出中のSetActorLocationは製品の絵への吸い込みだけで、QAの移動は通常AddMovementInputとSpace。QAはマップ到着を確かめてから入力連打を止め、メニューのEnterを誤って押さない。通常セーブを使わず新規Validationスロットを確認。二重遷移はFinishPaintingの状態遷移を一度しか受理しない。今からビルド、新規map作成、実画像と動画で検証する。実画面で見つかった物理カメラ露出の不足を修正。初回QAは遷移確定からOpenLevel適用までの1Tickを誤判定していたため、確定済みSelectingStar状態も認め、確定後の入力連打は止めた。製品の二重遷移防止は維持。Editor/Game7396両成功。新規Painting_Bが実歩行とSpaceで絵に入り、入力連打を防いでスター選択へ到着、PASS222フレーム UTC06:10:21.772。通常セーブABSENT不変。明るい部屋と絵、入場中の実画面を確認。


## 実行結果

```text
PASS
Normal walking and Space jump into painting, ripple/dive/fade to star selection; R/M/Space/Ctrl/Enter spam blocked during transition; fresh progress remains zero and unsaved.
CaptureFrames: 222
UTC: 2026-10-16T06:10:21.772Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 14.6, "frames": 220, "bytes": 580820}

SHA-256: 66aa9923fc061f1fd40855aa3d38dcb19b784e052dea4672b3e5bc0c22885cb8

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。