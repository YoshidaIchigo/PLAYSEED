# PR #45 実ゲーム検証

対象コード: 83e7e75ca29592cdeb9f8777193ee42e3603e596

Garden削除後の同じソース83e7e75/EditorGame7377で、以前に全7個を実取得したValidation_PR42_Campaign_Aをそのまま読みました。実1UPで3→4、Rで残機・取得済みを保持、イバラ4実接触で死亡4→3、通常歩行で床の端から3回落下し3→2→1→0、0の間の追加消費とR回避が起きないことを確認しました。Gardenの旧選択を挟まずMissionSelectへ実帰還し、目的2を選んで新Worldの残機3・体力8・1UP未取得と全ID1〜7を照合。最終目的選択までPASS759frames、UTC2026-10-16T03:39:46.943Z。通常セーブ前後ABSENT、実全7個セーブの前後SHA256も同一です。自動入力の実ゲームで、座標・体力・残機・報酬の直接設定は使いません。

## 実行結果

```text
PASS
Single course entry, actual movement/jump/camera, R, invalid numbers, new World mission2 and return to mission selection passed. Garden removed.
SelectionJumps: 1
CaptureFrames: 759
UTC: 2026-10-16T03:39:46.943Z

```

[実ゲーム動画](CourseRegression.mp4) / [完了画面](CourseRegressionComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 50.4, "frames": 757, "bytes": 8639812}

SHA-256: 925150b61cf538661b16c8eec44ffd7b853c7735ba88ee59c5128f83bb2c65db

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。