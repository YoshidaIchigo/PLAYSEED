# 水玉大砲のセルフレビュー

- 乗り込む大砲とは別のActorが、自分の台座・砲身・水玉だけを所有します。全構成部品をUPROPERTYで保持し、ステージの配列もUPROPERTYです。
- 山側はUE(1388,2383,2918)、yaw-135度で常設。野原側はUE(-5694,-5600,128)、yaw45度で目的1のみ。野原側のパラメータ1は発射せず、山側のパラメータ0のみ発射します。
- 距離2000cmで表示開始、2500cmで非表示。台座の半径220cm/高さ300cmは実カプセルとの衝突を持ちます。
- 水玉は当たり判定なし、30Hz換算100フレームの直進です。2秒の照準後に発射、130/30秒周期。落下水玉のダメージ処理と混同しません。水玉を生成するQA窓口はありません。
- Rは自分の水玉・照準・時計を初期化し、読取専用の発射累計を残します。検証は通常歩行、Space、カメラ、R/Mのみで、主人公の位置・速度・体力・進行を設定しません。
- Source e30e713217880c0096482afd845870cd823fedf7のEditor7445/Game Developmentビルドは両方成功。最初の目的からの通常歩行で野原側の実台座/砲身と3秒以上の非発射を確認し、M→絵→目的6→実落下で再入場しました。山側への実経路を撮影中です。

一次資料: [大砲の挙動](https://github.com/n64decomp/sm64/blob/master/src/game/behaviors/water_bomb_cannon.inc.c)、[水玉](https://github.com/n64decomp/sm64/blob/master/src/game/behaviors/water_bomb.inc.c)、[配置](https://github.com/n64decomp/sm64/blob/master/levels/bob/script.c)、[マクロ配置](https://github.com/n64decomp/sm64/blob/master/levels/bob/areas/1/macro.inc.c)。形・素材・運動処理は自作です。


## 最終C

- Source 724e6e21d766a7e25e04c95dadcbde4aac797d5e、Editor7447/Game両ビルド成功。2677フレーム/UTC2026-10-16T18:13:18.113ZでPASS。A/Bも機能検証はPASSですが、水玉が中央の体力表示と重なるため通常カメラを横60度/上5度へ修正しました。
- 全2677フレーム/178.466667秒/960×540/15fps。MP4 SHA-256 beb81cccf8fced3c9f357a38c48972a2825c021d97ecec8516851c45c2e15d62、31155766bytes。元AVI c150ba6ea856894ac38be6c0a4c74a18d058fcf0d88d95797e7d76e0bb7ed16d は不変で、全フレーム数と連続PTSを検証。
- 最終MP4を直接1秒ごとに全編復号し、山側の観察169/170、照準173、R後HP8/入口175、絵前7の178を目視。167秒から7秒を全15fpsで直接復号し、25/26/27で砲口を離れる実青水玉を確認。パネルは最終MP4の26と実撮影した写真を使います。
- 同じ実7スターの専用セーブ6691511A3066D8C2DBE962B5CD0CB5A727F93A009FC3B81F71D28E57F329B683不変、通常セーブABSENT。野原側の非発射/再入場/実山道とジャンプ/2発以上/観察中HP不変/R水玉消去とHP8/M帰還を検証。
