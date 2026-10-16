# PR #62 実ゲーム検証

対象コード: 7f151e69d4794be8536223f51c5551ec725ecdf4

PR62 最終ソース7f151e69d4794be8536223f51c5551ec725ecdf4。Editor7430/Game Development両ビルド成功。通常操作の専用Validation_ReferenceRace_B、実獲得済み1〜4から続けて、入口の実Space/Ctrlで羽根解放を保存し、実頭突きと二度目のジャンプで帽子を一度だけ取得。普通に歩いて砲台へ戻り、G/実マウス軸/左クリック、実飛行マウスとW/Sで5中央コインを取得し、固定スター5を1回だけ生成。実取得・保存5・地上ポーズ・白フェード・絵前帰還・再入場で獲得1〜5と次の6を確認。座標・速度・帽子・進行・報酬をQAから直接設定していない。通常セーブは前後ABSENT、Recovery0。Bは1817全フレーム、UTC2026-10-16T14:10:48.835Z。Aはスイッチへの直線経路で本体に当たりFAIL141、横を回る通常歩行へ修正して再ビルド後にB成功。正当に保存された羽根解放を戻していない。100枚7と通常起動統合は次の単位で継続する。

## 実行結果

```text
PASS
Actual entrance-switch ground pound and saved wing access, ordinary cap-box head bump and actual pickup, normal field cannon G/mouse/LMB, real controlled flight through five actual centre coins, fixed star5 save total5, pose/fades/painting return and earned1/2/3/4/5 next6 menu.
CaptureFrames: 1817
UTC: 2026-10-16T14:10:48.835Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 121.134, "frames": 1817, "bytes": 27022620}

SHA-256: 5f51d14f3c031b4f32d764bcaeb237bb227520328b6b276b89d7e7a89f463bb5

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。

この初回取得動画は入口案内文の表示位置修正前です。画面下の見切れを修正した最終ソースの全経路はPlaythrough.mp4（獲得済み5の再プレイ・NEW0）です。セーブを巻き戻さず、初回取得4→5と最終版の再取得5→5を分けて記録します。
