# PR #55 実ゲーム検証

対象コード: 83197d31fcb8c84e52d6a25aca2ad40842d6a411

# PR55 Draft後セルフレビュー

対象ソース: 53d624b5f03ead85f87a092a105df1d2e3164114

- 検証専用フラグの追加のみ。製品の保存/報酬処理を変えず、実スターActorの接触からSaveGameToSlotを呼ばせる。
- 隔離された新規Validationスロット・報酬0・既存ファイル/ディレクトリなしを要求する。自分の保存先と同じ名前の空ディレクトリを作る。保存内容や権限を書き換えない。
- 実保存失敗でProblemが出た後、報酬0・ファイルなし・スター可視/未取得/成功回数0・通常操作状態を確認する。演出や帰還へ進んでいないことも確認する。
- 実際に歩いて離れ、床に着いて250cm以上離れた状態で1.25秒待つ。自分で作った同一の空ディレクトリであることを再確認し、再帰なしでのみ除去する。中身や別のパスがあれば中止する。
- 実歩行/ジャンプでスターに再接触し、保存成功/エラー解除/取得ポーズ/カメラ/自動絵帰還と再入場後の表示まで確認する。体力・座標・速度・報酬データの注入はない。
- 途中で失敗して自分の空ディレクトリが残っても、通常スロットや既存検証スロットには触れない。通常セーブのSHAを起動前後に照合する。

- build7408両方成功。Save_Aは実保存失敗（UTC08:41:08.371）→実離脱と空ディレクトリのみ除去（08:41:11.001）→再接触による実保存（08:41:11.659）→絵帰還と再入場でPASSした。一方、失敗画面の実画像で日本語メッセージの欠字を検出し、完成とはしない。
- コースと選択画面の保存エラーに、日本語対応書体を明示する修正を追加した。失敗したQAの結果文に成功シナリオを追記しない修正も加えた。

- 修正ソース83197d31fcb8c84e52d6a25aca2ad40842d6a411のEditor/Game build7409成功。fresh Validation_SaveRetry_BでPASS1681フレーム（UTC08:47:00.718）。実失敗08:46:34.007、実離脱/自分の空フォルダ除去08:46:36.594、再接触による保存成功08:46:37.235。失敗画面で「ほぞんにしっぱい。はなれて もういちど ふれてください。」が読めることを目視確認した。通常セーブはABSENTのまま。

再試行で保存したデータの別プロセス読み取りもPASS249。実セーブのSHA-256一致、通常セーブABSENTを確認。[復元結果](ReadValidation.md) / [最終セルフレビュー](SelfReview.md)。新地形の他目標・全7取得、実死亡と獲得済み再取得の検証は残る。


## 実行結果

```text
PASS
Actual M return and painting reentry, second physical drop, reference post3 and gate collision, actual star6 save, pose/camera/held star, transition input spam and duplicate guard, automatic painting return, earned6 menu display persisted. Actual SaveGameToSlot failure from own empty QA directory, rollback0 and visible star, physical exit, nonrecursive removal of own empty obstacle, real recontact saved6 once and returned to painting.
CaptureFrames: 1681
UTC: 2026-10-16T08:47:00.718Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 112.067, "frames": 1681, "bytes": 23352465}

SHA-256: 8e4d5bbdf9c5b5aff67eec0396c447e54719a8d4083b2c9d24e2308eb8946a4f

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。