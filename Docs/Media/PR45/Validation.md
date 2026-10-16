# PR #45 実ゲーム検証

対象コード: 83e7e75ca29592cdeb9f8777193ee42e3603e596

Garden削除後のソース83e7e75ca29592cdeb9f8777193ee42e3603e596、Editor/Game7377成功。新規隔離Validation_PR45_Entry_Aで目的選択から直接入場し、実W+Shift+Space、MouseX、Rの復帰・カメラ変化、Mの帰還、新しいWorldの目的2、無効番号拒否を確認しました。Gardenマップや専用クラスを経由しません。実録画185frames、UTC2026-10-16T03:36:21.465ZにPASS、通常セーブ前後ABSENTです。共通の素材・Character・移動・保存・録画は保持し、旧StageSelectは見える選択UIなしの転送だけです。
自動入力による実ゲームで、人による入力機器の検証とは区別します。この動画は入口と共通機能の変更を示す段階で、HUD・スターの表現・地形の参照一致は後続PRで修正します。

## 実行結果

```text
PASS
Single course entry, actual movement/jump/camera, R, invalid numbers, new World mission2 and return to mission selection passed. Garden removed.
SelectionJumps: 1
CaptureFrames: 185
UTC: 2026-10-16T03:36:21.465Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 12.133, "frames": 183, "bytes": 1949451}

SHA-256: 92bb7b935e5a46f10a6acf6f2a5a7a6e2ec63021146bc15ce4e19656c817467d

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。