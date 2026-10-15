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
雷鳴の丘では体力が8点あり、南の赤い「SPARKING THISTLE」に触れると2点減って押し戻されます。被弾後は1.2秒間無敵になります。体力切れや場外落下で仕掛けと体力を初期化して開始位置へ戻り、獲得済み報酬は保持します。Rでも体力を戻せます。Gardenの落下時は収集を保持する従来の動作です。`-ThunderDamageTest` と選択画面・Garden検証フラグで、実際の接触・無敵・押し戻し・死亡・落下・再開を確認します。
空中で左Ctrl / ゲームパッドRBを押すとヒップドロップ（dive）で急降下します。地上や被弾後の無敵中は開始しません。着地で一度だけ衝撃を通知し、R・被弾・死亡では解除します。`-ThunderPoundTest` と選択画面・Garden検証フラグで、通常のジャンプからの急降下、着地一回、R・実接触による被弾の中断、通常移動の復元を確認できます。
青緑の「THE TIDESTONE」に近づいてE / ゲームパッドXを押すと持ち上げ、もう一度押すと前へ投げます。持って歩けます。被弾・ヒップドロップで手放し、R・死亡時は物も元の位置へ戻します。`-ThunderCarryTest` と選択画面・Garden検証フラグで、届かない物の拒否、E/Xでの保持、運搬、物理投擲と着地、R・被弾・落下の解除を確認できます。
西側の「THE HUSHBLOOM TREE」へ近づきF / ゲームパッドYで取り付きます。W/Sまたは前後スティックで上下し、頂上の枝の足場へ立てます。Space / Aで飛び離れ、F/Yでも手を離します。R・被弾・死亡でも解除します。`-ThunderClimbTest` と選択画面・Garden検証フラグで、取り付き、上下、頂上の衝突、ジャンプ、R後の幹の衝突復元を確認できます。
南の「THE TUFTLING」は踏みつけを試せる静止した敵です。通常ジャンプで上から足元を当てると倒して跳ね返り、横から触れると2点被弾します。倒した後は重複して接触せず、Rで復活します。追跡AIは別途追加する段階です。`-ThunderStompTest` と選択画面・Garden検証フラグで、実接触と通常ジャンプによる二度の踏みつけ・横との区別・R復活を確認できます。

左クリック / ゲームパッドRTで、近くの正面へパンチ→パンチ→キックを繰り出します。短い間隔で三段がつながり、少し待つと最初のパンチへ戻ります。歩きながら攻撃でき、ジャンプ・被弾・R・木登り・急降下で解除されます。物を持っている間は使えません。練習敵はパンチ2回とキック1回で倒れ、踏みつけでも倒せます。-ThunderCombatTest と選択画面・Garden検証フラグで、範囲外・横向きの空振り、三段の威力、連打制限、R・実被弾の解除を確認します。

光コインは金1点・赤2点・青5点です。開始地点の北東に三種類を置いています。触れると消え、同じIDは一度だけ集計します。光コインと赤コイン数は現在の入場中の値で、R・死亡・再入場で戻ります。保存済みランタンは保持します。-ThunderCoinTest と選択画面・Garden検証フラグで三種類の値、重複防止、Rと再入場を確認します。

東の銅色の箱「THE COPPER CACHE」は、左クリック/RTの攻撃、または通常ジャンプから上面への急降下で壊せます。金の光コインが三個だけ出ます。壊れた箱を続けて攻撃しても増えません。Rで箱が復活し、出した三個は片付け、集計も0へ戻ります。-ThunderBoxTest と選択画面・Garden検証フラグでパンチと急降下の両方、単一生成、回収、再開後の再生成を確認します。

北の草原に四つの花輪状の金コイン80個、山道に20個を配置しています。地上から続けて歩いて100点分を集められる経路です。これはPLAYSEED独自の配置で、参照映像の全コイン配置を復元したものではありません。100点のランタン報酬は後続の実装です。-ThunderCoinTrailTest と選択画面・Garden検証フラグで、全100個の実回収、Rと再入場を確認します。

山道の「THE STORMSTONE RUN」では、琥珀色の筋がある岩球が一定の経路を下ります。道の端や通常ジャンプで避け、触れると2点被弾して押し戻されます。終点で消え、1.5秒後に上側へ戻ります。出現地点に近い間は待機します。Rで経路を初期化します。これは経路に沿って動く仕掛けで、自由落下の物理シミュレーションではありません。-ThunderBoulderTest と選択画面・Garden検証フラグで、実接触・致命傷からの再開・R・徒歩回避・安全な再出現を確認します。

南の「THE RAINFALL PEARL」は、琥珀色の円で1.6秒予告してから水弾を落とします。円から離れると避けられ、着弾の範囲内では2点被弾して押し戻されます。判定は着弾ごとに一回です。Rで落下中でも古い弾を止め、警告から戻ります。-ThunderWaterTest と選択画面・Garden検証フラグで警告・実被弾・一度だけの着弾・落下中のR・警告からの退避を確認します。

東の「THE ROAMING TUFTLING」は、近くで見つけるとプレイヤーを追い、遠くへ逃げると元の場所へ帰ります。横の接触では2点被弾します。先に左クリック/RTの攻撃を当てるか踏みつけて倒すと、金コインが一個だけ出ます。Rで復活し、その敵が出したコインは片付きます。-ThunderChaseTest と選択画面・Garden検証フラグで発見・追跡・逃走・帰還・二度の撃破と単一ドロップを確認します。

南東の赤い敵「THE SCARLET SEEDBURST」は、450cm以内へ近づくと2秒の導火線が始まります。半径300cmの爆発から離れて避けるか、先に攻撃・踏みつけで倒して止められます。爆発内では2点被弾します。Rで復活し、進行中の導火線も止まります。-ThunderBurstTest と選択画面・Garden検証フラグで実爆発・退避・先制攻撃・導火線中のRと遅れた爆発が残らないことを確認します。
