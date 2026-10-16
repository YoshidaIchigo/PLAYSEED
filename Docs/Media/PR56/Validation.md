# PR #56 実ゲーム検証

対象コード: 90b457837f2c777edac9812783756bd1ca2c4adb

# PR56 Draft後セルフレビュー

初回Draft: 40dcac8995b3732aa0d3b0854ceb6869e44bdb50 / 最終コード: 90b457837f2c777edac9812783756bd1ca2c4adb

- 新しいComponentはTG_PostUpdateWorkで、SpringArmをTickの前提にする。希望アーム長ではなく、壁判定後の実Socket位置を測る。専用CameraActorへのブレンド中は実CameraManagerの位置を使う。
- 260cmで隠す/340cmで戻す差を付け、境界の表示ちらつきを抑える。変更時だけ見た目の子PrimitiveへHiddenInGameを設定する。
- 子のVisibilityをまとめてtrueにすると羽やピースが常時見えるため、その方法は使わない。HiddenInGameだけを個別に設定し、各ポーズ/パワーのVisibility判定を保持する。帽子の文字もVisualの子なので同時に隠れる。
- Actor全体やカプセル・影・HUDは隠さず、衝突/移動/保存条件も変更しない。
- QAはPR55で露出した実保存失敗後の通常離脱・急旋回を再現。実際に身体が隠れた後に再表示されたことを確認し、専用カメラへ移った取得ポーズが隠れたままでないことも確認する。
- 収集ルートは普通の移動とジャンプ/杭の操作を使う。カメラ操作以外の座標・体力・速度・報酬の注入はない。保存失敗の障害物は自分の新規Validationスロットの空ディレクトリのみ。

- 初回build7410はローカル変数ChildrenがAActor::Childrenを隠してC4458で失敗。VisualChildrenへ改名した。ゲーム実行や成功扱いはしていない。

- 修正コードe23b296096096cdd0887ad7fab8260a1c573effdのEditor/Game build7411両方成功。fresh Validation_Camera_AでPASS1692フレーム（UTC09:09:45.540）。実距離139.2cmで非表示（09:09:18.950）、200.5cmで再表示（09:09:19.682）を確認した。
- 非表示/再表示の実スクリーンショットと、身体が見える取得ポーズを目視確認した。通常の保存失敗/実離脱/再接触保存、取得演出、絵帰還と再入場も完了。通常セーブはABSENT。

- Aの復帰画面で、旧190cmの閾値では再表示直後の身体がまだ画面を大きく占めることを確認した。身長約170cmの全身に対して視界を確保するため、非表示260cm/再表示340cmへ調整し、再検証する。Aの録画は保管して最終証跡として扱わない。

- 調整後コード90b457837f2c777edac9812783756bd1ca2c4adbのEditor/Game build7412両方成功。fresh Validation_Camera_BでPASS1700フレーム（UTC09:17:49.350）。実距離159.1cmで非表示（09:17:22.772）から340.3cmで再表示（09:17:23.779）を確認。復帰画像では身体の周囲の地形も見え、近すぎる復帰を解消した。
- 近接時の身体非表示、離れた後の全身再表示、取得ポーズの表示を実画像で確認。実保存失敗/通常離脱/再接触保存・絵帰還・再入場も完了し、通常セーブはABSENT。

最終MP4を復号し近接時の身体非表示・距離を取った後の全身復帰・見える取得ポーズ・白フェード帰還を確認。[最終セルフレビュー](SelfReview.md) / [修正前後比較](CameraBeforeAfter.png)。参照UI/素材・残り目的物配置・全7同一セーブの実取得、実死亡/獲得済み再取得は引き続き残る。


## 実行結果

```text
PASS
Actual M return and painting reentry, second physical drop, reference post3 and gate collision, actual star6 save, pose/camera/held star, transition input spam and duplicate guard, automatic painting return, earned6 menu display persisted. Actual SaveGameToSlot failure from own empty QA directory, rollback0 and visible star, physical exit, nonrecursive removal of own empty obstacle, real recontact saved6 once and returned to painting.
CaptureFrames: 1700
UTC: 2026-10-16T09:17:49.350Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 113.334, "frames": 1700, "bytes": 23798195}

SHA-256: e44956195a38e87ed1abd34f5c69b2be2099b5155d9044110c000606213a3dfb

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。