# PR #54 実ゲーム検証

対象コード: 6eb1217cbcfa457d428475434afc3ffa71f09fcd

# PR54 Draft後セルフレビュー

対象ソース: 4c5b22489f4249141315d6a6aa0f0e79153cfd34

- 参照動画の01:30.25〜01:31.25および01:35〜01:36.75の実フレームを確認。4:3白矩形の全画面フェード。円/スター形マスクではない。実装はout1.00秒、白保持0.15秒以上、in0.75秒。
- 不透明度をGameInstanceで保持。OutをCoveredへ変更してからOpenLevelするので、フレームごとの重複呼び出しを防ぐ。移動先World、Controller、HUD、必要なPawnが揃うまで白を保持する。
- 3画面のHUDの最後に共通の白を重ねる。コースの完全被覆時はPawn準備前にも白を描く。明転完了までControllerの移動/視点とPawn入力を止める。新WorldのControllerにも一度だけロックを適用し、カウンターの重複を防ぐ。
- 入場中と取得ポーズ中の状態は引き続きロックする。選択画面は候補変更と確定の両方をロックする。報酬保存の成功条件は変更しない。
- 取得後のフェード中もポーズとカメラを保つ。M帰還、ゲームオーバー帰還、降下の回復帰還も共通の1回だけの遷移へ集約する。
- コース操作の再開は明転と実際の着地が両方完了してから。検証も白化の完了を待ってから歩行する。
- QAはfresh Validationスロットで実スター6を取得する。7回全てのOut/Covered/Inと完了、0〜1の不透明度、完全白のままの切替、連打中の選択候補不変、実カメラ/ポーズ/保存/入口位置を監視する。報酬や移動座標の注入をしない。

- build7405両方成功後のFade_Aで、明転直前の確定キーが解除後に処理され、選択画面を飛ばす不具合を検出（FAIL101フレーム、UTC08:14:56.457）。成功扱いにしない。
- EngineのPlayerInput.cppを確認し、FlushPressedKeysだけでは未処理のEventAccumulatorを消さないため、明転完了時にFlushPressedKeysとDiscardPlayerInputを両方行う修正を追加した。キー状態と蓄積イベントの両方を消してから操作を戻す。

- 修正ソース f8b3e34af26fbb21c723383dce6866b92eef2a65 のEditor/Game build7406成功。fresh Validation_WhiteFade_BでPASS1645フレーム（UTC08:20:28.926）。7回全てのOut/Covered/Inと完了を確認。連打中の選択表示は不変で、明転後に入力が持ち越される不具合も再発しなかった。
- 実スター6はUTC08:20:05.509に一度だけ保存し、取得ポーズ→白化→新World完全被覆→絵前明転→実再入場と保存表示まで通った。通常セーブはABSENTのまま。

- BのMP4全1645フレームを復号し、中央4:3では7回の完全白（各2〜3フレーム）を確認。一方、実画像で外側の黒帯に新Worldの仮視点とEngineの空表示が見える瞬間を検出した。状態QAのPASSだけで完了としない。
- 共通UIで4:3の外側を毎フレーム黒で覆う修正を追加した。各HUDの中央の白に加えて外側も確実に隠し、場面切替中の露出を防ぐ。

- 最終ソース6eb1217cbcfa457d428475434afc3ffa71f09fcdのEditor/Game build7407成功。fresh Validation_WhiteFade_Cで再度PASS1647フレーム（UTC08:27:17.996）。7回すべての白化・完全被覆・明転・完了と、実スター6取得帰還・再入場・保存表示を確認。通常セーブはABSENT。

最終MP4全1647フレームを元解像度で検査し、中央の完全白7区間と左右黒帯を確認。[画素結果](FadePixels.txt)、[7回の遷移画像](FadeContactSheet.png)、[参照比較](FadeReferenceComparison.png)、[最終セルフレビュー](SelfReview.md)を参照。実保存失敗/死亡と新地形の他目標/全7実取得は引き続き別作業として残る。


## 実行結果

```text
PASS
Actual M return and painting reentry, second physical drop, reference post3 and gate collision, actual star6 save, pose/camera/held star, transition input spam and duplicate guard, automatic painting return, earned6 menu display persisted.
CaptureFrames: 1647
UTC: 2026-10-16T08:27:17.996Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 109.8, "frames": 1647, "bytes": 23169918}

SHA-256: 589c285aecdaafcde7a2fa5843d99a70d0714f21c1660c9422b13b910461aae2

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。