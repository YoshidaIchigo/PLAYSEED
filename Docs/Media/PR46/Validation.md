# PR #46 実ゲーム検証

対象コード: 637ef8882c44c3fb97f8eaa9cd86e58c35cc3269

Draft46作成後のセルフレビュー。HUD専用BattlefieldUIは320x240基準の描画のみを持ち、ゲーム進行・保存値を変更しない。4:3のカメラと描画位置をそろえ、常時説明帯を外す。体力変化時のみ8分割POWERを見せ、Hの説明・会話・照準・GameOverも同じ枠内に描く。日本語はEngine付属DroidSansFallbackで描画し、ローカルWindows専用フォントに依存しない。
目的の候補はGameMode内だけ、Enter/Aなどで決定した時に初めて既存SelectMissionを呼ぶ。左/右・Dpad・スティックは同じNavigateへ集約し、数字1-6の直接入場は互換の短縮操作として残す。金色スターは同梱形状から5先端と目を組み、ID1-7/取得距離/重複ガード/保存失敗時再試行を保持する。コーンの複数MaterialスロットもTintを設定する。
検証は候補1→3→2→6の左右入力で保存や確定番号が変わらないこと、Enter確定、H開閉、既存の実3急降下・番獣衝突・門の奥のスター実取得6を使う。実データfixtureや取得IDを直接付与しない。地形・配置・主スター取得後の退出演出は後続作業であり、本PRだけで原作完全一致とはしない。

Draft後の実画面レビューで2件を修正。Canvasの有効なフォント判定にUFontが必要だったため、Engine同梱フォントをRuntime UFontとして所有するよう6c4fb93で修正。開閉をフレーム別に検証した結果、F1が開発ビルドのワイヤーフレーム切替と重なったため637ef88でHへ変更。旧録画A/Bは失敗証拠として私的作業フォルダに保全し、PR添付には使わない。637ef88のEditor/Game両ビルド7381成功。新規専用セーブValidation_PR46_Presentation_Cで撮ったメニューと説明のPNGを実際に開き、日本語・通常の塗りつぶし描画・4:3枠を目視確認した。


## 実行結果

```text
PASS
Single course entry, actual movement/jump/camera, R, invalid numbers, new World mission2 and return to mission selection passed. Garden removed.
SelectionJumps: 1
CaptureFrames: 1034
UTC: 2026-10-16T04:14:00.690Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 68.733, "frames": 1032, "bytes": 12672377}

SHA-256: 0044564c4b0b3f10ea171a8448dcbac2b60fd1d9bfd0e9c104d284e79b8fb2bd

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。