# PR #47 実ゲーム検証

対象コード: 0912fcaf05c8aeb32aef7dd0f18117bc805237f0

Draft PR47作成後のセルフレビュー。描画1101三角形と衝突1060三角形を分け、元の軸を(X,-Z,Y)へ変換していることを資料と照合した。既存ユーザーのEditor28308は保持。通常セーブを使わず、新規Validation_スロットかつ報酬0をQA開始条件にしている。ReferenceCourseの明示オプション以外で旧配置の実行経路は変えない。
新地形はProceduralMeshComponentで同期Collisionを生成してからGameModeが通常の開始点へ戻す。地形QAはCharacterMovementの歩行・通常Jump・R・Mを通し、座標・速度・体力・報酬の注入は行わない。木橋と山道の段差は実歩行で確認する。Floor helperは今後の配置にも使うが、QAの合格を座標設定で作らない。
素材は自作の固定乱数パターン。元画像・音・ROMを含まない。生成スクリプトは既存アセットを保全する。素材生成・両ビルド・実画面比較はこれから。
実レビュー追記: 初回ビルドでUE必須include順序と巨大なFVector配列の動的初期化C4883を検出。0e8074fで定数POD配列へ変更してEditor/Game7383成功。初回実歩行Aは開始砲台の台座で停止しFAIL312フレーム、通常セーブABSENT。実画面でM_ActorのClamp未接続とM_SkyのSamplerNormal残存を検出。5def43aで接続成功をassertし、色Samplerと専用素材のみを再生成、露出を固定し、128cmの台座へ通常Jumpを使う検証ルートを修正した。A録画は破棄せず私的作業フォルダへ保管する。
根本原因の修正: 実歩行D/Eで地面に接触中もMovementMode=Falling、Z速度0、通常Spaceでもジャンプ不可を実測。EngineのProceduralMeshComponentがPhysicsTriMeshData.bFlipNormals=trueとする実装を確認し、参照CCW三角形をUEの衝突用CWへ反転した(0db12aa)。開始点で実LineTraceの上向き法線とIsMovingOnGroundを必須条件に追加。Editor/Game7389成功。Reference_Fは開始の実ジャンプと橋を通過し、現時点で山道の高さ約2086cmまで実歩行、回復・座標注入なし。完走結果は未確定。
実歩行Reference_Fは0db12aaでPASS、2749フレーム、UTC2026-10-16T05:03:56.057Z。通常セーブABSENT、専用セーブのスター0・ファイルなしを保持。開始の通常ジャンプ、木橋、山道の実ジャンプ、山頂4293cm、R、Mを実測した。橋・山道・山頂のPNGを実際に開き、路面に誤った草帯、灰色の斜面、過度な最近傍表示を確認。参照動画140秒の実画面（POWER・ボム兵・草地・格子門）とも照合し、路面と岩・金属柵の区分、草色、補間と雲の高さを修正中。元動画画像のファイル書出しはブラウザの許可APIで成功していないため、参照の並列PNGは未完と明示する。素材修正後の最新コミットで再録画する。
最終0912fca: Editor/Game7390成功。Reference_GはPASS2761フレーム、UTC2026-10-16T05:14:19.687Z。通常セーブ前後ABSENT、専用スター0・保存ファイルなしを保持。修正後の開始点と橋PNGを実際に開き、路面の誤った草帯がなくなり、岩・土・草と柵を区別できることを確認した。素材コンパイル失敗はログになし。参照ファイルとの並列PNGは未完であり、この単位を原作のピクセル完全一致・配置統合の完了とは扱わない。次の単位で敵・砲台・収集物と入口を統合する。

## 実行結果

```text
PASS
Reference terrain ordinary walking: start, wooden bridge, winding mountain summit, actual R and M; no deaths, no reward fixtures or progress writes.
CaptureFrames: 2761
UTC: 2026-10-16T05:14:19.687Z

```

[実ゲーム動画](Playthrough.mp4) / [完了画面](Complete.png)

MP4情報: {"width": 960, "height": 540, "duration_seconds": 183.867, "frames": 2759, "bytes": 33448849}

SHA-256: 73e4243750ad1fd0f9ea72138bd7a336052767859894cb80f4ddf6e9b4fece19

動画は実際のUnrealゲームワールドを自動操作した記録です。人が入力機器を操作した動画ではありません。意図的な落下は復帰の検証だけで、収集ルートは通常の移動入力とジャンプを使います。