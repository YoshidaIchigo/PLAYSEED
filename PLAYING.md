# PLAYSEEDの遊び方

Unreal Engine 5.8.1で`PLAYSEED/PLAYSEED.uproject`を開き、Playを押します。既定マップは`/Game/ThunderMeadow/MissionSelect`で、1〜6で目的を選んで入場します。Mで目的選択へ戻ります。旧ステージ選択リンクは同じ目的選択へ転送します。

WASD/左スティックで移動、Space/Aでジャンプ、Shift/LBで走る、マウス/右スティックでカメラ、ホイールで距離、R/Bでやり直し、Ctrl/RBで急降下、LMB/RTで攻撃、E/Xで持つ・投げる、F/Yで木登り、Q/上で滑走、G/下で会話・大砲に入ります。照準中はマウス/右スティックで狙いLMB/RTで発射します。

6つの主報酬と同じ入場中の100コイン報酬を集めます。獲得済みIDと解放状況は`Saved/SaveGames/Playseed_ThunderProgress.sav`に保存し、Rや再入場で失いません。通常/赤/青のコインの値は1/2/5です。

一回の入場の残機は3。1UPで増え、死亡と場外落下で1減ります。Rは消費せず、同じ入場の取得済み1UPは復活しません。0で選択へ戻り、再入場で3から再挑戦できます。回復点は地上の接触で体力2回復、上限8、5秒待って離れれば再利用できます。

検証は`-StageSelectionSmokeTest -ThunderValidation -PlayseedValidationSlot=Validation_<固有名>`に個別の検証フラグを付け、専用スロットを使います。実ワールドの自動入力による検証で、人による実機入力とは区別します。録画は`Saved/Validation/CoursePlaythrough.avi`、結果は`Result.txt`です。過去のGarden専用の検証フラグとマップは使用しません。
