# PR #64 実ゲーム検証

対象コード: 9385546c7277f8f0fbfc39c6a598847ce7049ff0

最終ソース9385546c7277f8f0fbfc39c6a598847ce7049ff0。Editor7435/Game Development両ビルド成功。新規専用Validation_ReferenceGround_Bで、普通の入口/落下/ジャンプから、木箱の地上実攻撃と3枚、最初の金コイン列5枚、4本の木の杭の実3周と各5枚、北西の輪8枚を実接触で回収し、金36枚/スコア36を確認。各杭は約5.8〜5.9秒で累積1080度を超え一度だけ5枚生成。Rでスコア0/静的コイン23枚の未取得/杭の周回とドロップ/箱を戻し、Mで絵前へ帰還。座標/速度/進行/コイン/角度/タイマーの直接設定なし。通常セーブ前後ABSENT、スター進行0と専用セーブ未保存。B PASS4449フレーム、UTC2026-10-16T15:01:53.952Z。Aは28枚まで実回収後の斜面で、実接地とグラフ中心の足元に110cm差があり経由点234でFAIL4343。実接地中の経由点だけ高さ差160cmを許容し、実コイン接触の条件を変えず両ビルド後Bで成功。残る10枚は山道と高台砲台横の2列で、実床への配置までを確認し実回収は次の100枚の入場で行う。Native結果文のMountain/islandという広い表現に対し、実際のXYは山道(1851,2234)と高台(4836,2707)である。100枚スター7は今回未実装・未取得で、次の単位へ継続する。

## 実行結果

```text
PASS
Actual36 field golds: normal wooden box punch and three pickups, source ground line5 and ring8, four posts with actual3circles under200/30 seconds and five pickups each, one generation per post. Real R restored score0/static coins/posts/box and removed only own drops, then M returned to painting. No progress awarded or injected. Mountain/island10 golds are placed but their actual pickup follows in the hundred-star visit.
CaptureFrames: 4449
UTC: 2026-10-16T15:01:53.952Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 296.6, "frames": 4449, "bytes": 55973131}

SHA-256: 444b4a34793750618c1c22729079466637f0bd9d5b9a96d49ada6099d6fcfc0f

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。