# PR #42 実ゲーム検証

対象コード: d822eaf27fc395f17845dccaac7dd78a26e59857

一つの新規セーブと連続した実ゲームで全7個を取得した Validation_PR42_Campaign_A を、別ゲームプロセスからそのまま読みます。主1〜6と追加7の完全一致、実解放した発射台と翼、新しい赤コインと100コイン訪問、R・新しいWorldでも全7個を保持すること、Gardenを確認します。報酬fixtureや複数セーブの合成は使わず、通常セーブの前後指紋を残します。


## 実行結果

```text
PASS
Fall recovery, sprint, movement, camera follow, platform jumps, all 6 seeds, goal and restart passed.
Jumps: 6
CaptureFrames: 700
UTC: 2026-10-16T02:43:07.286Z

```

[実ゲーム動画](CampaignRead.mp4) / [完了画面](CampaignReadComplete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 46.467, "frames": 698, "bytes": 9180125}

SHA-256: e245bc83222029d1952923b141557e317811b921b50ef8a982a98e86ba64f867

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。