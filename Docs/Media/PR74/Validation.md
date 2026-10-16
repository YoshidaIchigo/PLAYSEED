# PR #74 実ゲーム検証

対象コード: c764b8d328b18becfeea532548c4ca55cde167b5

最終ソースc764b8d328b18becfeea532548c4ca55cde167b5、Editor7466/Game両ビルド後。実閉鎖柵と周回鉄球の範囲を普通に歩いて迂回。通常LMB/実歩行敵コイン回収、実爆弾の通常LMB蹴り/物理着地・爆発待ち。実8赤コイン、シーソー/黒い軸の足場へ実接地とジャンプ、通常G/マウス/LMB大砲、F/W木登り、獲得済青スター4/NEW0/7保持、ポーズ・白フェード・絵前帰還・主6個獲得済表示。11286フレームの新しいPASSとプロセス終了を確認。実7専用セーブSHA-256 6691511A3066D8C2DBE962B5CD0CB5A727F93A009FC3B81F71D28E57F329B683不変/通常セーブABSENT。ゲームの位置/速度/体力/導火線/コイン/進行をQAから設定しない。MP4全編の直接復号レビューは出力後に実施する。

## 実行結果

```text
PASS
Own black checker axles with unchanged collision and actual CharacterMovement ride/jump. Ordinary closed-gate detour, normal enemy LMB/real gold, eight real red coins, seesaw, G/mouse/LMB cannon and actual island tree F/W climb, owned blue star4 NEW0/count7 unchanged, pose/white fades/painting return and all-six-main-stars selection. No injected hero/enemy position, health, coins or progress.
CaptureFrames: 11286
UTC: 2026-10-16T22:21:45.131Z

```

[動画1](PlaythroughPart01.mp4) / [動画2](PlaythroughPart02.mp4) / [動画3](PlaythroughPart03.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 752.4, "frames": 11286, "bytes": 114024518}

SHA-256: 0082bfc7744bccb1c6236cc7877168b69435af2b55ecec28a5eadbccca828a25

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。

全編のMP4は単一ファイル制限を超えるため、上記のMP4情報とSHAは保護した全編ファイルの値です。添付は再圧縮なしの分割動画です。全編と分割の全フレームMD5・順序・総数一致、各部分のPTS0と連続時刻、寸法と全編SHA不変を検証しました。[検証情報](SegmentVerification.json)。

|動画|全体の範囲（秒）|フレーム|SHA-256|
|---|---|---|---|
|[PlaythroughPart01.mp4](PlaythroughPart01.mp4)|0.000〜361.867|5428|ea7d0c13ad40d26f6cc439736939530691ea8296f68d84d987dca831ef1af853|
|[PlaythroughPart02.mp4](PlaythroughPart02.mp4)|361.867〜720.067|5373|7ebf6acf8428ad22ddf41a699673a92a2096cb81062bcbe551c101e3d51888c9|
|[PlaythroughPart03.mp4](PlaythroughPart03.mp4)|720.067〜752.400|485|ba7d2a63b746bfcde4c2b694c76f98d602d2b9e4b74d0541b17e1caca0e8e5cf|
