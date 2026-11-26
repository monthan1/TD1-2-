#include <Novice.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <Xinput.h>
#include "Controller.h"

const char kWindowTitle[] = "TD1_2回目!!!";

enum SCENE
{
	SCENE_TUTORIAL,
	SCENE_GAME,
};

// 砲台データ構造体
struct BoxData
{
	float x;
	float y;
	int width;
	int height;
	unsigned int color;
	int delay;
	int isExist;
};

// ひし形砲台データ構造体
struct Diamond
{
	float x;
	float y;
	float angle;
	float openSize;
	int isExist;
};

// ダッシュ軌道線データ構造体
struct DashLine
{
	float startX;
	float startY;
	float endX;
	float endY;
	int isExist;
};

// 丸い弾構造体
struct BlasterBeam
{
	float x;
	float y;
	float velocityX;
	float velocityY;
	float speed;
	int isShot;

	float trailX[8];
	float trailY[8];
};

// レーザー構造体
struct LaserBeam
{
	float x;
	float y;
	float velocityX;
	float velocityY;
	float length;
	float maxLength;
	int timer;
	int isShot;
	int sparkMade;
	int shakeMade;
};

// 残像データ（プレイヤー・ボス共通）
struct AfterImage
{
	float x;
	float y;
	float startX;
	float startY;
	int life;
	int isExist;
};

// 火花パーティクル構造体
struct Spark
{
	float x;
	float y;
	float velocityX;
	float velocityY;
	int life;
	int isExist;
};

// バリアが割れたときに飛び散る破片
struct BarrierShard
{
	float x;
	float y;
	float velocityX;
	float velocityY;
	float angle;
	float spin;
	float size;
	int life;
	int isExist;
};

// 放射状のヒットエフェクト
struct HitBurst
{
	float x;
	float y;
	float radius;
	int life;
	int isExist;
};

// ボスの爆発エフェクト
struct BossExplosion
{
	float x;
	float y;
	float radius;
	int life;
	int isExist;

	int chainMade;
	int generation;
};

// ボスの爆発パーティクル
struct BossExplosionParticle
{
	float x;
	float y;
	float velocityX;
	float velocityY;
	float radius;
	float size;
	int life;
	unsigned int color;
	int isExist;
};

// 爆弾構造体
struct Bomb
{
	float x;
	float y;
	float velocityX;
	float velocityY;
	int timer;
	int life;
	int isExist;
	int exploded;
	int isFalling;
	int isGlitch;
};

// グリッチパーティクル構造体
struct GlitchParticle
{
	float x;
	float y;
	float velocityX;
	float velocityY;
	int width;
	int height;
	int life;
	unsigned int color;
	int isExist;
};

// ステージの敵データ構造体（赤＝野生、青＝仲間）
struct StageEnemy
{
	float x;
	float y;
	float velocityX;
	float velocityY;
	int wanderTimer;
	int flash;
	int shootTimer;
	int isAlly;
	int isExist;
};

// 集めちゃダメな敵の構造体（呼びかけに触れると爆発する）
struct BadEnemy
{
	float x;
	float y;
	float velocityX;
	float velocityY;
	int wanderTimer;
	int isCollected;   // 1 = 呼びかけで集めてしまった（群れについてくる）
	int bombTimer;     // 集めてから爆発するまでのフレーム数
	int isExist;
};

// 小さな爆発エフェクトの構造体
struct SmallExplosion
{
	float x;
	float y;
	float scale;
	int life;
	int isExist;
};

// 仲間の弾構造体
struct AllyBullet
{
	float x;
	float y;
	float velocityX;
	float velocityY;
	int isExist;
};

// ボスの分身
struct BossClone
{
	float x;
	float y;
	float velocityX;
	float velocityY;
	int isExist;
};

// ボスが召喚する敵（赤）
struct SummonEnemy
{
	float x;
	float y;
	float velocityX;
	float velocityY;
	int life;
	int isExist;
};

// 残像の共通描画処理
void DrawAfterImage(float x, float y, float radius, int life, int maxLife, unsigned int baseColor)
{
	float ratio = (float)life / (float)maxLife;

	// 時間が経つほど小さくする
	int size = (int)(radius * (0.5f + ratio * 0.5f));

	// 時間が経つほど薄くする
	unsigned int alpha = (unsigned int)(ratio * 90.0f);

	unsigned int color = (alpha << 24) | baseColor;

	Novice::DrawEllipse(
		(int)x,
		(int)y,
		size,
		size,
		0.0f,
		color,
		kFillModeSolid
	);
}

// Windowsアプリでのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1920, 1180);

	Controller::Initialize();

	// time(NULL)現在の時刻を取得する　(unsigned int)数値の型を変換する　srand()ランダム数値の初期設定をする
	srand((unsigned int)time(NULL));

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	int bokasiTexture = Novice::LoadTexture("./Resources/bokasi.png");
	int bokasi2Texture = Novice::LoadTexture("./Resources/bokasi2.png");
	int hosiTexture = Novice::LoadTexture("./Resources/hikarin.png");
	int bakuhatuTexture = Novice::LoadTexture("./Resources/bakuhatu.png");
	int enTexture = Novice::LoadTexture("./Resources/en.png");
	int namiTexture = Novice::LoadTexture("./Resources/nami.png");

	int bossBgm = Novice::LoadAudio("./Resources/dopa3.wav");
	int bossBgmVoice = -1;   // 再生中のBGMのハンドル（SHIFTスキップで止めるために保存）
	/*int hikariTexture = Novice::LoadTexture("./Resources/hikari.png");
	int flashTexture = Novice::LoadTexture("./Resources/flash.png");*/

	// 爆発画像の拡大率
	const float kExplosionScale = 2.5f;
	const int kExplosionHalfSize = (int)(62.0f * kExplosionScale);
	// ボスのデータ
	float bossX = 960.0f;
	float bossY = 295.0f;
	float bossRadius = 90.0f;
	float bossSpeedX = 6.0f;
	int bossTrailTimer = 0;

	// ボスのシールド
	const int kBossShieldMax = 1800;
	int bossShield = kBossShieldMax;

	// ボスの形態（1:第1形態 2:第2形態）
	int bossPhase = 1;

	// 1 = ボスが無敵（回転レーザー中。シールドが減らない）
	int bossIsInvincible = 0;

	// 形態ごとのシールドの量（ゲージの割合の計算に使う）
	const int kBossShieldMax2 = kBossShieldMax + 500;   // 第2形態
	const int kBossShieldMax3 = 3000;                   // 第3形態（第2形態より多い）
	int bossShieldMax = kBossShieldMax;                 // 今の形態の最大シールド

	// 第3形態のバリア（プレイヤーの弾だけが壊せる）
	// const int kBossBarrierMax = kBossShieldMax2;         // バリアの耐久値（第2形態のシールドと同じ）
	const int kBossBarrierMax = 600;                     // バリアの耐久値（第3形態）
	const float kBossBarrierExtraRadius = 50.0f;         // ボスの体よりどれだけ外側までバリアがあるか
	int bossBarrier = 0;
	int bossBarrierBreakTimer = 0;                       // バリアが壊れたときの演出

	// バリアは、弾が当たったときだけ見える
	int barrierHitTimer = 0;                             // 残りの表示フレーム数
	const int kBarrierShowFrames = 45;                   // 当たってから見えている長さ

	// バリアが割れたときの破片
	const int kMaxBarrierShards = 40;
	const int kBarrierShardLife = 45;
	BarrierShard barrierShards[kMaxBarrierShards] = {};

	// 回転レーザー（シールドが半分以下になったら、各形態で1回だけ使う）
	int bossRotLaserUsed = 0;              // 今の形態でもう使ったか
	int bossRotLaserTimer = 0;
	float bossRotLaserAngle = 0.0f;        // 1本目のレーザーの向き（2本目は反対側）
	float bossRotLaserDir = 1.0f;          // 回る向き（1:時計回り、-1:反時計回り）
	int bossRotLaserHit = 0;               // 1 = この攻撃でプレイヤーに当たった
	// const int kRotLaserWarningFrames = 60;   // 予告線が出ている時間
	const int kRotLaserWarningFrames = 90;   // 溜め＋予告線が出ている時間
	const int kRotLaserFireFrames = 120;     // レーザーが回っている時間（半周）
	const float kRotLaserWidth = 110.0f;     // レーザーの太さ（極太）
	const float kRotLaserLength = 2600.0f;   // レーザーの長さ（画面の外まで届く）
	const float kRotLaserCenterX = 960.0f;   // レーザー中にボスがいる位置（画面のまん中）
	const float kRotLaserCenterY = 590.0f;

	// 第2・第3形態の大技（十字レーザー・バリア爆発・全画面ビーム）の共通
	int bossCrossUsed = 0;                 // 第2形態：十字レーザーを使ったか
	int bossBlastUsed = 0;                 // 第3形態：バリア爆発を使ったか（シールド2/3以下）
	int bossBeamUsed = 0;                  // 第3形態：全画面ビームを使ったか（シールド1/3以下）
	int bossBigTimer = 0;                  // 大技のタイマー
	int bossBigHit = 0;                    // 1 = この攻撃でプレイヤーに当たった
	const int kBigChargeFrames = 90;       // 溜めの長さ

	// 十字レーザー
	const float kCrossLaserWidth = 90.0f;  // レーザーの太さ
	const int kCrossWarnFrames = 60;       // 予告の長さ
	const int kCrossFireFrames = 30;       // レーザーが出ている長さ

	// バリア爆発（すき間のある衝撃の輪が何重にも広がる）
	const int kBlastRings = 4;
	const int kBlastRingInterval = 30;     // 輪を出す間隔
	const float kBlastRingSpeed = 10.0f;   // 輪が広がる速さ
	const float kBlastRingThickness = 36.0f;
	const float kBlastGapHalf = 0.6f;      // すき間の広さ（片側のラジアン）
	float blastGapAngle[kBlastRings] = {};
	int blastRingHit[kBlastRings] = {};

	// 全画面ビーム（画面の下の左か右の安全地帯以外すべて）
	const int kFullBeamChargeFrames = 150;
	const int kFullBeamFireFrames = 40;
	const float kFullBeamSafeW = 420.0f;
	const float kFullBeamSafeH = 320.0f;
	int fullBeamSafeSide = 0;              // 0:左下 1:右下

	// 反射レーザー（第3形態の大技。画面の端で何度も跳ね返る）
	int bossReflectUsed = 0;
	const int kReflectBounces = 3;               // 跳ね返る回数
	const int kReflectWarnFrames = 60;           // 予告の長さ
	const int kReflectFireFrames = 30;           // レーザーが出ている長さ
	const float kReflectLaserWidth = 44.0f;      // レーザーの太さ（細め）
	const int kReflectWaves = 5;                   // レーザーを出す回数
	const int kReflectMaxBeams = 12;               // 1回で出るレーザーの最大本数
	float reflectAngle[kReflectMaxBeams] = {};     // 1回の攻撃で出るレーザーの向き（ランダム）
	int reflectActiveCount = 0;                    // 今の回で出すレーザーの本数

	// 1 にすると、前の第3形態の大技（バリア爆発・全画面ビーム）に戻る
	const int kPhase3OldBigAttack = 0;

	// シールドブレイク
	int bossShieldBreakTimer = 0;


	// シールド破壊エフェクト
	int shieldBreakEffectTimer = 0;
	const int kShieldBreakEffectMax = 60;

	// スローモーション演出
	float gameSpeed = 1.0f;
	int slowMotionTimer = 0;

	const int kSlowMotionMax = 30;

	// ボスのシールドを減らす（弾・レーザーが当たったときに使う）
	const int kBeamDamage = 5;     // 弾(ブラスターの弾)が当たったときのダメージ
	const int kLaserDamage = 10;   // レーザーが当たったときのダメージ
	// canBreakBarrier : 1 = プレイヤーの弾（バリアを壊せる）、0 = レーザーなど（バリアに防がれる）
	auto damageBossShield = [&](int damage, int canBreakBarrier) {

		// 無敵中はダメージを受けない
		if (bossIsInvincible) {
			return;
		}

		// バリアが張ってあるときは、シールドには届かない
		if (bossBarrier > 0) {

			if (canBreakBarrier) {
				bossBarrier -= damage;

				if (bossBarrier <= 0) {
					bossBarrier = 0;
					bossBarrierBreakTimer = 30;
				}
			}

			return;
		}

		if (bossShield > 0) {
			bossShield -= damage;

			if (bossShield <= 0) {
				bossShield = 0;
				bossShieldBreakTimer = 120;

				// シールド破壊演出を開始
				shieldBreakEffectTimer = kShieldBreakEffectMax;

				// スローモーション開始
				gameSpeed = 0.3f;
				slowMotionTimer = kSlowMotionMax;
			}
		}
		};

	// boss
	int bossState = 0;
	int bossAttackTimer = 180;
	float bossAuraRadius = 0.0f;
	float bossAuraAlpha = 0.0f;
	float bossChargeVelocityX = 0.0f;
	float bossChargeVelocityY = 0.0f;
	float bossRotationAngle = 0.0f;
	int bossSpinTimer = 0;
	int ufoMoveTimer = 0;               // UFOのふわふわ用
	float bossSweepDirection = 1.0f;    // 爆撃で進む向き（1:右へ、-1:左へ）

	int bossPatternIndex = 0;      // 次の攻撃（0:通常突進 1:分身突進 2:召喚）
	int bossIsCloneCharge = 0;     // 1 = 今の突進は分身つき
	int bossCloneBombCount = 0;    // 分身の爆弾を間引く用
	int bossSummonTimer = 0;       // 召喚の間隔用
	int bossSummonCount = 0;       // 今回の召喚で出した数

	// プレイヤーのデータ
	float playerX = 960.0f;
	float playerY = 901.0f;
	float playerRadius = 45.0f;
	float playerAngle = -1.5708f;

	float speed = 12.0f;
	float dashDistance = 300.0f;

	int dashStunTimer = 0;
	const int kDashStunMax = 30;

	const int kDashMoveFrames = 6;
	int dashMoveTimer = 0;

	float dashStartX = 0.0f;
	float dashStartY = 0.0f;
	float dashTargetX = 0.0f;
	float dashTargetY = 0.0f;

	// 残像設定
	const int kMaxPlayerAfterImages = 6;
	const int kMaxBossAfterImages = 12;
	const int kMaxAfterImages = kMaxPlayerAfterImages + kMaxBossAfterImages;

	const int kAfterImageLife = 20;

	AfterImage afterImages[kMaxAfterImages] = {};

	// ボス残像発生タイマー
	int afterImageTimer = 0;

	// 火花パーティクル設定
	const int kMaxSparks = 200;
	Spark sparks[kMaxSparks] = {};

	// 放射状ヒットエフェクト設定
	const int kMaxHitBursts = 20;
	HitBurst hitBursts[kMaxHitBursts] = {};

	// ボス爆発エフェクト設定
	const int kMaxBossExplosions = 100;
	BossExplosion bossExplosions[kMaxBossExplosions] = {};

	// ボス爆発パーティクル設定
	const int kMaxBossExplosionParticles = 300;
	BossExplosionParticle bossExplosionParticles[kMaxBossExplosionParticles] = {};

	// 爆弾の設定
	const int kMaxBombs = 30;
	Bomb bombs[kMaxBombs] = {};
	int bombDropTimer = 0;

	// 分身の設定
	const int kMaxBossClones = 2;
	BossClone bossClones[kMaxBossClones] = {};

	// 召喚された敵（赤）の設定
	const int kMaxSummons = 8;
	SummonEnemy summons[kMaxSummons] = {};

	// 落ちない爆弾を置く（分身の爆弾・召喚した敵の自爆で使う）
	// ボスから伸びる太いレーザーにプレイヤーが当たっているか
	auto isPlayerInBeam = [&](float beamAngle, float beamHalfWidth) {

		float beamDirX = cosf(beamAngle);
		float beamDirY = sinf(beamAngle);

		float beamDx = playerX - bossX;
		float beamDy = playerY - bossY;

		float beamAlong = beamDx * beamDirX + beamDy * beamDirY;
		float beamSide = -beamDx * beamDirY + beamDy * beamDirX;

		return beamAlong >= 0.0f &&
			beamAlong <= kRotLaserLength &&
			fabsf(beamSide) <= beamHalfWidth + playerRadius;
		};

	// 反射レーザーの道すじを計算する（ボスから出て、画面の端で跳ね返る）
	// pathX, pathY には、折れ曲がる点が kReflectBounces + 2 個入る
	auto calcReflectPath = [&](float rayAngle, float* pathX, float* pathY) {

		float rayX = bossX;
		float rayY = bossY;
		float rayDx = cosf(rayAngle);
		float rayDy = sinf(rayAngle);

		pathX[0] = rayX;
		pathY[0] = rayY;

		for (int bounceIndex = 0; bounceIndex <= kReflectBounces; bounceIndex++) {

			// 壁にぶつかるまでの長さ（横の壁・縦の壁）
			float wallTx = 1.0e9f;
			float wallTy = 1.0e9f;

			if (rayDx > 0.0001f) {
				wallTx = (1920.0f - rayX) / rayDx;
			}
			else if (rayDx < -0.0001f) {
				wallTx = (0.0f - rayX) / rayDx;
			}

			if (rayDy > 0.0001f) {
				wallTy = (1180.0f - rayY) / rayDy;
			}
			else if (rayDy < -0.0001f) {
				wallTy = (0.0f - rayY) / rayDy;
			}

			float wallT = (wallTx < wallTy) ? wallTx : wallTy;

			rayX += rayDx * wallT;
			rayY += rayDy * wallT;

			pathX[bounceIndex + 1] = rayX;
			pathY[bounceIndex + 1] = rayY;

			// 当たった壁の向きを反転させる
			if (wallTx < wallTy) {
				rayDx = -rayDx;
			}
			else {
				rayDy = -rayDy;
			}
		}
		};

	// プレイヤーから線分 (ax,ay)-(bx,by) までの距離
	auto distPlayerToSegment = [&](float ax, float ay, float bx, float by) {

		float segDx = bx - ax;
		float segDy = by - ay;
		float segLen2 = segDx * segDx + segDy * segDy;
		float segT = 0.0f;

		if (segLen2 > 0.0f) {
			segT = ((playerX - ax) * segDx + (playerY - ay) * segDy) / segLen2;
		}

		if (segT < 0.0f) {
			segT = 0.0f;
		}

		if (segT > 1.0f) {
			segT = 1.0f;
		}

		float nearX = ax + segDx * segT;
		float nearY = ay + segDy * segT;

		return sqrtf((playerX - nearX) * (playerX - nearX) + (playerY - nearY) * (playerY - nearY));
		};

	// 点 (px,py) から線分 (ax,ay)-(bx,by) までの距離
	auto distPointToSegment = [&](float px, float py, float ax, float ay, float bx, float by) {

		float pointSegDx = bx - ax;
		float pointSegDy = by - ay;
		float pointSegLen2 = pointSegDx * pointSegDx + pointSegDy * pointSegDy;
		float pointSegT = 0.0f;

		if (pointSegLen2 > 0.0f) {
			pointSegT = ((px - ax) * pointSegDx + (py - ay) * pointSegDy) / pointSegLen2;
		}

		if (pointSegT < 0.0f) {
			pointSegT = 0.0f;
		}

		if (pointSegT > 1.0f) {
			pointSegT = 1.0f;
		}

		float pointNearX = ax + pointSegDx * pointSegT;
		float pointNearY = ay + pointSegDy * pointSegT;

		return sqrtf((px - pointNearX) * (px - pointNearX) + (py - pointNearY) * (py - pointNearY));
		};

	// 反射レーザー（reflectAngle の先頭 count 本）に、プレイヤーが入れる隙間があるか調べる
	// 隙間 = レーザーから十分離れていて、プレイヤーの今の位置から歩いて間に合う場所
	auto reflectHasGap = [&](int count) {

		// プレイヤーが入っても当たらない（ちょっと余裕をもたせた）距離
		const float kGapClearance = kReflectLaserWidth * 0.5f + playerRadius + 25.0f;

		// プレイヤーの今の位置から、この距離以内の場所なら間に合う
		const float kGapReach = 450.0f;

		// 全レーザーの道すじを先に計算しておく
		float gapPathX[kReflectMaxBeams][kReflectBounces + 2];
		float gapPathY[kReflectMaxBeams][kReflectBounces + 2];

		for (int gapBeamIndex = 0; gapBeamIndex < count; gapBeamIndex++) {
			calcReflectPath(reflectAngle[gapBeamIndex], gapPathX[gapBeamIndex], gapPathY[gapBeamIndex]);
		}

		// 画面を格子状に調べて、安全な場所が1つでもあれば隙間あり
		for (float gapX = playerRadius; gapX <= 1920.0f - playerRadius; gapX += 30.0f) {

			for (float gapY = playerRadius; gapY <= 1180.0f - playerRadius; gapY += 30.0f) {

				float gapReachDx = gapX - playerX;
				float gapReachDy = gapY - playerY;

				if (gapReachDx * gapReachDx + gapReachDy * gapReachDy > kGapReach * kGapReach) {
					continue;
				}

				int isGapSafe = 1;

				for (int gapBeamIndex = 0; gapBeamIndex < count && isGapSafe; gapBeamIndex++) {

					for (int gapSegIndex = 0; gapSegIndex <= kReflectBounces; gapSegIndex++) {

						if (distPointToSegment(
							gapX, gapY,
							gapPathX[gapBeamIndex][gapSegIndex], gapPathY[gapBeamIndex][gapSegIndex],
							gapPathX[gapBeamIndex][gapSegIndex + 1], gapPathY[gapBeamIndex][gapSegIndex + 1]) < kGapClearance) {

							isGapSafe = 0;
							break;
						}
					}
				}

				if (isGapSafe) {
					return true;
				}
			}
		}

		return false;
		};

	// 十字レーザーの向き（0回目:＋  1回目:×  2回目:＋と×を同時）
	auto crossBeamAngle = [&](int crossWave, int crossIndex) {

		if (crossWave == 0) {
			return 3.14159265f * 0.5f * (float)crossIndex;
		}

		if (crossWave == 1) {
			return 3.14159265f * 0.25f + 3.14159265f * 0.5f * (float)crossIndex;
		}

		return 3.14159265f * 0.25f * (float)crossIndex;
		};

	auto placeMineBomb = [&](float mineX, float mineY, int mineTimer) {

		for (int mineIndex = 0; mineIndex < kMaxBombs; mineIndex++) {

			if (!bombs[mineIndex].isExist) {

				bombs[mineIndex].x = mineX;
				bombs[mineIndex].y = mineY;
				bombs[mineIndex].velocityX = 0.0f;
				bombs[mineIndex].velocityY = 0.0f;
				bombs[mineIndex].isFalling = 0;
				bombs[mineIndex].isGlitch = 0;

				bombs[mineIndex].timer = mineTimer;
				bombs[mineIndex].life = 0;
				bombs[mineIndex].exploded = 0;
				bombs[mineIndex].isExist = 1;

				break;
			}
		}
		};

	// グリッチエフェクトの設定
	const int kMaxGlitchParticles = 200;
	GlitchParticle glitchParticles[kMaxGlitchParticles] = {};

	// 画面シェイク
	int screenShakeTimer = 0;
	int vibrationTimer = 0;
	int screenShakeX = 0;
	int screenShakeY = 0;

	// 放射状ヒットエフェクトの初期化
	for (int hitBurstInitIndex = 0;
		hitBurstInitIndex < kMaxHitBursts;
		hitBurstInitIndex++) {

		hitBursts[hitBurstInitIndex].isExist = 0;
	}

	// ボス爆発エフェクトの初期化
	for (int explosionInitIndex = 0;
		explosionInitIndex < kMaxBossExplosions;
		explosionInitIndex++) {

		bossExplosions[explosionInitIndex].isExist = 0;
	}

	// ボス爆発パーティクルの初期化
	for (int particleInitIndex = 0;
		particleInitIndex < kMaxBossExplosionParticles;
		particleInitIndex++) {

		bossExplosionParticles[particleInitIndex].isExist = 0;
	}

	// 爆弾の初期化
	for (int bombInitIndex = 0; bombInitIndex < kMaxBombs; bombInitIndex++) {
		bombs[bombInitIndex].isExist = 0;
		bombs[bombInitIndex].isGlitch = 0;
	}

	// グリッチパーティクルの初期化
	for (int glitchInitIndex = 0; glitchInitIndex < kMaxGlitchParticles; glitchInitIndex++) {
		glitchParticles[glitchInitIndex].isExist = 0;
	}

	for (int sparkInitIndex = 0; sparkInitIndex < kMaxSparks; sparkInitIndex++) {
		sparks[sparkInitIndex].isExist = 0;
	}

	for (int afterInitIndex = 0; afterInitIndex < kMaxAfterImages; afterInitIndex++) {
		afterImages[afterInitIndex].isExist = 0;
	}

	int dashCooldown = 0;
	const int kDashCooldownMax = 10;

	int dashCount = 0;
	const int kMaxDashes = 4;

	const int kMaxBoxes = 16;
	BoxData boxes[kMaxBoxes] = {};
	int boxCount = 0;

	Diamond diamonds[kMaxBoxes] = {};

	BlasterBeam beams[kMaxBoxes] = {};
	LaserBeam lasers[kMaxBoxes] = {};

	const int kMaxLines = 100;
	DashLine lines[kMaxLines] = {};
	int lineCount = 0;

	// 溜め技（レーザーモードで4回のダッシュが四角になると、四角の真ん中から超極太レーザーを出す）
	int megaLaserState = 0;                      // 0:なし 1:溜め 2:発射中
	int megaLaserTimer = 0;
	float megaLaserX = 0.0f;                     // 四角の真ん中（レーザーの出る場所）
	float megaLaserY = 0.0f;
	float megaLaserAngle = 0.0f;                 // レーザーの向き（ボスのほうを向き続ける）
	const int kMegaChargeFrames = 90;            // 溜めの長さ（1.5秒）
	const int kMegaFireFrames = 300;             // レーザーを出している長さ（5秒）
	const float kMegaLaserHalfWidth = 110.0f;    // レーザーの太さの半分（超極太）
	const float kMegaLaserLength = 2600.0f;      // レーザーの長さ（画面の外まで届く）
	const int kMegaLaserDamage = 3;              // 1フレームあたりのダメージ（300フレームで最大900）

	// プレイヤーのHP（ボスの攻撃に当たるとへる）
	const int kPlayerMaxHp = 5;
	int playerHp = kPlayerMaxHp;
	int playerInvincibleTimer = 0;               // 当たったあとの無敵時間（点滅する）
	const int kPlayerInvincibleFrames = 90;      // 無敵の長さ（1.5秒）
	int isPlayerDead = 0;                        // 1 = やられた
	int playerDeadTimer = 0;
	const int kPlayerDeadFrames = 150;           // やられてから敵集めに戻るまで（2.5秒）

	// 爆弾の爆発の大きさ（当たる範囲と、爆発の円の大きさ）
	const float kBombExplosionRadius = 120.0f;         // 通常の爆弾
	const float kGlitchBombExplosionRadius = 120.0f;   // グリッチ爆弾

	// 弾の追尾
	const float kBeamHomingRate = 1.0f;          // 1.0 = 毎フレームボスのほうへ向きを合わせる（絶対に当たる）。小さくすると曲がり方がゆるくなる

	// ダッシュのスタミナ
	const float kStaminaMax = 100.0f;
	const float kDashStaminaCost = 20.0f;        // ダッシュ1回で減る量（満タンで5回）
	const float kStaminaRegen = 0.6f;            // 1フレームで回復する量
	const int kStaminaRegenDelay = 40;           // ダッシュしてから回復が始まるまでのフレーム数
	float stamina = kStaminaMax;
	int staminaRegenTimer = 0;
	int staminaFlashTimer = 0;                   // スタミナが足りないとき、ゲージを赤く点滅させる

	// 砲台の初期化
	for (int boxInitIndex = 0; boxInitIndex < kMaxBoxes; boxInitIndex++) {
		boxes[boxInitIndex].isExist = 0;
		diamonds[boxInitIndex].isExist = 0;
		diamonds[boxInitIndex].openSize = 0.0f;
		boxes[boxInitIndex].delay = 0;
		beams[boxInitIndex].isShot = 0;

		lasers[boxInitIndex].isShot = 0;
		lasers[boxInitIndex].length = 0.0f;
		lasers[boxInitIndex].timer = 100;
		lasers[boxInitIndex].sparkMade = 0;
		lasers[boxInitIndex].shakeMade = 0;
	}

	for (int lineInitIndex = 0; lineInitIndex < kMaxLines; lineInitIndex++) {
		lines[lineInitIndex].isExist = 0;
	}

	int isAttacking = 0;
	int attackTimer = 0;
	int currentAttackIndex = 0;
	int currentFrame = 0;
	int isCurrentFrameCount = 0;

	int attackType = 1;

	// ステージ（ボス戦前）の設定
	const int kSceneStage = 0;
	const int kSceneBoss = 1;
	int scene = kSceneStage;

	// ステージのプレイヤー
	float stagePlayerX = 960.0f;
	float stagePlayerY = 1000.0f;
	const float kStageSpeed = 8.0f;

	// 扉の設定
	const float kDoorX = 960.0f;
	const float kDoorY = 170.0f;
	const float kDoorWidth = 160.0f;
	const float kDoorHeight = 220.0f;

	// 敵（野生＝赤、仲間＝青）の設定
	const int kMaxStageEnemies = 50;
	const float kStageEnemyRadius = 12.0f;
	StageEnemy stageEnemies[kMaxStageEnemies] = {};
	int allyCount = 0;

	// 呼びかけ（仲間を集める波）の設定
	const int kCallFrames = 20;
	const float kCallStartRadius = 30.0f;
	const float kCallSpeed = 14.0f;
	const int kCallCooldownMax = 30;
	int callTimer = 0;
	int callCooldown = 0;
	float callRadius = 0.0f;

	// エイム線の設定（SPACE長押しで表示、離したら呼びかけ）
	// const float kAimLength = kCallStartRadius + kCallSpeed * kCallFrames;
	const float kAimLength = kCallStartRadius + kCallSpeed * (kCallFrames - 1);   // エイム線の長さ（呼びかけの円の端まで）
	float aimDirX = 0.0f;              // エイムの向き（最後に動いた向き。最初は上＝扉の方向）
	float aimDirY = -1.0f;
	float aimEndX = 0.0f;              // エイム線の先端
	float aimEndY = 0.0f;
	float callCenterX = 0.0f;          // 呼びかけの波の中心
	float callCenterY = 0.0f;
	int isAiming = 0;                  // 1 = SPACE長押し中
	int preAiming = 0;

	// 仲間がプレイヤーの後ろに群れでついてくる設定（ピクミン風）
	const float kFlockSpacing = 18.0f;      // 群れの密度（小さいほどぎゅっと集まる）
	const float kFlockGap = 45.0f;          // プレイヤーと群れの間の距離
	float behindDirX = 0.0f;                // 「後ろ」の向き（プレイヤーの進行方向の逆）
	float behindDirY = 1.0f;
	float prevFollowX = 0.0f;
	float prevFollowY = 0.0f;
	int followTimer = 0;

	// 後ろの向きをリセットする
	auto resetFollow = [&](float x, float y) {
		behindDirX = 0.0f;
		behindDirY = 1.0f;
		prevFollowX = x;
		prevFollowY = y;
		};

	// プレイヤーの動きから「後ろ」の向きをゆっくり更新する
	auto updateBehindDir = [&](float x, float y) {
		float moveDx = x - prevFollowX;
		float moveDy = y - prevFollowY;
		float moveLen = sqrtf(moveDx * moveDx + moveDy * moveDy);

		prevFollowX = x;
		prevFollowY = y;
		followTimer++;

		if (moveLen > 0.5f) {
			behindDirX += (-moveDx / moveLen - behindDirX) * 0.08f;
			behindDirY += (-moveDy / moveLen - behindDirY) * 0.08f;

			float dirLen = sqrtf(behindDirX * behindDirX + behindDirY * behindDirY);

			if (dirLen > 0.001f) {
				behindDirX /= dirLen;
				behindDirY /= dirLen;
			}
			else {
				behindDirX = 0.0f;
				behindDirY = 1.0f;
			}
		}
		};

	// 仲間 index 番目の「群れの中の定位置」（プレイヤーの後ろのかたまり）
	auto getAllySlot = [&](int index, int total, float playerPosX, float playerPosY, float& slotOutX, float& slotOutY) {
		float maxRadius = kFlockSpacing * sqrtf((float)(total > 1 ? total : 1));

		// 群れの中心はプレイヤーの後ろ
		float centerX = playerPosX + behindDirX * (kFlockGap + maxRadius);
		float centerY = playerPosY + behindDirY * (kFlockGap + maxRadius);

		// 画面の中に収める範囲
		float limitLeft = kStageEnemyRadius + 8.0f;
		float limitRight = 1920.0f - limitLeft;
		float limitTop = limitLeft;
		float limitBottom = 1180.0f - limitLeft;

		// 群れ全体が画面に収まるように、中心を内側へ寄せる
		if (centerX < limitLeft + maxRadius) centerX = limitLeft + maxRadius;
		if (centerX > limitRight - maxRadius) centerX = limitRight - maxRadius;
		if (centerY < limitTop + maxRadius) centerY = limitTop + maxRadius;
		if (centerY > limitBottom - maxRadius) centerY = limitBottom - maxRadius;

		// ひまわり状に敷き詰める（人数が増えても密度が一定）
		float angle = (float)index * 2.39996f;
		float radius = kFlockSpacing * sqrtf((float)index + 0.5f);

		// ちょっとふらふらさせてゆるい感じにする
		float wobbleX = sinf((float)followTimer * 0.07f + (float)index * 1.7f) * 4.0f;
		float wobbleY = cosf((float)followTimer * 0.06f + (float)index * 2.3f) * 4.0f;

		slotOutX = centerX + cosf(angle) * radius + wobbleX;
		slotOutY = centerY + sinf(angle) * radius + wobbleY;

		// 念のため、1体ずつも画面の中に収める
		if (slotOutX < limitLeft) slotOutX = limitLeft;
		if (slotOutX > limitRight) slotOutX = limitRight;
		if (slotOutY < limitTop) slotOutY = limitTop;
		if (slotOutY > limitBottom) slotOutY = limitBottom;
		};

	// 仲間ごとに追いかける速さを少しずつ変える（遅れてついてくる子がいる）
	auto getAllyFollowRate = [&](int index) {
		return 0.07f + (float)(index % 5) * 0.012f;
		};

	// 仲間の弾の設定
	const int kMaxAllyBullets = 64;
	AllyBullet allyBullets[kMaxAllyBullets] = {};
	const float kAllyBulletSpeed = 18.0f;

	for (int allyBulletInitIndex = 0; allyBulletInitIndex < kMaxAllyBullets; allyBulletInitIndex++) {
		allyBullets[allyBulletInitIndex].isExist = 0;
	}

	// 集めちゃダメな敵（紫）の設定
	const int kMaxBadEnemies = 8;
	const int kBadBombFrames = 120;             // 集めてから爆発するまで（2秒）
	const float kBadBlastRadius = 90.0f;        // 巻き込む範囲
	const float kBadBlastScale = 1.3f;          // 爆発画像の大きさ（本体）
	const float kBadVictimScale = 0.6f;         // 爆発画像の大きさ（巻き込まれた仲間）
	const int kMaxSmallExplosions = 64;
	const int kSmallExplosionFrames = 25;       // 爆発アニメの長さ（フレーム）
	BadEnemy badEnemies[kMaxBadEnemies] = {};
	SmallExplosion smallExplosions[kMaxSmallExplosions] = {};

	// 爆発エフェクトを1つ出す
	auto spawnSmallExplosion = [&](float x, float y, float scale) {

		for (int spawnIndex = 0; spawnIndex < kMaxSmallExplosions; spawnIndex++) {

			if (!smallExplosions[spawnIndex].isExist) {
				smallExplosions[spawnIndex].x = x;
				smallExplosions[spawnIndex].y = y;
				smallExplosions[spawnIndex].scale = scale;
				smallExplosions[spawnIndex].life = kSmallExplosionFrames;
				smallExplosions[spawnIndex].isExist = 1;
				return;
			}
		}
		};

	for (int badEnemyInitIndex = 0; badEnemyInitIndex < kMaxBadEnemies; badEnemyInitIndex++) {

		badEnemies[badEnemyInitIndex].x = 100.0f + (float)(rand() % 1720);
		badEnemies[badEnemyInitIndex].y = 400.0f + (float)(rand() % 500);
		badEnemies[badEnemyInitIndex].velocityX = 0.0f;
		badEnemies[badEnemyInitIndex].velocityY = 0.0f;
		badEnemies[badEnemyInitIndex].wanderTimer = rand() % 60;
		badEnemies[badEnemyInitIndex].isCollected = 0;
		badEnemies[badEnemyInitIndex].bombTimer = 0;
		badEnemies[badEnemyInitIndex].isExist = 1;
	}

	for (int smallExplosionInitIndex = 0; smallExplosionInitIndex < kMaxSmallExplosions; smallExplosionInitIndex++) {
		smallExplosions[smallExplosionInitIndex].isExist = 0;
	}

	// ステージの敵の初期化
	for (int stageEnemyInitIndex = 0; stageEnemyInitIndex < kMaxStageEnemies; stageEnemyInitIndex++) {

		stageEnemies[stageEnemyInitIndex].x = 100.0f + (float)(rand() % 1720);
		stageEnemies[stageEnemyInitIndex].y = 400.0f + (float)(rand() % 500);
		stageEnemies[stageEnemyInitIndex].velocityX = 0.0f;
		stageEnemies[stageEnemyInitIndex].velocityY = 0.0f;
		stageEnemies[stageEnemyInitIndex].wanderTimer = rand() % 60;
		stageEnemies[stageEnemyInitIndex].flash = 0;
		stageEnemies[stageEnemyInitIndex].shootTimer = 0;
		stageEnemies[stageEnemyInitIndex].isAlly = 0;
		stageEnemies[stageEnemyInitIndex].isExist = 1;
	}

	// 後ろの向きの初期化
	resetFollow(stagePlayerX, stagePlayerY);

	// ==============================
	// SHIFTスキップ用：状態のリセット処理
	// ==============================

	// プレイヤーにダメージを与える。当たったときだけ true を返す
	// （無敵中・ダッシュで移動中・やられたあとは当たらない）
	auto damagePlayer = [&](int damage) {

		if (isPlayerDead || playerInvincibleTimer > 0 || dashMoveTimer > 0) {
			return false;
		}

		playerHp -= damage;
		playerInvincibleTimer = kPlayerInvincibleFrames;

		screenShakeTimer = 10 + damage * 5;
		Controller::SetVibration(60000, 60000);
		vibrationTimer = 3;

		if (playerHp <= 0) {
			playerHp = 0;
			isPlayerDead = 1;
			playerDeadTimer = 0;
		}

		return true;
		};

	// 4回のダッシュの軌道線が「四角」になっているか調べる（なっていれば真ん中の位置を返す）
	// 四角の条件：
	//   ・4本とも、ある程度の長さがある（壁にぶつかって短くなっていない）
	//   ・線がつながっていて、4本目の終わりが1本目の始まりに戻ってくる
	//   ・毎回およそ90度ずつ、同じ向きに曲がっている
	auto detectSquareDash = [&](float& centerOutX, float& centerOutY) {

		if (lineCount < 4) {
			return false;
		}

		const int firstLine = lineCount - 4;
		const float kSquareJoinTolerance = 150.0f;   // つなぎ目・閉じ目のずれの許容
		const float kSquareMinSide = 60.0f;          // これより短い辺は四角と認めない
		const float kPi = 3.14159265f;

		float sideAngle[4];

		for (int squareIndex = 0; squareIndex < 4; squareIndex++) {

			float sideDx = lines[firstLine + squareIndex].endX - lines[firstLine + squareIndex].startX;
			float sideDy = lines[firstLine + squareIndex].endY - lines[firstLine + squareIndex].startY;

			if (sideDx * sideDx + sideDy * sideDy < kSquareMinSide * kSquareMinSide) {
				return false;
			}

			sideAngle[squareIndex] = atan2f(sideDy, sideDx);
		}

		float turnSign = 0.0f;

		for (int squareIndex = 0; squareIndex < 4; squareIndex++) {

			int nextIndex = (squareIndex + 1) % 4;

			// つながっているか（4本目→1本目は閉じているか）
			float joinDx = lines[firstLine + nextIndex].startX - lines[firstLine + squareIndex].endX;
			float joinDy = lines[firstLine + nextIndex].startY - lines[firstLine + squareIndex].endY;

			if (joinDx * joinDx + joinDy * joinDy > kSquareJoinTolerance * kSquareJoinTolerance) {
				return false;
			}

			// 曲がる角度（60度〜120度）と、曲がる向き
			float turn = sideAngle[nextIndex] - sideAngle[squareIndex];

			while (turn > kPi) {
				turn -= kPi * 2.0f;
			}

			while (turn < -kPi) {
				turn += kPi * 2.0f;
			}

			if (fabsf(turn) < kPi / 3.0f || fabsf(turn) > kPi * 2.0f / 3.0f) {
				return false;
			}

			float thisSign = (turn > 0.0f) ? 1.0f : -1.0f;

			if (squareIndex == 0) {
				turnSign = thisSign;
			}
			else if (thisSign != turnSign) {
				return false;
			}
		}

		// 真ん中（8つの端点の平均）
		float sumX = 0.0f;
		float sumY = 0.0f;

		for (int squareIndex = 0; squareIndex < 4; squareIndex++) {
			sumX += lines[firstLine + squareIndex].startX + lines[firstLine + squareIndex].endX;
			sumY += lines[firstLine + squareIndex].startY + lines[firstLine + squareIndex].endY;
		}

		centerOutX = sumX / 8.0f;
		centerOutY = sumY / 8.0f;

		return true;
		};

	// ボス戦の状態を、指定した形態の開始時点に戻す（1:第1形態 2:第2形態 3:第3形態）
	auto resetBossFight = [&](int phase) {

		// ボス本体
		bossPhase = phase;
		bossShieldMax = (phase == 1) ? kBossShieldMax : (phase == 2) ? kBossShieldMax2 : kBossShieldMax3;
		bossShield = bossShieldMax;
		bossBarrier = (phase == 3) ? kBossBarrierMax : 0;
		bossBarrierBreakTimer = 0;
		barrierHitTimer = 0;

		for (int shardResetIndex = 0; shardResetIndex < kMaxBarrierShards; shardResetIndex++) {
			barrierShards[shardResetIndex].isExist = 0;
		}
		bossIsInvincible = 0;
		bossShieldBreakTimer = 0;
		shieldBreakEffectTimer = 0;
		gameSpeed = 1.0f;
		slowMotionTimer = 0;

		bossX = 960.0f;
		bossY = 295.0f;
		bossSpeedX = (phase == 1) ? 6.0f : (phase == 2) ? 9.0f : 11.0f;
		bossTrailTimer = 0;

		// 攻撃の状態
		bossState = 0;
		bossAttackTimer = (phase == 1) ? 180 : 60;
		bossAuraRadius = 0.0f;
		bossAuraAlpha = 0.0f;
		bossChargeVelocityX = 0.0f;
		bossChargeVelocityY = 0.0f;
		bossRotationAngle = 0.0f;
		bossSpinTimer = 0;
		ufoMoveTimer = 0;
		bossSweepDirection = 1.0f;
		bossPatternIndex = 0;
		bossIsCloneCharge = 0;
		bossCloneBombCount = 0;
		bossSummonTimer = 0;
		bossSummonCount = 0;
		bombDropTimer = 0;
		afterImageTimer = 0;

		// 大技（使用済みフラグ）。回転レーザーは第1形態だけ
		bossRotLaserUsed = (phase == 1) ? 0 : 1;
		bossRotLaserTimer = 0;
		bossRotLaserHit = 0;
		bossCrossUsed = 0;
		bossBlastUsed = 0;
		bossBeamUsed = 0;
		bossReflectUsed = 0;
		reflectActiveCount = 0;
		bossBigTimer = 0;
		bossBigHit = 0;

		// 溜め技
		megaLaserState = 0;
		megaLaserTimer = 0;

		// プレイヤーのHPとスタミナ
		playerHp = kPlayerMaxHp;
		playerInvincibleTimer = 0;
		isPlayerDead = 0;
		playerDeadTimer = 0;
		stamina = kStaminaMax;
		staminaRegenTimer = 0;
		staminaFlashTimer = 0;

		// プレイヤー
		playerX = 960.0f;
		playerY = 901.0f;
		playerAngle = -1.5708f;
		dashStunTimer = 0;
		dashMoveTimer = 0;
		dashCooldown = 0;
		dashCount = 0;
		isAttacking = 0;
		attackTimer = 0;
		currentAttackIndex = 0;
		currentFrame = 0;
		isCurrentFrameCount = 0;
		boxCount = 0;
		lineCount = 0;

		// 演出
		screenShakeTimer = 0;
		screenShakeX = 0;
		screenShakeY = 0;
		vibrationTimer = 0;
		Controller::StopVibration();

		// 画面に残っているものを全部消す
		for (int i = 0; i < kMaxBossClones; i++) { bossClones[i].isExist = 0; }
		for (int i = 0; i < kMaxSummons; i++) { summons[i].isExist = 0; }
		for (int i = 0; i < kMaxBombs; i++) { bombs[i].isExist = 0; bombs[i].isGlitch = 0; }
		for (int i = 0; i < kMaxAfterImages; i++) { afterImages[i].isExist = 0; }
		for (int i = 0; i < kMaxSparks; i++) { sparks[i].isExist = 0; }
		for (int i = 0; i < kMaxHitBursts; i++) { hitBursts[i].isExist = 0; }
		for (int i = 0; i < kMaxBossExplosions; i++) { bossExplosions[i].isExist = 0; }
		for (int i = 0; i < kMaxBossExplosionParticles; i++) { bossExplosionParticles[i].isExist = 0; }
		for (int i = 0; i < kMaxGlitchParticles; i++) { glitchParticles[i].isExist = 0; }
		for (int i = 0; i < kMaxAllyBullets; i++) { allyBullets[i].isExist = 0; }
		for (int i = 0; i < kMaxLines; i++) { lines[i].isExist = 0; }

		for (int i = 0; i < kMaxBoxes; i++) {
			boxes[i].isExist = 0;
			boxes[i].delay = 0;
			diamonds[i].isExist = 0;
			diamonds[i].openSize = 0.0f;
			beams[i].isShot = 0;
			lasers[i].isShot = 0;
			lasers[i].length = 0.0f;
			lasers[i].timer = 100;
			lasers[i].sparkMade = 0;
			lasers[i].shakeMade = 0;
		}
		};

	// 指定した形態のボス戦へスキップする（集めた仲間はそのまま連れて行く）
	auto skipToBossPhase = [&](int phase) {

		resetBossFight(phase);

		scene = kSceneBoss;

		// BGMが流れていなければ再生する
		if (bossBgmVoice == -1 || !Novice::IsPlayingAudio(bossBgmVoice)) {
			bossBgmVoice = Novice::PlayAudio(bossBgm, 1, 0.5f);
		}

		// 仲間をプレイヤーの後ろに並べ直す
		resetFollow(playerX, playerY);

		int skipAllyTotal = 0;

		for (int i = 0; i < kMaxStageEnemies; i++) {
			if (stageEnemies[i].isExist && stageEnemies[i].isAlly) {
				skipAllyTotal++;
			}
		}

		int skipAllyIndex = 0;

		for (int i = 0; i < kMaxStageEnemies; i++) {

			if (stageEnemies[i].isExist && stageEnemies[i].isAlly) {

				float skipSlotX = 0.0f;
				float skipSlotY = 0.0f;
				getAllySlot(skipAllyIndex, skipAllyTotal, playerX, playerY, skipSlotX, skipSlotY);

				stageEnemies[i].x = skipSlotX;
				stageEnemies[i].y = skipSlotY;
				stageEnemies[i].shootTimer = 20 + rand() % 60;

				skipAllyIndex++;
			}
		}
		};

	// 敵集めのステージに戻す（敵は全員もとの野生・集める前の状態に戻る）
	auto skipToStage = [&]() {

		// ボス戦は第1形態の開始状態に戻しておく（扉から入り直せるように）
		resetBossFight(1);

		scene = kSceneStage;

		// BGMを止める
		if (bossBgmVoice != -1) {
			Novice::StopAudio(bossBgmVoice);
			bossBgmVoice = -1;
		}

		// ステージのプレイヤー
		stagePlayerX = 960.0f;
		stagePlayerY = 1000.0f;

		// 呼びかけ・エイム
		callTimer = 0;
		callCooldown = 0;
		callRadius = 0.0f;
		isAiming = 0;
		preAiming = 0;
		aimDirX = 0.0f;
		aimDirY = -1.0f;

		allyCount = 0;
		followTimer = 0;
		resetFollow(stagePlayerX, stagePlayerY);

		// 赤い敵（野生）を配り直す
		for (int i = 0; i < kMaxStageEnemies; i++) {

			stageEnemies[i].x = 100.0f + (float)(rand() % 1720);
			stageEnemies[i].y = 400.0f + (float)(rand() % 500);
			stageEnemies[i].velocityX = 0.0f;
			stageEnemies[i].velocityY = 0.0f;
			stageEnemies[i].wanderTimer = rand() % 60;
			stageEnemies[i].flash = 0;
			stageEnemies[i].shootTimer = 0;
			stageEnemies[i].isAlly = 0;
			stageEnemies[i].isExist = 1;
		}

		// 集めちゃダメな敵（紫）も配り直す
		for (int i = 0; i < kMaxBadEnemies; i++) {

			badEnemies[i].x = 100.0f + (float)(rand() % 1720);
			badEnemies[i].y = 400.0f + (float)(rand() % 500);
			badEnemies[i].velocityX = 0.0f;
			badEnemies[i].velocityY = 0.0f;
			badEnemies[i].wanderTimer = rand() % 60;
			badEnemies[i].isCollected = 0;
			badEnemies[i].bombTimer = 0;
			badEnemies[i].isExist = 1;
		}

		for (int i = 0; i < kMaxSmallExplosions; i++) {
			smallExplosions[i].isExist = 0;
		}
		};

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {

		// フレームの開始
		Novice::BeginFrame();

		Controller::Update();

		// 振動タイマーの減衰
		if (vibrationTimer > 0) {
			vibrationTimer--;
			if (vibrationTimer == 0) {
				Controller::StopVibration();
			}
		}

		// 画面シェイク更新
		if (screenShakeTimer > 0) {
			screenShakeX = rand() % 7 - 3;
			screenShakeY = rand() % 7 - 3;
			screenShakeTimer--;
		}
		else {
			screenShakeX = 0;
			screenShakeY = 0;
		}

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// SHIFTキーでスキップ（押すたびに：第2形態 → 第3形態 → 敵集め → 第2形態 → …）
		if ((keys[DIK_LSHIFT] && !preKeys[DIK_LSHIFT]) || (keys[DIK_RSHIFT] && !preKeys[DIK_RSHIFT])) {

			if (scene == kSceneStage || bossPhase < 2) {
				skipToBossPhase(2);
			}
			else if (bossPhase == 2) {
				skipToBossPhase(3);
			}
			else {
				skipToStage();
			}
		}

		// やられたら、少しだけ間をおいてから敵集めに戻る
		if (isPlayerDead) {

			playerDeadTimer++;

			if (playerDeadTimer >= kPlayerDeadFrames) {
				skipToStage();
			}
		}

		// ==============================
		// ステージ（ボス戦前）の処理
		// ==============================
		if (scene == kSceneStage) {

			// --- 更新処理 ---

			// 入力方向
			float stageMoveX = Controller::GetLeftStickX();
			float stageMoveY = Controller::GetLeftStickY();

			if (Controller::IsDown(kPadButton0)) stageMoveY -= 1.0f;
			if (Controller::IsDown(kPadButton1)) stageMoveY += 1.0f;
			if (Controller::IsDown(kPadButton2)) stageMoveX -= 1.0f;
			if (Controller::IsDown(kPadButton3)) stageMoveX += 1.0f;

			if (keys[DIK_W]) stageMoveY -= 1.0f;
			if (keys[DIK_S]) stageMoveY += 1.0f;
			if (keys[DIK_A]) stageMoveX -= 1.0f;
			if (keys[DIK_D]) stageMoveX += 1.0f;

			// プレイヤーの移動
			float stageMoveLength = sqrtf(stageMoveX * stageMoveX + stageMoveY * stageMoveY);

			if (stageMoveLength > 0.0f) {
				stagePlayerX += (stageMoveX / stageMoveLength) * kStageSpeed;
				stagePlayerY += (stageMoveY / stageMoveLength) * kStageSpeed;
			}

			// 画面外に出ないようにする
			if (stagePlayerX < playerRadius) stagePlayerX = playerRadius;
			if (stagePlayerX > 1920.0f - playerRadius) stagePlayerX = 1920.0f - playerRadius;
			if (stagePlayerY < playerRadius) stagePlayerY = playerRadius;
			if (stagePlayerY > 1180.0f - playerRadius) stagePlayerY = 1180.0f - playerRadius;

			// プレイヤーの「後ろ」の向きを更新する
			updateBehindDir(stagePlayerX, stagePlayerY);

			// 呼びかけのクールダウン
			if (callCooldown > 0) {
				callCooldown--;
			}

			// エイムの向きを更新（動いている間は進行方向を向く）
			if (stageMoveLength > 0.0f) {
				aimDirX = stageMoveX / stageMoveLength;
				aimDirY = stageMoveY / stageMoveLength;
			}

			// 仲間（紫の爆弾も含む）は必ずプレイヤーが向いている方向の逆（後ろ）に行く
			behindDirX = -aimDirX;
			behindDirY = -aimDirY;

			// エイム線の先端（画面の中に収める）
			aimEndX = stagePlayerX + aimDirX * kAimLength;
			aimEndY = stagePlayerY + aimDirY * kAimLength;

			if (aimEndX < 0.0f) aimEndX = 0.0f;
			if (aimEndX > 1920.0f) aimEndX = 1920.0f;
			if (aimEndY < 0.0f) aimEndY = 0.0f;
			if (aimEndY > 1180.0f) aimEndY = 1180.0f;

			// 呼びかけ（SPACEキーを長押しでエイム、離したら呼びかけ）
			// 1フレーム前の状態を覚えておく
			preAiming = isAiming;
			// SPACEかボタンを押してる間はエイム中
			isAiming = (keys[DIK_SPACE] || Controller::IsDown(kPadButton10)) ? 1 : 0;

			// 離した瞬間にプレイヤーを中心に呼びかけ
			if (preAiming && !isAiming &&
				callTimer == 0 && callCooldown == 0) {

				callTimer = kCallFrames;
				callRadius = kCallStartRadius;
				callCooldown = kCallCooldownMax;
				callCenterX = stagePlayerX;
				callCenterY = stagePlayerY;
			}

			// 呼びかけの波を広げる
			if (callTimer > 0) {
				callTimer--;
				callRadius += kCallSpeed;
				callCenterX = stagePlayerX;   // 波はプレイヤーについてくる
				callCenterY = stagePlayerY;
			}

			// 敵の更新
			int allyIndex = 0;
			allyCount = 0;

			int stageAllyTotal = 0;

			for (int stageAllyCountIndex = 0; stageAllyCountIndex < kMaxStageEnemies; stageAllyCountIndex++) {
				if (stageEnemies[stageAllyCountIndex].isExist && stageEnemies[stageAllyCountIndex].isAlly) {
					stageAllyTotal++;
				}
			}

			for (int stageEnemyUpdateIndex = 0;
				stageEnemyUpdateIndex < kMaxStageEnemies;
				stageEnemyUpdateIndex++) {

				if (!stageEnemies[stageEnemyUpdateIndex].isExist) {
					continue;
				}

				if (stageEnemies[stageEnemyUpdateIndex].flash > 0) {
					stageEnemies[stageEnemyUpdateIndex].flash--;
				}

				// 野生の敵（赤）の処理
				if (!stageEnemies[stageEnemyUpdateIndex].isAlly) {

					// 呼びかけの波に触れたら仲間（青）になる
					if (callTimer > 0) {

						float callDx = stageEnemies[stageEnemyUpdateIndex].x - callCenterX;
						float callDy = stageEnemies[stageEnemyUpdateIndex].y - callCenterY;

						if (callDx * callDx + callDy * callDy <= callRadius * callRadius) {
							stageEnemies[stageEnemyUpdateIndex].isAlly = 1;
							stageEnemies[stageEnemyUpdateIndex].flash = 12;
						}
					}

					// ふらふら歩き回る
					stageEnemies[stageEnemyUpdateIndex].wanderTimer--;

					if (stageEnemies[stageEnemyUpdateIndex].wanderTimer <= 0) {

						stageEnemies[stageEnemyUpdateIndex].wanderTimer = 60 + rand() % 90;

						if (rand() % 3 == 0) {
							stageEnemies[stageEnemyUpdateIndex].velocityX = 0.0f;
							stageEnemies[stageEnemyUpdateIndex].velocityY = 0.0f;
						}
						else {
							float wanderAngle = (float)(rand() % 628) / 100.0f;
							float wanderSpeed = 0.8f + (float)(rand() % 15) / 10.0f;

							stageEnemies[stageEnemyUpdateIndex].velocityX = cosf(wanderAngle) * wanderSpeed;
							stageEnemies[stageEnemyUpdateIndex].velocityY = sinf(wanderAngle) * wanderSpeed;
						}
					}

					stageEnemies[stageEnemyUpdateIndex].x += stageEnemies[stageEnemyUpdateIndex].velocityX;
					stageEnemies[stageEnemyUpdateIndex].y += stageEnemies[stageEnemyUpdateIndex].velocityY;

					// 歩ける範囲から出たら跳ね返る
					if (stageEnemies[stageEnemyUpdateIndex].x < 30.0f ||
						stageEnemies[stageEnemyUpdateIndex].x > 1890.0f) {
						stageEnemies[stageEnemyUpdateIndex].velocityX *= -1.0f;
					}

					if (stageEnemies[stageEnemyUpdateIndex].y < 330.0f ||
						stageEnemies[stageEnemyUpdateIndex].y > 1150.0f) {
						stageEnemies[stageEnemyUpdateIndex].velocityY *= -1.0f;
					}

					if (stageEnemies[stageEnemyUpdateIndex].x < 30.0f) stageEnemies[stageEnemyUpdateIndex].x = 30.0f;
					if (stageEnemies[stageEnemyUpdateIndex].x > 1890.0f) stageEnemies[stageEnemyUpdateIndex].x = 1890.0f;
					if (stageEnemies[stageEnemyUpdateIndex].y < 330.0f) stageEnemies[stageEnemyUpdateIndex].y = 330.0f;
					if (stageEnemies[stageEnemyUpdateIndex].y > 1150.0f) stageEnemies[stageEnemyUpdateIndex].y = 1150.0f;
				}
				// 仲間（青）の処理
				else {

					// プレイヤーの後ろの群れの中の定位置を決める
					float slotX = 0.0f;
					float slotY = 0.0f;
					getAllySlot(allyIndex, stageAllyTotal, stagePlayerX, stagePlayerY, slotX, slotY);

					// 定位置に向かってついていく
					float followRate = getAllyFollowRate(allyIndex);
					float followX = (slotX - stageEnemies[stageEnemyUpdateIndex].x) * followRate;
					float followY = (slotY - stageEnemies[stageEnemyUpdateIndex].y) * followRate;

					float followLength = sqrtf(followX * followX + followY * followY);

					if (followLength > 16.0f) {
						followX = followX / followLength * 16.0f;
						followY = followY / followLength * 16.0f;
					}

					stageEnemies[stageEnemyUpdateIndex].x += followX;
					stageEnemies[stageEnemyUpdateIndex].y += followY;

					allyIndex++;
					allyCount++;
				}
			}

			// 集めちゃダメな敵（紫）の更新
			// 集めちゃった紫の数
			int badCollectedTotal = 0;

			for (int badCountIndex = 0; badCountIndex < kMaxBadEnemies; badCountIndex++) {
				if (badEnemies[badCountIndex].isExist && badEnemies[badCountIndex].isCollected) {
					badCollectedTotal++;
				}
			}

			int badCollectedIndex = 0;

			// 紫の敵を1体ずつ更新
			for (int badUpdateIndex = 0; badUpdateIndex < kMaxBadEnemies; badUpdateIndex++) {

				if (!badEnemies[badUpdateIndex].isExist) {
					continue;
				}

				// 集めてしまった紫の敵：群れについていき、2秒後に爆発する
				if (badEnemies[badUpdateIndex].isCollected) {

					// 群れの外側の定位置についていく
					float badSlotX = 0.0f;
					float badSlotY = 0.0f;
					getAllySlot(stageAllyTotal + badCollectedIndex, stageAllyTotal + badCollectedTotal, stagePlayerX, stagePlayerY, badSlotX, badSlotY);

					// 紫は少し速めに追いかける
					float badFollowX = (badSlotX - badEnemies[badUpdateIndex].x) * 0.12f;
					float badFollowY = (badSlotY - badEnemies[badUpdateIndex].y) * 0.12f;
					float badFollowLength = sqrtf(badFollowX * badFollowX + badFollowY * badFollowY);

					// 速さに上限をつける
					if (badFollowLength > 16.0f) {
						badFollowX = badFollowX / badFollowLength * 16.0f;
						badFollowY = badFollowY / badFollowLength * 16.0f;
					}

					// 動かす
					badEnemies[badUpdateIndex].x += badFollowX;
					badEnemies[badUpdateIndex].y += badFollowY;

					badCollectedIndex++;

					// 爆発までのカウントダウン
					badEnemies[badUpdateIndex].bombTimer--;

					// 時間になったら爆発して、周りの仲間を巻き込む
					if (badEnemies[badUpdateIndex].bombTimer <= 0) {

						float blastX = badEnemies[badUpdateIndex].x;
						float blastY = badEnemies[badUpdateIndex].y;

						badEnemies[badUpdateIndex].isExist = 0;
						spawnSmallExplosion(blastX, blastY, kBadBlastScale);

						// 範囲内の仲間を消す
						for (int victimIndex = 0; victimIndex < kMaxStageEnemies; victimIndex++) {

							if (!stageEnemies[victimIndex].isExist || !stageEnemies[victimIndex].isAlly) {
								continue;
							}

							float victimDx = stageEnemies[victimIndex].x - blastX;
							float victimDy = stageEnemies[victimIndex].y - blastY;

							if (victimDx * victimDx + victimDy * victimDy <= kBadBlastRadius * kBadBlastRadius) {
								stageEnemies[victimIndex].isExist = 0;
								// 爆発の絵は本体の1個だけにするため、巻き込まれた仲間の爆発エフェクトは出さない
								// spawnSmallExplosion(stageEnemies[victimIndex].x, stageEnemies[victimIndex].y, kBadVictimScale);
								(void)kBadVictimScale; // 未使用警告を避けるため（巻き込まれた仲間の爆発を出さなくなったので）
							}
						}

						// 範囲内にいる他の紫の敵も誘爆させる
						for (int chainIndex = 0; chainIndex < kMaxBadEnemies; chainIndex++) {

							if (!badEnemies[chainIndex].isExist || !badEnemies[chainIndex].isCollected) {
								continue;
							}

							float chainDx = badEnemies[chainIndex].x - blastX;
							float chainDy = badEnemies[chainIndex].y - blastY;

							if (chainDx * chainDx + chainDy * chainDy <= kBadBlastRadius * kBadBlastRadius &&
								badEnemies[chainIndex].bombTimer > 6) {
								badEnemies[chainIndex].bombTimer = 6;
							}
						}
					}

					continue;
				}

				// 呼びかけの波に触れたら集めてしまう（2秒後に爆発）
				if (callTimer > 0) {

					float badCallDx = badEnemies[badUpdateIndex].x - callCenterX;
					float badCallDy = badEnemies[badUpdateIndex].y - callCenterY;

					if (badCallDx * badCallDx + badCallDy * badCallDy <= callRadius * callRadius) {

						badEnemies[badUpdateIndex].isCollected = 1;
						badEnemies[badUpdateIndex].bombTimer = kBadBombFrames;
						continue;
					}
				}

				// ふらふら歩き回る
				badEnemies[badUpdateIndex].wanderTimer--;

				if (badEnemies[badUpdateIndex].wanderTimer <= 0) {

					badEnemies[badUpdateIndex].wanderTimer = 60 + rand() % 90;

					if (rand() % 3 == 0) {
						badEnemies[badUpdateIndex].velocityX = 0.0f;
						badEnemies[badUpdateIndex].velocityY = 0.0f;
					}
					else {
						float badWanderAngle = (float)(rand() % 628) / 100.0f;
						float badWanderSpeed = 0.8f + (float)(rand() % 15) / 10.0f;

						badEnemies[badUpdateIndex].velocityX = cosf(badWanderAngle) * badWanderSpeed;
						badEnemies[badUpdateIndex].velocityY = sinf(badWanderAngle) * badWanderSpeed;
					}
				}

				// 動かす
				badEnemies[badUpdateIndex].x += badEnemies[badUpdateIndex].velocityX;
				badEnemies[badUpdateIndex].y += badEnemies[badUpdateIndex].velocityY;

				// 歩ける範囲から出たら跳ね返る
				if (badEnemies[badUpdateIndex].x < 30.0f || badEnemies[badUpdateIndex].x > 1890.0f) {
					badEnemies[badUpdateIndex].velocityX *= -1.0f;
				}

				if (badEnemies[badUpdateIndex].y < 330.0f || badEnemies[badUpdateIndex].y > 1150.0f) {
					badEnemies[badUpdateIndex].velocityY *= -1.0f;
				}

				// 念のため範囲内に戻す
				if (badEnemies[badUpdateIndex].x < 30.0f) badEnemies[badUpdateIndex].x = 30.0f;
				if (badEnemies[badUpdateIndex].x > 1890.0f) badEnemies[badUpdateIndex].x = 1890.0f;
				if (badEnemies[badUpdateIndex].y < 330.0f) badEnemies[badUpdateIndex].y = 330.0f;
				if (badEnemies[badUpdateIndex].y > 1150.0f) badEnemies[badUpdateIndex].y = 1150.0f;
			}

			// 小さな爆発エフェクトの更新
			for (int smallExplosionUpdateIndex = 0; smallExplosionUpdateIndex < kMaxSmallExplosions; smallExplosionUpdateIndex++) {

				if (smallExplosions[smallExplosionUpdateIndex].isExist) {

					smallExplosions[smallExplosionUpdateIndex].life--;

					if (smallExplosions[smallExplosionUpdateIndex].life <= 0) {
						smallExplosions[smallExplosionUpdateIndex].isExist = 0;
					}
				}
			}

			// 扉に入ったらボス戦へ
			if (stagePlayerX > kDoorX - kDoorWidth / 2.0f &&
				stagePlayerX < kDoorX + kDoorWidth / 2.0f &&
				stagePlayerY > kDoorY - kDoorHeight / 2.0f &&
				stagePlayerY < kDoorY + kDoorHeight / 2.0f) {

				scene = kSceneBoss;

				// ボス戦BGMを再生（ループあり・音量50%）
				bossBgmVoice = Novice::PlayAudio(bossBgm, 1, 0.5f);

				// 仲間をボス戦のプレイヤーの後ろに並べる
				resetFollow(playerX, playerY);
				int bossAllyIndex = 0;
				int bossAllyTotal = 0;

				for (int bossAllyCountIndex = 0; bossAllyCountIndex < kMaxStageEnemies; bossAllyCountIndex++) {
					if (stageEnemies[bossAllyCountIndex].isExist && stageEnemies[bossAllyCountIndex].isAlly) {
						bossAllyTotal++;
					}
				}

				for (int bossAllySetIndex = 0; bossAllySetIndex < kMaxStageEnemies; bossAllySetIndex++) {

					if (stageEnemies[bossAllySetIndex].isExist && stageEnemies[bossAllySetIndex].isAlly) {

						float bossSlotX = 0.0f;
						float bossSlotY = 0.0f;
						getAllySlot(bossAllyIndex, bossAllyTotal, playerX, playerY, bossSlotX, bossSlotY);

						stageEnemies[bossAllySetIndex].x = bossSlotX;
						stageEnemies[bossAllySetIndex].y = bossSlotY;
						stageEnemies[bossAllySetIndex].shootTimer = 20 + rand() % 60;

						bossAllyIndex++;
					}
				}
			}

			// --- 描画処理 ---

			// 地面
			Novice::DrawBox(0, 0, 1920, 1180, 0, 0x14201AFF, kFillModeSolid);

			// 扉の光（プレイヤーが近づくと明るくなる）
			float doorDx = stagePlayerX - kDoorX;
			float doorDy = stagePlayerY - kDoorY;
			unsigned int doorInnerColor = 0x1A0F05FF;

			if (doorDx * doorDx + doorDy * doorDy < 350.0f * 350.0f) {
				doorInnerColor = 0xFFD27AFF;
			}

			// 扉の枠
			Novice::DrawBox(
				(int)(kDoorX - kDoorWidth / 2.0f - 14.0f),
				(int)(kDoorY - kDoorHeight / 2.0f - 14.0f),
				(int)(kDoorWidth + 28.0f),
				(int)(kDoorHeight + 14.0f),
				0.0f,
				0x8B5A2BFF,
				kFillModeSolid
			);

			// 扉の内側
			Novice::DrawBox(
				(int)(kDoorX - kDoorWidth / 2.0f),
				(int)(kDoorY - kDoorHeight / 2.0f),
				(int)kDoorWidth,
				(int)kDoorHeight,
				0.0f,
				doorInnerColor,
				kFillModeSolid
			);

			// 扉のドアノブ
			Novice::DrawEllipse(
				(int)(kDoorX + kDoorWidth / 2.0f - 22.0f),
				(int)kDoorY,
				7,
				7,
				0.0f,
				0xFFD700FF,
				kFillModeSolid
			);

			// エイム線（SPACE長押し中）
			if (isAiming) {

				// プレイヤーから先端までの線
				Novice::DrawLine(
					(int)stagePlayerX,
					(int)stagePlayerY,
					(int)aimEndX,
					(int)aimEndY,
					0x66AAFFFF
				);

				// 呼びかけが届く範囲（プレイヤーを中心にした円）
				Novice::DrawEllipse(
					(int)stagePlayerX,
					(int)stagePlayerY,
					// (int)(kCallStartRadius + kCallSpeed * kCallFrames),
					// (int)(kCallStartRadius + kCallSpeed * kCallFrames),
					(int)(kCallStartRadius + kCallSpeed * (kCallFrames - 1)),
					(int)(kCallStartRadius + kCallSpeed * (kCallFrames - 1)),
					0.0f,
					0x88CCFFFF,
					kFillModeWireFrame
				);

				// 先端の印
				Novice::DrawEllipse(
					(int)aimEndX,
					(int)aimEndY,
					8,
					8,
					0.0f,
					0x66AAFFFF,
					kFillModeSolid
				);
			}

			// 呼びかけの波
			if (callTimer > 0) {

				// float callScale = callRadius / 300.0f;
				// en.pngは600x600だが、輪の部分は直径約385px、輪の中心は(301, 297)
				const float kEnCircleSize = 385.0f;
				const float kEnCenterX = 301.0f;
				const float kEnCenterY = 297.0f;
				float callScale = (callRadius * 2.0f) / kEnCircleSize;

				Novice::DrawSprite(
					// (int)(stagePlayerX - callRadius),
					// (int)(stagePlayerY - callRadius),
					(int)(stagePlayerX - kEnCenterX * callScale),
					(int)(stagePlayerY - kEnCenterY * callScale),
					enTexture,
					callScale,
					callScale,
					0.0f,
					0xAADDFFFF
				);


			}

			// 敵の描画（野生＝赤、仲間＝青）
			for (int stageEnemyDrawIndex = 0;
				stageEnemyDrawIndex < kMaxStageEnemies;
				stageEnemyDrawIndex++) {

				if (!stageEnemies[stageEnemyDrawIndex].isExist) {
					continue;
				}

				unsigned int enemyColor = 0xFF2222FF;
				unsigned int enemyRimColor = 0x990000FF;

				if (stageEnemies[stageEnemyDrawIndex].isAlly) {
					enemyColor = 0x3366FFFF;
					enemyRimColor = 0x112299FF;
				}

				// 縁
				Novice::DrawEllipse(
					(int)stageEnemies[stageEnemyDrawIndex].x,
					(int)stageEnemies[stageEnemyDrawIndex].y,
					(int)kStageEnemyRadius + 3,
					(int)kStageEnemyRadius + 3,
					0.0f,
					enemyRimColor,
					kFillModeSolid
				);

				// 本体
				Novice::DrawEllipse(
					(int)stageEnemies[stageEnemyDrawIndex].x,
					(int)stageEnemies[stageEnemyDrawIndex].y,
					(int)kStageEnemyRadius,
					(int)kStageEnemyRadius,
					0.0f,
					enemyColor,
					kFillModeSolid
				);

				// 仲間になった瞬間の光
				if (stageEnemies[stageEnemyDrawIndex].flash > 0) {
					Novice::DrawEllipse(
						(int)stageEnemies[stageEnemyDrawIndex].x,
						(int)stageEnemies[stageEnemyDrawIndex].y,
						(int)kStageEnemyRadius + 6 + stageEnemies[stageEnemyDrawIndex].flash * 2,
						(int)kStageEnemyRadius + 6 + stageEnemies[stageEnemyDrawIndex].flash * 2,
						0.0f,
						WHITE,
						kFillModeWireFrame
					);
				}
			}

			// 集めちゃダメな敵（紫）の描画
			for (int badDrawIndex = 0; badDrawIndex < kMaxBadEnemies; badDrawIndex++) {

				if (!badEnemies[badDrawIndex].isExist) {
					continue;
				}

				unsigned int badColor = 0xAA33FFFF;
				unsigned int badRimColor = 0x550099FF;

				// 集めてしまった敵は、爆発が近づくほど速く点滅する
				if (badEnemies[badDrawIndex].isCollected) {

					int blinkInterval = 12;
					if (badEnemies[badDrawIndex].bombTimer <= 60) blinkInterval = 6;
					if (badEnemies[badDrawIndex].bombTimer <= 30) blinkInterval = 3;

					if ((badEnemies[badDrawIndex].bombTimer / blinkInterval) % 2 == 0) {
						badColor = WHITE;
					}

					// 巻き込まれる範囲
					Novice::DrawEllipse(
						(int)badEnemies[badDrawIndex].x,
						(int)badEnemies[badDrawIndex].y,
						(int)kBadBlastRadius,
						(int)kBadBlastRadius,
						0.0f,
						0xAA33FF88,
						kFillModeWireFrame
					);
				}

				// 縁
				Novice::DrawEllipse(
					(int)badEnemies[badDrawIndex].x,
					(int)badEnemies[badDrawIndex].y,
					(int)kStageEnemyRadius + 3,
					(int)kStageEnemyRadius + 3,
					0.0f,
					badRimColor,
					kFillModeSolid
				);

				// 本体
				Novice::DrawEllipse(
					(int)badEnemies[badDrawIndex].x,
					(int)badEnemies[badDrawIndex].y,
					(int)kStageEnemyRadius,
					(int)kStageEnemyRadius,
					0.0f,
					badColor,
					kFillModeSolid
				);
			}

			// 小さな爆発エフェクトの描画（bakuhatu.png）
			for (int smallExplosionDrawIndex = 0; smallExplosionDrawIndex < kMaxSmallExplosions; smallExplosionDrawIndex++) {

				if (!smallExplosions[smallExplosionDrawIndex].isExist) {
					continue;
				}

				// 爆発のアニメーションフレームを計算（9コマ）
				int smallFrame = (kSmallExplosionFrames - smallExplosions[smallExplosionDrawIndex].life) * 9 / kSmallExplosionFrames;
				if (smallFrame < 0) smallFrame = 0;
				if (smallFrame > 8) smallFrame = 8;

				float smallScale = smallExplosions[smallExplosionDrawIndex].scale;
				int smallHalfSize = (int)(62.0f * smallScale);

				// 画像の一部だけ切り出して描く
				Novice::DrawSpriteRect(
					(int)smallExplosions[smallExplosionDrawIndex].x - smallHalfSize,
					(int)smallExplosions[smallExplosionDrawIndex].y - smallHalfSize,
					smallFrame * 124,
					0,
					124,
					124,
					bakuhatuTexture,
					(124.0f / 1116.0f) * smallScale,
					smallScale,
					0.0f,
					WHITE
				);
			}

			// プレイヤーの描画
			Novice::DrawSprite(
				(int)(stagePlayerX - 52),
				(int)(stagePlayerY - 50),
				bokasiTexture,
				0.15f,
				0.15f,
				0.0f,
				0x330033FF
			);

			Novice::DrawSprite(
				(int)(stagePlayerX - 32),
				(int)(stagePlayerY - 32),
				hosiTexture,
				1.0f,
				1.0f,
				0.0f,
				WHITE
			);

			// UI表示
			Novice::ScreenPrintf(10, 10, "Move : WASD");
			// Novice::ScreenPrintf(10, 30, "Call : SPACE (red -> blue)");
			Novice::ScreenPrintf(10, 30, "Call : hold SPACE to aim, release to call (red -> blue)");
			Novice::ScreenPrintf(10, 50, "Allies : %d / %d", allyCount, kMaxStageEnemies);
			Novice::ScreenPrintf(10, 70, "Enter the door to fight the boss");
			Novice::ScreenPrintf(10, 90, "SHIFT : skip to FORM 2");

			// フレームの終了
			Novice::EndFrame();

			// ESCキーが押されたらループを抜ける
			if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
				break;
			}

			continue;
		}

		///
		/// ↓更新処理ここから
		///

		// シールドブレイク中のタイマー
		if (bossShieldBreakTimer > 0) {
			bossShieldBreakTimer--;
		}

		// バリアが壊れたときの演出タイマー
		if (bossBarrierBreakTimer > 0) {

			if (bossBarrierBreakTimer == 30) {

				screenShakeTimer = 25;
				Controller::SetVibration(60000, 60000);
				vibrationTimer = 8;

				// バリアのふちから、破片が外へ飛び散る
				const int kBarrierShardCount = 28;

				for (int shardSpawnCount = 0; shardSpawnCount < kBarrierShardCount; shardSpawnCount++) {

					for (int shardSpawnIndex = 0; shardSpawnIndex < kMaxBarrierShards; shardSpawnIndex++) {

						if (!barrierShards[shardSpawnIndex].isExist) {

							float shardAngle = 6.2831853f * (float)shardSpawnCount / (float)kBarrierShardCount
								+ (float)(rand() % 100 - 50) / 400.0f;
							float shardSpeed = 5.0f + (float)(rand() % 100) / 10.0f;
							float shardRadius = bossRadius + kBossBarrierExtraRadius;

							barrierShards[shardSpawnIndex].x = bossX + cosf(shardAngle) * shardRadius;
							barrierShards[shardSpawnIndex].y = bossY + sinf(shardAngle) * shardRadius;
							barrierShards[shardSpawnIndex].velocityX = cosf(shardAngle) * shardSpeed;
							barrierShards[shardSpawnIndex].velocityY = sinf(shardAngle) * shardSpeed;
							barrierShards[shardSpawnIndex].angle = (float)(rand() % 628) / 100.0f;
							barrierShards[shardSpawnIndex].spin = (float)(rand() % 100 - 50) / 200.0f;
							barrierShards[shardSpawnIndex].size = 14.0f + (float)(rand() % 14);
							barrierShards[shardSpawnIndex].life = kBarrierShardLife;
							barrierShards[shardSpawnIndex].isExist = 1;
							break;
						}
					}
				}
			}

			bossBarrierBreakTimer--;
		}

		// 弾が当たったときのバリアの表示タイマー
		if (barrierHitTimer > 0) {
			barrierHitTimer--;
		}

		// バリアの破片の更新
		for (int shardUpdateIndex = 0; shardUpdateIndex < kMaxBarrierShards; shardUpdateIndex++) {

			if (!barrierShards[shardUpdateIndex].isExist) {
				continue;
			}

			barrierShards[shardUpdateIndex].x += barrierShards[shardUpdateIndex].velocityX;
			barrierShards[shardUpdateIndex].y += barrierShards[shardUpdateIndex].velocityY;
			barrierShards[shardUpdateIndex].velocityX *= 0.94f;
			barrierShards[shardUpdateIndex].velocityY *= 0.94f;
			barrierShards[shardUpdateIndex].angle += barrierShards[shardUpdateIndex].spin;
			barrierShards[shardUpdateIndex].life--;

			if (barrierShards[shardUpdateIndex].life <= 0) {
				barrierShards[shardUpdateIndex].isExist = 0;
			}
		}

		// シールド破壊エフェクトのタイマー
		if (shieldBreakEffectTimer > 0) {
			shieldBreakEffectTimer--;

			// 第1形態・第2形態が倒されて破壊演出が終わったら、次の形態になる
			// if (shieldBreakEffectTimer == 0 && bossPhase == 1 && bossShield <= 0) {
			if (shieldBreakEffectTimer == 0 && (bossPhase == 1 || bossPhase == 2) && bossShield <= 0) {

				// bossPhase = 2;
				// bossShield = kBossShieldMax + 500;   // シールドを満タンに戻す
				bossPhase++;   // 1→2、2→3

				if (bossPhase == 2) {
					bossShieldMax = kBossShieldMax2;
				}
				else {
					// 第3形態：シールドが増えて、バリアを張る
					bossShieldMax = kBossShieldMax3;
					bossBarrier = kBossBarrierMax;
				}

				bossShield = bossShieldMax;   // シールドを満タンに戻す
				bossShieldBreakTimer = 0;

				// 攻撃を最初からやり直す
				bossState = 0;
				bossAttackTimer = 60;
				bossIsCloneCharge = 0;
				// bossRotLaserUsed = 0;
				bossRotLaserUsed = 1;   // 回転レーザーは第1形態だけ（第2・第3形態は別の大技）
				bossIsInvincible = 0;

				// 第2・第3形態は横の動きが速い（向きはそのまま）
				float phaseSpeed = (bossPhase == 2) ? 9.0f : 11.0f;

				if (bossSpeedX > 0.0f) {
					bossSpeedX = phaseSpeed;
				}
				else {
					bossSpeedX = -phaseSpeed;
				}

				// 残っている分身は消す
				for (int phaseCloneIndex = 0; phaseCloneIndex < kMaxBossClones; phaseCloneIndex++) {
					bossClones[phaseCloneIndex].isExist = 0;
				}

				// 登場の合図に画面を揺らす
				screenShakeTimer = 30;
			}
		}

		// シールドが半分以下になったら、今の攻撃を中断して無敵になり、回転レーザーを使う
		if (!bossRotLaserUsed && bossShield > 0 && bossShield <= bossShieldMax / 2 && bossState != 6) {

			bossRotLaserUsed = 1;
			bossIsInvincible = 1;
			bossState = 6;
			bossRotLaserTimer = 0;
			bossRotLaserHit = 0;
			bossIsCloneCharge = 0;
			bossRotLaserDir = (rand() % 2 == 0) ? 1.0f : -1.0f;

			// 画面の上のまん中から見て、プレイヤーより少し手前の向きから回り始める
			bossRotLaserAngle = atan2f(playerY - kRotLaserCenterY, playerX - kRotLaserCenterX) - bossRotLaserDir * 1.0f;

			// 突進中の分身は消す
			for (int rotStartCloneIndex = 0; rotStartCloneIndex < kMaxBossClones; rotStartCloneIndex++) {
				bossClones[rotStartCloneIndex].isExist = 0;
			}

			screenShakeTimer = 20;
		}

		// 大技を始める共通の処理（無敵になって、今の攻撃を中断する）
		auto startBigAttack = [&](int nextState) {

			bossBigTimer = 0;
			bossBigHit = 0;
			bossIsInvincible = 1;
			bossState = nextState;
			bossIsCloneCharge = 0;

			for (int bigCloneIndex = 0; bigCloneIndex < kMaxBossClones; bigCloneIndex++) {
				bossClones[bigCloneIndex].isExist = 0;
			}

			screenShakeTimer = 20;
			};

		// 第2形態：シールドが半分以下で十字レーザー
		// 第3形態：シールドが2/3以下でバリア爆発、1/3以下で全画面ビーム
		if (bossState < 6 && bossShield > 0) {

			if (bossPhase == 2 && !bossCrossUsed && bossShield <= bossShieldMax / 2) {

				bossCrossUsed = 1;
				startBigAttack(7);
			}
			else if (bossPhase == 3 && !bossReflectUsed && bossShield <= bossShieldMax / 2) {

				bossReflectUsed = 1;
				startBigAttack(10);
			}
			else if (kPhase3OldBigAttack && bossPhase == 3 && !bossBlastUsed && bossShield <= bossShieldMax * 2 / 3) {

				bossBlastUsed = 1;
				startBigAttack(8);

				// 輪のすき間の向きを決める
				for (int blastInitIndex = 0; blastInitIndex < kBlastRings; blastInitIndex++) {
					blastGapAngle[blastInitIndex] = (float)(rand() % 628) / 100.0f - 3.14f;
					blastRingHit[blastInitIndex] = 0;
				}
			}
			else if (kPhase3OldBigAttack && bossPhase == 3 && !bossBeamUsed && bossShield <= bossShieldMax / 3) {

				bossBeamUsed = 1;
				startBigAttack(9);
				fullBeamSafeSide = rand() % 2;
			}
		}

		// ボスの移動・突進処理 

		if (bossState == 0) {

			// シールドが半分以下になったら、回転レーザーを使う
			if (!bossRotLaserUsed && bossShield > 0 && bossShield <= bossShieldMax / 2) {

				bossRotLaserUsed = 1;
				bossState = 6;
				bossRotLaserTimer = 0;
				bossRotLaserHit = 0;
				bossRotLaserDir = (rand() % 2 == 0) ? 1.0f : -1.0f;

				// 画面の上のまん中から見て、プレイヤーより少し手前の向きから回り始める
				bossRotLaserAngle = atan2f(playerY - kRotLaserCenterY, playerX - kRotLaserCenterX) - bossRotLaserDir * 1.0f;

				screenShakeTimer = 20;
			}

			// ふわふわ動くためのカウント
			ufoMoveTimer++;

			// 横に滑る（速さが少しずつ変わる）
			// 速さを揺らしながら横に動く
			bossX += bossSpeedX * (1.0f + sinf(ufoMoveTimer * 0.06f) * 0.5f);

			// 上下にふわふわ揺れる
			float hoverY = 295.0f + sinf(ufoMoveTimer * 0.07f) * 35.0f;
			bossY += (hoverY - bossY) * 0.08f;

			// 画面の端で向きを変える
			if (bossX - bossRadius <= 0.0f) {
				// bossSpeedX = 6.0f;
				bossSpeedX = (bossPhase == 1) ? 6.0f : (bossPhase == 2) ? 9.0f : 11.0f;
			}

			if (bossX + bossRadius >= 1920.0f) {
				// bossSpeedX = -6.0f;
				bossSpeedX = (bossPhase == 1) ? -6.0f : (bossPhase == 2) ? -9.0f : -11.0f;
			}

			// 攻撃までのカウントダウン
			bossAttackTimer--;

			if (bossAttackTimer <= 0) {
				bossState = 1;
				bossAttackTimer = 45;

				// 近いほうの画面の端から爆撃を始める
				if (bossX < 960.0f) {
					bossSweepDirection = 1.0f;
				}
				else {
					bossSweepDirection = -1.0f;
				}

				// 今回の攻撃を決める（0:通常突進 1:分身突進 2:召喚）
				bossIsCloneCharge = 0;

				if (bossPatternIndex == 1) {
					bossIsCloneCharge = 1;
				}
				else if (bossPatternIndex == 2) {
					// 召喚（突進ではなく状態5へ）
					bossState = 5;
					bossAttackTimer = 100;
					bossSummonTimer = 0;
					bossSummonCount = 0;
				}

				// 次の攻撃はランダム（同じ攻撃が2回続かないようにする）
				int nextPattern = rand() % 3;

				if (nextPattern == bossPatternIndex) {
					nextPattern = (nextPattern + 1 + rand() % 2) % 3;
				}

				bossPatternIndex = nextPattern;
			}
		}

		else if (bossState == 1) {

			bossAttackTimer--;

			if (bossAttackTimer <= 0) {

				// 今のプレイヤーの方向に向かって突進する
				// プレイヤーの方向を求める
				float chargeDx = playerX - bossX;
				float chargeDy = playerY - bossY;
				float chargeLength = sqrtf(chargeDx * chargeDx + chargeDy * chargeDy);

				if (chargeLength > 0.0f) {
					// 方向×速さで突進の速度にする
					bossChargeVelocityX = chargeDx / chargeLength * 22.0f;
					bossChargeVelocityY = chargeDy / chargeLength * 22.0f;
				}
				else {
					bossChargeVelocityX = 0.0f;
					bossChargeVelocityY = 22.0f;
				}

				bossState = 2;
				bossTrailTimer = 0;
				bossAttackTimer = 30;

				// 通常突進はプレイヤーの手前ギリギリで止まる（分身つきは今まで通り）
				if (!bossIsCloneCharge) {
					// 止まる距離＝プレイヤーまでの距離 − (ボスの半径＋プレイヤーの半径＋すき間20)
					float stopLength = chargeLength - (bossRadius + playerRadius + 20.0f);
					bossAttackTimer = (int)(stopLength / 22.0f);

					// プレイヤーが近すぎたときも最低1フレームは動く
					if (bossAttackTimer < 1) {
						bossAttackTimer = 1;
					}
				}
				else {
					// プレイヤーの近くまで届く長さにする（近すぎ・遠すぎは調整）
					bossAttackTimer = (int)(chargeLength / 22.0f);

					if (bossAttackTimer < 10) {
						bossAttackTimer = 10;
					}
				}

				if (bossAttackTimer > 100) {
					bossAttackTimer = 100;
				}

				// 爆弾を落とすタイマーをリセット
				bombDropTimer = 0;

				screenShakeTimer = 10;

				// 分身突進のときは、分身を2体出して、3体を左・真ん中・右に分けて突進させる
				// 本体がどの位置になるかはランダム（右から出たり、左から出たりする）
				if (bossIsCloneCharge) {

					// 突進の向きをずらす角度（3体分）
					float cloneAngleOffset[3] = { 0.0f, 0.0f, 0.0f };

					// 左右にずらす角度は毎回ランダム（0.3〜0.79ラジアン）
					cloneAngleOffset[0] = 0.3f + (float)(rand() % 50) / 100.0f;
					cloneAngleOffset[2] = -(0.3f + (float)(rand() % 50) / 100.0f);

					// 本体がどの位置に入るか（0〜2）
					int bossSlotIndex = rand() % 3;

					// プレイヤーに向かう元の向き
					float baseVelocityX = bossChargeVelocityX;
					float baseVelocityY = bossChargeVelocityY;

					int cloneSpawnIndex = 0;

					// 3体分の向きを作る
					for (int slotIndex = 0; slotIndex < 3; slotIndex++) {

						float slotCos = cosf(cloneAngleOffset[slotIndex]);
						float slotSin = sinf(cloneAngleOffset[slotIndex]);

						// 突進の向きを回転させる
						float slotVelocityX = baseVelocityX * slotCos - baseVelocityY * slotSin;
						float slotVelocityY = baseVelocityX * slotSin + baseVelocityY * slotCos;

						if (slotIndex == bossSlotIndex) {

							// 本体
							bossChargeVelocityX = slotVelocityX;
							bossChargeVelocityY = slotVelocityY;
						}
						else {

							// 分身
							bossClones[cloneSpawnIndex].x = bossX;
							bossClones[cloneSpawnIndex].y = bossY;
							bossClones[cloneSpawnIndex].velocityX = slotVelocityX;
							bossClones[cloneSpawnIndex].velocityY = slotVelocityY;
							bossClones[cloneSpawnIndex].isExist = 1;

							cloneSpawnIndex++;
						}
					}

					bossCloneBombCount = 0;
				}

			}
		}

		else if (bossState == 2) {

			// ボスの残像を発生させる
			afterImageTimer++;

			if (afterImageTimer >= 2) {

				afterImageTimer = 0;

				for (int bossAfterSpawnIndex = kMaxPlayerAfterImages;
					bossAfterSpawnIndex < kMaxAfterImages;
					bossAfterSpawnIndex++) {

					if (!afterImages[bossAfterSpawnIndex].isExist) {

						// 移動する前のボスの位置を保存
						afterImages[bossAfterSpawnIndex].x = bossX;
						afterImages[bossAfterSpawnIndex].y = bossY;

						afterImages[bossAfterSpawnIndex].startX = bossX;
						afterImages[bossAfterSpawnIndex].startY = bossY;

						afterImages[bossAfterSpawnIndex].life = kAfterImageLife;
						afterImages[bossAfterSpawnIndex].isExist = 1;

						break;
					}
				}
			}

			// ボスの移動
			bossX += bossChargeVelocityX;
			bossY += bossChargeVelocityY;

			// 突進してくるボス本体と分身に当たったらダメージ（ダッシュで避けられる）
			{
				float chargeBodyDx = playerX - bossX;
				float chargeBodyDy = playerY - bossY;

				if (sqrtf(chargeBodyDx * chargeBodyDx + chargeBodyDy * chargeBodyDy) <= bossRadius + playerRadius * 0.7f) {
					damagePlayer(1);
				}

				for (int cloneHitIndex = 0; cloneHitIndex < kMaxBossClones; cloneHitIndex++) {

					if (!bossClones[cloneHitIndex].isExist) {
						continue;
					}

					float cloneHitDx = playerX - bossClones[cloneHitIndex].x;
					float cloneHitDy = playerY - bossClones[cloneHitIndex].y;

					if (sqrtf(cloneHitDx * cloneHitDx + cloneHitDy * cloneHitDy) <= bossRadius + playerRadius * 0.7f) {
						damagePlayer(1);
					}
				}
			}

			// 爆弾を落とす
			bombDropTimer++;

			if (bombDropTimer >= 6) {

				bombDropTimer = 0;

				for (int bombSpawnIndex = 0;
					bombSpawnIndex < kMaxBombs;
					bombSpawnIndex++) {

					if (!bombs[bombSpawnIndex].isExist) {

						bombs[bombSpawnIndex].x = bossX - bossChargeVelocityX * 1.5f;
						bombs[bombSpawnIndex].y = bossY - bossChargeVelocityY * 1.5f;
						bombs[bombSpawnIndex].velocityX = 0.0f;
						bombs[bombSpawnIndex].velocityY = 0.0f;
						bombs[bombSpawnIndex].isFalling = 0;
						bombs[bombSpawnIndex].isGlitch = 0;

						bombs[bombSpawnIndex].timer = 45;
						bombs[bombSpawnIndex].life = 0;
						bombs[bombSpawnIndex].exploded = 0;
						bombs[bombSpawnIndex].isExist = 1;

						break;
					}
				}
			}

			// 分身の移動
			for (int cloneUpdateIndex = 0; cloneUpdateIndex < kMaxBossClones; cloneUpdateIndex++) {

				if (!bossClones[cloneUpdateIndex].isExist) {
					continue;
				}

				bossClones[cloneUpdateIndex].x += bossClones[cloneUpdateIndex].velocityX;
				bossClones[cloneUpdateIndex].y += bossClones[cloneUpdateIndex].velocityY;

				// 画面の外に出たら消す
				if (bossClones[cloneUpdateIndex].x < 0.0f || bossClones[cloneUpdateIndex].x > 1920.0f ||
					bossClones[cloneUpdateIndex].y < 0.0f || bossClones[cloneUpdateIndex].y > 1180.0f) {

					bossClones[cloneUpdateIndex].isExist = 0;
				}
			}

			// 本物が爆弾を置いたタイミングで、分身は2回に1回だけ後ろに爆弾を置く
			if (bombDropTimer == 0) {

				bossCloneBombCount++;

				if (bossCloneBombCount % 2 == 0) {

					for (int cloneBombIndex = 0; cloneBombIndex < kMaxBossClones; cloneBombIndex++) {

						if (bossClones[cloneBombIndex].isExist) {
							placeMineBomb(
								bossClones[cloneBombIndex].x - bossClones[cloneBombIndex].velocityX * 1.5f,
								bossClones[cloneBombIndex].y - bossClones[cloneBombIndex].velocityY * 1.5f,
								45);
						}
					}
				}
			}

			// 突進のカウントダウン
			bossAttackTimer--;

			if (bossX - bossRadius < 0.0f ||
				bossX + bossRadius > 1920.0f ||
				bossY - bossRadius < 0.0f ||
				bossY + bossRadius > 1180.0f ||
				bossAttackTimer <= 0) {

				// 画面の外に出ないように戻す
				if (bossX < bossRadius) {
					bossX = bossRadius;
				}
				if (bossX > 1920.0f - bossRadius) {
					bossX = 1920.0f - bossRadius;
				}
				if (bossY < bossRadius) {
					bossY = bossRadius;
				}
				if (bossY > 1180.0f - bossRadius) {
					bossY = 1180.0f - bossRadius;
				}

				afterImageTimer = 0;

				// 分身を消す
				for (int cloneEndIndex = 0; cloneEndIndex < kMaxBossClones; cloneEndIndex++) {
					bossClones[cloneEndIndex].isExist = 0;
				}

				// 硬直へ
				bossState = 3;
				bossAttackTimer = 60;
			}
		}

		else if (bossState == 3) {

			bossAttackTimer--;

			// 突進が終わったあと、元の高さに戻っていく
			bossY += (295.0f - bossY) * 0.06f;

			if (bossAttackTimer <= 0) {
				bossState = 4;
				bossAttackTimer = 120;
				bossSpinTimer = 0;
			}
		}

		else if (bossState == 4) {

			// その場で回転
			bossRotationAngle += 0.18f;

			// 爆弾を投げるタイマー
			bossSpinTimer++;

			if (bossSpinTimer >= 18) {

				bossSpinTimer = 0;

				for (int bombSpawnIndex = 0;
					bombSpawnIndex < kMaxBombs;
					bombSpawnIndex++) {

					if (!bombs[bombSpawnIndex].isExist) {

						float dx = playerX - bossX;
						float dy = playerY - bossY;

						float length = sqrtf(dx * dx + dy * dy);

						if (length > 0.0f) {
							bombs[bombSpawnIndex].velocityX = dx / length * 7.0f;
							bombs[bombSpawnIndex].velocityY = dy / length * 7.0f;
						}

						bombs[bombSpawnIndex].x = bossX;
						bombs[bombSpawnIndex].y = bossY;
						bombs[bombSpawnIndex].isFalling = 1;
						bombs[bombSpawnIndex].isGlitch = 1;

						bombs[bombSpawnIndex].timer = 55;
						bombs[bombSpawnIndex].life = 0;
						bombs[bombSpawnIndex].exploded = 0;
						bombs[bombSpawnIndex].isExist = 1;

						break;
					}
				}
			}

			bossAttackTimer--;

			if (bossAttackTimer <= 0) {
				bossState = 0;
				// bossAttackTimer = 180;
				bossAttackTimer = (bossPhase == 1) ? 180 : (bossPhase == 2) ? 90 : 70;
			}
		}

		// 召喚（赤い敵を出す）
		else if (bossState == 5) {

			// 画面の上のほうのまん中へ移動する
			bossX += (960.0f - bossX) * 0.08f;
			bossY += (260.0f - bossY) * 0.08f;

			// 一定間隔で赤い敵を出す（4体）
			bossSummonTimer++;

			if (bossSummonTimer >= 22 && bossSummonCount < 4) {

				bossSummonTimer = 0;
				bossSummonCount++;

				// 空いてる枠に召喚する
				for (int summonSpawnIndex = 0; summonSpawnIndex < kMaxSummons; summonSpawnIndex++) {

					if (!summons[summonSpawnIndex].isExist) {

						// ボスの位置からランダムな向きに飛び出す
						float summonAngle = (float)(rand() % 628) / 100.0f;

						summons[summonSpawnIndex].x = bossX;
						summons[summonSpawnIndex].y = bossY;
						summons[summonSpawnIndex].velocityX = cosf(summonAngle) * 14.0f;
						summons[summonSpawnIndex].velocityY = sinf(summonAngle) * 14.0f;
						summons[summonSpawnIndex].life = 300;
						summons[summonSpawnIndex].isExist = 1;

						screenShakeTimer = 4;

						break;
					}
				}
			}

			bossAttackTimer--;

			if (bossAttackTimer <= 0) {
				// 召喚が終わったら、硬直→回転攻撃へつなぐ
				bossState = 3;
				bossAttackTimer = 60;
			}
		}

		// 回転レーザー（赤い極太レーザーが2本、反対向きに半周する）
		else if (bossState == 6) {

			bossRotLaserTimer++;

			// 画面のまん中へ移動する
			// bossX += (960.0f - bossX) * 0.1f;
			// bossY += (300.0f - bossY) * 0.1f;
			bossX += (kRotLaserCenterX - bossX) * 0.1f;
			bossY += (kRotLaserCenterY - bossY) * 0.1f;

			// 溜めの終わりが近づくと、ボスが震えるように画面が小さく揺れる
			if (bossRotLaserTimer > kRotLaserWarningFrames - 40 &&
				bossRotLaserTimer <= kRotLaserWarningFrames &&
				bossRotLaserTimer % 6 == 0) {
				screenShakeTimer = 3;
			}

			// 発射の瞬間は大きく揺らす
			if (bossRotLaserTimer == kRotLaserWarningFrames + 1) {
				screenShakeTimer = 15;
			}

			// 予告のあと、レーザーが回る
			if (bossRotLaserTimer > kRotLaserWarningFrames) {

				bossRotLaserAngle += bossRotLaserDir * (3.14159265f / (float)kRotLaserFireFrames);

				// 発射したあと10フレームかけて、レーザーが太くなる（見た目と当たり判定を合わせる）
				float rotHitGrow = (float)(bossRotLaserTimer - kRotLaserWarningFrames) / 10.0f;

				if (rotHitGrow > 1.0f) {
					rotHitGrow = 1.0f;
				}

				// 2本のレーザーとプレイヤーの当たり判定
				for (int rotHitIndex = 0; rotHitIndex < 2; rotHitIndex++) {

					float rotHitAngle = bossRotLaserAngle + 3.14159265f * (float)rotHitIndex;
					float rotDirX = cosf(rotHitAngle);
					float rotDirY = sinf(rotHitAngle);

					float rotDx = playerX - bossX;
					float rotDy = playerY - bossY;

					// レーザーの向きにどれだけ進んでいるか／横にどれだけ離れているか
					float rotAlong = rotDx * rotDirX + rotDy * rotDirY;
					float rotSide = -rotDx * rotDirY + rotDy * rotDirX;

					if (!bossRotLaserHit &&
						rotAlong >= 0.0f &&
						rotAlong <= kRotLaserLength &&
						fabsf(rotSide) <= kRotLaserWidth * 0.5f * rotHitGrow + playerRadius) {

						if (damagePlayer(1)) {
							bossRotLaserHit = 1;
						}
					}
				}
			}

			// 回り終わったら、硬直へ
			if (bossRotLaserTimer >= kRotLaserWarningFrames + kRotLaserFireFrames) {
				bossIsInvincible = 0;
				bossState = 3;
				bossAttackTimer = 60;
			}
		}

		// 十字レーザー（第2形態の大技。＋ → × → 8本同時）
		else if (bossState == 7) {

			bossBigTimer++;

			// 画面のまん中へ移動する
			bossX += (kRotLaserCenterX - bossX) * 0.1f;
			bossY += (kRotLaserCenterY - bossY) * 0.1f;

			if (bossBigTimer <= kBigChargeFrames) {

				// 溜めの終わりが近づくと画面が小さく揺れる
				if (bossBigTimer > kBigChargeFrames - 40 && bossBigTimer % 6 == 0) {
					screenShakeTimer = 3;
				}
			}
			else {

				int crossTime = bossBigTimer - kBigChargeFrames - 1;
				int crossWave = crossTime / (kCrossWarnFrames + kCrossFireFrames);
				int crossInWave = crossTime % (kCrossWarnFrames + kCrossFireFrames);

				if (crossWave < 3) {

					// レーザーが出る瞬間
					if (crossInWave == kCrossWarnFrames) {
						screenShakeTimer = 12;
						bossBigHit = 0;
					}

					if (crossInWave >= kCrossWarnFrames) {

						int crossBeamCount = (crossWave == 2) ? 8 : 4;

						for (int crossHitIndex = 0; crossHitIndex < crossBeamCount; crossHitIndex++) {

							if (!bossBigHit && isPlayerInBeam(crossBeamAngle(crossWave, crossHitIndex), kCrossLaserWidth * 0.5f)) {

								if (damagePlayer(1)) {
									bossBigHit = 1;
								}
							}
						}
					}
				}
			}

			// 3回出し終わったら、硬直へ
			if (bossBigTimer >= kBigChargeFrames + 3 * (kCrossWarnFrames + kCrossFireFrames)) {
				bossIsInvincible = 0;
				bossState = 3;
				bossAttackTimer = 60;
			}
		}

		// バリア爆発（第3形態の大技。すき間のある衝撃の輪が何重にも広がる）
		else if (bossState == 8) {

			bossBigTimer++;

			bossX += (kRotLaserCenterX - bossX) * 0.1f;
			bossY += (kRotLaserCenterY - bossY) * 0.1f;

			if (bossBigTimer > kBigChargeFrames - 40 && bossBigTimer <= kBigChargeFrames && bossBigTimer % 6 == 0) {
				screenShakeTimer = 3;
			}

			for (int blastIndex = 0; blastIndex < kBlastRings; blastIndex++) {

				// この輪が出てから何フレーム経ったか
				int blastAge = bossBigTimer - (kBigChargeFrames + blastIndex * kBlastRingInterval);

				if (blastAge == 0) {
					screenShakeTimer = 10;
				}

				if (blastAge > 0) {

					float blastRadius = 60.0f + (float)blastAge * kBlastRingSpeed;

					float blastDx = playerX - bossX;
					float blastDy = playerY - bossY;
					float blastDist = sqrtf(blastDx * blastDx + blastDy * blastDy);

					// すき間の向きとの角度の差
					float blastDiff = atan2f(blastDy, blastDx) - blastGapAngle[blastIndex];

					while (blastDiff > 3.14159265f) {
						blastDiff -= 6.2831853f;
					}

					while (blastDiff < -3.14159265f) {
						blastDiff += 6.2831853f;
					}

					// 輪に重なっていて、すき間の外にいたら当たり
					if (!blastRingHit[blastIndex] &&
						fabsf(blastDist - blastRadius) <= kBlastRingThickness * 0.5f + playerRadius &&
						fabsf(blastDiff) > kBlastGapHalf) {

						if (damagePlayer(1)) {
							blastRingHit[blastIndex] = 1;
						}
					}
				}
			}

			// 全部の輪が画面の外へ出たら、硬直へ
			if (bossBigTimer >= kBigChargeFrames + (kBlastRings - 1) * kBlastRingInterval + 150) {
				bossIsInvincible = 0;
				bossState = 3;
				bossAttackTimer = 60;
			}
		}

		// 反射レーザー（第3形態の大技。画面の端で跳ね返るレーザーが、3回・だんだん増えて出る）
		else if (bossState == 10) {

			bossBigTimer++;

			// 画面のまん中へ移動する
			bossX += (kRotLaserCenterX - bossX) * 0.1f;
			bossY += (kRotLaserCenterY - bossY) * 0.1f;

			// 溜めの終わりが近づくと画面が小さく揺れる
			if (bossBigTimer > kBigChargeFrames - 40 && bossBigTimer <= kBigChargeFrames && bossBigTimer % 6 == 0) {
				screenShakeTimer = 3;
			}

			if (bossBigTimer > kBigChargeFrames) {

				int reflectTime = bossBigTimer - kBigChargeFrames - 1;
				int reflectWave = reflectTime / (kReflectWarnFrames + kReflectFireFrames);
				int reflectInWave = reflectTime % (kReflectWarnFrames + kReflectFireFrames);

				if (reflectWave < kReflectWaves) {

					// 予告が始まるときに、本数と向きを決める
					// 本数は回を重ねるごとに2本ずつ増える（3本、5本、7本、9本、11本）
					// 向きはランダムだが、プレイヤーが入れる隙間ができるまで決め直す
					if (reflectInWave == 0) {

						reflectActiveCount = reflectWave * 2 + 3;

						int reflectTryCount = 0;

						for (;;) {

							for (int reflectSetIndex = 0; reflectSetIndex < reflectActiveCount; reflectSetIndex++) {
								reflectAngle[reflectSetIndex] = (float)(rand() % 6283) / 1000.0f - 3.1415f;
							}

							if (reflectHasGap(reflectActiveCount)) {
								break;
							}

							reflectTryCount++;

							// 何回決め直しても隙間ができないときは、本数を2本減らす
							if (reflectTryCount >= 40) {

								if (reflectActiveCount <= 1) {
									break;
								}

								reflectActiveCount -= 2;

								if (reflectActiveCount < 1) {
									reflectActiveCount = 1;
								}

								reflectTryCount = 0;
							}
						}
					}

					int reflectCount = reflectActiveCount;

					// レーザーが出る瞬間
					if (reflectInWave == kReflectWarnFrames) {
						screenShakeTimer = 12;
						bossBigHit = 0;
					}

					// レーザーが出ている間の当たり判定
					if (reflectInWave >= kReflectWarnFrames) {

						for (int reflectHitIndex = 0; reflectHitIndex < reflectCount; reflectHitIndex++) {

							float hitPathX[kReflectBounces + 2];
							float hitPathY[kReflectBounces + 2];

							calcReflectPath(reflectAngle[reflectHitIndex], hitPathX, hitPathY);

							for (int reflectSegIndex = 0; reflectSegIndex <= kReflectBounces; reflectSegIndex++) {

								if (!bossBigHit &&
									distPlayerToSegment(hitPathX[reflectSegIndex], hitPathY[reflectSegIndex], hitPathX[reflectSegIndex + 1], hitPathY[reflectSegIndex + 1])
									<= kReflectLaserWidth * 0.5f + playerRadius) {

									if (damagePlayer(1)) {
										bossBigHit = 1;
									}
								}
							}
						}
					}
				}
			}

			// 3回出し終わったら、硬直へ
			if (bossBigTimer >= kBigChargeFrames + kReflectWaves * (kReflectWarnFrames + kReflectFireFrames)) {
				bossIsInvincible = 0;
				bossState = 3;
				bossAttackTimer = 60;
			}
		}

		// 全画面ビーム（第3形態の大技。画面の下の左か右の安全地帯だけが安全）
		else if (bossState == 9) {

			bossBigTimer++;

			bossX += (kRotLaserCenterX - bossX) * 0.1f;
			bossY += (kRotLaserCenterY - bossY) * 0.1f;

			if (bossBigTimer > kFullBeamChargeFrames - 40 && bossBigTimer <= kFullBeamChargeFrames && bossBigTimer % 6 == 0) {
				screenShakeTimer = 3;
			}

			// 発射の瞬間
			if (bossBigTimer == kFullBeamChargeFrames + 1) {
				screenShakeTimer = 30;
				bossBigHit = 0;
			}

			// 発射中に、安全地帯の外にいたら当たり
			if (bossBigTimer > kFullBeamChargeFrames && bossBigTimer <= kFullBeamChargeFrames + kFullBeamFireFrames) {

				float fullSafeX = (fullBeamSafeSide == 0) ? 0.0f : 1920.0f - kFullBeamSafeW;
				float fullSafeY = 1180.0f - kFullBeamSafeH;

				bool isInSafe =
					playerX >= fullSafeX && playerX <= fullSafeX + kFullBeamSafeW &&
					playerY >= fullSafeY && playerY <= fullSafeY + kFullBeamSafeH;

				if (!bossBigHit && !isInSafe) {

					if (damagePlayer(2)) {
						bossBigHit = 1;
					}
				}
			}

			// 終わったら、硬直へ
			if (bossBigTimer >= kFullBeamChargeFrames + kFullBeamFireFrames + 30) {
				bossIsInvincible = 0;
				bossState = 3;
				bossAttackTimer = 60;
			}
		}

		// ボスの突進準備中にオーラを大きくする
		if (bossState == 1) {
			bossAuraRadius += 2.0f;

			if (bossAuraRadius > 100.0f) {
				bossAuraRadius = 100.0f;
			}

			bossAuraAlpha += 0.03f;

			if (bossAuraAlpha > 1.0f) {
				bossAuraAlpha = 1.0f;
			}
		}
		else {
			bossAuraRadius = 0.0f;
			bossAuraAlpha = 0.0f;
		}

		// 召喚された敵（赤）の更新
		for (int summonUpdateIndex = 0; summonUpdateIndex < kMaxSummons; summonUpdateIndex++) {

			if (!summons[summonUpdateIndex].isExist) {
				continue;
			}

			// 飛び出したあとは、ゆっくりプレイヤーに近づく
			summons[summonUpdateIndex].velocityX *= 0.9f;
			summons[summonUpdateIndex].velocityY *= 0.9f;

			float summonDx = playerX - summons[summonUpdateIndex].x;
			float summonDy = playerY - summons[summonUpdateIndex].y;
			float summonLength = sqrtf(summonDx * summonDx + summonDy * summonDy);

			summons[summonUpdateIndex].x += summons[summonUpdateIndex].velocityX;
			summons[summonUpdateIndex].y += summons[summonUpdateIndex].velocityY;

			if (summonLength > 0.0f) {
				summons[summonUpdateIndex].x += summonDx / summonLength * 2.5f;
				summons[summonUpdateIndex].y += summonDy / summonLength * 2.5f;
			}

			// プレイヤーが触れたら仲間（青）になる
			if (summonLength < playerRadius + kStageEnemyRadius) {

				// 空いてる枠に仲間として入れる
				for (int summonAllyIndex = 0; summonAllyIndex < kMaxStageEnemies; summonAllyIndex++) {

					// 空きか、ステージで集められなかった赤い敵の枠を使う
					if (!stageEnemies[summonAllyIndex].isAlly) {

						stageEnemies[summonAllyIndex].x = summons[summonUpdateIndex].x;
						stageEnemies[summonAllyIndex].y = summons[summonUpdateIndex].y;
						stageEnemies[summonAllyIndex].velocityX = 0.0f;
						stageEnemies[summonAllyIndex].velocityY = 0.0f;
						stageEnemies[summonAllyIndex].wanderTimer = 0;
						stageEnemies[summonAllyIndex].flash = 0;
						stageEnemies[summonAllyIndex].shootTimer = 20 + rand() % 60;
						stageEnemies[summonAllyIndex].isAlly = 1;
						stageEnemies[summonAllyIndex].isExist = 1;

						break;
					}
				}

				summons[summonUpdateIndex].isExist = 0;
				continue;
			}

			// 放っておくと自爆する
			summons[summonUpdateIndex].life--;

			if (summons[summonUpdateIndex].life <= 0) {
				placeMineBomb(summons[summonUpdateIndex].x, summons[summonUpdateIndex].y, 30);
				summons[summonUpdateIndex].isExist = 0;
			}
		}

		// currentFrameのカウント
		if (isCurrentFrameCount)
		{
			if (currentFrame < 150)
			{
				currentFrame++;
			}
			else
			{
				currentFrame = 150;
				isCurrentFrameCount = 0;
			}
		}

		// クールダウン
		if (dashCooldown > 0) {
			dashCooldown--;
		}

		if (dashStunTimer > 0) {
			dashStunTimer--;
		}

		// ダッシュ移動
		if (dashMoveTimer > 0) {

			dashMoveTimer--;

			float t = 1.0f - (float)dashMoveTimer / kDashMoveFrames;

			playerX = dashStartX + (dashTargetX - dashStartX) * t;
			playerY = dashStartY + (dashTargetY - dashStartY) * t;
		}

		// 入力方向
		float moveX = Controller::GetLeftStickX();
		float moveY = Controller::GetLeftStickY();

		// 十字ボタン入力
		if (Controller::IsDown(kPadButton0)) moveY -= 1.0f;
		if (Controller::IsDown(kPadButton1)) moveY += 1.0f;
		if (Controller::IsDown(kPadButton2)) moveX -= 1.0f;
		if (Controller::IsDown(kPadButton3)) moveX += 1.0f;

		if (keys[DIK_W]) moveY -= 1.0f;
		if (keys[DIK_S]) moveY += 1.0f;
		if (keys[DIK_A]) moveX -= 1.0f;
		if (keys[DIK_D]) moveX += 1.0f;

		// やられている間は動けない
		if (isPlayerDead) {
			moveX = 0.0f;
			moveY = 0.0f;
		}

		// 攻撃タイプ切り替え
		if (keys[DIK_1] && !preKeys[DIK_1]) {
			attackType = 1;
		}

		if (keys[DIK_2] && !preKeys[DIK_2]) {
			attackType = 2;
		}

		if (moveX != 0.0f || moveY != 0.0f) {

			playerAngle = atan2f(moveY, moveX);

			if (dashStunTimer == 0) {

				float len = sqrtf(moveX * moveX + moveY * moveY);

				if (len > 0.0f) {
					playerX += (moveX / len) * speed;
					playerY += (moveY / len) * speed;
				}
			}
		}

		// プレイヤーの「後ろ」の向きを更新する
		updateBehindDir(playerX, playerY);

		int bossAllyTotalNow = 0;

		for (int bossAllyTotalIndex = 0; bossAllyTotalIndex < kMaxStageEnemies; bossAllyTotalIndex++) {
			if (stageEnemies[bossAllyTotalIndex].isExist && stageEnemies[bossAllyTotalIndex].isAlly) {
				bossAllyTotalNow++;
			}
		}

		// 仲間の更新処理（プレイヤーの後ろについていき、ボスに向かって弾を撃つ）
		int bossAllyUpdateIndex = 0;

		for (int allyUpdateIndex = 0; allyUpdateIndex < kMaxStageEnemies; allyUpdateIndex++) {

			if (!stageEnemies[allyUpdateIndex].isExist || !stageEnemies[allyUpdateIndex].isAlly) {
				continue;
			}

			// プレイヤーの後ろの群れの定位置に向かってついていく
			float allySlotX = 0.0f;
			float allySlotY = 0.0f;
			getAllySlot(bossAllyUpdateIndex, bossAllyTotalNow, playerX, playerY, allySlotX, allySlotY);

			float allyFollowRate = getAllyFollowRate(bossAllyUpdateIndex);
			float allyFollowX = (allySlotX - stageEnemies[allyUpdateIndex].x) * allyFollowRate;
			float allyFollowY = (allySlotY - stageEnemies[allyUpdateIndex].y) * allyFollowRate;

			float allyFollowLength = sqrtf(allyFollowX * allyFollowX + allyFollowY * allyFollowY);

			if (allyFollowLength > 16.0f) {
				allyFollowX = allyFollowX / allyFollowLength * 16.0f;
				allyFollowY = allyFollowY / allyFollowLength * 16.0f;
			}

			stageEnemies[allyUpdateIndex].x += allyFollowX;
			stageEnemies[allyUpdateIndex].y += allyFollowY;

			// 弾を撃つタイマー
			stageEnemies[allyUpdateIndex].shootTimer--;

			if (stageEnemies[allyUpdateIndex].shootTimer <= 0) {

				stageEnemies[allyUpdateIndex].shootTimer = 50 + rand() % 40;

				for (int allyBulletSpawnIndex = 0; allyBulletSpawnIndex < kMaxAllyBullets; allyBulletSpawnIndex++) {

					if (!allyBullets[allyBulletSpawnIndex].isExist) {

						float shootDx = bossX - stageEnemies[allyUpdateIndex].x;
						float shootDy = bossY - stageEnemies[allyUpdateIndex].y;
						float shootLength = sqrtf(shootDx * shootDx + shootDy * shootDy);

						if (shootLength > 0.0f) {
							allyBullets[allyBulletSpawnIndex].x = stageEnemies[allyUpdateIndex].x;
							allyBullets[allyBulletSpawnIndex].y = stageEnemies[allyUpdateIndex].y;
							allyBullets[allyBulletSpawnIndex].velocityX = shootDx / shootLength * kAllyBulletSpeed;
							allyBullets[allyBulletSpawnIndex].velocityY = shootDy / shootLength * kAllyBulletSpeed;
							allyBullets[allyBulletSpawnIndex].isExist = 1;
						}

						break;
					}
				}
			}

			bossAllyUpdateIndex++;
		}

		// 仲間の弾の更新処理（ボスに向かって少し曲がる）
		for (int allyBulletUpdateIndex = 0; allyBulletUpdateIndex < kMaxAllyBullets; allyBulletUpdateIndex++) {

			if (!allyBullets[allyBulletUpdateIndex].isExist) {
				continue;
			}

			float bulletDx = bossX - allyBullets[allyBulletUpdateIndex].x;
			float bulletDy = bossY - allyBullets[allyBulletUpdateIndex].y;
			float bulletLength = sqrtf(bulletDx * bulletDx + bulletDy * bulletDy);

			if (bulletLength > 0.0f) {

				float turnX = allyBullets[allyBulletUpdateIndex].velocityX + bulletDx / bulletLength * kAllyBulletSpeed * 0.25f;
				float turnY = allyBullets[allyBulletUpdateIndex].velocityY + bulletDy / bulletLength * kAllyBulletSpeed * 0.25f;
				float turnLength = sqrtf(turnX * turnX + turnY * turnY);

				if (turnLength > 0.0f) {
					allyBullets[allyBulletUpdateIndex].velocityX = turnX / turnLength * kAllyBulletSpeed;
					allyBullets[allyBulletUpdateIndex].velocityY = turnY / turnLength * kAllyBulletSpeed;
				}
			}

			allyBullets[allyBulletUpdateIndex].x += allyBullets[allyBulletUpdateIndex].velocityX;
			allyBullets[allyBulletUpdateIndex].y += allyBullets[allyBulletUpdateIndex].velocityY;

			// ボスに当たったら火花を出して消える
			float hitDx = bossX - allyBullets[allyBulletUpdateIndex].x;
			float hitDy = bossY - allyBullets[allyBulletUpdateIndex].y;

			// バリアがあるときは、バリアの外側で当たる
			float allyBarrierPad = (bossBarrier > 0) ? kBossBarrierExtraRadius : 0.0f;

			// if (hitDx * hitDx + hitDy * hitDy < (bossRadius + 8.0f) * (bossRadius + 8.0f)) {
			if (hitDx * hitDx + hitDy * hitDy < (bossRadius + 8.0f + allyBarrierPad) * (bossRadius + 8.0f + allyBarrierPad)) {

				// バリアに当たったら、バリアが見えるようになる
				if (bossBarrier > 0) {
					barrierHitTimer = kBarrierShowFrames;
				}

				// ボスのシールドを減らす（無敵中・バリア中は減らさない）
				if (bossShield > 0 && !bossIsInvincible && bossBarrier <= 0) {
					bossShield -= 1;

					if (bossShield <= 0) {
						bossShield = 0;
						bossShieldBreakTimer = 120;

						// シールド破壊演出を開始
						shieldBreakEffectTimer = kShieldBreakEffectMax;

						// スローモーション開始
						gameSpeed = 0.3f;
						slowMotionTimer = kSlowMotionMax;
					}
				}

				allyBullets[allyBulletUpdateIndex].isExist = 0;
				screenShakeTimer = 2;

				for (int allySparkCount = 0; allySparkCount < 6; allySparkCount++) {

					for (int allySparkIndex = 0; allySparkIndex < kMaxSparks; allySparkIndex++) {

						if (!sparks[allySparkIndex].isExist) {

							sparks[allySparkIndex].x = allyBullets[allyBulletUpdateIndex].x;
							sparks[allySparkIndex].y = allyBullets[allyBulletUpdateIndex].y;
							sparks[allySparkIndex].velocityX = static_cast<float>(rand() % 100 - 50) / 10.0f;
							sparks[allySparkIndex].velocityY = static_cast<float>(rand() % 100 - 50) / 10.0f;
							sparks[allySparkIndex].life = 20;
							sparks[allySparkIndex].isExist = 1;

							break;
						}
					}
				}
			}
			// 画面外に出たら消す
			else if (allyBullets[allyBulletUpdateIndex].x < -50 || allyBullets[allyBulletUpdateIndex].x > 1970 ||
				allyBullets[allyBulletUpdateIndex].y < -50 || allyBullets[allyBulletUpdateIndex].y > 1230) {

				allyBullets[allyBulletUpdateIndex].isExist = 0;
			}
		}

		// 残像の更新処理
		for (int afterUpdateIndex = 0; afterUpdateIndex < kMaxAfterImages; afterUpdateIndex++) {

			if (afterImages[afterUpdateIndex].isExist) {

				// 残像の寿命を減らす
				afterImages[afterUpdateIndex].life--;

				if (afterImages[afterUpdateIndex].life <= 0) {
					afterImages[afterUpdateIndex].isExist = 0;
				}
			}
		}

		// ひし形砲台の先端は、攻撃が始まるまで毎フレームずっとボスのほうを向き続ける
		// （4回目のダッシュで攻撃が始まるので、その瞬間の向きで固定される）
		if (!isAttacking) {
			for (int diamondAngleIndex = 0;
				diamondAngleIndex < kMaxBoxes;
				diamondAngleIndex++) {

				if (diamonds[diamondAngleIndex].isExist) {

					float dx = bossX - diamonds[diamondAngleIndex].x;
					float dy = bossY - diamonds[diamondAngleIndex].y;

					diamonds[diamondAngleIndex].angle = atan2f(dy, dx);
				}
			}
		}

		// --- 無敵時間とスタミナの更新 ---
		if (playerInvincibleTimer > 0) {
			playerInvincibleTimer--;
		}

		if (staminaFlashTimer > 0) {
			staminaFlashTimer--;
		}

		// ダッシュしてから少し待つと、スタミナが回復していく
		if (staminaRegenTimer > 0) {
			staminaRegenTimer--;
		}
		else if (stamina < kStaminaMax) {

			stamina += kStaminaRegen;

			if (stamina > kStaminaMax) {
				stamina = kStaminaMax;
			}
		}

		// スタミナが足りないのにダッシュを押したら、ゲージを赤く点滅させる
		if ((Controller::IsPressed(kPadButton10) || (keys[DIK_SPACE] && !preKeys[DIK_SPACE])) &&
			!isAttacking && !isPlayerDead && stamina < kDashStaminaCost) {
			staminaFlashTimer = 24;
		}

		// --- ダッシュ実行 ---
		//　ダッシュボタンが押されたとき、ダッシュが最大回数より小さかった時
		if ((Controller::IsPressed(kPadButton10) ||
			(keys[DIK_SPACE] && !preKeys[DIK_SPACE])) &&
			dashCount < kMaxDashes)
		{


			//　ダッシュがクールダウン中ではない時かつ、攻撃中じゃないなら
			if (dashCooldown <= 0 && !isAttacking && !isPlayerDead && stamina >= kDashStaminaCost)
			{
				currentFrame = 0;
				isCurrentFrameCount = 1;

				float startX = playerX;
				float startY = playerY;

				float dashDirX = cosf(playerAngle);
				float dashDirY = sinf(playerAngle);

				dashStartX = startX;
				dashStartY = startY;

				dashTargetX = startX + dashDirX * dashDistance;
				dashTargetY = startY + dashDirY * dashDistance;

				if (dashTargetX < playerRadius) dashTargetX = playerRadius;
				if (dashTargetX > 1920.0f - playerRadius) dashTargetX = 1920.0f - playerRadius;
				if (dashTargetY < playerRadius) dashTargetY = playerRadius;
				if (dashTargetY > 1180.0f - playerRadius) dashTargetY = 1180.0f - playerRadius;

				dashMoveTimer = kDashMoveFrames;
				dashStunTimer = kDashStunMax;

				if (currentFrame <= 0)
				{
					currentFrame++;
				}

				// プレイヤーの残像を発生させる
				for (int afterDashIndex = 0; afterDashIndex < kMaxPlayerAfterImages; afterDashIndex++) {

					float ratio = (float)afterDashIndex / (kMaxPlayerAfterImages - 1);

					afterImages[afterDashIndex].x = dashStartX + (dashTargetX - dashStartX) * ratio;
					afterImages[afterDashIndex].y = dashStartY + (dashTargetY - dashStartY) * ratio;

					afterImages[afterDashIndex].startX = afterImages[afterDashIndex].x;
					afterImages[afterDashIndex].startY = afterImages[afterDashIndex].y;

					afterImages[afterDashIndex].life = kAfterImageLife;
					afterImages[afterDashIndex].isExist = 1;
				}

				// メイン砲台
				if (boxCount < kMaxBoxes) {
					boxes[boxCount].x = startX;
					boxes[boxCount].y = startY;
					boxes[boxCount].width = 12;
					boxes[boxCount].height = 28;
					boxes[boxCount].color = 0x88FFFFFF;
					boxes[boxCount].delay = 0;

					// ひし形砲台の生成
					diamonds[boxCount].x = startX;
					diamonds[boxCount].y = startY;

					float dx = bossX - startX;
					float dy = bossY - startY;

					diamonds[boxCount].angle = atan2f(dy, dx);
					diamonds[boxCount].openSize = 0.0f;
					diamonds[boxCount].isExist = 1;

					boxes[boxCount].isExist = 1;
					boxCount++;
				}

				for (int p = 0; p < 3; p++) {
					if (boxCount < kMaxBoxes) {

						float ratio = (float)(p + 1) / 4.0f;

						boxes[boxCount].x = startX + (dashTargetX - startX) * ratio;
						boxes[boxCount].y = startY + (dashTargetY - startY) * ratio;
						boxes[boxCount].width = 12;
						boxes[boxCount].height = 28;
						boxes[boxCount].color = 0x88FFFFFF;
						boxes[boxCount].delay = (p + 1) * 3;

						diamonds[boxCount].x = boxes[boxCount].x;
						diamonds[boxCount].y = boxes[boxCount].y;

						float dx = bossX - boxes[boxCount].x;
						float dy = bossY - boxes[boxCount].y;

						diamonds[boxCount].angle = atan2f(dy, dx);
						diamonds[boxCount].openSize = 0.0f;
						diamonds[boxCount].isExist = 1;

						boxes[boxCount].isExist = 1;
						boxCount++;
					}
				}

				// 軌道線
				if (lineCount < kMaxLines) {
					lines[lineCount].startX = startX;
					lines[lineCount].startY = startY;
					lines[lineCount].endX = dashTargetX;
					lines[lineCount].endY = dashTargetY;
					lines[lineCount].isExist = 1;
					lineCount++;
				}

				dashCount++;
				dashCooldown = kDashCooldownMax;

				// スタミナを使う
				stamina -= kDashStaminaCost;
				staminaRegenTimer = kStaminaRegenDelay;

				if (dashCount == kMaxDashes) {
					isAttacking = true;
					currentAttackIndex = 0;
					attackTimer = 10;

					// レーザーモードで四角を作っていたら、普通の発射ではなく溜め技にする
					if (attackType == 2 && detectSquareDash(megaLaserX, megaLaserY)) {

						megaLaserState = 1;
						megaLaserTimer = 0;
						megaLaserAngle = atan2f(bossY - megaLaserY, bossX - megaLaserX);

						// 砲台（ひし形・小さな長方形）は出さない
						for (int megaHideIndex = 0; megaHideIndex < kMaxBoxes; megaHideIndex++) {
							boxes[megaHideIndex].isExist = 0;
							diamonds[megaHideIndex].isExist = 0;
						}
					}
				}
			}

		}

		// 小長方形の表示ディレイ
		for (int delayIndex = 0; delayIndex < boxCount; delayIndex++) {
			if (boxes[delayIndex].isExist && boxes[delayIndex].delay > 0) {
				boxes[delayIndex].delay--;
			}
		}

		// ひし形が開く処理
		for (int diamondUpdateIndex = 0;
			diamondUpdateIndex < kMaxBoxes;
			diamondUpdateIndex++) {

			if (diamonds[diamondUpdateIndex].isExist) {

				if (isAttacking) {

					diamonds[diamondUpdateIndex].openSize += 3.0f;

					if (diamonds[diamondUpdateIndex].openSize > 25.0f) {
						diamonds[diamondUpdateIndex].openSize = 25.0f;
					}
				}
			}
		}

		// --- 溜め技（超極太レーザー）の処理 ---
		if (megaLaserState != 0) {

			megaLaserTimer++;

			// ボスのほうを向き続ける
			megaLaserAngle = atan2f(bossY - megaLaserY, bossX - megaLaserX);

			if (megaLaserState == 1) {

				// 溜めの終わりが近づくと画面が小さく揺れる
				if (megaLaserTimer > kMegaChargeFrames - 40 && megaLaserTimer % 6 == 0) {
					screenShakeTimer = 3;
				}

				// 溜まったら発射
				if (megaLaserTimer >= kMegaChargeFrames) {
					megaLaserState = 2;
					megaLaserTimer = 0;
					screenShakeTimer = 20;
				}
			}
			else {

				// 出るときは8フレームで太くなり、消えるときは15フレームで細くなる
				float megaHitGrow = (float)megaLaserTimer / 8.0f;
				float megaHitFade = (float)(kMegaFireFrames - megaLaserTimer) / 15.0f;

				if (megaHitGrow > 1.0f) {
					megaHitGrow = 1.0f;
				}

				if (megaHitFade < megaHitGrow) {
					megaHitGrow = megaHitFade;
				}

				if (megaHitGrow < 0.0f) {
					megaHitGrow = 0.0f;
				}

				float megaDirX = cosf(megaLaserAngle);
				float megaDirY = sinf(megaLaserAngle);
				float megaToBossX = bossX - megaLaserX;
				float megaToBossY = bossY - megaLaserY;

				// レーザーの進行方向にどれだけ進んでいるか／横にどれだけ離れているか
				float megaAlong = megaToBossX * megaDirX + megaToBossY * megaDirY;
				float megaSide = -megaToBossX * megaDirY + megaToBossY * megaDirX;

				// バリアがあるときは、バリアの外側で当たる
				float megaBarrierPad = (bossBarrier > 0) ? kBossBarrierExtraRadius : 0.0f;
				float megaHitRange = kMegaLaserHalfWidth * megaHitGrow + bossRadius + megaBarrierPad;

				// 出ている間はずっと画面を揺らす
				screenShakeTimer = 3;

				// ボスに当たっている間、毎フレームシールドを減らす
				if (megaAlong >= 0.0f && megaAlong <= kMegaLaserLength + bossRadius && fabsf(megaSide) <= megaHitRange) {

					// バリアに当たっている間は、バリアが見えている
					if (bossBarrier > 0) {
						barrierHitTimer = kBarrierShowFrames;
					}

					damageBossShield(kMegaLaserDamage, 0);   // レーザーはバリアを壊せない

					// 3フレームに1回、レーザーが当たっている場所から火花を出す
					if (megaLaserTimer % 3 == 0) {

						float megaHitX = bossX - megaDirX * (bossRadius + megaBarrierPad);
						float megaHitY = bossY - megaDirY * (bossRadius + megaBarrierPad);

						for (int megaSparkCount = 0; megaSparkCount < 8; megaSparkCount++) {

							for (int megaSparkIndex = 0; megaSparkIndex < kMaxSparks; megaSparkIndex++) {

								if (!sparks[megaSparkIndex].isExist) {

									sparks[megaSparkIndex].x = megaHitX;
									sparks[megaSparkIndex].y = megaHitY;

									sparks[megaSparkIndex].velocityX =
										static_cast<float>(rand() % 160 - 80) / 10.0f;

									sparks[megaSparkIndex].velocityY =
										static_cast<float>(rand() % 160 - 80) / 10.0f;

									sparks[megaSparkIndex].life = 40;
									sparks[megaSparkIndex].isExist = 1;
									break;
								}
							}
						}
					}
				}

				// 5秒たった、またはボスのシールドが壊れたら終わり（普通の攻撃の終わりと同じ後片付け）
				if (megaLaserTimer >= kMegaFireFrames || bossShield <= 0 || shieldBreakEffectTimer > 0) {

					megaLaserState = 0;
					megaLaserTimer = 0;

					for (int megaClearIndex = 0; megaClearIndex < kMaxBoxes; megaClearIndex++) {
						diamonds[megaClearIndex].isExist = 0;
					}

					dashCount = 0;
					boxCount = 0;
					isAttacking = 0;

					for (int megaClearLineIndex = 0; megaClearLineIndex < kMaxLines; megaClearLineIndex++) {
						lines[megaClearLineIndex].isExist = 0;
					}

					lineCount = 0;
				}
			}
		}

		// --- 連続照射処理 ---
		if (isAttacking && megaLaserState == 0) {

			if (attackTimer > 0) {
				attackTimer--;
			}
			else {

				if (currentAttackIndex < boxCount) {

					if (boxes[currentAttackIndex].isExist) {

						float startX = boxes[currentAttackIndex].x;
						float startY = boxes[currentAttackIndex].y;

						// ひし形の先端が向いている方向を取得
						float directionX = cosf(diamonds[currentAttackIndex].angle);
						float directionY = sinf(diamonds[currentAttackIndex].angle);

						if (attackType == 1) {

							beams[currentAttackIndex].velocityX = directionX;
							beams[currentAttackIndex].velocityY = directionY;

							beams[currentAttackIndex].x = startX;
							beams[currentAttackIndex].y = startY;

							beams[currentAttackIndex].speed = 0.5f;
							beams[currentAttackIndex].isShot = 1;

							for (int trailInitIndex = 0; trailInitIndex < 8; trailInitIndex++) {
								beams[currentAttackIndex].trailX[trailInitIndex] = startX;
								beams[currentAttackIndex].trailY[trailInitIndex] = startY;
							}
						}
						else if (attackType == 2) {

							lasers[currentAttackIndex].x = startX;
							lasers[currentAttackIndex].y = startY;

							lasers[currentAttackIndex].velocityX = directionX;
							lasers[currentAttackIndex].velocityY = directionY;

							lasers[currentAttackIndex].length = 0.0f;
							lasers[currentAttackIndex].maxLength = 2000.0f;
							lasers[currentAttackIndex].timer = 100;
							lasers[currentAttackIndex].isShot = 1;
							lasers[currentAttackIndex].sparkMade = 0;
							lasers[currentAttackIndex].shakeMade = 0;
						}

						boxes[currentAttackIndex].isExist = 0;
					}

					currentAttackIndex++;
					attackTimer = 2;
				}
				else {

					for (int diamondClearIndex = 0;
						diamondClearIndex < kMaxBoxes;
						diamondClearIndex++) {

						diamonds[diamondClearIndex].isExist = 0;
					}
					dashCount = 0;
					boxCount = 0;
					isAttacking = 0;

					for (int clearLineIndex = 0; clearLineIndex < kMaxLines; clearLineIndex++) {
						lines[clearLineIndex].isExist = 0;
					}

					lineCount = 0;
				}
			}
		}

		// 火花パーティクル更新
		for (int sparkUpdateIndex = 0; sparkUpdateIndex < kMaxSparks; sparkUpdateIndex++) {

			if (sparks[sparkUpdateIndex].isExist) {

				sparks[sparkUpdateIndex].x += sparks[sparkUpdateIndex].velocityX;
				sparks[sparkUpdateIndex].y += sparks[sparkUpdateIndex].velocityY;

				sparks[sparkUpdateIndex].velocityX *= 0.94f;
				sparks[sparkUpdateIndex].velocityY *= 0.94f;

				sparks[sparkUpdateIndex].life--;

				if (sparks[sparkUpdateIndex].life <= 0) {
					sparks[sparkUpdateIndex].isExist = 0;
				}
			}
		}

		// 放射状ヒットエフェクトの更新
		for (int hitBurstUpdateIndex = 0;
			hitBurstUpdateIndex < kMaxHitBursts;
			hitBurstUpdateIndex++) {

			if (hitBursts[hitBurstUpdateIndex].isExist) {

				hitBursts[hitBurstUpdateIndex].radius += 14.0f;
				hitBursts[hitBurstUpdateIndex].life--;

				if (hitBursts[hitBurstUpdateIndex].life <= 0) {
					hitBursts[hitBurstUpdateIndex].isExist = 0;
				}
			}
		}

		// 爆弾の更新
		for (int bombUpdateIndex = 0; bombUpdateIndex < kMaxBombs; bombUpdateIndex++) {

			if (bombs[bombUpdateIndex].isExist) {

				if (!bombs[bombUpdateIndex].exploded) {
					// 爆弾を移動させる
					if (bombs[bombUpdateIndex].isFalling) {

						bombs[bombUpdateIndex].x += bombs[bombUpdateIndex].velocityX;
						bombs[bombUpdateIndex].y += bombs[bombUpdateIndex].velocityY;

						// 少しずつ下に落ちる
						bombs[bombUpdateIndex].velocityY += 0.12f;
					}

					bombs[bombUpdateIndex].timer--;

					if (bombs[bombUpdateIndex].timer <= 0) {

						bombs[bombUpdateIndex].exploded = 1;

						// 爆発に巻き込まれたらダメージ
						{
							float bombHitRadius = bombs[bombUpdateIndex].isGlitch ? kGlitchBombExplosionRadius : kBombExplosionRadius;
							float bombHitDx = playerX - bombs[bombUpdateIndex].x;
							float bombHitDy = playerY - bombs[bombUpdateIndex].y;

							if (sqrtf(bombHitDx * bombHitDx + bombHitDy * bombHitDy) <= bombHitRadius + playerRadius) {
								damagePlayer(1);
							}
						}

						if (bombs[bombUpdateIndex].isGlitch) {
							bombs[bombUpdateIndex].life = 12;
							screenShakeTimer = 4;

							// グリッチパーティクルを発生
							for (int p = 0; p < 50; p++) {

								for (int glitchSpawnIndex = 0;
									glitchSpawnIndex < kMaxGlitchParticles;
									glitchSpawnIndex++) {

									if (!glitchParticles[glitchSpawnIndex].isExist) {

										float angle = (float)(rand() % 628) / 100.0f;
										float particleSpeed = 2.0f + (float)(rand() % 90) / 10.0f;

										glitchParticles[glitchSpawnIndex].x = bombs[bombUpdateIndex].x;
										glitchParticles[glitchSpawnIndex].y = bombs[bombUpdateIndex].y;
										glitchParticles[glitchSpawnIndex].velocityX = cosf(angle) * particleSpeed;
										glitchParticles[glitchSpawnIndex].velocityY = sinf(angle) * particleSpeed;
										glitchParticles[glitchSpawnIndex].width = 7 + rand() % 14;
										glitchParticles[glitchSpawnIndex].height = 5 + rand() % 9;
										glitchParticles[glitchSpawnIndex].life = 8 + rand() % 9;

										int colorType = rand() % 4;
										if (colorType == 0) {
											glitchParticles[glitchSpawnIndex].color = 0xFF2255FF;
										}
										else if (colorType == 1) {
											glitchParticles[glitchSpawnIndex].color = 0xAA22FFFF;
										}
										else if (colorType == 2) {
											glitchParticles[glitchSpawnIndex].color = 0x00FFFFFF;
										}
										else {
											glitchParticles[glitchSpawnIndex].color = WHITE;
										}

										glitchParticles[glitchSpawnIndex].isExist = 1;
										break;
									}
								}
							}
						}
						else {
							bombs[bombUpdateIndex].life = 25;
							screenShakeTimer = 12;
						}
					}
				}
				else {

					bombs[bombUpdateIndex].life--;

					if (bombs[bombUpdateIndex].life <= 0) {
						bombs[bombUpdateIndex].isExist = 0;
					}
				}
			}
		}

		// グリッチパーティクルの更新
		for (int glitchUpdateIndex = 0;
			glitchUpdateIndex < kMaxGlitchParticles;
			glitchUpdateIndex++) {

			if (glitchParticles[glitchUpdateIndex].isExist) {

				glitchParticles[glitchUpdateIndex].x += glitchParticles[glitchUpdateIndex].velocityX;
				glitchParticles[glitchUpdateIndex].y += glitchParticles[glitchUpdateIndex].velocityY;

				glitchParticles[glitchUpdateIndex].velocityX *= 0.88f;
				glitchParticles[glitchUpdateIndex].velocityY *= 0.88f;

				glitchParticles[glitchUpdateIndex].life--;

				if (glitchParticles[glitchUpdateIndex].life <= 0) {
					glitchParticles[glitchUpdateIndex].isExist = 0;
				}
			}
		}

		// ボス爆発エフェクトの更新
		for (int explosionUpdateIndex = 0;
			explosionUpdateIndex < kMaxBossExplosions;
			explosionUpdateIndex++) {

			if (bossExplosions[explosionUpdateIndex].isExist) {

				// 爆発を大きくする
				bossExplosions[explosionUpdateIndex].radius += 7.0f;

				// 寿命を減らす
				bossExplosions[explosionUpdateIndex].life--;

				// 寿命がなくなったら消す
				if (bossExplosions[explosionUpdateIndex].life <= 0) {
					bossExplosions[explosionUpdateIndex].isExist = 0;
				}
			}
		}

		// ボス爆発パーティクルの更新
		for (int particleUpdateIndex = 0;
			particleUpdateIndex < kMaxBossExplosionParticles;
			particleUpdateIndex++) {

			if (bossExplosionParticles[particleUpdateIndex].isExist) {

				bossExplosionParticles[particleUpdateIndex].x +=
					bossExplosionParticles[particleUpdateIndex].velocityX;

				bossExplosionParticles[particleUpdateIndex].y +=
					bossExplosionParticles[particleUpdateIndex].velocityY;

				bossExplosionParticles[particleUpdateIndex].velocityX *= 0.94f;
				bossExplosionParticles[particleUpdateIndex].velocityY *= 0.94f;

				bossExplosionParticles[particleUpdateIndex].radius *= 0.94f;

				bossExplosionParticles[particleUpdateIndex].life--;

				if (bossExplosionParticles[particleUpdateIndex].life <= 0) {
					bossExplosionParticles[particleUpdateIndex].isExist = 0;
				}
			}
		}

		// 弾の移動・加速処理
		for (int beamUpdateIndex = 0; beamUpdateIndex < kMaxBoxes; beamUpdateIndex++) {

			if (beams[beamUpdateIndex].isShot) {

				for (int trailMoveIndex = 7; trailMoveIndex > 0; trailMoveIndex--) {
					beams[beamUpdateIndex].trailX[trailMoveIndex] = beams[beamUpdateIndex].trailX[trailMoveIndex - 1];
					beams[beamUpdateIndex].trailY[trailMoveIndex] = beams[beamUpdateIndex].trailY[trailMoveIndex - 1];
				}

				beams[beamUpdateIndex].trailX[0] = beams[beamUpdateIndex].x;
				beams[beamUpdateIndex].trailY[0] = beams[beamUpdateIndex].y;

				beams[beamUpdateIndex].speed += 0.1f + beams[beamUpdateIndex].speed * 0.2f;

				if (beams[beamUpdateIndex].speed > 35.0f) {
					beams[beamUpdateIndex].speed = 35.0f;
				}

				// 弾はボスを追いかける（ボスが動いても絶対に当たる）
				{
					float homingDx = bossX - beams[beamUpdateIndex].x;
					float homingDy = bossY - beams[beamUpdateIndex].y;
					float homingLength = sqrtf(homingDx * homingDx + homingDy * homingDy);

					if (homingLength > 0.001f) {

						// いまの向きに、ボスの向きを混ぜて、長さを1にそろえる
						float homingVx = beams[beamUpdateIndex].velocityX * (1.0f - kBeamHomingRate) + homingDx / homingLength * kBeamHomingRate;
						float homingVy = beams[beamUpdateIndex].velocityY * (1.0f - kBeamHomingRate) + homingDy / homingLength * kBeamHomingRate;
						float homingVLength = sqrtf(homingVx * homingVx + homingVy * homingVy);

						if (homingVLength > 0.001f) {
							beams[beamUpdateIndex].velocityX = homingVx / homingVLength;
							beams[beamUpdateIndex].velocityY = homingVy / homingVLength;
						}
						else {
							beams[beamUpdateIndex].velocityX = homingDx / homingLength;
							beams[beamUpdateIndex].velocityY = homingDy / homingLength;
						}
					}
				}

				beams[beamUpdateIndex].x += beams[beamUpdateIndex].velocityX * beams[beamUpdateIndex].speed;
				beams[beamUpdateIndex].y += beams[beamUpdateIndex].velocityY * beams[beamUpdateIndex].speed;

				float dx = bossX - beams[beamUpdateIndex].x;
				float dy = bossY - beams[beamUpdateIndex].y;

				// バリアがあるときは、バリアの外側で当たる
				float beamBarrierPad = (bossBarrier > 0) ? kBossBarrierExtraRadius : 0.0f;

				// ボスに弾が当たったら
				if (dx * dx + dy * dy <
					(bossRadius + 10.0f + beamBarrierPad) * (bossRadius + 10.0f + beamBarrierPad)) {

					beams[beamUpdateIndex].isShot = 0;

					// ボスのシールドを減らす
					// バリアに当たったら、バリアが見えるようになる（当たった向きが光る）
					if (bossBarrier > 0) {
						barrierHitTimer = kBarrierShowFrames;
					}

					damageBossShield(kBeamDamage, 1);   // プレイヤーの弾はバリアを壊せる

					screenShakeTimer = 8;
					Controller::SetVibration(60000, 60000);
					vibrationTimer = 3;

					// 弾が当たった場所（弾からボスの中心への線が、ボスの表面と交わるところ）
					float beamHitDistance = sqrtf(dx * dx + dy * dy);
					float beamHitRadius = bossRadius + beamBarrierPad;
					float beamHitX = beams[beamUpdateIndex].x;
					float beamHitY = beams[beamUpdateIndex].y;

					if (beamHitDistance > 0.001f) {
						beamHitX = bossX - dx / beamHitDistance * beamHitRadius;
						beamHitY = bossY - dy / beamHitDistance * beamHitRadius;
					}

					// 弾が当たった場所から火花を発生
					for (int p = 0; p < 25; p++) {

						for (int sparkSpawnIndex = 0; sparkSpawnIndex < kMaxSparks; sparkSpawnIndex++) {

							if (!sparks[sparkSpawnIndex].isExist) {



								sparks[sparkSpawnIndex].x = beamHitX;
								sparks[sparkSpawnIndex].y = beamHitY;

								// X・Y方向の速度をランダムに設定
								sparks[sparkSpawnIndex].velocityX =
									static_cast<float>(rand() % 160 - 80) / 10.0f;

								sparks[sparkSpawnIndex].velocityY =
									static_cast<float>(rand() % 160 - 80) / 10.0f;

								sparks[sparkSpawnIndex].life = 40;
								sparks[sparkSpawnIndex].isExist = 1;

								break;
							}
						}
					}
				}

				if (beams[beamUpdateIndex].x < -50 || beams[beamUpdateIndex].x > 1970 ||
					beams[beamUpdateIndex].y < -50 || beams[beamUpdateIndex].y > 1230) {
					beams[beamUpdateIndex].isShot = 0;
				}
			}
		}

		// レーザーの伸びる処理
		for (int laserUpdateIndex = 0; laserUpdateIndex < kMaxBoxes; laserUpdateIndex++) {

			if (lasers[laserUpdateIndex].isShot) {

				if (lasers[laserUpdateIndex].length < lasers[laserUpdateIndex].maxLength) {

					lasers[laserUpdateIndex].length += 60.0f;

					if (lasers[laserUpdateIndex].length > lasers[laserUpdateIndex].maxLength) {
						lasers[laserUpdateIndex].length = lasers[laserUpdateIndex].maxLength;
					}
				}
				else {

					lasers[laserUpdateIndex].timer++;

					if (lasers[laserUpdateIndex].timer >= 30) {
						lasers[laserUpdateIndex].isShot = 0;
					}
				}

				// レーザーがボスに当たったか判定
				float dx = bossX - lasers[laserUpdateIndex].x;
				float dy = bossY - lasers[laserUpdateIndex].y;

				// レーザーの進行方向にどれだけ進んでいるか
				float along =
					dx * lasers[laserUpdateIndex].velocityX +
					dy * lasers[laserUpdateIndex].velocityY;

				// レーザーの横方向にどれだけ離れているか
				float side =
					dx * (-lasers[laserUpdateIndex].velocityY) +
					dy * lasers[laserUpdateIndex].velocityX;

				// バリアがあるときは、バリアの外側で当たる
				float laserBarrierPad = (bossBarrier > 0) ? kBossBarrierExtraRadius : 0.0f;

				if (lasers[laserUpdateIndex].isShot &&
					along >= 0.0f &&
					along <= lasers[laserUpdateIndex].length + bossRadius + laserBarrierPad &&
					side * side <= (bossRadius + laserBarrierPad) * (bossRadius + laserBarrierPad) &&
					!lasers[laserUpdateIndex].sparkMade) {

					// バリアに当たったら、バリアが見えるようになる
					if (bossBarrier > 0) {
						barrierHitTimer = kBarrierShowFrames;
					}

					// ボスのシールドを減らす
					damageBossShield(kLaserDamage, 0);   // レーザーはバリアを壊せない

					screenShakeTimer = 8;
					Controller::SetVibration(60000, 60000);
					vibrationTimer = 3;

					// 放射状ヒットエフェクトを発生
					for (int hitBurstSpawnIndex = 0;
						hitBurstSpawnIndex < kMaxHitBursts;
						hitBurstSpawnIndex++) {

						if (!hitBursts[hitBurstSpawnIndex].isExist) {

							hitBursts[hitBurstSpawnIndex].x = bossX;
							hitBursts[hitBurstSpawnIndex].y = bossY;
							hitBursts[hitBurstSpawnIndex].radius = 5.0f;
							hitBursts[hitBurstSpawnIndex].life = 8;
							hitBursts[hitBurstSpawnIndex].isExist = 1;

							break;
						}
					}

					// 火花パーティクルを発生
					for (int p = 0; p < 25; p++) {

						for (int laserSparkIndex = 0;
							laserSparkIndex < kMaxSparks;
							laserSparkIndex++) {

							if (!sparks[laserSparkIndex].isExist) {

								sparks[laserSparkIndex].x = bossX;
								sparks[laserSparkIndex].y = bossY;

								sparks[laserSparkIndex].velocityX =
									static_cast<float>(rand() % 160 - 80) / 10.0f;

								sparks[laserSparkIndex].velocityY =
									static_cast<float>(rand() % 160 - 80) / 10.0f;

								sparks[laserSparkIndex].life = 40;
								sparks[laserSparkIndex].isExist = 1;
								break;
							}
						}
					}

					lasers[laserUpdateIndex].sparkMade = 1;
					lasers[laserUpdateIndex].shakeMade = 1;
				}
			}
		}

		// 画面外制限
		if (playerX - playerRadius < 0.0f) playerX = playerRadius;
		if (playerX + playerRadius > 1920.0f) playerX = 1920.0f - playerRadius;
		if (playerY - playerRadius < 0.0f) playerY = playerRadius;
		if (playerY + playerRadius > 1180.0f) playerY = 1180.0f - playerRadius;

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		Novice::DrawBox(0, 0, 1920, 1180, 0, BLACK, kFillModeSolid);

		// ダッシュ予測ガイド線
		if (!isAttacking && dashCount < kMaxDashes) {

			float targetX = playerX + cosf(playerAngle) * dashDistance;
			float targetY = playerY + sinf(playerAngle) * dashDistance;

			Novice::DrawLine(
				(int)playerX,
				(int)playerY,
				(int)targetX,
				(int)targetY,
				0xFFFFFF55
			);
		}

		// ダッシュ軌道線
		for (int lineDrawIndex = 0; lineDrawIndex < lineCount; lineDrawIndex++) {

			if (lines[lineDrawIndex].isExist) {

				Novice::DrawLine(
					(int)lines[lineDrawIndex].startX,
					(int)lines[lineDrawIndex].startY,
					(int)lines[lineDrawIndex].endX,
					(int)lines[lineDrawIndex].endY,
					0x00FFFFFF
				);
			}
		}

		// ひし形砲台の描画
		for (int diamondDrawIndex = 0;
			diamondDrawIndex < kMaxBoxes;
			diamondDrawIndex++) {

			if (diamonds[diamondDrawIndex].isExist &&
				boxes[diamondDrawIndex].isExist &&
				boxes[diamondDrawIndex].delay == 0) {

				float x = diamonds[diamondDrawIndex].x;
				float y = diamonds[diamondDrawIndex].y;
				float angle = diamonds[diamondDrawIndex].angle;
				float open = diamonds[diamondDrawIndex].openSize;

				float frontX = cosf(angle);
				float frontY = sinf(angle);

				float sideX = -sinf(angle);
				float sideY = cosf(angle);

				float size = 20.0f;

				// 先端
				int tipX = (int)(x + frontX * size);
				int tipY = (int)(y + frontY * size);

				// 上側
				int topX = (int)(x + sideX * (size - open));
				int topY = (int)(y + sideY * (size - open));

				// 下側
				int bottomX = (int)(x - sideX * (size - open));
				int bottomY = (int)(y - sideY * (size - open));

				// 後ろ
				int backX = (int)(x - frontX * size);
				int backY = (int)(y - frontY * size);

				// 上側の三角形
				Novice::DrawTriangle(
					tipX, tipY,
					topX, topY,
					backX, backY,
					boxes[diamondDrawIndex].color,
					kFillModeSolid
				);

				// 下側の三角形
				Novice::DrawTriangle(
					tipX, tipY,
					backX, backY,
					bottomX, bottomY,
					boxes[diamondDrawIndex].color,
					kFillModeSolid
				);
			}
		}

		// 丸い弾の描画
		for (int beamDrawIndex = 0; beamDrawIndex < kMaxBoxes; beamDrawIndex++) {

			if (beams[beamDrawIndex].isShot) {

				for (int trailDrawIndex = 7; trailDrawIndex >= 0; trailDrawIndex--) {

					int size = 3 + (7 - trailDrawIndex);

					// 円の直径に合わせてぼかしのサイズを決める
					float blurScale = (size * 2.0f) / 700.0f;

					Novice::DrawSprite(
						(int)(beams[beamDrawIndex].trailX[trailDrawIndex] - size),
						(int)(beams[beamDrawIndex].trailY[trailDrawIndex] - size),
						bokasiTexture,
						blurScale,
						blurScale,
						0.0f,
						0x33FFFFFF
					);

					// 残像本体
					Novice::DrawEllipse(
						(int)beams[beamDrawIndex].trailX[trailDrawIndex],
						(int)beams[beamDrawIndex].trailY[trailDrawIndex],
						size,
						size,
						0.0f,
						0x00FFFFFF,
						kFillModeSolid
					);

				}

				// 弾のぼかし
				Novice::DrawSprite(
					(int)(beams[beamDrawIndex].x - 21.0f),
					(int)(beams[beamDrawIndex].y - 21.0f),
					bokasiTexture,
					0.06f,
					0.06f,
					0.0f,
					0x33FFFFFF
				);

				// 弾の外側
				Novice::DrawEllipse(
					(int)beams[beamDrawIndex].x,
					(int)beams[beamDrawIndex].y,
					12,
					12,
					0.0f,
					0x00FFFFFF,
					kFillModeSolid
				);

				Novice::DrawEllipse(
					(int)beams[beamDrawIndex].x,
					(int)beams[beamDrawIndex].y,
					6,
					6,
					0.0f,
					WHITE,
					kFillModeSolid
				);
			}
		}

		// レーザー描画
		for (int laserDrawIndex = 0; laserDrawIndex < kMaxBoxes; laserDrawIndex++) {

			if (lasers[laserDrawIndex].isShot) {

				float endX = lasers[laserDrawIndex].x + lasers[laserDrawIndex].velocityX * lasers[laserDrawIndex].length;
				float endY = lasers[laserDrawIndex].y + lasers[laserDrawIndex].velocityY * lasers[laserDrawIndex].length;

				float sideX = -lasers[laserDrawIndex].velocityY;
				float sideY = lasers[laserDrawIndex].velocityX;

				float outerWidth = 14.0f;

				// レーザーの光（ぼかし画像 2400x100 をレーザーの向きに合わせて伸ばして描く）
				float laserGlowHalfWidth = 20.0f;   // 光の太さの半分
				float laserGlowPad = 30.0f;         // レーザーの前後にはみ出す長さ

				float glowStartX = lasers[laserDrawIndex].x - lasers[laserDrawIndex].velocityX * laserGlowPad;
				float glowStartY = lasers[laserDrawIndex].y - lasers[laserDrawIndex].velocityY * laserGlowPad;
				float glowEndX = endX + lasers[laserDrawIndex].velocityX * laserGlowPad;
				float glowEndY = endY + lasers[laserDrawIndex].velocityY * laserGlowPad;

				Novice::DrawQuad(
					(int)(glowStartX - sideX * laserGlowHalfWidth), (int)(glowStartY - sideY * laserGlowHalfWidth),  // 左上
					(int)(glowEndX - sideX * laserGlowHalfWidth), (int)(glowEndY - sideY * laserGlowHalfWidth),      // 右上
					(int)(glowStartX + sideX * laserGlowHalfWidth), (int)(glowStartY + sideY * laserGlowHalfWidth),  // 左下
					(int)(glowEndX + sideX * laserGlowHalfWidth), (int)(glowEndY + sideY * laserGlowHalfWidth),      // 右下
					0, 0, 2400, 100,
					bokasi2Texture,
					0x33FFFFFF
				);

				int outerX1 = (int)(lasers[laserDrawIndex].x + sideX * outerWidth);
				int outerY1 = (int)(lasers[laserDrawIndex].y + sideY * outerWidth);

				int outerX2 = (int)(lasers[laserDrawIndex].x - sideX * outerWidth);
				int outerY2 = (int)(lasers[laserDrawIndex].y - sideY * outerWidth);

				int outerX3 = (int)(endX + sideX * outerWidth);
				int outerY3 = (int)(endY + sideY * outerWidth);

				int outerX4 = (int)(endX - sideX * outerWidth);
				int outerY4 = (int)(endY - sideY * outerWidth);

				Novice::DrawTriangle(
					outerX1, outerY1,
					outerX2, outerY2,
					outerX3, outerY3,
					0x00FFFFFF,
					kFillModeSolid
				);

				Novice::DrawTriangle(
					outerX2, outerY2,
					outerX3, outerY3,
					outerX4, outerY4,
					0x00FFFFFF,
					kFillModeSolid
				);

				float innerWidth = 5.0f;

				int innerX1 = (int)(lasers[laserDrawIndex].x + sideX * innerWidth);
				int innerY1 = (int)(lasers[laserDrawIndex].y + sideY * innerWidth);

				int innerX2 = (int)(lasers[laserDrawIndex].x - sideX * innerWidth);
				int innerY2 = (int)(lasers[laserDrawIndex].y - sideY * innerWidth);

				int innerX3 = (int)(endX + sideX * innerWidth);
				int innerY3 = (int)(endY + sideY * innerWidth);

				int innerX4 = (int)(endX - sideX * innerWidth);
				int innerY4 = (int)(endY - sideY * innerWidth);

				Novice::DrawTriangle(
					innerX1, innerY1,
					innerX2, innerY2,
					innerX3, innerY3,
					WHITE,
					kFillModeSolid
				);

				Novice::DrawTriangle(
					innerX2, innerY2,
					innerX3, innerY3,
					innerX4, innerY4,
					WHITE,
					kFillModeSolid
				);
			}
		}

		// 第3形態のバリア（弾が当たったときだけ見える）
		if (bossBarrier > 0 && barrierHitTimer > 0) {

			float barrierRatio = (float)bossBarrier / (float)kBossBarrierMax;
			float barrierShow = (float)barrierHitTimer / (float)kBarrierShowFrames;   // 1から0へ、だんだん消えていく
			float barrierDrawRadius = bossRadius + kBossBarrierExtraRadius;

			// 残りが少ないほど薄く、当たってから時間がたつほど薄くなる
			unsigned int barrierFillAlpha = (unsigned int)((0x20 + 0x40 * barrierRatio) * barrierShow);
			unsigned int barrierLineAlpha = (unsigned int)((0x70 + 0x8F * barrierRatio) * barrierShow);

			Novice::DrawEllipse(
				(int)bossX + screenShakeX,
				(int)bossY + screenShakeY,
				(int)barrierDrawRadius,
				(int)barrierDrawRadius,
				0.0f,
				0x66CCFF00 | barrierFillAlpha,
				kFillModeSolid
			);

			Novice::DrawEllipse(
				(int)bossX + screenShakeX,
				(int)bossY + screenShakeY,
				(int)barrierDrawRadius,
				(int)barrierDrawRadius,
				0.0f,
				0xCCEEFF00 | barrierLineAlpha,
				kFillModeWireFrame
			);

			Novice::DrawEllipse(
				(int)bossX + screenShakeX,
				(int)bossY + screenShakeY,
				(int)(barrierDrawRadius - 10.0f),
				(int)(barrierDrawRadius - 10.0f),
				0.0f,
				0x66CCFF00 | (unsigned int)(0x60 * barrierShow),
				kFillModeWireFrame
			);

		}

		// バリアが割れたときの演出
		if (bossBarrierBreakTimer > 0) {

			float breakAge = (float)(30 - bossBarrierBreakTimer);
			float breakRate = (float)bossBarrierBreakTimer / 30.0f;
			float breakBaseRadius = bossRadius + kBossBarrierExtraRadius;

			// 割れた瞬間に、バリア全体が白く光る
			if (bossBarrierBreakTimer > 24) {

				Novice::DrawEllipse(
					(int)bossX + screenShakeX,
					(int)bossY + screenShakeY,
					(int)breakBaseRadius,
					(int)breakBaseRadius,
					0.0f,
					0xFFFFFF00 | (unsigned int)(0xC0 * (float)(bossBarrierBreakTimer - 24) / 6.0f),
					kFillModeSolid
				);
			}

			// 外へ広がる輪を2つ（少しずらして出す）
			for (int breakRingIndex = 0; breakRingIndex < 2; breakRingIndex++) {

				float breakRingRadius = breakBaseRadius + (breakAge - (float)breakRingIndex * 4.0f) * 14.0f;

				if (breakRingRadius > breakBaseRadius) {

					Novice::DrawEllipse(
						(int)bossX + screenShakeX,
						(int)bossY + screenShakeY,
						(int)breakRingRadius,
						(int)breakRingRadius,
						0.0f,
						0xCCEEFF00 | (unsigned int)(0xFF * breakRate),
						kFillModeWireFrame
					);
				}
			}
		}

		// バリアの破片（回りながら外へ飛んで、だんだん消える）
		for (int shardDrawIndex = 0; shardDrawIndex < kMaxBarrierShards; shardDrawIndex++) {

			if (!barrierShards[shardDrawIndex].isExist) {
				continue;
			}

			float shardRate = (float)barrierShards[shardDrawIndex].life / (float)kBarrierShardLife;
			float shardSize = barrierShards[shardDrawIndex].size * (0.5f + 0.5f * shardRate);
			float shardAngle = barrierShards[shardDrawIndex].angle;

			float shardX = barrierShards[shardDrawIndex].x + (float)screenShakeX;
			float shardY = barrierShards[shardDrawIndex].y + (float)screenShakeY;

			// 形がいびつな三角形にする
			int shardX1 = (int)(shardX + cosf(shardAngle) * shardSize);
			int shardY1 = (int)(shardY + sinf(shardAngle) * shardSize);
			int shardX2 = (int)(shardX + cosf(shardAngle + 2.3f) * shardSize * 0.6f);
			int shardY2 = (int)(shardY + sinf(shardAngle + 2.3f) * shardSize * 0.6f);
			int shardX3 = (int)(shardX + cosf(shardAngle + 4.0f) * shardSize * 0.8f);
			int shardY3 = (int)(shardY + sinf(shardAngle + 4.0f) * shardSize * 0.8f);

			unsigned int shardAlpha = (unsigned int)(0xFF * shardRate);

			Novice::DrawTriangle(shardX1, shardY1, shardX2, shardY2, shardX3, shardY3, 0x88DDFF00 | shardAlpha, kFillModeSolid);
			Novice::DrawTriangle(shardX1, shardY1, shardX2, shardY2, shardX3, shardY3, 0xFFFFFF00 | shardAlpha, kFillModeWireFrame);
		}

		// 無敵中はボスのまわりに青い輪を出す
		if (bossIsInvincible) {

			Novice::DrawEllipse(
				(int)bossX + screenShakeX,
				(int)bossY + screenShakeY,
				(int)(bossRadius + 25.0f),
				(int)(bossRadius + 25.0f),
				0.0f,
				0x66CCFFFF,
				kFillModeWireFrame
			);
		}

		// 大技の描画に使う関数
		// 太いレーザーを描く（ボスから、向き beamAngle に伸びる）
		auto drawBossBeam = [&](float beamAngle, float beamHalfWidth) {

			float beamDirX = cosf(beamAngle);
			float beamDirY = sinf(beamAngle);
			float beamSideX = -beamDirY;
			float beamSideY = beamDirX;

			float beamStartX = bossX + (float)screenShakeX;
			float beamStartY = bossY + (float)screenShakeY;
			float beamEndX = beamStartX + beamDirX * kRotLaserLength;
			float beamEndY = beamStartY + beamDirY * kRotLaserLength;

			// 後ろのぼかし
			float beamGlow = beamHalfWidth * 1.8f;

			Novice::DrawQuad(
				(int)(beamStartX - beamSideX * beamGlow), (int)(beamStartY - beamSideY * beamGlow),
				(int)(beamEndX - beamSideX * beamGlow), (int)(beamEndY - beamSideY * beamGlow),
				(int)(beamStartX + beamSideX * beamGlow), (int)(beamStartY + beamSideY * beamGlow),
				(int)(beamEndX + beamSideX * beamGlow), (int)(beamEndY + beamSideY * beamGlow),
				0, 0, 2400, 100,
				bokasi2Texture,
				0xFF3333AA
			);

			// 外側の赤い部分と、中心の明るい部分
			for (int beamLayer = 0; beamLayer < 2; beamLayer++) {

				float beamLayerWidth = (beamLayer == 0) ? beamHalfWidth : beamHalfWidth * 0.4f;
				unsigned int beamLayerColor = (beamLayer == 0) ? 0xFF0000CC : 0xFF9999FF;

				Novice::DrawTriangle(
					(int)(beamStartX + beamSideX * beamLayerWidth), (int)(beamStartY + beamSideY * beamLayerWidth),
					(int)(beamEndX + beamSideX * beamLayerWidth), (int)(beamEndY + beamSideY * beamLayerWidth),
					(int)(beamEndX - beamSideX * beamLayerWidth), (int)(beamEndY - beamSideY * beamLayerWidth),
					beamLayerColor,
					kFillModeSolid
				);

				Novice::DrawTriangle(
					(int)(beamStartX + beamSideX * beamLayerWidth), (int)(beamStartY + beamSideY * beamLayerWidth),
					(int)(beamEndX - beamSideX * beamLayerWidth), (int)(beamEndY - beamSideY * beamLayerWidth),
					(int)(beamStartX - beamSideX * beamLayerWidth), (int)(beamStartY - beamSideY * beamLayerWidth),
					beamLayerColor,
					kFillModeSolid
				);
			}
			};

		// 線分の形のレーザーを描く（反射レーザー用。点 (x0,y0) から (x1,y1) まで）
		auto drawBeamSegment = [&](float x0, float y0, float x1, float y1, float segHalfWidth) {

			float segDx = x1 - x0;
			float segDy = y1 - y0;
			float segLength = sqrtf(segDx * segDx + segDy * segDy);

			if (segLength <= 0.0f || segHalfWidth <= 0.0f) {
				return;
			}

			float segSideX = -segDy / segLength;
			float segSideY = segDx / segLength;

			float segStartX = x0 + (float)screenShakeX;
			float segStartY = y0 + (float)screenShakeY;
			float segEndX = x1 + (float)screenShakeX;
			float segEndY = y1 + (float)screenShakeY;

			// 後ろのぼかし
			float segGlow = segHalfWidth * 1.8f;

			Novice::DrawQuad(
				(int)(segStartX - segSideX * segGlow), (int)(segStartY - segSideY * segGlow),
				(int)(segEndX - segSideX * segGlow), (int)(segEndY - segSideY * segGlow),
				(int)(segStartX + segSideX * segGlow), (int)(segStartY + segSideY * segGlow),
				(int)(segEndX + segSideX * segGlow), (int)(segEndY + segSideY * segGlow),
				0, 0, 2400, 100,
				bokasi2Texture,
				0xFF3333AA
			);

			// 外側の赤い部分と、中心の明るい部分
			for (int segLayer = 0; segLayer < 2; segLayer++) {

				float segLayerWidth = (segLayer == 0) ? segHalfWidth : segHalfWidth * 0.4f;
				unsigned int segLayerColor = (segLayer == 0) ? 0xFF0000CC : 0xFF9999FF;

				Novice::DrawTriangle(
					(int)(segStartX + segSideX * segLayerWidth), (int)(segStartY + segSideY * segLayerWidth),
					(int)(segEndX + segSideX * segLayerWidth), (int)(segEndY + segSideY * segLayerWidth),
					(int)(segEndX - segSideX * segLayerWidth), (int)(segEndY - segSideY * segLayerWidth),
					segLayerColor,
					kFillModeSolid
				);

				Novice::DrawTriangle(
					(int)(segStartX + segSideX * segLayerWidth), (int)(segStartY + segSideY * segLayerWidth),
					(int)(segEndX - segSideX * segLayerWidth), (int)(segEndY - segSideY * segLayerWidth),
					(int)(segStartX - segSideX * segLayerWidth), (int)(segStartY - segSideY * segLayerWidth),
					segLayerColor,
					kFillModeSolid
				);
			}
			};

		// 予告線を描く（レーザーが通る太さの範囲を細い線で見せる）
		auto drawBossWarningLine = [&](float beamAngle, float beamHalfWidth) {

			float warnDirX = cosf(beamAngle);
			float warnDirY = sinf(beamAngle);
			float warnSideX = -warnDirY;
			float warnSideY = warnDirX;

			float warnStartX = bossX + (float)screenShakeX;
			float warnStartY = bossY + (float)screenShakeY;
			float warnEndX = warnStartX + warnDirX * kRotLaserLength;
			float warnEndY = warnStartY + warnDirY * kRotLaserLength;

			Novice::DrawLine((int)warnStartX, (int)warnStartY, (int)warnEndX, (int)warnEndY, 0xFF0000FF);

			for (int warnSign = -1; warnSign <= 1; warnSign += 2) {

				Novice::DrawLine(
					(int)(warnStartX + warnSideX * beamHalfWidth * (float)warnSign), (int)(warnStartY + warnSideY * beamHalfWidth * (float)warnSign),
					(int)(warnEndX + warnSideX * beamHalfWidth * (float)warnSign), (int)(warnEndY + warnSideY * beamHalfWidth * (float)warnSign),
					0xFF000088
				);
			}
			};

		// 予告線を線分1本ぶん描く（反射レーザー用。跳ね返るたびに少しずつ薄くする）
		auto drawWarningSegment = [&](float x0, float y0, float x1, float y1, float beamHalfWidth, int bounceIndex) {

			float warnSegDx = x1 - x0;
			float warnSegDy = y1 - y0;
			float warnSegLength = sqrtf(warnSegDx * warnSegDx + warnSegDy * warnSegDy);

			if (warnSegLength <= 0.0f) {
				return;
			}

			float warnSegSideX = -warnSegDy / warnSegLength;
			float warnSegSideY = warnSegDx / warnSegLength;

			float warnSegStartX = x0 + (float)screenShakeX;
			float warnSegStartY = y0 + (float)screenShakeY;
			float warnSegEndX = x1 + (float)screenShakeX;
			float warnSegEndY = y1 + (float)screenShakeY;

			// 真ん中の線と、レーザーの太さを示す両脇の線（跳ね返るほど薄い）
			unsigned int warnCenterAlpha = 0xFF - (unsigned int)bounceIndex * 0x22;
			unsigned int warnSideAlpha = 0x88 - (unsigned int)bounceIndex * 0x14;

			Novice::DrawLine((int)warnSegStartX, (int)warnSegStartY, (int)warnSegEndX, (int)warnSegEndY, 0xFF000000 | warnCenterAlpha);

			for (int warnSegSign = -1; warnSegSign <= 1; warnSegSign += 2) {

				Novice::DrawLine(
					(int)(warnSegStartX + warnSegSideX * beamHalfWidth * (float)warnSegSign), (int)(warnSegStartY + warnSegSideY * beamHalfWidth * (float)warnSegSign),
					(int)(warnSegEndX + warnSegSideX * beamHalfWidth * (float)warnSegSign), (int)(warnSegEndY + warnSegSideY * beamHalfWidth * (float)warnSegSign),
					0xFF000000 | warnSideAlpha
				);
			}
			};

		// 溜めの演出（まわりから光の粒が集まって、中心の光の球が大きくなる）
		auto drawBossCharge = [&](int chargeTimer, int chargeFrames, unsigned int dotColor) {

			float chargeRate = (float)chargeTimer / (float)chargeFrames;

			for (int chargeDotIndex = 0; chargeDotIndex < 16; chargeDotIndex++) {

				float dotAngle = 6.2831853f * (float)chargeDotIndex / 16.0f + (float)chargeTimer * 0.12f;
				float dotDistance = (1.0f - chargeRate) * (320.0f + (float)(chargeDotIndex % 3) * 40.0f) + 40.0f;

				Novice::DrawEllipse(
					(int)(bossX + cosf(dotAngle) * dotDistance) + screenShakeX,
					(int)(bossY + sinf(dotAngle) * dotDistance) + screenShakeY,
					6,
					6,
					0.0f,
					dotColor,
					kFillModeSolid
				);
			}

			float chargeOrb = 15.0f + chargeRate * 100.0f + sinf((float)chargeTimer * 0.6f) * 6.0f * chargeRate;

			Novice::DrawEllipse((int)bossX + screenShakeX, (int)bossY + screenShakeY, (int)chargeOrb, (int)chargeOrb, 0.0f, 0xFF0000AA, kFillModeSolid);
			Novice::DrawEllipse((int)bossX + screenShakeX, (int)bossY + screenShakeY, (int)(chargeOrb * 0.5f), (int)(chargeOrb * 0.5f), 0.0f, 0xFFCCCCFF, kFillModeSolid);
			};

		// 溜め技（超極太レーザー）の描画
		if (megaLaserState == 1) {

			// 溜め：まわりから光の粒が集まって、四角の真ん中の小さな丸がだんだん大きくなる
			float megaChargeRate = (float)megaLaserTimer / (float)kMegaChargeFrames;

			for (int megaDotIndex = 0; megaDotIndex < 16; megaDotIndex++) {

				float megaDotAngle = 6.2831853f * (float)megaDotIndex / 16.0f + (float)megaLaserTimer * 0.12f;
				float megaDotDistance = (1.0f - megaChargeRate) * (320.0f + (float)(megaDotIndex % 3) * 40.0f) + 40.0f;

				Novice::DrawEllipse(
					(int)(megaLaserX + cosf(megaDotAngle) * megaDotDistance) + screenShakeX,
					(int)(megaLaserY + sinf(megaDotAngle) * megaDotDistance) + screenShakeY,
					6,
					6,
					0.0f,
					0x66CCFFFF,
					kFillModeSolid
				);
			}

			float megaOrb = 8.0f + megaChargeRate * 110.0f + sinf((float)megaLaserTimer * 0.6f) * 6.0f * megaChargeRate;

			Novice::DrawEllipse((int)megaLaserX + screenShakeX, (int)megaLaserY + screenShakeY, (int)megaOrb, (int)megaOrb, 0.0f, 0x3399FFAA, kFillModeSolid);
			Novice::DrawEllipse((int)megaLaserX + screenShakeX, (int)megaLaserY + screenShakeY, (int)(megaOrb * 0.5f), (int)(megaOrb * 0.5f), 0.0f, 0xFFFFFFFF, kFillModeSolid);
		}
		else if (megaLaserState == 2) {

			// 出るときは8フレームで太くなり、消えるときは15フレームで細くなる
			float megaDrawGrow = (float)megaLaserTimer / 8.0f;
			float megaDrawFade = (float)(kMegaFireFrames - megaLaserTimer) / 15.0f;

			if (megaDrawGrow > 1.0f) {
				megaDrawGrow = 1.0f;
			}

			if (megaDrawFade < megaDrawGrow) {
				megaDrawGrow = megaDrawFade;
			}

			if (megaDrawGrow > 0.0f) {

				// 太さを少しだけ脈打たせる
				float megaHalfWidth = kMegaLaserHalfWidth * megaDrawGrow * (1.0f + sinf((float)megaLaserTimer * 0.8f) * 0.04f);

				float megaDrawDirX = cosf(megaLaserAngle);
				float megaDrawDirY = sinf(megaLaserAngle);
				float megaSideX = -megaDrawDirY;
				float megaSideY = megaDrawDirX;

				float megaStartX = megaLaserX + (float)screenShakeX;
				float megaStartY = megaLaserY + (float)screenShakeY;
				float megaEndX = megaStartX + megaDrawDirX * kMegaLaserLength;
				float megaEndY = megaStartY + megaDrawDirY * kMegaLaserLength;

				// 後ろのぼかし
				float megaGlow = megaHalfWidth * 1.8f;

				Novice::DrawQuad(
					(int)(megaStartX - megaSideX * megaGlow), (int)(megaStartY - megaSideY * megaGlow),
					(int)(megaEndX - megaSideX * megaGlow), (int)(megaEndY - megaSideY * megaGlow),
					(int)(megaStartX + megaSideX * megaGlow), (int)(megaStartY + megaSideY * megaGlow),
					(int)(megaEndX + megaSideX * megaGlow), (int)(megaEndY + megaSideY * megaGlow),
					0, 0, 2400, 100,
					bokasi2Texture,
					0x66CCFF99
				);

				// 外側の水色の部分と、中心の白い部分
				for (int megaLayer = 0; megaLayer < 2; megaLayer++) {

					float megaLayerWidth = (megaLayer == 0) ? megaHalfWidth : megaHalfWidth * 0.45f;
					unsigned int megaLayerColor = (megaLayer == 0) ? 0x66CCFFCC : 0xFFFFFFFF;

					Novice::DrawTriangle(
						(int)(megaStartX + megaSideX * megaLayerWidth), (int)(megaStartY + megaSideY * megaLayerWidth),
						(int)(megaEndX + megaSideX * megaLayerWidth), (int)(megaEndY + megaSideY * megaLayerWidth),
						(int)(megaEndX - megaSideX * megaLayerWidth), (int)(megaEndY - megaSideY * megaLayerWidth),
						megaLayerColor,
						kFillModeSolid
					);

					Novice::DrawTriangle(
						(int)(megaStartX + megaSideX * megaLayerWidth), (int)(megaStartY + megaSideY * megaLayerWidth),
						(int)(megaEndX - megaSideX * megaLayerWidth), (int)(megaEndY - megaSideY * megaLayerWidth),
						(int)(megaStartX - megaSideX * megaLayerWidth), (int)(megaStartY - megaSideY * megaLayerWidth),
						megaLayerColor,
						kFillModeSolid
					);
				}

				// レーザーの出る場所の光の球
				Novice::DrawEllipse((int)megaStartX, (int)megaStartY, (int)(megaHalfWidth * 1.1f), (int)(megaHalfWidth * 1.1f), 0.0f, 0x66CCFFCC, kFillModeSolid);
				Novice::DrawEllipse((int)megaStartX, (int)megaStartY, (int)(megaHalfWidth * 0.6f), (int)(megaHalfWidth * 0.6f), 0.0f, 0xFFFFFFFF, kFillModeSolid);
			}
		}

		// 十字レーザーの描画
		if (bossState == 7) {

			if (bossBigTimer <= kBigChargeFrames) {

				drawBossCharge(bossBigTimer, kBigChargeFrames, 0xFF4444FF);
			}
			else {

				int crossTime = bossBigTimer - kBigChargeFrames - 1;
				int crossWave = crossTime / (kCrossWarnFrames + kCrossFireFrames);
				int crossInWave = crossTime % (kCrossWarnFrames + kCrossFireFrames);

				if (crossWave < 3) {

					int crossBeamCount = (crossWave == 2) ? 8 : 4;

					for (int crossDrawIndex = 0; crossDrawIndex < crossBeamCount; crossDrawIndex++) {

						float crossAngle = crossBeamAngle(crossWave, crossDrawIndex);

						if (crossInWave < kCrossWarnFrames) {

							// 予告（最後の15フレームは点滅をやめて出しっぱなし）
							if ((crossInWave / 6) % 2 == 0 || crossInWave > kCrossWarnFrames - 15) {
								drawBossWarningLine(crossAngle, kCrossLaserWidth * 0.5f);
							}
						}
						else {

							// 出るときは4フレームで太くなり、消えるときは8フレームで細くなる
							float crossGrow = (float)(crossInWave - kCrossWarnFrames + 1) / 4.0f;
							float crossFade = (float)(kCrossWarnFrames + kCrossFireFrames - crossInWave) / 8.0f;

							if (crossGrow > 1.0f) {
								crossGrow = 1.0f;
							}

							if (crossFade < crossGrow) {
								crossGrow = crossFade;
							}

							drawBossBeam(crossAngle, kCrossLaserWidth * 0.5f * crossGrow);
						}
					}
				}
			}
		}

		// バリア爆発の描画
		if (bossState == 8) {

			// 溜め：バリアが光ってふくらむ
			if (bossBigTimer <= kBigChargeFrames) {

				float blastChargeRate = (float)bossBigTimer / (float)kBigChargeFrames;
				float blastSwell = bossRadius + kBossBarrierExtraRadius + blastChargeRate * 90.0f;
				unsigned int blastFlash = ((bossBigTimer / 5) % 2 == 0) ? 0xA0 : 0x50;

				drawBossCharge(bossBigTimer, kBigChargeFrames, 0x66CCFFFF);

				Novice::DrawEllipse((int)bossX + screenShakeX, (int)bossY + screenShakeY, (int)blastSwell, (int)blastSwell, 0.0f, 0x66CCFF00 | blastFlash, kFillModeSolid);
				Novice::DrawEllipse((int)bossX + screenShakeX, (int)bossY + screenShakeY, (int)blastSwell, (int)blastSwell, 0.0f, 0xCCEEFFFF, kFillModeWireFrame);
			}

			// 広がっていく輪（すき間の向きだけ輪が切れている）
			for (int blastDrawIndex = 0; blastDrawIndex < kBlastRings; blastDrawIndex++) {

				int blastDrawAge = bossBigTimer - (kBigChargeFrames + blastDrawIndex * kBlastRingInterval);

				if (blastDrawAge > 0) {

					float blastDrawRadius = 60.0f + (float)blastDrawAge * kBlastRingSpeed;

					if (blastDrawRadius < 1700.0f) {

						for (int ringSegment = 0; ringSegment < 72; ringSegment++) {

							float segAngle0 = 6.2831853f * (float)ringSegment / 72.0f;
							float segAngle1 = 6.2831853f * (float)(ringSegment + 1) / 72.0f;

							float segDiff = (segAngle0 + segAngle1) * 0.5f - blastGapAngle[blastDrawIndex];

							while (segDiff > 3.14159265f) {
								segDiff -= 6.2831853f;
							}

							while (segDiff < -3.14159265f) {
								segDiff += 6.2831853f;
							}

							// すき間の部分は描かない
							if (fabsf(segDiff) <= kBlastGapHalf) {
								continue;
							}

							for (int ringLayer = -1; ringLayer <= 1; ringLayer++) {

								float layerRadius = blastDrawRadius + (float)ringLayer * 10.0f;

								Novice::DrawLine(
									(int)(bossX + cosf(segAngle0) * layerRadius) + screenShakeX,
									(int)(bossY + sinf(segAngle0) * layerRadius) + screenShakeY,
									(int)(bossX + cosf(segAngle1) * layerRadius) + screenShakeX,
									(int)(bossY + sinf(segAngle1) * layerRadius) + screenShakeY,
									(ringLayer == 0) ? 0xFFFFFFFF : 0x66CCFFAA
								);
							}
						}
					}
				}
			}
		}

		// 全画面ビームの描画
		if (bossState == 9) {

			float fullSafeX = (fullBeamSafeSide == 0) ? 0.0f : 1920.0f - kFullBeamSafeW;
			float fullSafeY = 1180.0f - kFullBeamSafeH;

			if (bossBigTimer <= kFullBeamChargeFrames) {
				drawBossCharge(bossBigTimer, kFullBeamChargeFrames, 0xFF4444FF);
			}

			// 溜めの途中から、安全地帯以外が赤くなっていく
			if (bossBigTimer > 30) {

				unsigned int fullAlpha = 0x10;

				if (bossBigTimer <= kFullBeamChargeFrames) {
					fullAlpha = 0x10 + (unsigned int)((bossBigTimer - 30) * 0x40 / (kFullBeamChargeFrames - 30));
				}
				else if (bossBigTimer <= kFullBeamChargeFrames + kFullBeamFireFrames) {
					fullAlpha = (bossBigTimer % 2 == 0) ? 0xC0 : 0xE0;
				}
				else {
					fullAlpha = 0xC0 - (unsigned int)((bossBigTimer - kFullBeamChargeFrames - kFullBeamFireFrames) * 0xC0 / 30);
				}

				// 上の部分
				Novice::DrawBox(0, 0, 1920, (int)fullSafeY, 0.0f, 0xFF000000 | fullAlpha, kFillModeSolid);

				// 安全地帯の横の部分
				Novice::DrawBox(
					(fullBeamSafeSide == 0) ? (int)kFullBeamSafeW : 0,
					(int)fullSafeY,
					1920 - (int)kFullBeamSafeW,
					(int)kFullBeamSafeH,
					0.0f,
					0xFF000000 | fullAlpha,
					kFillModeSolid
				);

				// 安全地帯の枠
				if (bossBigTimer <= kFullBeamChargeFrames + kFullBeamFireFrames) {

					Novice::DrawBox((int)fullSafeX, (int)fullSafeY, (int)kFullBeamSafeW, (int)kFullBeamSafeH, 0.0f, 0x66FF66FF, kFillModeWireFrame);
					Novice::ScreenPrintf((int)(fullSafeX + kFullBeamSafeW * 0.5f) - 20, (int)(fullSafeY + kFullBeamSafeH * 0.5f), "SAFE");
				}
			}

			// 発射中：ボスから全方向にビームが出る
			if (bossBigTimer > kFullBeamChargeFrames && bossBigTimer <= kFullBeamChargeFrames + kFullBeamFireFrames) {

				for (int fullBeamIndex = 0; fullBeamIndex < 8; fullBeamIndex++) {
					drawBossBeam(3.14159265f * 0.25f * (float)fullBeamIndex + 0.2f, 70.0f);
				}
			}
		}

		// 反射レーザーの描画
		if (bossState == 10) {

			if (bossBigTimer <= kBigChargeFrames) {

				drawBossCharge(bossBigTimer, kBigChargeFrames, 0xFF4444FF);
			}
			else {

				int reflectTime = bossBigTimer - kBigChargeFrames - 1;
				int reflectWave = reflectTime / (kReflectWarnFrames + kReflectFireFrames);
				int reflectInWave = reflectTime % (kReflectWarnFrames + kReflectFireFrames);

				if (reflectWave < kReflectWaves) {

					int reflectCount = reflectActiveCount;

					for (int reflectDrawIndex = 0; reflectDrawIndex < reflectCount; reflectDrawIndex++) {

						if (reflectInWave < kReflectWarnFrames) {

							// 予告線は、跳ね返りも含めた全部の道すじを出す
							// （最後の15フレームは点滅をやめて出しっぱなし）
							if (reflectInWave > 0 &&
								((reflectInWave / 6) % 2 == 0 || reflectInWave > kReflectWarnFrames - 15)) {

								float warnPathX[kReflectBounces + 2];
								float warnPathY[kReflectBounces + 2];

								calcReflectPath(reflectAngle[reflectDrawIndex], warnPathX, warnPathY);

								for (int warnPathSegIndex = 0; warnPathSegIndex <= kReflectBounces; warnPathSegIndex++) {

									drawWarningSegment(
										warnPathX[warnPathSegIndex], warnPathY[warnPathSegIndex],
										warnPathX[warnPathSegIndex + 1], warnPathY[warnPathSegIndex + 1],
										kReflectLaserWidth * 0.5f,
										warnPathSegIndex
									);
								}
							}
						}
						else {

							// 出るときは4フレームで太くなり、消えるときは8フレームで細くなる
							float reflectGrow = (float)(reflectInWave - kReflectWarnFrames + 1) / 4.0f;
							float reflectFade = (float)(kReflectWarnFrames + kReflectFireFrames - reflectInWave) / 8.0f;

							if (reflectGrow > 1.0f) {
								reflectGrow = 1.0f;
							}

							if (reflectFade < reflectGrow) {
								reflectGrow = reflectFade;
							}

							float drawPathX[kReflectBounces + 2];
							float drawPathY[kReflectBounces + 2];

							calcReflectPath(reflectAngle[reflectDrawIndex], drawPathX, drawPathY);

							for (int reflectDrawSeg = 0; reflectDrawSeg <= kReflectBounces; reflectDrawSeg++) {

								drawBeamSegment(
									drawPathX[reflectDrawSeg], drawPathY[reflectDrawSeg],
									drawPathX[reflectDrawSeg + 1], drawPathY[reflectDrawSeg + 1],
									kReflectLaserWidth * 0.5f * reflectGrow
								);
							}

							// 跳ね返る場所に光を出す
							for (int reflectFlashIndex = 1; reflectFlashIndex <= kReflectBounces; reflectFlashIndex++) {

								Novice::DrawEllipse(
									(int)drawPathX[reflectFlashIndex] + screenShakeX,
									(int)drawPathY[reflectFlashIndex] + screenShakeY,
									(int)(26.0f * reflectGrow),
									(int)(26.0f * reflectGrow),
									0.0f,
									0xFFCCCCFF,
									kFillModeSolid
								);
							}
						}
					}
				}
			}
		}

		// 回転レーザーの描画（赤い極太レーザー）
		if (bossState == 6) {

			// 溜め（レーザーを出す前に、ボスにエネルギーが集まる）
			if (bossRotLaserTimer <= kRotLaserWarningFrames) {

				// 0から1までの溜まり具合
				float chargeRate = (float)bossRotLaserTimer / (float)kRotLaserWarningFrames;

				// まわりから集まってくる光の粒（だんだん中心に寄ってくる）
				const int kChargeDots = 16;

				for (int chargeDotIndex = 0; chargeDotIndex < kChargeDots; chargeDotIndex++) {

					float dotAngle = 6.2831853f * (float)chargeDotIndex / (float)kChargeDots + (float)bossRotLaserTimer * 0.12f;
					float dotDistance = (1.0f - chargeRate) * (320.0f + (float)(chargeDotIndex % 3) * 40.0f) + 40.0f;

					Novice::DrawEllipse(
						(int)(bossX + cosf(dotAngle) * dotDistance) + screenShakeX,
						(int)(bossY + sinf(dotAngle) * dotDistance) + screenShakeY,
						6,
						6,
						0.0f,
						(chargeDotIndex % 2 == 0) ? 0xFF4444FF : 0xFFAAAAFF,
						kFillModeSolid
					);
				}

				// ボスの中心の光の球（だんだん大きく、脈打つ）
				float orbRadius = 15.0f + chargeRate * 100.0f + sinf((float)bossRotLaserTimer * 0.6f) * 6.0f * chargeRate;

				Novice::DrawEllipse(
					(int)bossX + screenShakeX,
					(int)bossY + screenShakeY,
					(int)orbRadius,
					(int)orbRadius,
					0.0f,
					0xFF0000AA,
					kFillModeSolid
				);

				Novice::DrawEllipse(
					(int)bossX + screenShakeX,
					(int)bossY + screenShakeY,
					(int)(orbRadius * 0.5f),
					(int)(orbRadius * 0.5f),
					0.0f,
					0xFFCCCCFF,
					kFillModeSolid
				);
			}

			for (int rotDrawIndex = 0; rotDrawIndex < 2; rotDrawIndex++) {

				float rotDrawAngle = bossRotLaserAngle + 3.14159265f * (float)rotDrawIndex;
				float rotDrawDirX = cosf(rotDrawAngle);
				float rotDrawDirY = sinf(rotDrawAngle);
				float rotSideX = -rotDrawDirY;
				float rotSideY = rotDrawDirX;

				float rotStartX = bossX + (float)screenShakeX;
				float rotStartY = bossY + (float)screenShakeY;
				float rotEndX = rotStartX + rotDrawDirX * kRotLaserLength;
				float rotEndY = rotStartY + rotDrawDirY * kRotLaserLength;

				if (bossRotLaserTimer <= kRotLaserWarningFrames) {

					// 予告線（点滅する。レーザーが通る太さの範囲を細い線で見せる）
					// if ((bossRotLaserTimer / 6) % 2 == 0) {
					if ((bossRotLaserTimer / 6) % 2 == 0 || bossRotLaserTimer > kRotLaserWarningFrames - 20) {

						float rotHalf = kRotLaserWidth * 0.5f;

						Novice::DrawLine((int)rotStartX, (int)rotStartY, (int)rotEndX, (int)rotEndY, 0xFF0000FF);
						Novice::DrawLine((int)(rotStartX + rotSideX * rotHalf), (int)(rotStartY + rotSideY * rotHalf),
							(int)(rotEndX + rotSideX * rotHalf), (int)(rotEndY + rotSideY * rotHalf), 0xFF000088);
						Novice::DrawLine((int)(rotStartX - rotSideX * rotHalf), (int)(rotStartY - rotSideY * rotHalf),
							(int)(rotEndX - rotSideX * rotHalf), (int)(rotEndY - rotSideY * rotHalf), 0xFF000088);
					}
				}
				else {

					// 外側の赤い太いレーザー
					// 発射したあと10フレームかけて太くなる
					float rotGrow = (float)(bossRotLaserTimer - kRotLaserWarningFrames) / 10.0f;

					if (rotGrow > 1.0f) {
						rotGrow = 1.0f;
					}

					float rotOuter = kRotLaserWidth * 0.5f * rotGrow;

					// レーザーの後ろのぼかし（プレイヤーのレーザーと同じ bokasi2Texture を、レーザーの向きに合わせて伸ばして描く）
					float rotGlowHalfWidth = rotOuter * 1.8f;   // 光の太さの半分
					float rotGlowPad = 30.0f;                   // レーザーの前後にはみ出す長さ

					float rotGlowStartX = rotStartX - rotDrawDirX * rotGlowPad;
					float rotGlowStartY = rotStartY - rotDrawDirY * rotGlowPad;
					float rotGlowEndX = rotEndX + rotDrawDirX * rotGlowPad;
					float rotGlowEndY = rotEndY + rotDrawDirY * rotGlowPad;

					Novice::DrawQuad(
						(int)(rotGlowStartX - rotSideX * rotGlowHalfWidth), (int)(rotGlowStartY - rotSideY * rotGlowHalfWidth),  // 左上
						(int)(rotGlowEndX - rotSideX * rotGlowHalfWidth), (int)(rotGlowEndY - rotSideY * rotGlowHalfWidth),      // 右上
						(int)(rotGlowStartX + rotSideX * rotGlowHalfWidth), (int)(rotGlowStartY + rotSideY * rotGlowHalfWidth),  // 左下
						(int)(rotGlowEndX + rotSideX * rotGlowHalfWidth), (int)(rotGlowEndY + rotSideY * rotGlowHalfWidth),      // 右下
						0, 0, 2400, 100,
						bokasi2Texture,
						0xFF3333AA
					);

					Novice::DrawTriangle(
						(int)(rotStartX + rotSideX * rotOuter), (int)(rotStartY + rotSideY * rotOuter),
						(int)(rotEndX + rotSideX * rotOuter), (int)(rotEndY + rotSideY * rotOuter),
						(int)(rotEndX - rotSideX * rotOuter), (int)(rotEndY - rotSideY * rotOuter),
						0xFF0000CC,
						kFillModeSolid
					);

					Novice::DrawTriangle(
						(int)(rotStartX + rotSideX * rotOuter), (int)(rotStartY + rotSideY * rotOuter),
						(int)(rotEndX - rotSideX * rotOuter), (int)(rotEndY - rotSideY * rotOuter),
						(int)(rotStartX - rotSideX * rotOuter), (int)(rotStartY - rotSideY * rotOuter),
						0xFF0000CC,
						kFillModeSolid
					);

					// 中心の明るい部分
					float rotInner = kRotLaserWidth * 0.2f * rotGrow;

					Novice::DrawTriangle(
						(int)(rotStartX + rotSideX * rotInner), (int)(rotStartY + rotSideY * rotInner),
						(int)(rotEndX + rotSideX * rotInner), (int)(rotEndY + rotSideY * rotInner),
						(int)(rotEndX - rotSideX * rotInner), (int)(rotEndY - rotSideY * rotInner),
						0xFF9999FF,
						kFillModeSolid
					);

					Novice::DrawTriangle(
						(int)(rotStartX + rotSideX * rotInner), (int)(rotStartY + rotSideY * rotInner),
						(int)(rotEndX - rotSideX * rotInner), (int)(rotEndY - rotSideY * rotInner),
						(int)(rotStartX - rotSideX * rotInner), (int)(rotStartY - rotSideY * rotInner),
						0xFF9999FF,
						kFillModeSolid
					);
				}
			}
		}

		// ボス
		// unsigned int bossColor = RED;
		unsigned int bossColor = (bossPhase == 3) ? 0xFF8800FF : (bossPhase == 2) ? 0xAA00FFFF : RED;

		if (bossState == 1 || bossState == 2) {
			bossColor = WHITE;
		}
		else if (bossState == 4) {
			bossColor = 0xFF6600FF;
		}

		// ボスの危険な赤いオーラ
		if (bossState == 1) {

			float pulse = sinf((float)bossAttackTimer * 0.5f) * 15.0f;

			// 外側の大きなオーラ
			Novice::DrawEllipse(
				(int)bossX + screenShakeX,
				(int)bossY + screenShakeY,
				(int)(bossAuraRadius + 25.0f + pulse),
				(int)(bossAuraRadius + 25.0f + pulse),
				0.0f,
				0xFF0000FF,
				kFillModeWireFrame
			);

			// 中間のオーラ
			Novice::DrawEllipse(
				(int)bossX + screenShakeX,
				(int)bossY + screenShakeY,
				(int)(bossAuraRadius + pulse),
				(int)(bossAuraRadius + pulse),
				0.0f,
				0xFF2222FF,
				kFillModeWireFrame
			);

			// 内側のオーラ
			Novice::DrawEllipse(
				(int)bossX + screenShakeX,
				(int)bossY + screenShakeY,
				(int)(bossAuraRadius * 0.65f),
				(int)(bossAuraRadius * 0.65f),
				0.0f,
				0xFF4444FF,
				kFillModeWireFrame
			);

			// 危険を知らせるトゲ状の光
			for (int auraLineIndex = 0; auraLineIndex < 16; auraLineIndex++) {

				float angle = (float)auraLineIndex * 3.14159f / 8.0f;

				float innerRadius = bossAuraRadius * 0.75f;
				float outerRadius = bossAuraRadius + 35.0f + pulse;

				float innerX = bossX + cosf(angle) * innerRadius;
				float innerY = bossY + sinf(angle) * innerRadius;

				float outerX = bossX + cosf(angle) * outerRadius;
				float outerY = bossY + sinf(angle) * outerRadius;

				Novice::DrawLine(
					(int)innerX + screenShakeX,
					(int)innerY + screenShakeY,
					(int)outerX + screenShakeX,
					(int)outerY + screenShakeY,
					0xFF1111FF
				);
			}
		}

		// 突進前の警告線
		if (bossState == 1) {

			Novice::DrawLine(
				(int)bossX + screenShakeX,
				(int)bossY + screenShakeY,
				(int)playerX + screenShakeX,
				(int)playerY + screenShakeY,
				0xFF2222FF
			);
		}

		// 爆弾の描画
		for (int bombDrawIndex = 0; bombDrawIndex < kMaxBombs; bombDrawIndex++) {

			if (bombs[bombDrawIndex].isExist) {

				float x = bombs[bombDrawIndex].x + screenShakeX;
				float y = bombs[bombDrawIndex].y + screenShakeY;

				if (!bombs[bombDrawIndex].exploded) {

					int flash = bombs[bombDrawIndex].timer / 5 % 2;

					// 爆弾の膨張率を計算
					float expandRatio = 0.0f;
					if (!bombs[bombDrawIndex].isGlitch) {
						expandRatio = (45.0f - (float)bombs[bombDrawIndex].timer) / 45.0f;
					}
					if (expandRatio < 0.0f) expandRatio = 0.0f;
					if (expandRatio > 1.0f) expandRatio = 1.0f;

					float bombRadius = 18.0f + expandRatio * 11.0f;

					// 爆弾の本体
					Novice::DrawEllipse(
						(int)x,
						(int)y,
						(int)bombRadius,
						(int)bombRadius,
						0.0f,
						0x171717FF,
						kFillModeSolid
					);

					// 爆弾の外側の光
					Novice::DrawEllipse(
						(int)x,
						(int)y,
						(int)bombRadius,
						(int)bombRadius,
						0.0f,
						flash == 0 ? 0xFF5500FF : 0xAA2200FF,
						kFillModeWireFrame
					);
				}
				else if (bombs[bombDrawIndex].isGlitch) {

					// グリッチ爆発の中心フラッシュ
					if (bombs[bombDrawIndex].life > 8) {
						Novice::DrawEllipse(
							(int)x,
							(int)y,
							(int)(kGlitchBombExplosionRadius * 0.5f),
							(int)(kGlitchBombExplosionRadius * 0.5f),
							0.0f,
							WHITE,
							kFillModeSolid
						);
					}

					// 横方向にずれたデジタルノイズ
					for (int glitchLineIndex = 0; glitchLineIndex < 12; glitchLineIndex++) {

						int offsetX = rand() % 181 - 90;
						int offsetY = rand() % 141 - 70;
						int lineWidth = 18 + rand() % 70;
						int lineHeight = 4 + rand() % 8;

						unsigned int glitchColor = 0x00FFFFFF;
						if (glitchLineIndex % 4 == 0) {
							glitchColor = 0xFF2255FF;
						}
						else if (glitchLineIndex % 4 == 1) {
							glitchColor = 0xAA22FFFF;
						}
						else if (glitchLineIndex % 4 == 2) {
							glitchColor = WHITE;
						}

						Novice::DrawBox(
							(int)x + offsetX,
							(int)y + offsetY,
							lineWidth,
							lineHeight,
							0.0f,
							glitchColor,
							kFillModeSolid
						);
					}
				}
				else {

					// 通常の爆弾の爆発（当たる範囲を、だんだん薄くなる円で見せる）
					unsigned int bombFadeAlpha = (unsigned int)(255.0f * (float)bombs[bombDrawIndex].life / 25.0f);

					Novice::DrawEllipse((int)x, (int)y, (int)kBombExplosionRadius, (int)kBombExplosionRadius, 0.0f, 0xFF883300 | bombFadeAlpha, kFillModeSolid);
					Novice::DrawEllipse((int)x, (int)y, (int)(kBombExplosionRadius * 0.5f), (int)(kBombExplosionRadius * 0.5f), 0.0f, 0xFFFFFF00 | bombFadeAlpha, kFillModeSolid);
				}
			}
		}

		// ボスの描画
		float bossSize = bossRadius;

		float bossCos = cosf(bossRotationAngle);
		float bossSin = sinf(bossRotationAngle);

		float cornerX[4] = {};
		float cornerY[4] = {};

		float localX[4] = {
			-bossSize,
			 bossSize,
			 bossSize,
			-bossSize
		};

		float localY[4] = {
			-bossSize,
			-bossSize,
			 bossSize,
			 bossSize
		};

		// 四角形の頂点を回転させる
		for (int cornerIndex = 0; cornerIndex < 4; cornerIndex++) {

			cornerX[cornerIndex] =
				bossX + localX[cornerIndex] * bossCos -
				localY[cornerIndex] * bossSin;

			cornerY[cornerIndex] =
				bossY + localX[cornerIndex] * bossSin +
				localY[cornerIndex] * bossCos;
		}

		// 四角形を描画
		Novice::DrawTriangle(
			(int)cornerX[0] + screenShakeX,
			(int)cornerY[0] + screenShakeY,
			(int)cornerX[1] + screenShakeX,
			(int)cornerY[1] + screenShakeY,
			(int)cornerX[2] + screenShakeX,
			(int)cornerY[2] + screenShakeY,
			bossColor,
			kFillModeSolid
		);

		Novice::DrawTriangle(
			(int)cornerX[0] + screenShakeX,
			(int)cornerY[0] + screenShakeY,
			(int)cornerX[2] + screenShakeX,
			(int)cornerY[2] + screenShakeY,
			(int)cornerX[3] + screenShakeX,
			(int)cornerY[3] + screenShakeY,
			bossColor,
			kFillModeSolid
		);

		// 分身の描画（ボスと同じ四角形、色だけ紫がかった半透明）
		for (int cloneDrawIndex = 0; cloneDrawIndex < kMaxBossClones; cloneDrawIndex++) {

			if (!bossClones[cloneDrawIndex].isExist) {
				continue;
			}

			float cloneCornerX[4] = {};
			float cloneCornerY[4] = {};

			for (int cloneCornerIndex = 0; cloneCornerIndex < 4; cloneCornerIndex++) {

				cloneCornerX[cloneCornerIndex] =
					bossClones[cloneDrawIndex].x + localX[cloneCornerIndex] * bossCos -
					localY[cloneCornerIndex] * bossSin;

				cloneCornerY[cloneCornerIndex] =
					bossClones[cloneDrawIndex].y + localX[cloneCornerIndex] * bossSin +
					localY[cloneCornerIndex] * bossCos;
			}

			Novice::DrawTriangle(
				(int)cloneCornerX[0] + screenShakeX, (int)cloneCornerY[0] + screenShakeY,
				(int)cloneCornerX[1] + screenShakeX, (int)cloneCornerY[1] + screenShakeY,
				(int)cloneCornerX[2] + screenShakeX, (int)cloneCornerY[2] + screenShakeY,
				0xAA66FFAA,
				kFillModeSolid
			);

			Novice::DrawTriangle(
				(int)cloneCornerX[0] + screenShakeX, (int)cloneCornerY[0] + screenShakeY,
				(int)cloneCornerX[2] + screenShakeX, (int)cloneCornerY[2] + screenShakeY,
				(int)cloneCornerX[3] + screenShakeX, (int)cloneCornerY[3] + screenShakeY,
				0xAA66FFAA,
				kFillModeSolid
			);
		}

		// スローモーションの更新
		if (slowMotionTimer > 0) {
			slowMotionTimer--;
		}
		else {
			gameSpeed = 1.0f;
		}

		// シールドブレイク演出
		if (shieldBreakEffectTimer > 0) {


			// 演出が始まってから何フレーム経過したか
			int effectElapsed = kShieldBreakEffectMax - shieldBreakEffectTimer;

			// フェード用の透明度
			float effectAlpha = shieldBreakEffectTimer / 60.0f;

			if (shieldBreakEffectTimer < 20) {
				effectAlpha = shieldBreakEffectTimer / 20.0f;
			}

			//// 光専用の拡大率（徐々に広がる）
			/*float hikariScale = 2.0f + effectElapsed * 0.04f;*/

			// 波紋専用の拡大率
			float waveScale = 0.5f + effectElapsed * 0.04f;

			// フラッシュ専用の拡大率
			float flashScale = 2.0f - effectElapsed * 0.15f;

			if (flashScale < 0.0f) {
				flashScale = 0.0f;
			}

			// ボスの中心から広がる波
			Novice::DrawSprite(
				(int)(bossX - 256.0f * waveScale) + screenShakeX,
				(int)(bossY - 256.0f * waveScale) + screenShakeY,
				namiTexture,
				waveScale, waveScale,
				0.0f,
				0xFFFFFF00 | (unsigned int)(effectAlpha * 255.0f)
			);

			//// ボスの中心に光を表示
			//Novice::DrawSprite(
			//	(int)(bossX - 256.0f * hikariScale) + screenShakeX,
			//	(int)(bossY - 256.0f * hikariScale) + screenShakeY,
			//	hikariTexture,
			//	hikariScale, hikariScale,
			//	0.0f,
			//	0xFFFFFF00 | (unsigned int)(effectAlpha * 255.0f)
			//);
			//// フラッシュは最初の12フレームだけ表示
			//if (effectElapsed < 3) {

			//	Novice::DrawSprite(
			//		(int)(bossX - 256.0f * flashScale) + screenShakeX,
			//		(int)(bossY - 256.0f * flashScale) + screenShakeY,
			//		flashTexture,
			//		flashScale, flashScale,
			//		0.0f,
			//		0xFFFFFF00 | (unsigned int)(effectAlpha * 255.0f)
			//	);
			//}
		}

		// ダッシュ爆弾の爆発画像を描画
		for (int dashBombExplosionDrawIndex = 0;
			dashBombExplosionDrawIndex < kMaxBombs;
			dashBombExplosionDrawIndex++) {

			// 爆発している爆弾が存在する場合のみ描画
			if (bombs[dashBombExplosionDrawIndex].isExist &&
				bombs[dashBombExplosionDrawIndex].exploded &&
				!bombs[dashBombExplosionDrawIndex].isGlitch) {

				// 爆発のアニメーションフレームを計算
				int frame = (25 - bombs[dashBombExplosionDrawIndex].life) * 9 / 25;
				if (frame < 0) frame = 0;
				if (frame > 8) frame = 8;

				// 爆発画像の描画
				Novice::DrawSpriteRect(
					(int)bombs[dashBombExplosionDrawIndex].x - kExplosionHalfSize + screenShakeX,
					(int)bombs[dashBombExplosionDrawIndex].y - kExplosionHalfSize + screenShakeY,
					frame * 124,
					0,
					124,
					124,
					bakuhatuTexture,
					(124.0f / 1116.0f) * kExplosionScale,
					kExplosionScale,
					0.0f,
					WHITE
				);
			}
		}

		// ボスの爆発エフェクト描画
		for (int explosionDrawIndex = 0;
			explosionDrawIndex < kMaxBossExplosions;
			explosionDrawIndex++) {

			if (bossExplosions[explosionDrawIndex].isExist) {

				Novice::DrawEllipse(
					(int)bossExplosions[explosionDrawIndex].x + screenShakeX,
					(int)bossExplosions[explosionDrawIndex].y + screenShakeY,
					(int)bossExplosions[explosionDrawIndex].radius,
					(int)bossExplosions[explosionDrawIndex].radius,
					0.0f,
					0xFF4422FF,
					kFillModeWireFrame
				);

				Novice::DrawEllipse(
					(int)bossExplosions[explosionDrawIndex].x + screenShakeX,
					(int)bossExplosions[explosionDrawIndex].y + screenShakeY,
					(int)(bossExplosions[explosionDrawIndex].radius * 0.6f),
					(int)(bossExplosions[explosionDrawIndex].radius * 0.6f),
					0.0f,
					0xFFAA22FF,
					kFillModeWireFrame
				);
			}
		}

		for (int particleDrawIndex = 0;
			particleDrawIndex < kMaxBossExplosionParticles;
			particleDrawIndex++) {

			if (bossExplosionParticles[particleDrawIndex].isExist) {

				float x = bossExplosionParticles[particleDrawIndex].x;
				float y = bossExplosionParticles[particleDrawIndex].y;

				int particleSize = (int)bossExplosionParticles[particleDrawIndex].radius;

				float velocityX = bossExplosionParticles[particleDrawIndex].velocityX;
				float velocityY = bossExplosionParticles[particleDrawIndex].velocityY;

				float size = bossExplosionParticles[particleDrawIndex].size;

				// 火花の後ろに伸びる光
				Novice::DrawLine(
					(int)(x - velocityX * 1.5f) + screenShakeX,
					(int)(y - velocityY * 1.5f) + screenShakeY,
					(int)(x + velocityX * size * 0.5f) + screenShakeX,
					(int)(y + velocityY * size * 0.5f) + screenShakeY,
					bossExplosionParticles[particleDrawIndex].color
				);

				// 外側の火花
				Novice::DrawEllipse(
					(int)x + screenShakeX,
					(int)y + screenShakeY,
					particleSize,
					particleSize,
					0.0f,
					bossExplosionParticles[particleDrawIndex].color,
					kFillModeSolid
				);

				// 中心の明るい部分
				Novice::DrawEllipse(
					(int)x + screenShakeX,
					(int)y + screenShakeY,
					particleSize / 2,
					particleSize / 2,
					0.0f,
					WHITE,
					kFillModeSolid
				);
			}
		}

		// プレイヤーとボスの残像を共通処理で描画
		for (int afterDrawIndex = 0; afterDrawIndex < kMaxAfterImages; afterDrawIndex++) {

			if (afterImages[afterDrawIndex].isExist) {

				// プレイヤーの残像
				if (afterDrawIndex < kMaxPlayerAfterImages) {

					// 残像の寿命に合わせて透明度を下げる
					float afterImageRatio =
						(float)afterImages[afterDrawIndex].life / (float)kAfterImageLife;

					if (afterImageRatio < 0.0f) afterImageRatio = 0.0f;
					if (afterImageRatio > 1.0f) afterImageRatio = 1.0f;

					unsigned int afterImageAlpha =
						(unsigned int)(afterImageRatio * 100.0f);

					float outerScale = 0.18f;
					float innerScale = 0.13f;

					// 外側のぼかし
					Novice::DrawSprite(
						(int)(afterImages[afterDrawIndex].x - 63) + screenShakeX,
						(int)(afterImages[afterDrawIndex].y - 63) + screenShakeY,
						bokasiTexture,
						outerScale,
						outerScale,
						0.0f,
						(afterImageAlpha / 2) << 24 | 0x000033FF
					);

					// 内側のぼかし
					Novice::DrawSprite(
						(int)(afterImages[afterDrawIndex].x - 45) + screenShakeX,
						(int)(afterImages[afterDrawIndex].y - 45) + screenShakeY,
						bokasiTexture,
						innerScale,
						innerScale,
						0.0f,
						afterImageAlpha << 24 | 0x000066FF
					);

					// 星の残像本体
					Novice::DrawSprite(
						(int)(afterImages[afterDrawIndex].x - 32) + screenShakeX,
						(int)(afterImages[afterDrawIndex].y - 32) + screenShakeY,
						hosiTexture,
						1.0f,
						1.0f,
						0.0f,
						afterImageAlpha << 24 | 0x00FFFFFF
					);
				}
				// ボスの残像
				else {

					DrawAfterImage(
						afterImages[afterDrawIndex].x + screenShakeX,
						afterImages[afterDrawIndex].y + screenShakeY,
						bossRadius,
						afterImages[afterDrawIndex].life,
						kAfterImageLife,
						0x000000FF
					);
				}
			}
		}

		// 仲間の弾の描画
		for (int allyBulletDrawIndex = 0; allyBulletDrawIndex < kMaxAllyBullets; allyBulletDrawIndex++) {

			if (allyBullets[allyBulletDrawIndex].isExist) {

				// 弾の後ろに伸びる光
				Novice::DrawLine(
					(int)(allyBullets[allyBulletDrawIndex].x - allyBullets[allyBulletDrawIndex].velocityX * 1.2f) + screenShakeX,
					(int)(allyBullets[allyBulletDrawIndex].y - allyBullets[allyBulletDrawIndex].velocityY * 1.2f) + screenShakeY,
					(int)allyBullets[allyBulletDrawIndex].x + screenShakeX,
					(int)allyBullets[allyBulletDrawIndex].y + screenShakeY,
					0x66AAFFFF
				);

				// 弾の本体
				Novice::DrawEllipse(
					(int)allyBullets[allyBulletDrawIndex].x + screenShakeX,
					(int)allyBullets[allyBulletDrawIndex].y + screenShakeY,
					7,
					7,
					0.0f,
					0x3366FFFF,
					kFillModeSolid
				);

				Novice::DrawEllipse(
					(int)allyBullets[allyBulletDrawIndex].x + screenShakeX,
					(int)allyBullets[allyBulletDrawIndex].y + screenShakeY,
					3,
					3,
					0.0f,
					WHITE,
					kFillModeSolid
				);
			}
		}

		// 仲間（青）の描画
		for (int allyDrawIndex = 0; allyDrawIndex < kMaxStageEnemies; allyDrawIndex++) {

			if (stageEnemies[allyDrawIndex].isExist && stageEnemies[allyDrawIndex].isAlly) {

				Novice::DrawEllipse(
					(int)stageEnemies[allyDrawIndex].x + screenShakeX,
					(int)stageEnemies[allyDrawIndex].y + screenShakeY,
					(int)kStageEnemyRadius + 3,
					(int)kStageEnemyRadius + 3,
					0.0f,
					0x112299FF,
					kFillModeSolid
				);

				Novice::DrawEllipse(
					(int)stageEnemies[allyDrawIndex].x + screenShakeX,
					(int)stageEnemies[allyDrawIndex].y + screenShakeY,
					(int)kStageEnemyRadius,
					(int)kStageEnemyRadius,
					0.0f,
					0x3366FFFF,
					kFillModeSolid
				);
			}
		}

		// 召喚された敵（赤）の描画
		for (int summonDrawIndex = 0; summonDrawIndex < kMaxSummons; summonDrawIndex++) {

			if (!summons[summonDrawIndex].isExist) {
				continue;
			}

			// 縁
			Novice::DrawEllipse(
				(int)summons[summonDrawIndex].x + screenShakeX,
				(int)summons[summonDrawIndex].y + screenShakeY,
				(int)kStageEnemyRadius + 3,
				(int)kStageEnemyRadius + 3,
				0.0f,
				0x990000FF,
				kFillModeSolid
			);

			// 本体（自爆が近づくと点滅する）
			unsigned int summonColor = 0xFF2222FF;

			if (summons[summonDrawIndex].life < 60 && summons[summonDrawIndex].life / 5 % 2 == 0) {
				summonColor = WHITE;
			}

			Novice::DrawEllipse(
				(int)summons[summonDrawIndex].x + screenShakeX,
				(int)summons[summonDrawIndex].y + screenShakeY,
				(int)kStageEnemyRadius,
				(int)kStageEnemyRadius,
				0.0f,
				summonColor,
				kFillModeSolid
			);
		}

		// プレイヤーの描画（無敵の間は点滅する。やられたら消える）
		if (!isPlayerDead && (playerInvincibleTimer == 0 || (playerInvincibleTimer / 4) % 2 == 0)) {
			Novice::DrawSprite(
				(int)(playerX - 52) + screenShakeX,
				(int)(playerY - 50) + screenShakeY,
				bokasiTexture,
				0.15f,
				0.15f,
				0.0f,
				0x330033FF
			);

			Novice::DrawSprite(
				(int)(playerX - 32) + screenShakeX,
				(int)(playerY - 32) + screenShakeY,
				hosiTexture,
				1.0f,
				1.0f,
				0.0f,
				WHITE
			);
		}

		// 火花パーティクル描画
		for (int sparkDrawIndex = 0; sparkDrawIndex < kMaxSparks; sparkDrawIndex++) {

			if (sparks[sparkDrawIndex].isExist) {

				int size = 1 + sparks[sparkDrawIndex].life / 10;
				float size2 = 0.01f + (float)sparks[sparkDrawIndex].life / 2000.0f;

				// 火花の後ろにぼかしを描画
				Novice::DrawSprite(
					(int)(sparks[sparkDrawIndex].x - 4),
					(int)(sparks[sparkDrawIndex].y - 5),
					bokasiTexture,
					size2,
					size2,
					0.0f,
					0xFF663333
				);

				// 火花本体
				Novice::DrawBox(
					(int)sparks[sparkDrawIndex].x,
					(int)sparks[sparkDrawIndex].y,
					size,
					size,
					0.0f,
					0xFFFF00FF,
					kFillModeSolid
				);
			}
		}

		// プレイヤーの視線
		float lookX = playerX + cosf(playerAngle) * (playerRadius + 10.0f);
		float lookY = playerY + sinf(playerAngle) * (playerRadius + 10.0f);

		Novice::DrawLine(
			(int)playerX,
			(int)playerY,
			(int)lookX,
			(int)lookY,
			WHITE
		);

		// ==============================
// ボスのシールドゲージ
// ==============================

		int shieldBarX = 660;
		int shieldBarY = 60;
		int shieldBarWidth = 600;
		int shieldBarHeight = 24;

		// 背景
		Novice::DrawBox(
			shieldBarX,
			shieldBarY,
			shieldBarWidth,
			shieldBarHeight,
			0.0f,
			0x222222FF,
			kFillModeSolid
		);

		// シールドの残量
		float shieldRatio = (float)bossShield / (float)bossShieldMax;

		int currentShieldWidth = (int)(shieldBarWidth * shieldRatio);

		// 残量ゲージ
		if (bossShield > 0) {
			Novice::DrawBox(
				shieldBarX,
				shieldBarY,
				currentShieldWidth,
				shieldBarHeight,
				0.0f,
				// 0x00CCFFFF,
				(bossPhase == 3) ? 0xFFCC33FF : (bossPhase == 2) ? 0xFF66CCFF : 0x00CCFFFF,
				kFillModeSolid
			);
		}

		// 外枠
		Novice::DrawBox(
			shieldBarX,
			shieldBarY,
			shieldBarWidth,
			shieldBarHeight,
			0.0f,
			WHITE,
			kFillModeWireFrame
		);

		// 数値表示
		Novice::ScreenPrintf(
			shieldBarX,
			shieldBarY - 25,
			"BOSS ARMOR : %d / %d",
			bossShield,
			bossShieldMax
		);

		// 今の形態
		Novice::ScreenPrintf(shieldBarX + shieldBarWidth + 20, shieldBarY - 25, "FORM : %d", bossPhase);

		// バリアのゲージ（シールドのゲージの下に出す）
		if (bossBarrier > 0) {

			int barrierBarY = shieldBarY + 34;
			int barrierBarHeight = 14;

			Novice::DrawBox(shieldBarX, barrierBarY, shieldBarWidth, barrierBarHeight, 0.0f, 0x222222FF, kFillModeSolid);

			Novice::DrawBox(
				shieldBarX,
				barrierBarY,
				(int)(shieldBarWidth * (float)bossBarrier / (float)kBossBarrierMax),
				barrierBarHeight,
				0.0f,
				0x66CCFFFF,
				kFillModeSolid
			);

			Novice::DrawBox(shieldBarX, barrierBarY, shieldBarWidth, barrierBarHeight, 0.0f, WHITE, kFillModeWireFrame);

			Novice::ScreenPrintf(shieldBarX, barrierBarY + 16, "BARRIER : %d / %d", bossBarrier, kBossBarrierMax);
		}

		if (bossIsInvincible) {
			Novice::ScreenPrintf(shieldBarX + shieldBarWidth + 20, shieldBarY, "INVINCIBLE");
		}

		// UI表示
		Novice::ScreenPrintf(10, 10, "Move : WASD (Free Movement)");
		Novice::ScreenPrintf(10, 30, "Dash : SPACE");
		Novice::ScreenPrintf(10, 50, "Dash Count : %d / %d", dashCount, kMaxDashes);

		if (attackType == 1) {
			Novice::ScreenPrintf(10, 70, "Attack : 1 - Round Bullet");
		}
		else {
			Novice::ScreenPrintf(10, 70, "Attack : 2 - Laser");
		}
		Novice::ScreenPrintf(50, 90, "Current Frame : %d", currentFrame);
		Novice::ScreenPrintf(10, 110, "Allies : %d", allyCount);
		if (attackType == 2) {
			Novice::ScreenPrintf(10, 150, "Dash x4 in a SQUARE : MEGA LASER");
		}
		Novice::ScreenPrintf(10, 130, "SHIFT : skip (%s)", (bossPhase == 2) ? "to FORM 3" : (bossPhase >= 3) ? "to GATHER" : "to FORM 2");

		// プレイヤーのHP（右上）
		Novice::ScreenPrintf(1440, 40, "HP");

		for (int hpDrawIndex = 0; hpDrawIndex < kPlayerMaxHp; hpDrawIndex++) {

			if (hpDrawIndex < playerHp) {
				Novice::DrawBox(1480 + hpDrawIndex * 52, 30, 40, 40, 0.0f, 0xFF4466FF, kFillModeSolid);
			}
			else {
				Novice::DrawBox(1480 + hpDrawIndex * 52, 30, 40, 40, 0.0f, 0x886677FF, kFillModeWireFrame);
			}
		}

		// ダッシュのスタミナ（HPの下）。目盛りの1つぶんがダッシュ1回
		Novice::ScreenPrintf(1424, 92, "DASH");

		unsigned int staminaColor = 0x66DDFFFF;

		if (staminaFlashTimer > 0 && (staminaFlashTimer / 4) % 2 == 0) {
			staminaColor = 0xFF4444FF;
		}

		Novice::DrawBox(1480, 90, (int)(260.0f * stamina / kStaminaMax), 18, 0.0f, staminaColor, kFillModeSolid);
		Novice::DrawBox(1480, 90, 260, 18, 0.0f, 0xFFFFFFFF, kFillModeWireFrame);

		for (int staminaTickIndex = 1; staminaTickIndex < (int)(kStaminaMax / kDashStaminaCost); staminaTickIndex++) {

			int staminaTickX = 1480 + (int)(260.0f * (float)staminaTickIndex * kDashStaminaCost / kStaminaMax);
			Novice::DrawLine(staminaTickX, 90, staminaTickX, 108, 0xFFFFFF88);
		}

		// やられたとき
		if (isPlayerDead) {
			Novice::DrawBox(0, 0, 1920, 1180, 0.0f, 0x00000099, kFillModeSolid);
			Novice::ScreenPrintf(880, 540, "GAME OVER");
		}

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Controller::StopVibration();
	Novice::Finalize();
	return 0;
}