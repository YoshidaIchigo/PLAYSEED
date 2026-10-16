# PR #62 実ゲーム検証

対象コード: ec6f3bcbc933823060e5f2593981a1e85ee6af8e

最終ソースec6f3bcbc933823060e5f2593981a1e85ee6af8e、Editor7432/Game両ビルド成功。初回Bで実獲得した正規1〜5セーブを戻さず、Dでは入口解放を読み、実箱頭突き・実帽子Overlap・普通の歩行・G/マウス/左クリック大砲・マウスとW/Sによる飛行で中央5コインを実取得し、固定位置の青い獲得済みスター5を取得。NEW0、総数5、地上ポーズ、白フェード、絵前帰還、再入場メニューで1〜5獲得と次の6を確認。通常セーブ前後ABSENT、専用セーブSHA C19B118708A831BC25E640246AAE59A03F4519D92F97F202E17364215B529DD1で不変。D全1725フレーム、UTC2026-10-16T14:21:12.246Z。初回4→5の実保存B動画はNewStar5.mp4へ別添し、ソース7f151e69...と1817フレームの出典を分ける。Bを直接確認して見つけた入口案内の見切れはy204/220へ修正。Cは新しいOwned検証フラグの参照地形登録漏れによるFAIL142で、登録を修正後D成功。座標/速度/進行/帽子/報酬の直接注入やセーブのコピー・巻き戻しをしていない。

## 実行結果

```text
PASS
Owned star5 full replay: legitimate saved wing unlock, ordinary cap-box head bump and actual pickup, normal G/mouse/LMB cannon, mouse/W/S flight through five actual centre coins, one fixed blue owned star5, NEW0 and earned count unchanged5, visible pose, white fades, painting return and next6 menu. No save copy/reset/injection.
CaptureFrames: 1725
UTC: 2026-10-16T14:21:12.246Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 115.0, "frames": 1725, "bytes": 26153811}

SHA-256: a8a64b5f07bf51313c6d5b6500313a251740502386081b2c8518ea265eac5acb

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。