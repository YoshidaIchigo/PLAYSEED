# PR #8 実ゲーム検証

対象コード: 1d35945ec3c07ab45c4ca722f1c53a46ec7e2ced

セルフレビューでは地形とルールの役割分離、World/Actor/Componentの所有、基本メッシュの100cm単位、草と岩の衝突面、Skyの衝突無効、開始位置の維持を確認しました。必要なWorld/StaticMeshヘッダーを明示し、地形Actorの登録とBeginPlayの説明を修正しました。実画像で看板の裏から文字が反転して見えることを発見し、案内文字をカメラへ向けて垂直に保つ修正後にEditor/Gameを再ビルドし、ルートと映像を再検証しました。日本語コメントでActor、Component、GC参照、登録とライフサイクルを説明しています。

実ゲームで草原から6地点を通常移動と3回のジャンプで巡り、最上段へ到達しました。ルート中の意図しないRecoveryCount増加は0です。足場間をテレポートしていません。最上段からの画像を保存し、再入場後も正しいマップ・ミッションになることを確認しました。

その後、既存Windseed Gardenの落下復帰、W/Shift/Space/マウス/ズーム、通常移動と6回のジャンプによる全6収集・ゴール・R再開も回帰検証しました。入力はUnrealの自動イベントとCharacterMovementを使用しています。人による入力機器の確認は未実施です。山道やミッション本体は後続PRで追加します。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 840
UTC: 2026-10-15T14:58:59.478Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 55.8, "frames": 838, "bytes": 10301798}

SHA-256: 090c104102d20916b89df77b22930324a804bd7cc75b84a824df62b8f34ce6d5

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。