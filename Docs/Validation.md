# Windseed Garden 検証記録

検証日: 2026-10-15 UTC。対象ゲームコード: `e54947cc84fab34c4f643130bd3ee268e01f8fad`。

## ビルド

- Unreal Engine 5.8.1 / Windows Win64 Development。
- Editor対象: 成功。既存のEditorを保護するため`-ModuleWithSuffix=PLAYSEED,7315`で別名DLLへリンク。
- Game対象: 成功。MSVC 14.51.36256 / Windows SDK 10.0.26100.0。
- 既存Editorは終了・リセットしていません。そこで新機能を使うには、ユーザー作業を保存してプロジェクトを開き直してください。

## 実ゲーム検証

`PLAYING.md`記載の`-game -GardenSmokeTest -GardenCapture`で実際のゲームワールドとレンダラーを起動しました。ゲーム内の自動操作であり、人が入力機器を操作した動画ではありません。

- 落下を検出し、スタートへ復帰。
- エンジンのシミュレーション入力イベントでW・Shift・Space・マウス移動・ホイールの割り当てを確認。
- CharacterMovementの通常移動入力で展望台と灯台ルートを移動。島間のテレポートなしで6回ジャンプ。
- 6個すべてを収集し、灯台ゴールを起動。
- リスタートで収集数・種の表示・ゴール状態が初期化。
- 最終結果: **PASS**、2026-10-15T12:57:49.624Z。

## 動画と画像

- [実ゲーム動画（H.264 MP4、約23秒、約4.1MB）](Media/WindseedPlaythrough.mp4)
- [ゴール画面](Media/GardenComplete.png)
- 元録画: 960×540、15fps、350フレーム。開始・探索中・終盤のフレームを確認し、変換後MP4のデコードとゴール表示も確認しました。
- MP4 SHA-256: `4fb25dd66d37637b8267d4f61762b14e69ef0168b58a728c84f2e1b0a73fae5f`。
- 元AVI・生ログ・Result.txtは`PLAYSEED/Saved/Validation/`に保持し、Gitには含めません。

## PR作成後のセルフレビュー

- 背景と環境光を改善し、HUDを拡大。
- 灯台の支柱に衝突判定を追加し、キャラクターの重なりを防止。
- 録画の読み出しを描画完了後の描画スレッドへ移動。最終実行には以前の読み出し時のhandled ensureなし。
- 録画フレームが150未満なら検証を失敗にし、古い動画を合格扱いしないように修正。
- CIの履歴取得深度を2へ修正。対象コードの[Repository checksは成功](https://github.com/YoshidaIchigo/PLAYSEED/actions/runs/38738854233)。
- プロジェクトとマップ・マテリアルの整合性、Python構文、生成物・秘密設定の混入、空白を確認。

## 確認範囲

人によるキーボード操作、実機ゲームパッド、配布用のパッケージは未検証です。操作可能な初期ステージとしてEditorからPlayできます。エンジン内部の非推奨API警告と、MSVCがエンジンの推奨版より新しいという警告は残ります。
