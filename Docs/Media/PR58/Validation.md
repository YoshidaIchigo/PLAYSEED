# PR #58 実ゲーム検証

対象コード: eb3afb621e6e039317a96a6fcb4f1c055f985c69

# PR58 Draft後セルフレビュー

初回Draft: 9ea292150b8d3046e86a18d529bfb1e86aa94f12

- 元資料のボス(1636,4242,-5567)、向き-147度を確認。UE(X,-Z,Y)のXY(1636,5567)・yaw123度へ変換し、球の底は実床4293cmに置く。スターは元資料(2000,4500,-4500)に対応する固定位置(2000,4500,4500)。
- 400cm球に対して従来の170cm範囲では持ち上げられないため、この相手だけ340cmへ変更。CarryComponentは対象自身のCanBePickedUpを尊重し、箱と旧ステージの170cm制限は変えない。実入力のE→CarryComponent→TryPickup/Throwを検証する。
- 有効着地では実着地点で起き上がる。従来のホーム半径へのClampとResetToSpawnは新地形の有効投げに適用しない。崖外とRの通常リセットはホームへ戻す。足元5点の床で追跡先を制限する。
- 子モデルは親の4倍スケールが掛かるため、目・手足・王冠の局所位置と寸法を換算。素材は自作M_Actor。プレイヤーの当たり判定・重力・速度は変えない。
- 新規Validationスロットで絵入場/M帰還/再入場/物理落下後に既検証の226点登山ルートを通常移動とSpaceで通る。背後を回ってE持ち上げ、通常歩行で向き、Eで投げる。3回のPickup/Throw/ThrownLanding/ValidThrowsが一致し、崖外復帰0・プレイヤーRecovery0を要求する。QAから座標・速度・報酬を設定しない。
- レビューで、報酬番号の一般化後も選択画面の次候補を確認していないことを発見。実スター1取得後に表示2個、選択1/2、3は不可という検査を追加した。

最終コードeb3afb621e6e039317a96a6fcb4f1c055f985c69のEditor/Game build7419は成功。最終MP4は全3638フレームと正確な1/15秒の連続PTSを検査済み。直接デコードした絵前・接近・投擲・ポーズ・白フェード・帰還・獲得後の選択画面を目視確認済み。詳しくはSelfReview.mdを参照。残る目的2/3/4/5/7の参照配置と全7新規同一セーブ攻略は後続。
- build7415はEditor/Gameとも成功。初回Validation_ReferenceBoss_Aは山頂226点を通過したが、外周を一周する回り込みで崖側へ出てEの背後取得がタイムアウトしFAIL3495（UTC10:18:40.397）。通常セーブABSENT、報酬未取得。地形自身のFindFloorで追跡先を確認し、通常歩行で内側へ誘ってから回り込むルートへ変更した。次の実ゲームで再検証する。

- build7416もEditor/Game成功。Validation_ReferenceBoss_Bは山頂到達後の内側待機(2418,5200)まで安全に歩いたが、ボスが初期位置から動かずFAIL3503（UTC10:27:15.104）。通常セーブABSENT。ボスの配置・実着地点での復帰を実Boundsの底+3cmに合わせ、床やSweepが移動を拒む場合に一度だけ診断を残す修正を追加。プレイヤーへの座標操作は追加しない。

- build7417はEditor/Game成功。Validation_ReferenceBoss_Cでは床4293・半径200・中心4496を実ログ確認し、内側への追跡と通常Eの背後持ち上げが成立。持ち上げ画像も目視確認した。直後の投擲判定でFAIL3190（UTC10:34:00.801）。InputKey送信と同一フレームで古い保持状態を検査していたため、次フレームの実ThrowCount・非保持・物理飛行を待つQAへ修正する。通常セーブABSENT、報酬なし。

- build7418両成功。Validation_ReferenceBoss_Dで通常Eの投擲1/2/3と実着地1/2/3を確認（UTC10:40:05.809〜10:40:12.593）。崖外復帰0、Pickup/Throw/ThrownLandingとValidThrowsは各3。スターはTickで±8cm上下するため、見た目の現在座標と固定座標を1cm以内で比較した検査がFAIL3291（UTC10:40:12.797）。配置の基準位置RestLocationを読み取るgetterを追加し、固定位置(2000,4500,4500)・報酬1で検査するよう変更。通常セーブABSENT、まだスター未取得のため完走とは扱わない。

- 最終Validation_ReferenceBoss_EはPASS3638フレーム（UTC10:45:59.903）。実M帰還/絵再入場/2回目の物理落下/226点の登山/通常Eの背後持ち上げ3回・投擲3回・物理着地3回/固定位置の実スター1取得/ポーズ/白フェード/絵前帰還/再入場で獲得1と次の目的2を検証。Recovery0・崖外復帰0。ポーズ画面と獲得1/次2の選択画面を目視確認。通常セーブは前後ABSENT。実スロットValidation_ReferenceBoss_EのSHA-256は7B987C85109CD51E4C4A5198F736EEF14CBB2766CFE1910D853BC453C7057F81。
- ひげ・金色の手/青い腕・光沢・専用の丸影、芝・壁・空の素材差は残る。参照175/195/215秒の実フレームを確認し、素材/形の後続調整事項としてart-review.mdへ記録。新配置の実取得確認はスター1と6の2個になったが、同一の新規セーブで全7個を取った検証はまだ行っていない。


## 実行結果

```text
PASS
Actual M return and painting reentry, ordinary mountain route, three E rear pickups and real physical summit throws, actual fixed-position star1 save, pose/fades/painting return and earned1 menu persistence.
CaptureFrames: 3638
UTC: 2026-10-16T10:45:59.903Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 242.534, "frames": 3638, "bytes": 53115034}

SHA-256: c837b7779e8ace1136b46e3289241f4c91dfb1773a1ebf88671ebcf921e4aa0f

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。