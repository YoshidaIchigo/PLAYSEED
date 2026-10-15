# Windseed Garden

PLAYSEED最初の小さな3D探索アクションステージです。種の探索者が浮島を巡り、6個のwindseedを集めて花の灯台を起動します。キャラクター、名称、ステージ配置と配色はこのプロジェクト用のオリジナルです。形状はUnreal Engine同梱の基本メッシュから構成し、外部素材は使っていません。

## 起動

Unreal Engine **5.8.1**とC++開発環境が必要です。`PLAYSEED/PLAYSEED.uproject`を開き、モジュールのビルド後にPlayを押してください。既定マップは`/Game/Windseed/WindseedGarden`です。PIE開始後にゲーム画面をクリックするとマウス操作できます。

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

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' PLAYSEEDEditor Win64 Development '-Project=C:\PLAYSEED\PLAYSEED\PLAYSEED.uproject' -WaitMutex -NoHotReloadFromIDE
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' PLAYSEED Win64 Development '-Project=C:\PLAYSEED\PLAYSEED\PLAYSEED.uproject' -WaitMutex
```

開発ビルドの`-GardenSmokeTest`は、実際のゲームワールド内で落下復帰、走行速度、移動入力によるルート探索、ジャンプ、6個の収集、ゴールとリスタートを検証して終了します。ルート上の移動にはCharacterMovementの通常の移動入力を使い、島間をテレポートしません。落下復帰の検証だけは意図的にプレイヤーを島の下へ移します。通常プレイでは自動操作されません。

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' 'C:\PLAYSEED\PLAYSEED\PLAYSEED.uproject' -game -windowed -ResX=960 -ResY=540 -GardenSmokeTest -GardenCapture -unattended -nosound -NoSplash -log
```

`-GardenCapture`を付けると実ゲームのビューポートとHUDをAVIへ記録します。出力は`PLAYSEED/Saved/Validation/`の`Result.txt`、`GardenComplete.png`、`WindseedPlaythrough.avi`です。録画は開発用で、通常プレイでは動作しません。これらの生成物はGitへ追加しません。PRには検証済み動画へのリンクを別途添付します。自動操作での検証と、人がキーボードやゲームパッドで行う確認は区別して記録してください。

## アセットの再生成

マップと配色マテリアルはリポジトリに含めます。失われた場合のみ、Editorを閉じて次を実行してください。既存アセットを上書きしません。

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\PLAYSEED\PLAYSEED\PLAYSEED.uproject' -run=pythonscript '-script=C:\PLAYSEED\Tools\create_garden_assets.py' -unattended -NullRHI -nosound
```

ホストCIは生成物・秘密設定の混入、プロジェクトとマップの整合性、Python構文、空白を検証します。Unrealのビルドと実ゲーム確認はUEをインストールしたWindows環境で別に実行する必要があります。
