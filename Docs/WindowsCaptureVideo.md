# Windowsでの検証動画の書き出し

Unrealの元AVIを保護し、FFmpegで先頭から全フレームを順にMP4へ書き出します。動画を短縮・並べ替え・ループする処理はありません。元AVIのフレーム数、MP4の復号フレーム数、全フレームの時刻が0秒から連続すること、元ファイルのSHA-256不変を確認します。

```powershell
python Tools/export_capture.py Original.avi Playthrough.mp4 Review --ffmpeg C:/path/to/ffmpeg.exe
```

出力が既に存在する場合は停止します。ReviewにはMP4から直接復号した先頭・入場付近・末尾の画像とExportVerification.jsonが残ります。マージ前に、この画像で対象の操作が実際に見えることを確認します。撮影時のPNGと、公開動画を復号したPNGの両方を確認してください。

FFmpeg実行ファイルはリポジトリへ含めません。取得が必要な場合は[公式ダウンロード案内](https://ffmpeg.org/download.html)が案内する配布元を使用します。今回の検証では[Gyan Windows builds](https://www.gyan.dev/ffmpeg/builds/)の9.0.2 essentials版を作業フォルダへ取得し、[公開ZIP SHA-256](https://www.gyan.dev/ffmpeg/builds/packages/ffmpeg-9.0.2-essentials_build.zip.sha256)との一致を確認しました。ZIP SHA-256は60f467265b1e312373dbcd92200c2618a74850f98d3d078e94296bb3fa2047baです。PATHやシステム設定は変更していません。

PR49の初版MP4では冒頭の再生に欠落を確認しました。元AVIと検証結果は保持しています。修正版動画はPR50の証跡を参照してください。今後はこの書き出しとフレーム時刻の検査を使用します。
