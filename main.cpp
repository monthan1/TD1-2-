#include <Novice.h>
#include <math.h>

const char kWindowTitle[] = "TD1_2回目";

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

// ガスターブラスター風極太レーザー構造体
struct BlasterBeam {
	float startX;
	float startY;
	float angle;
	int timer; // 照射持続時間
	int isShot;
};

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力受け取り用の配列
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// ボスのデータ
	float bossX = 640.0f;
	float bossY = 180.0f;
	float bossRadius = 60.0f;
	float bossSpeedX = 4.0f;

	// プレイヤーのデータ
	float playerX = 640.0f;
	float playerY = 550.0f;
	float playerRadius = 30.0f;
	float playerAngle = -1.5708f; // 向き（初期値：上向き）

	// 移動・ダッシュ設定
	float speed = 4.0f;
	float dashDistance = 200.0f;

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

	// ダッシュ軌道線データ
	const int kMaxLines = 100;
	DashLine lines[kMaxLines];
	int lineCount = 0;

	// サンズ風ビームデータ
	BlasterBeam beams[kMaxBoxes];
	for (int i = 0; i < kMaxBoxes; i++) {
		boxes[i].isExist = 0;
		boxes[i].delay = 0;
		beams[i].isShot = 0;
		beams[i].timer = 0;
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

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		// --- 1. ボスの移動 ---
		bossX += bossSpeedX;
		if (bossX - bossRadius <= 0.0f || bossX + bossRadius >= 1280.0f) {
			bossSpeedX *= -1.0f;
		}

		// --- 2. クールダウンタイマーの減衰 ---
		if (dashCooldown > 0) { dashCooldown--; }

		// --- 3. 入力方向の取得 & 常時自由移動（ロック解除） ---
		float moveX = 0.0f;
		float moveY = 0.0f;

		if (keys[DIK_W]) moveY -= 1.0f;
		if (keys[DIK_S]) moveY += 1.0f;
		if (keys[DIK_A]) moveX -= 1.0f;
		if (keys[DIK_D]) moveX += 1.0f;

		// WASD入力があれば向きを更新＆プレイヤーを動かす
		if (moveX != 0.0f || moveY != 0.0f) {
			playerAngle = atan2f(moveY, moveX);

			// ベクトルの正規化
			float len = sqrtf(moveX * moveX + moveY * moveY);
			if (len > 0.0f) {
				playerX += (moveX / len) * speed;
				playerY += (moveY / len) * speed;
			}
		}

		// --- 4. ダッシュ実行（移動制限なし・いつでもダッシュ可能） ---
		if (dashCooldown == 0 && !isAttacking && (keys[DIK_SPACE] && !preKeys[DIK_SPACE]) && dashCount < kMaxDashes) {
			float startX = playerX;
			float startY = playerY;

			float dashDirX = cosf(playerAngle);
			float dashDirY = sinf(playerAngle);

			playerX += dashDirX * dashDistance;
			playerY += dashDirY * dashDistance;

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
					boxes[boxCount].x = startX + (playerX - startX) * ratio;
					boxes[boxCount].y = startY + (playerY - startY) * ratio;
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
				lines[lineCount].endX = playerX;
				lines[lineCount].endY = playerY;
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

						float dx = bossX - startX;
						float dy = bossY - startY;

						beams[currentAttackIndex].startX = startX;
						beams[currentAttackIndex].startY = startY;
						beams[currentAttackIndex].angle = atan2f(dy, dx) - 1.5708f;
						beams[currentAttackIndex].timer = 8; // ビームの持続フレーム数
						beams[currentAttackIndex].isShot = 1;

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

		// ビームタイマー更新
		for (int i = 0; i < kMaxBoxes; i++) {
			if (beams[i].isShot) {
				beams[i].timer--;
				if (beams[i].timer <= 0) {
					beams[i].isShot = 0;
				}
			}
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
				float dx = bossX - boxes[i].x;
				float dy = bossY - boxes[i].y;
				float angle = atan2f(dy, dx) - 1.5708f;

				// 外枠（黒枠）
				Novice::DrawBox(
					(int)boxes[i].x - (boxes[i].width / 2) - 2,
					(int)boxes[i].y - 2,
					boxes[i].width + 4,
					boxes[i].height + 4,
					angle,
					0x000000FF,
					kFillModeSolid
				);

				// 本体
				Novice::DrawBox(
					(int)boxes[i].x - (boxes[i].width / 2),
					(int)boxes[i].y,
					boxes[i].width,
					boxes[i].height,
					angle,
					boxes[i].color,
					kFillModeSolid
				);
			}
		}

		// 3. サンズ風「ガスターブラスター極太レーザー」の多層描画
		for (int i = 0; i < kMaxBoxes; i++) {
			if (beams[i].isShot) {
				// 最外枠：シアンオーラ
				Novice::DrawBox(
					(int)beams[i].startX - 14,
					(int)beams[i].startY,
					28,
					1200,
					beams[i].angle,
					0x00FFFFFF,
					kFillModeSolid
				);

				// 中間層：黒い輪郭
				Novice::DrawBox(
					(int)beams[i].startX - 9,
					(int)beams[i].startY,
					18,
					1200,
					beams[i].angle,
					0x000000FF,
					kFillModeSolid
				);

				// 中心コア：ホワイトビーム
				Novice::DrawBox(
					(int)beams[i].startX - 5,
					(int)beams[i].startY,
					10,
					1200,
					beams[i].angle,
					0xFFFFFFFF,
					kFillModeSolid
				);
			}
		}

		// ボス（赤い丸）
		Novice::DrawEllipse((int)bossX, (int)bossY, (int)bossRadius, (int)bossRadius, 0.0f, RED, kFillModeSolid);

		// プレイヤー（常に自由に移動可能）
		Novice::DrawEllipse((int)playerX, (int)playerY, (int)playerRadius, (int)playerRadius, 0.0f, BLUE, kFillModeSolid);

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