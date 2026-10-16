# PR #72 実ゲーム検証

対象コード: 3455616a1167be1d2d6e53682cdda59cd33f088d

対象ソース3455616a1167be1d2d6e53682cdda59cd33f088d、Editor7459/Game Developmentは両方成功。自作Grass/Cave/Sky/Woodの4枚をUnrealへ取り込み、0エラー/0警告、変更アセット4枚だけを確認しました。

最終Cは1769フレーム/UTC2026-10-16T20:41:29.197ZでPASSし、正常終了も確認。実7スターの専用セーブから、通常の帽子箱の頭突き/実帽子取得、通常G/マウス/LMBの大砲、通常マウス/W/Sで実5中心、獲得済5の青スター/NEW0/7保持、実ポーズ/白フェード/絵前帰還、主6個獲得済み選択を確認しました。主人公の位置・速度・コイン・能力・進行をQAから直接設定していません。

専用セーブSHA-256は前後で6691511A3066D8C2DBE962B5CD0CB5A727F93A009FC3B81F71D28E57F329B683のまま、通常セーブはABSENTです。

Aは1308フレームで2番目の中心から約130cmの通常入力周回により時間切れ。ゲーム側の速度/能力/収集判定を保ち、次の中心の手前650cmから向きを揃え、通常Sで減速して通過する操作へ修正しました。

この単位は素材と色です。モデルと水平な飛行姿勢、必要な閉鎖柵の経路/実敵コイン集計を修正した後、同じ最終コードと資産の新規同一セーブ全7通しを行います。

Bも1771フレームで実動作PASSしましたが、全編動画で追加歩行敵が取得ポーズに重なる不具合を確認。取得と白フェード中だけ追跡/接触を停止し、近い700cmの個体を非表示、遠方4000cm超の既存非表示も維持する製品修正を入れました。最終Cの実ポーズで近い敵が隠れることを観測し、普通のプレイでの位置/体力/コインを変更していません。



## 実行結果

```text
PASS
Original calmer grass, gray cave, fine clouds/ocean/remote hills and wood-grain surface assets; reference gold/red coins use own unlit palette. Actual ordinary cap-box pickup, G/mouse/LMB cannon, mouse/W/S flight through five real centres, owned blue star5 NEW0/count7 unchanged, pose/white fades/painting return and all-six-main-stars selection. No injected coins, power, position or progress.
CaptureFrames: 1769
UTC: 2026-10-16T20:41:29.197Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 117.934, "frames": 1769, "bytes": 18675582}

SHA-256: d81434252e7d90e50655767d2d1d3cdb2affa949fdadb9d57613900872765c9b

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。