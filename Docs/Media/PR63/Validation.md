# PR #63 実ゲーム検証

対象コード: 6f7f4115956e0b69d2cc60f2d15ce24ff70cf8e4

Editor7433/Game Development両ビルド成功。最終ソース6f7f4115956e0b69d2cc60f2d15ce24ff70cf8e4で、UnrealEditor-Cmd.exeへuprojectと-gameだけを渡し、マップ引数もReferenceCourseもない通常既定から起動。ThunderDefaultStartTestと新規Validation_DefaultEntry_Aだけで、入口→通常移動とSpaceの絵ダイブ→選択→現行ReferenceHero/ReferenceBattlefieldTerrainへの物理1000cm落下→H操作説明→通常Spaceジャンプ→M→絵前帰還を確認。R/M/Space/Ctrl/Enter連打も遷移を中断せずRecovery0、進行0と専用セーブ未作成、通常セーブ前後ABSENT。PASS276フレーム、UTC2026-10-16T14:29:05.359Z。ログの最初のLoadMapは/Game/Battlefield/Entrance、コマンドラインにマップ名とReferenceCourseなし。3MapsToCookと3DirectoriesToAlwaysCook、Engineの2書体RuntimeDependenciesは静的確認。配布用cook/package/UATを実行していないためパッケージ実動作済みとは扱わない。

## 実行結果

```text
PASS
Painting dive -> actual star selection -> physical 1000cm drop -> ground contact and landing pose -> ordinary Space jump -> M returns to painting front. R/M/Space/Ctrl/Enter spam blocked during transition, no recovery and fresh zero progress unchanged.
CaptureFrames: 276
UTC: 2026-10-16T14:29:05.359Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 18.4, "frames": 276, "bytes": 2499156}

SHA-256: bd22e2682aa7fd05cacb0626c0ca530f36f0c15fb05d6042d1de1b3ab41ca9c2

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。