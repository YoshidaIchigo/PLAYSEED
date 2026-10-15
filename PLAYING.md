# Windseed Garden

PLAYSEED最初の小さな3D探索アクションステージです。種の探索者が浮島を巡り、6個のwindseedを集めて花の灯台を起動します。キャラクター、名称、ステージ配置と配色はこのプロジェクト用のオリジナルです。形状はUnreal Engine同梱の基本メッシュから構成し、外部素材は使っていません。

## 起動

Unreal Engine **5.8.1**とC++開発環境が必要です。`PLAYSEED/PLAYSEED.uproject`を開き、モジュールのビルド後にPlayを押してください。既定マップは`/Game/Playseed/StageSelect`です。1でWindseed Garden、2で追加ステージ「雷鳴の丘」（THUNDER MEADOW）へ入場します。プレイ中はMでステージ選択へ戻れます。PIE開始後にゲーム画面をクリックするとマウス操作できます。

雷鳴の丘を選ぶと6つの主ミッションの選択画面が開きます。1〜6で目的を選んで入場すると、HUDに選択した目的だけを表示します。100コインは同じ入場中に挑戦する追加報酬です。雷鳴の丘では草原・北西の段丘・北東の山道と山頂を探索できます。仕掛け・ミッションは引き続き開発中です。ステージ・ミッション選択にはキーボードを使います。

| 操作 | キーボード・マウス | ゲームパッド |
|---|---|---|
| 移動 | WASD | 左スティック |
| カメラ | マウス | 右スティック |
| ジャンプ（長押しで少し高く） | Space | 下ボタン（Xbox A） |
| 走る | 左Shift | 左ショルダー（LB） |
| やり直し | R | 右ボタン（Xbox B） |
| カメラ距離 | ホイール | — |
| マウス解放・再取得 | Tab | — |

メイン島の北にある展望台と、東へ登る灯台ルートを探してください。6個を集めたら、一番高い島の灯台に近づくとクリアです。落下するとスタート地点へ戻り、集めた種は保持されます。Rで種も含めて最初からやり直せます。

## ビルドと実ゲーム検証

雷鳴の丘の獲得済みランタンは `PLAYSEED/Saved/SaveGames/Playseed_ThunderProgress.sav` へ保存します。選択画面の `[earned]` と `LANTERNS n / 7` で確認できます。同じ報酬は一度だけ数え、Rでの再開や再入場では獲得状況を消しません。入場中のコイン数は保存対象に含めません。読めない既存セーブは上書きせず、HUDにエラーを表示します。現時点の報酬達成条件は後続PRで実装中です。

保存の自動検証は `-StageSelectionSmokeTest -GardenSmokeTest -GardenCapture -ProgressWriteTest -PlayseedValidationSlot=Validation_<固有名>` で新しい専用スロットへ試験データを保存し、別プロセスで `-ProgressReadTest` に変えて読み直します。検証用フラグには `Validation_` で始まる英数字・下線だけの専用名が必須で、通常プレイの進行には触れません。この試験データはミッションの実クリアを意味しません。

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' PLAYSEEDEditor Win64 Development '-Project=C:\PLAYSEED\PLAYSEED\PLAYSEED.uproject' -WaitMutex -NoHotReloadFromIDE
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' PLAYSEED Win64 Development '-Project=C:\PLAYSEED\PLAYSEED\PLAYSEED.uproject' -WaitMutex
```

開発ビルドの`-GardenSmokeTest`は、実際のゲームワールド内で落下復帰、W・Shift・Space・マウス・ホイールの入力イベント、移動入力によるルート探索、ジャンプ、6個の収集、ゴールとリスタートを検証して終了します。ルート上の移動にはCharacterMovementの通常の移動入力を使い、島間をテレポートしません。落下復帰の検証では意図的にプレイヤーを島の下へ移し、キー割り当ての検証後はスタートへ戻します。通常プレイでは自動操作されません。

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' 'C:\PLAYSEED\PLAYSEED\PLAYSEED.uproject' /Game/Windseed/WindseedGarden -game -windowed -ResX=960 -ResY=540 -GardenSmokeTest -GardenCapture -unattended -nosound -NoSplash -log
```

`-GardenCapture`を付けると実ゲームのビューポートとHUDをAVIへ記録します。出力は`PLAYSEED/Saved/Validation/`の`Result.txt`、`GardenComplete.png`、`WindseedPlaythrough.avi`です。録画は開発用で、通常プレイでは動作しません。これらの生成物はGitへ追加しません。PRには検証済み動画へのリンクを別途添付します。自動操作での検証と、人がキーボードやゲームパッドで行う確認は区別して記録してください。

`-ThunderTerraceTest`をステージ往復の検証フラグへ追加すると、草原から最上段まで通常移動とジャンプで巡り、意図しない落下復帰がないことを確認します。`Saved/Validation/ThunderTerraces.png`に最上段の画像を保存します。

`-ThunderMountainTest`をステージ往復の検証へ追加すると、山道をジャンプなしの通常移動だけで登って山頂へ到達し、`Saved/Validation/ThunderSummit.png`を保存します。

浮島「THE LANTERN ISLAND」を山の西側へ追加しました。現段階は地形と床衝突の確認で、発射台からの着地・箱の報酬は後続PRで実装します。`-ThunderIslandTest`は山頂まで普通に歩いて浮島を見渡し、実ワールドの床衝突をカプセル掃引で確認して`ThunderIsland.png`へ保存します。プレイヤーを浮島へテレポートしてクリア扱いにする検証ではありません。

草原の南西に木橋「THE TIMBER CROSSING」を追加しました。橋を歩いて渡り、中央の下を通常の身長で通れます。`-ThunderBridgeTest`をステージ往復検証へ追加すると、入口の段丘へジャンプ→橋上の徒歩→下の通路を歩く実ゲーム確認を行い、`ThunderBridge.png`へ記録します。

草原の東寄りに往復足場「THE DRIFTLEAF LIFT」を追加しました。飛び乗って運ばれ、東の段丘へ跳び移れます。Rで足場の動きも初期位置へ戻ります。`-ThunderPlatformTest`で、通常ジャンプでの乗り込み、入力なしで運ばれること、足場上のR復帰、再乗り込みと端へのジャンプを検証し、`ThunderMovingPlatform.png`へ保存します。

## アセットの再生成

`/Game/Playseed/StageSelect`から`-StageSelectionSmokeTest -GardenSmokeTest -GardenCapture`で起動すると、2→追加ステージの移動・ジャンプ・R→M→1→既存ステージ走破を連続検証します。録画はレベル移動をまたいで継続します。`-GardenCaptureTravelTest`は既存ステージを一度再読込して録画の寿命を確認する追加検証です。

マップ・配色・空のマテリアルはリポジトリに含めます。失われた場合のみ、Editorを閉じて次を実行してください。既存アセットを上書きしません。

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\PLAYSEED\PLAYSEED\PLAYSEED.uproject' -run=pythonscript '-script=C:\PLAYSEED\Tools\create_garden_assets.py' -unattended -NullRHI -nosound
```

ホストCIは生成物・秘密設定の混入、プロジェクトとマップの整合性、Python構文、空白を検証します。Unrealのビルドと実ゲーム確認はUEをインストールしたWindows環境で別に実行する必要があります。
雷鳴の丘の南側にある「THE BALANCE BOUGH」は、立つ位置に応じて傾く橋です。両端の高台から乗り、重みで低くなる側からジャンプして降ります。Rで水平へ戻ります。`-ThunderBalanceTest` を選択画面・Garden検証フラグと一緒に指定すると、通常の歩行で左右の傾きと再開を検証します。
雷鳴の丘の東側の近道には、紫の床スイッチがあります。上に立つと柵が上がり、入場中は通行できます。Rで柵とスイッチを初期化します。`-ThunderGateTest` と選択画面・Garden検証フラグで、閉じた柵の衝突、スイッチ、通行、R後の再操作を確認できます。
山道の「THE ECHO HOLLOW」に入り、紫の光へ歩くと上下のくぼみをワープできます。到着先で立ち止まっても戻されません。入口からいったん離れてから入り直すと逆方向へ移動できます。Rで作動状態を初期化します。`-ThunderWarpTest` と選択画面・Garden検証フラグで、山道からの到達・往復・暴発防止・山頂への徒歩・再開と再使用を確認します。
