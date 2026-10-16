# PR #71 実ゲーム検証

対象コード: 223f182b8abc22b821f69a8c33315bd1316eb5b7

対象ソース223f182b8abc22b821f69a8c33315bd1316eb5b7、Editor7456/Game Developmentは両方成功。最終Cは919フレーム/UTC2026-10-16T20:14:53.919ZでPASSし、プロセスの正常終了も確認しました。

実床の12体（歩く11体/静止1体）を確認。普通の歩行から実追跡/点火した個体を通常Eで持ち上げ、Eで実物理投擲。実着地から1回だけ爆発し、普通に歩いて実1コインを取得。Rで12体/HP8/入口/コインを復元後、再度保持して実導火線5秒の切れを待ち、通常Drop/膨張/爆発でHP8→6と実押し戻し、実1コインを確認。2回目のRで12体/HP8/入口/集計0/自分のコイン削除、Mで絵前7へ帰還しました。

主人公/敵の位置・体力・コイン・進行をQAから設定していません。実7スター専用セーブSHA-256は6691511A3066D8C2DBE962B5CD0CB5A727F93A009FC3B81F71D28E57F329B683のまま、通常セーブABSENTです。

Aは実Drop後の5/30秒の膨張をQAが保持物消失と誤判定し816フレームでFAIL。観測の待ち条件だけを修正して再実行しました。最終新規同一セーブ全7通しは、素材/モデル/飛行姿勢と必要な経路/100枚集計の修正後です。

Bも906フレームで実動作PASSしましたが、動画レビューで実取得直後のRが早く、HUDのコイン1表示が短いため、実取得後に0.45秒表示待ちしてから通常Rを押す最終Cを新たに撮影しました。ゲーム本体の爆発と集計を変更していません。



## 実行結果

```text
PASS
Twelve reference small bombs including one stationary actor on actual floors. Ordinary approach lit a five-second fuse and real chase. Normal E held/threw one; actual physics landing caused an explosion and one real gold pickup. After normal R, holding another until its real fuse expired dropped it and caused exactly two damage and physical knockback, with one real gold. R restored all twelve and removed own drops; M returned to painting with legitimate earned7 unchanged. No injected hero or enemy position, health, coins or progress.
CaptureFrames: 919
UTC: 2026-10-16T20:14:53.919Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 61.267, "frames": 919, "bytes": 9252166}

SHA-256: 875fc1b10aadb6c0080db15cea41da7498ec6352a25a7f28a4b0f640bea04930

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。