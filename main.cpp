#include <Novice.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <Xinput.h>
#include "Controller.h"

const char kWindowTitle[] = "TD1_2回目!!!";

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

// 仲間の弾構造体
struct AllyBullet
{
	float x;
	float y;
	float velocityX;
	float velocityY;
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
	int hosiTexture = Novice::LoadTexture("./Resources/hikarin.png");
	int bakuhatuTexture = Novice::LoadTexture("./Resources/bakuhatu.png");

	// 爆発画像の拡大率
	const float kExplosionScale = 2.5f;
	const int kExplosionHalfSize = (int)(62.0f * kExplosionScale);

	// ボスのデータ
	float bossX = 960.0f;
	float bossY = 295.0f;
	float bossRadius = 90.0f;
	float bossSpeedX = 6.0f;
	int bossTrailTimer = 0;

	// boss
	int bossState = 0;
	int bossAttackTimer = 180;
	float bossAuraRadius = 0.0f;
	float bossAuraAlpha = 0.0f;
	float bossChargeVelocityX = 0.0f;
	float bossChargeVelocityY = 0.0f;
	float bossRotationAngle = 0.0f;
	int bossSpinTimer = 0;

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
	const int kMaxStageEnemies = 200;
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

		// ひまわり状に敷き詰める（人数が増えても密度が一定）
		float angle = (float)index * 2.39996f;
		float radius = kFlockSpacing * sqrtf((float)index + 0.5f);

		// ちょっとふらふらさせてゆるい感じにする
		float wobbleX = sinf((float)followTimer * 0.07f + (float)index * 1.7f) * 4.0f;
		float wobbleY = cosf((float)followTimer * 0.06f + (float)index * 2.3f) * 4.0f;

		slotOutX = centerX + cosf(angle) * radius + wobbleX;
		slotOutY = centerY + sinf(angle) * radius + wobbleY;
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

			// 呼びかけ（SPACEキー）
			if ((Controller::IsPressed(kPadButton10) ||
				(keys[DIK_SPACE] && !preKeys[DIK_SPACE])) &&
				callTimer == 0 && callCooldown == 0) {

				callTimer = kCallFrames;
				callRadius = kCallStartRadius;
				callCooldown = kCallCooldownMax;
			}

			// 呼びかけの波を広げる
			if (callTimer > 0) {
				callTimer--;
				callRadius += kCallSpeed;
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

						float callDx = stageEnemies[stageEnemyUpdateIndex].x - stagePlayerX;
						float callDy = stageEnemies[stageEnemyUpdateIndex].y - stagePlayerY;

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

			// 扉に入ったらボス戦へ
			if (stagePlayerX > kDoorX - kDoorWidth / 2.0f &&
				stagePlayerX < kDoorX + kDoorWidth / 2.0f &&
				stagePlayerY > kDoorY - kDoorHeight / 2.0f &&
				stagePlayerY < kDoorY + kDoorHeight / 2.0f) {

				scene = kSceneBoss;

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

			// 呼びかけの波
			if (callTimer > 0) {

				Novice::DrawEllipse(
					(int)stagePlayerX,
					(int)stagePlayerY,
					(int)callRadius,
					(int)callRadius,
					0.0f,
					0x66AAFFFF,
					kFillModeWireFrame
				);

				Novice::DrawEllipse(
					(int)stagePlayerX,
					(int)stagePlayerY,
					(int)(callRadius * 0.9f),
					(int)(callRadius * 0.9f),
					0.0f,
					0x3366FF88,
					kFillModeWireFrame
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
			Novice::ScreenPrintf(10, 30, "Call : SPACE (red -> blue)");
			Novice::ScreenPrintf(10, 50, "Allies : %d / %d", allyCount, kMaxStageEnemies);
			Novice::ScreenPrintf(10, 70, "Enter the door to fight the boss");

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

		// ボスの移動・突進処理 

		if (bossState == 0) {

			bossX += bossSpeedX;

			if (bossX - bossRadius <= 0.0f ||
				bossX + bossRadius >= 1920.0f) {
				bossSpeedX *= -1.0f;
			}

			bossAttackTimer--;

			if (bossAttackTimer <= 0) {
				bossState = 1;
				bossAttackTimer = 45;
			}
		}

		else if (bossState == 1) {

			bossAttackTimer--;

			if (bossAttackTimer <= 0) {

				float dx = playerX - bossX;
				float dy = playerY - bossY;

				float length = sqrtf(dx * dx + dy * dy);

				if (length > 0.0f) {
					bossChargeVelocityX = dx / length * 22.0f;
					bossChargeVelocityY = dy / length * 22.0f;
				}

				bossState = 2;
				bossTrailTimer = 0;
				bossAttackTimer = 30;

				// 爆弾を落とすタイマーをリセット
				bombDropTimer = 0;

				screenShakeTimer = 10;

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

			bossAttackTimer--;

			if (bossX - bossRadius < 0.0f ||
				bossX + bossRadius > 1920.0f ||
				bossY - bossRadius < 0.0f ||
				bossY + bossRadius > 1180.0f ||
				bossAttackTimer <= 0) {

				afterImageTimer = 0;

				bossState = 3;
				bossAttackTimer = 60;
			}
		}

		else if (bossState == 3) {

			bossAttackTimer--;

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
				bossAttackTimer = 180;
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

			if (hitDx * hitDx + hitDy * hitDy < (bossRadius + 8.0f) * (bossRadius + 8.0f)) {

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

		// --- ダッシュ実行 ---
		//　ダッシュボタンが押されたとき、ダッシュが最大回数より小さかった時
		if ((Controller::IsPressed(kPadButton10) ||
			(keys[DIK_SPACE] && !preKeys[DIK_SPACE])) &&
			dashCount < kMaxDashes)
		{


			//　ダッシュがクールダウン中ではない時かつ、攻撃中じゃないなら
			if (dashCooldown <= 0 && !isAttacking)
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

				if (dashCount == kMaxDashes) {
					isAttacking = true;
					currentAttackIndex = 0;
					attackTimer = 10;

				}
			}

			// ひし形砲台の向きをボスに合わせる
			if (dashCount < 4) {
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

		// --- 連続照射処理 ---
		if (isAttacking) {

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

				beams[beamUpdateIndex].x += beams[beamUpdateIndex].velocityX * beams[beamUpdateIndex].speed;
				beams[beamUpdateIndex].y += beams[beamUpdateIndex].velocityY * beams[beamUpdateIndex].speed;

				float dx = bossX - beams[beamUpdateIndex].x;
				float dy = bossY - beams[beamUpdateIndex].y;

				// ボスに弾が当たったら
				if (dx * dx + dy * dy <
					(bossRadius + 10.0f) * (bossRadius + 10.0f)) {

					beams[beamUpdateIndex].isShot = 0;

					screenShakeTimer = 8;
					Controller::SetVibration(60000, 60000);
					vibrationTimer = 3;

					// ボスの中心から火花を発生
					for (int p = 0; p < 25; p++) {

						for (int sparkSpawnIndex = 0; sparkSpawnIndex < kMaxSparks; sparkSpawnIndex++) {

							if (!sparks[sparkSpawnIndex].isExist) {



								sparks[sparkSpawnIndex].x = bossX;
								sparks[sparkSpawnIndex].y = bossY;

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

				if (lasers[laserUpdateIndex].isShot &&
					along >= 0.0f &&
					along <= lasers[laserUpdateIndex].length + bossRadius &&
					side * side <= bossRadius * bossRadius &&
					!lasers[laserUpdateIndex].sparkMade) {

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

		// ボス
		unsigned int bossColor = RED;

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
							55,
							55,
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

		// プレイヤーの描画
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