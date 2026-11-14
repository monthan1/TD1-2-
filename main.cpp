#include <Novice.h>
#include <stdlib.h>
#include <math.h>
#include "Controller.h"

const char kWindowTitle[] = "TD1_2回目!!";

// 画面に設置されるすべての長方形（サンズ風ブラスター砲台）
struct BoxData {
	float x;         // 設置位置 X
	float y;         // 設置位置 Y
	int width;       // 長方形の幅
	int height;      // 長方形の高さ
	unsigned int color; // 色
	int delay;       // 出現ディレイ
	int isExist;     // 存在フラグ
};

// ダッシュの軌道線構造体
struct DashLine {
	float startX; // ダッシュ開始 X
	float startY; // ダッシュ開始 Y
	float endX;   // ダッシュ終了 X
	float endY;   // ダッシュ終了 Y
	int isExist;  // 存在フラグ
};

struct BlasterBeam {
	float x;
	float y;
	float velocityX;
	float velocityY;
	float speed;
	int isShot;

	// 弾の後ろに残る残像
	float trailX[8];
	float trailY[8];
};

// ダッシュ残像データ
struct AfterImage {
	float x;
	float y;
	float startX;
	float startY;
	int isExist;
};

// ビーム命中時の火花パーティクル
struct Spark {
	float x;
	float y;
	float velocityX;
	float velocityY;
	int life;
	int isExist;
};

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	Controller::Initialize();

	// キー入力受け取り用の配列
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	int bokasiTexture = Novice::LoadTexture("./Resources/bokasi.png");
	int hosiTexture = Novice::LoadTexture("./Resources/hosi.png");

	// ボスのデータ
	float bossX = 640.0f;
	float bossY = 180.0f;
	float bossRadius = 60.0f;
	float bossSpeedX = 4.0f;

	// ボスの突進設定
	int bossState = 0;
	// 0：通常移動
	// 1：狙いを定める
	// 2：突進
	// 3：休憩

	int bossAttackTimer = 180;
	float bossChargeVelocityX = 0.0f;
	float bossChargeVelocityY = 0.0f;

	// プレイヤーのデータ
	float playerX = 640.0f;
	float playerY = 550.0f;
	float playerRadius = 30.0f;
	float playerAngle = -1.5708f; // 向き（初期値：上向き）

	// 移動・ダッシュ設定
	float speed = 8.0f;
	float dashDistance = 200.0f;

	// ダッシュ後の硬直設定
	int dashStunTimer = 0;
	const int kDashStunMax = 30;

	// ダッシュ移動設定
	const int kDashMoveFrames = 6; // ダッシュ移動にかける時間
	int dashMoveTimer = 0;

	float dashStartX = 0.0f;
	float dashStartY = 0.0f;
	float dashTargetX = 0.0f;
	float dashTargetY = 0.0f;

	// 残像設定
	const int kMaxAfterImages = 6;
	const int kAfterImageConvergeFrames = 8;

	AfterImage afterImages[kMaxAfterImages];

	// 火花パーティクル設定
	const int kMaxSparks = 200;
	Spark sparks[kMaxSparks];

	for (int i = 0; i < kMaxSparks; i++) {
		sparks[i].isExist = 0;
	}

	for (int i = 0; i < kMaxAfterImages; i++) {
		afterImages[i].isExist = 0;
	}

	// ダッシュの連射防止用クールダウン（短時間の入力インターバルのみ）
	int dashCooldown = 0;
	const int kDashCooldownMax = 10;

	// ダッシュカウント（4回ダッシュで攻撃）
	int dashCount = 0;
	const int kMaxDashes = 4;

	// 長方形データ（大4個 + 小12個 = 最大16個）
	const int kMaxBoxes = 16;
	BoxData boxes[kMaxBoxes];
	int boxCount = 0;

	// 加速する弾データ
	BlasterBeam beams[kMaxBoxes];

	// ダッシュ軌道線データ
	const int kMaxLines = 100;
	DashLine lines[kMaxLines];
	int lineCount = 0;

	// サンズ風ビームデータ
	for (int i = 0; i < kMaxBoxes; i++) {
		boxes[i].isExist = 0;
		boxes[i].delay = 0;
		beams[i].isShot = 0;
	}
	for (int i = 0; i < kMaxLines; i++) {
		lines[i].isExist = 0;
	}

	// 攻撃シーケンス
	int isAttacking = 0;
	int attackTimer = 0;
	int currentAttackIndex = 0;

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// コントローラー入力を受け取る
		Controller::Update();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		// --- 1. ボスの移動・突進処理 ---

// 通常移動
		if (bossState == 0) {

			bossX += bossSpeedX;

			if (bossX - bossRadius <= 0.0f ||
				bossX + bossRadius >= 1280.0f) {
				bossSpeedX *= -1.0f;
			}

			bossAttackTimer--;

			// 突進準備開始
			if (bossAttackTimer <= 0) {
				bossState = 1;
				bossAttackTimer = 45;
			}
		}

		// 狙いを定める
		else if (bossState == 1) {

			bossAttackTimer--;

			// 45フレーム経過したら突進
			if (bossAttackTimer <= 0) {

				// プレイヤーの方向を計算
				float dx = playerX - bossX;
				float dy = playerY - bossY;

				float length = sqrtf(dx * dx + dy * dy);

				if (length > 0.0f) {
					bossChargeVelocityX = dx / length * 22.0f;
					bossChargeVelocityY = dy / length * 22.0f;
				}

				bossState = 2;
				bossAttackTimer = 30;
			}
		}

		// 突進
		else if (bossState == 2) {

			bossX += bossChargeVelocityX;
			bossY += bossChargeVelocityY;

			bossAttackTimer--;

			// 画面端に到達したら突進終了
			if (bossX - bossRadius < 0.0f ||
				bossX + bossRadius > 1280.0f ||
				bossY - bossRadius < 0.0f ||
				bossY + bossRadius > 720.0f ||
				bossAttackTimer <= 0) {

				bossState = 3;
				bossAttackTimer = 60;
			}
		}

		// 休憩
		else if (bossState == 3) {

			bossAttackTimer--;

			if (bossAttackTimer <= 0) {
				bossState = 0;
				bossAttackTimer = 150;
			}
		}

		// 画面シェイク設定
		int screenShakeTimer = 0;
		const int kScreenShakeDuration = 15;

		float screenShakeX = 0.0f;
		float screenShakeY = 0.0f;

		// --- 2. クールダウンタイマーの減衰 ---
		if (dashCooldown > 0) { dashCooldown--; }

		// ダッシュ後の硬直タイマー
		if (dashStunTimer > 0) {
			dashStunTimer--;
		}

		// ダッシュ移動アニメーション
		if (dashMoveTimer > 0) {

			dashMoveTimer--;

			float t = 1.0f - (float)dashMoveTimer / kDashMoveFrames;

			playerX = dashStartX + (dashTargetX - dashStartX) * t;
			playerY = dashStartY + (dashTargetY - dashStartY) * t;
		}

		// --- 3. 入力方向の取得 & 常時自由移動（ロック解除） ---
		
		// コントローラー入力(なんと-を付けると入力方向が逆になります)
float moveX = Controller::GetLeftStickX();
float moveY = Controller::GetLeftStickY();

// キーボード入力
if (keys[DIK_W]) moveY -= 1.0f;
if (keys[DIK_S]) moveY += 1.0f;
if (keys[DIK_A]) moveX -= 1.0f;
if (keys[DIK_D]) moveX += 1.0f;

// 入力があれば向きを更新
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


		// 残像の収束処理
		for (int i = 0; i < kMaxAfterImages; i++) {

			if (afterImages[i].isExist) {

				// 硬直終了まで残像を維持
				if (dashStunTimer <= kAfterImageConvergeFrames) {

					float t = 1.0f -
						(float)dashStunTimer / kAfterImageConvergeFrames;

					// プレイヤーの位置に向かって収束
					afterImages[i].x =
						afterImages[i].startX +
						(playerX - afterImages[i].startX) * t;

					afterImages[i].y =
						afterImages[i].startY +
						(playerY - afterImages[i].startY) * t;

					// 硬直終了と同時に消す
					if (dashStunTimer == 0) {
						afterImages[i].isExist = 0;
					}
				}
			}
		}

		// --- 4. ダッシュ実行（移動制限なし・いつでもダッシュ可能） ---

		if (dashCooldown == 0 &&
			!isAttacking &&
			Controller::IsPressed(kPadButton10) &&
			dashCount < kMaxDashes) {

			float startX = playerX;
			float startY = playerY;

			float dashDirX = cosf(playerAngle);
			float dashDirY = sinf(playerAngle);

			// ダッシュの移動先を設定
			dashStartX = startX;
			dashStartY = startY;

			dashTargetX = startX + dashDirX * dashDistance;
			dashTargetY = startY + dashDirY * dashDistance;

			// 画面外に出ないようにする
			if (dashTargetX < playerRadius) dashTargetX = playerRadius;
			if (dashTargetX > 1280.0f - playerRadius) dashTargetX = 1280.0f - playerRadius;
			if (dashTargetY < playerRadius) dashTargetY = playerRadius;
			if (dashTargetY > 720.0f - playerRadius) dashTargetY = 720.0f - playerRadius;

			// ダッシュ開始
			dashMoveTimer = kDashMoveFrames;

			// 硬直開始
			dashStunTimer = kDashStunMax;

			// 残像を生成
			for (int i = 0; i < kMaxAfterImages; i++) {

				float ratio = (float)i / (kMaxAfterImages - 1);

				afterImages[i].x = dashStartX + (dashTargetX - dashStartX) * ratio;
				afterImages[i].y = dashStartY + (dashTargetY - dashStartY) * ratio;

				// 収束開始時の位置を保存
				afterImages[i].startX = afterImages[i].x;
				afterImages[i].startY = afterImages[i].y;

				afterImages[i].isExist = 1;
			}

			// 【1】メインの大きな長方形（ブラスター砲台）
			if (boxCount < kMaxBoxes) {
				boxes[boxCount].x = startX;
				boxes[boxCount].y = startY;
				boxes[boxCount].width = 20;
				boxes[boxCount].height = 45;
				boxes[boxCount].color = 0xFFFFFFFF; // サンズ風ホワイト
				boxes[boxCount].delay = 0;
				boxes[boxCount].isExist = 1;
				boxCount++;
			}

			// 【2】軌道上に小型砲台を3つ追加
			for (int p = 0; p < 3; p++) {
				if (boxCount < kMaxBoxes) {
					float ratio = (float)(p + 1) / 4.0f;
					boxes[boxCount].x = startX + (dashTargetX - startX) * ratio;
					boxes[boxCount].y = startY + (dashTargetY - startY) * ratio;
					boxes[boxCount].width = 12;
					boxes[boxCount].height = 28;
					boxes[boxCount].color = 0x00FFFFFF; // シアンブルー
					boxes[boxCount].delay = (p + 1) * 3;
					boxes[boxCount].isExist = 1;
					boxCount++;
				}
			}

			// 軌道線追加
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
				isAttacking = 1;
				currentAttackIndex = 0;
				attackTimer = 10;
			}
		}

		// 小長方形の表示ディレイ
		for (int i = 0; i < boxCount; i++) {
			if (boxes[i].isExist && boxes[i].delay > 0) {
				boxes[i].delay--;
			}
		}

		// --- 5. ガスターブラスター風・連続照射処理 ---
		if (isAttacking) {
			if (attackTimer > 0) {
				attackTimer--;
			}
			else {
				if (currentAttackIndex < boxCount) {
					if (boxes[currentAttackIndex].isExist) {
						float startX = boxes[currentAttackIndex].x;
						float startY = boxes[currentAttackIndex].y;

						// ボスの中心座標を狙う
						float dx = bossX - startX;
						float dy = bossY - startY;

						float length = sqrtf(dx * dx + dy * dy);

						if (length > 0.0f) {
							beams[currentAttackIndex].velocityX = dx / length;
							beams[currentAttackIndex].velocityY = dy / length;
						}
						else {
							beams[currentAttackIndex].velocityX = 0.0f;
							beams[currentAttackIndex].velocityY = -1.0f;
						}

						// 弾の発射位置
						beams[currentAttackIndex].x = startX;
						beams[currentAttackIndex].y = startY;

						// 最初はゆっくり
						beams[currentAttackIndex].speed = 0.5f;

						beams[currentAttackIndex].isShot = 1;

						beams[currentAttackIndex].isShot = 1;

						// 残像の位置を発射位置に合わせる
						for (int i = 0; i < 8; i++) {
							beams[currentAttackIndex].trailX[i] = startX;
							beams[currentAttackIndex].trailY[i] = startY;
						}

						boxes[currentAttackIndex].isExist = 0;
					}

					currentAttackIndex++;
					attackTimer = 2; // 高速連射
				}
				else {
					// 全弾照射完了でリセット
					dashCount = 0;
					boxCount = 0;
					isAttacking = 0;

					for (int i = 0; i < kMaxLines; i++) { lines[i].isExist = 0; }
					lineCount = 0;
				}
			}
		}

		// 火花パーティクル更新
		for (int i = 0; i < kMaxSparks; i++) {

			if (sparks[i].isExist) {

				sparks[i].x += sparks[i].velocityX;
				sparks[i].y += sparks[i].velocityY;

				// 徐々に減速
				sparks[i].velocityX *= 0.94f;
				sparks[i].velocityY *= 0.94f;
				sparks[i].life--;

				if (sparks[i].life <= 0) {
					sparks[i].isExist = 0;
				}
			}
		}

		// 弾の移動・加速処理
		for (int i = 0; i < kMaxBoxes; i++) {
			if (beams[i].isShot) {

				// 残像の位置を後ろにずらす
				for (int j = 7; j > 0; j--) {
					beams[i].trailX[j] = beams[i].trailX[j - 1];
					beams[i].trailY[j] = beams[i].trailY[j - 1];
				}

				// 一番新しい残像の位置
				beams[i].trailX[0] = beams[i].x;
				beams[i].trailY[0] = beams[i].y;

				// どんどん加速する
				beams[i].speed += 0.1f + beams[i].speed * 0.2f;

				// 最高速度を上げる
				if (beams[i].speed > 35.0f)
				{
					beams[i].speed = 35.0f;
				}

				// 弾を移動
				beams[i].x += beams[i].velocityX * beams[i].speed;
				beams[i].y += beams[i].velocityY * beams[i].speed;

				float dx = bossX - beams[i].x;
				float dy = bossY - beams[i].y;

				// ボスに弾が当たったら
				if (dx * dx + dy * dy <
					(bossRadius + 10.0f) * (bossRadius + 10.0f)) {

					// 画面シェイク開始
					screenShakeTimer = kScreenShakeDuration;

					// 弾を消す
					beams[i].isShot = 0;

					// ボスの中心からパーティクルを発生
					for (int p = 0; p < 25; p++) {

						for (int j = 0; j < kMaxSparks; j++) {

							if (!sparks[j].isExist) {

								float sparkAngle = (float)p / 25.0f * 6.28318f;

								float particleSpeed = 3.0f + (float)(rand() % 70) / 10.0f;

								sparks[j].x = bossX;
								sparks[j].y = bossY;

								sparks[j].velocityX = cosf(sparkAngle) * particleSpeed;
								sparks[j].velocityY = sinf(sparkAngle) * particleSpeed;

								sparks[j].life = 40;
								sparks[j].isExist = 1;

								break;
							}
						}
					}
				}

				if (beams[i].x < -50 || beams[i].x > 1330 ||
					beams[i].y < -50 || beams[i].y > 770) {
					beams[i].isShot = 0;
				}
			}
		}

		// 画面シェイク更新
		if (screenShakeTimer > 0) {

			screenShakeTimer--;

			// 徐々に揺れを弱くする
			float shakePower = 12.0f *
				((float)screenShakeTimer / kScreenShakeDuration);

			screenShakeX = (float)(rand() % 200 - 100) / 100.0f * shakePower;
			screenShakeY = (float)(rand() % 200 - 100) / 100.0f * shakePower;

		} else {
			screenShakeX = 0.0f;
			screenShakeY = 0.0f;
		}

		// 画面外制限
		if (playerX - playerRadius < 0.0f) { playerX = playerRadius; }
		if (playerX + playerRadius > 1280.0f) { playerX = 1280.0f - playerRadius; }
		if (playerY - playerRadius < 0.0f) { playerY = playerRadius; }
		if (playerY + playerRadius > 720.0f) { playerY = 720.0f - playerRadius; }

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		Novice::DrawBox(
			(int)screenShakeX,
			(int)screenShakeY,
			1200,
			700,
			0,
			BLACK,
			kFillModeSolid
		);
		// 0. ダッシュ予測ガイド線
		if (!isAttacking && dashCount < kMaxDashes) {
			float targetX = playerX + cosf(playerAngle) * dashDistance;
			float targetY = playerY + sinf(playerAngle) * dashDistance;
			Novice::DrawLine((int)playerX, (int)playerY, (int)targetX, (int)targetY, 0xFFFFFF55);
		}

		// 1. ダッシュ軌道線
		for (int i = 0; i < lineCount; i++) {
			if (lines[i].isExist) {
				Novice::DrawLine((int)lines[i].startX, (int)lines[i].startY, (int)lines[i].endX, (int)lines[i].endY, 0x00FFFFFF);
			}
		}

		// 2. 敵をリアルタイム追尾するすべての砲台長方形
		for (int i = 0; i < boxCount; i++) {
			if (boxes[i].isExist && boxes[i].delay == 0) {

				// ボスの方向を向く小さなひし形
				float dx = bossX - boxes[i].x;
				float dy = bossY - boxes[i].y;
				float angle = atan2f(dy, dx);

				// ひし形の大きさ
				float size = 10.0f;

				// 4つの頂点
				int x1 = (int)(boxes[i].x + cosf(angle) * size);
				int y1 = (int)(boxes[i].y + sinf(angle) * size);

				int x2 = (int)(boxes[i].x + cosf(angle + 1.5708f) * size);
				int y2 = (int)(boxes[i].y + sinf(angle + 1.5708f) * size);

				int x3 = (int)(boxes[i].x + cosf(angle + 3.14159f) * size);
				int y3 = (int)(boxes[i].y + sinf(angle + 3.14159f) * size);

				int x4 = (int)(boxes[i].x + cosf(angle - 1.5708f) * size);
				int y4 = (int)(boxes[i].y + sinf(angle - 1.5708f) * size);

				// ひし形を描画
				Novice::DrawTriangle(x1, y1, x2, y2, x3, y3, boxes[i].color, kFillModeSolid);
				Novice::DrawTriangle(x1, y1, x3, y3, x4, y4, boxes[i].color, kFillModeSolid);
			}
		}

		// 3. ビーム弾の描画
		for (int i = 0; i < kMaxBoxes; i++) {
			if (beams[i].isShot) {

				// 後ろに残る残像
				for (int j = 7; j >= 0; j--) {

					int size = 3 + (7 - j);

					Novice::DrawEllipse(
						(int)beams[i].trailX[j],
						(int)beams[i].trailY[j],
						size,
						size,
						0.0f,
						0x00FFFFFF,
						kFillModeSolid
					);
				}

				// 弾の外側
				Novice::DrawEllipse(
					(int)beams[i].x,
					(int)beams[i].y,
					12,
					12,
					0.0f,
					0x00FFFFFF,
					kFillModeSolid
				);

				// 弾の中心
				Novice::DrawEllipse(
					(int)beams[i].x,
					(int)beams[i].y,
					6,
					6,
					0.0f,
					WHITE,
					kFillModeSolid
				);
			}
		}

		// ボス
		unsigned int bossColor = RED;

		// 突進前は白くなる
		if (bossState == 1) {
			bossColor = WHITE;
		}

		// 突進中は白くなる
		if (bossState == 2) {
			bossColor = WHITE;
		}

		Novice::DrawEllipse(
			(int)bossX,
			(int)bossY,
			(int)bossRadius,
			(int)bossRadius,
			0.0f,
			bossColor,
			kFillModeSolid
		);

		// ダッシュ残像描画
		for (int i = 0; i < kMaxAfterImages; i++) {

			if (afterImages[i].isExist) {

				Novice::DrawEllipse(
					(int)afterImages[i].x,
					(int)afterImages[i].y,
					(int)(playerRadius * 0.8f),
					(int)(playerRadius * 0.8f),
					0.0f,
					0x11AAFFFF,
					kFillModeSolid
				);
			}
		}

		Novice::DrawSprite(
			(int)(playerX - 52),
			(int)(playerY - 50),
			bokasiTexture,
			0.15f,
			0.15f,
			0.0f,
			0x330033FF
		);

		// プレイヤー（常に自由に移動可能）
		Novice::DrawSprite(
			(int)(playerX - 32),
			(int)(playerY - 32),
			hosiTexture,
			1.0f,
			1.0f,
			0.0f,
			WHITE
		);

		// 火花パーティクル描画
		for (int i = 0; i < kMaxSparks; i++) {

			if (sparks[i].isExist) {

				int size = 1 + sparks[i].life / 10;

				Novice::DrawBox(
					(int)sparks[i].x,
					(int)sparks[i].y,
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
		Novice::DrawLine((int)playerX, (int)playerY, (int)lookX, (int)lookY, WHITE);

		// UI表示
		Novice::ScreenPrintf(10, 10, "Move : WASD (Free Movement)");
		Novice::ScreenPrintf(10, 30, "Dash : SPACE");
		Novice::ScreenPrintf(10, 50, "Dash Count : %d / %d", dashCount, kMaxDashes);

		///
		/// ↑描画処理ここまで
		///

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}
