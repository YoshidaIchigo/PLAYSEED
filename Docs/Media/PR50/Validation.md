# PR #50 実ゲーム検証

対象コード: 105e54dd7d54f0a527428f7771b43261e8a4f528

Draft PR50後のセルフレビュー。PR49の元AVIには入口が残っているが初版MP4は再生冒頭が不正。FFmpegの独立変換では先頭画像と入場画像を正しく復号できた。元AVIと失敗したMP4は保持する。新ツールはFFmpegを引数配列で起動し、出力既存時は停止する。フレーム数だけでなく全フレームの時刻が0から連続すること、元AVIのSHA256不変も確認する。公開MP4から直接PNGを抽出して表示内容を確認する。未確認の独自AVI再構成の試行は採用していない。今からEditor/Gameビルドと新規Painting_Cの実録画を行う。

## 実行結果

```text
PASS
Normal walking and Space jump into painting, ripple/dive/fade to star selection; R/M/Space/Ctrl/Enter spam blocked during transition; fresh progress remains zero and unsaved.
CaptureFrames: 221
UTC: 2026-10-16T06:24:14.951Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 14.734, "frames": 221, "bytes": 459609}

SHA-256: ebd317b837c716304ed9746787ab13b7e5dd9bd6c833f194a9e197e822c22aeb

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。
Editor/Game7397両ビルド成功。新規Validation_PR50_Painting_Cの実歩行・Spaceジャンプ・絵への飛び込み・連打防止・スター選択到着がPASS221フレーム UTC2026-10-16T06:24:14.951Z。通常セーブABSENT不変。新MP4は元AVIと同じ221フレーム、960x540、14.733333秒、全フレーム時刻連続。元AVI SHA d4ed85b3d52f2778cd9a1628cd9d9de52d18b87acfeb5c86feebe9275d2e43f4不変。MP4から直接復号したVideoStart、2.35秒のVideoDive、VideoEndで入口・飛び込み・選択を確認。
