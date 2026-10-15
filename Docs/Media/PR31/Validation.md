# PR #31 実ゲーム検証

対象コード: ac321667d4bcec2371861fccb4306e28f6b7e9ec

## セルフレビュー

Draft作成後に、最寄り対象の距離と壁の判定、会話の確認段階、保存済み解放の重複防止、保存失敗時のロールバック、既存データ不正時の上書き禁止、旧Version1で追加フラグがfalseになる既定値、ActorとSubsystemの責務、Rで一時会話だけを消すことを確認しました。

ThunderValidationは安全なValidation_スロット名が欠けていると読込・保存を止め、通常スロットへ戻りません。報酬のfixture付与には使用しません。実行前後で通常セーブの有無とSHA-256を比較し、既存のユーザーデータを変更・削除しません。

実ゲームでは通常歩行で案内役に到達し、遠方のGで作動しないこと、初回会話ではまだ閉じていること、二回目の入力で開いて保存すること、繰り返し会話・R・再入場で解放を維持することを確認しました。その後に別プロセスで同じ専用スロットを読み、報酬0のまま解放を復元することを確認しました。両プロセスでGardenの全ルートも回帰します。

## 新規解放の実動作

```text
[2026.10.15-21.20.40:349][489]LogTemp: Display: GUIDE TEST PASS: unlocked through dialogue into a fresh isolated slot Validation_PR31_Guide_A; normal walking, distant interaction rejection, actual G/D-pad dialogue, idempotence, launcher visual and R persistence with zero quest rewards passed.
[2026.10.15-21.20.48:359][560]LogTemp: Display: GUIDE REENTRY TEST PASS: saved launcher is open in a newly created World; quest rewards remain zero.
[2026.10.15-21.20.52:403][583]LogTemp: Display: SELECTION TEST PASS: mission 6, R, re-entry with mission 2, invalid numbers and Garden entry passed.
[2026.10.15-21.21.18:107][404]LogTemp: Display: WINDSEED TEST PASS: Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
```

通常セーブの実行前後: ABSENT / ABSENT。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 878
UTC: 2026-10-15T21:21:18.106Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 58.333, "frames": 876, "bytes": 11657585}

SHA-256: c6207d6bda9ec02202750a99cf05c009bf25267668eff01090cd11bf8a6569c4

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。