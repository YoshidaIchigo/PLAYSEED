# PLAYSEEDの遊び方

Unreal Engine 5.8.1向けのプロジェクトは `C:\PLAYSEED\PLAYSEED\PLAYSEED.uproject` です。ビルド完了後、リポジトリ直下の `Play.cmd` をダブルクリックすると、最新モジュールを読む別ウィンドウで起動します。通常起動は絵の入口 `/Game/Battlefield/Entrance` で、プレビュー用の `-ReferenceCourse` は不要です。開いたままのEditorは古いコードを保持する場合があるため、更新後の確認には新しいプロセスを使います。

絵へ走ってジャンプするとスター選択へ移り、Enterで入場します。WASD/左スティックで移動、Space/Aでジャンプ、Shift/LBで走る、マウス/右スティックでカメラ、ホイールで距離、Hで操作説明、Mで絵前へ帰還、R/Bで同じコースをやり直します。

Ctrl/RBでヒップドロップ、左クリック/RTで攻撃、E/Xで持つ・投げる、F/Yで木登り、Q/上で滑走、G/下で会話・大砲へ入ります。木ではW/Sで上下、Spaceで飛び離れます。大砲ではマウス/右スティックで狙い、左クリック/RTで発射します。入口の赤スイッチをヒップドロップすると羽根箱が使えます。帽子を拾った後、空中Spaceで飛び、マウスで方向、W/Sで速さを調整します。

獲得済みスターと解放状況は開発起動では `PLAYSEED\Saved\SaveGames\Playseed_ThunderProgress.sav` に保存します。スター取得後はポーズと白フェードを経て絵前へ戻ります。残機は入場時3、死亡と場外落下で1減り、Rでのやり直しは消費しません。獲得済みスターは保持します。現在もコインや危険物・素材の統合を続けており、最終コードの新規同一セーブ全7スター検証は完了時に別途記録します。

## Windows用パッケージを自分で作る場合

このリポジトリの `PLAYSEED\Binaries\Win64\PLAYSEED.exe` は開発用のビルド成果物です。配布用パッケージの作成と実行は別の確認です。

1. 上記 `.uproject` をUE 5.8で開きます。最新コードに更新した直後は、作業を保存してEditorを開き直してください。
2. 上部の **Platforms（プラットフォーム）→ Windows** で **Development** を選び、**Package Project（プロジェクトをパッケージ化）** を押します。
3. 出力先はリポジトリ外の **`C:\PLAYSEED-Packages`** などにします。ソースやContentのフォルダを出力先にしないでください。
4. 成功したら出力先のWindowsフォルダ内に生成された `PLAYSEED.exe` を起動します。配布時はexeだけでなく生成されたフォルダ一式を渡します。

必要な3マップと動的に読む素材はプロジェクトのパッケージ設定に明記しています。日本語のDroidSansFallback.ttfと英字のRoboto-BlackItalic.ttfはEngineの実行時依存一覧に含まれます。ただし、この変更だけで配布用パッケージの実動作を保証するものではありません。パッケージを作成した際には、入口・コース入場・日本語・保存と再起動も確認してください。

## 開発時の自動検証

自動入力は `-ThunderValidation -PlayseedValidationSlot=Validation_<固有名>` と各検証フラグを使い、通常セーブと分離します。`ThunderDefaultStartTest` はマップ引数や参照プレビューフラグを渡さず、通常の既定マップから入口・絵・選択・実落下・ジャンプ・M帰還を確認します。録画は `Saved/Validation/CoursePlaythrough.avi`、結果は `Result.txt` です。動画は実ワールドの自動入力記録で、人による操作とは区別します。
