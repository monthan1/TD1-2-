#include <Novice.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <Xinput.h>
#include "Controller.h"

const char kWindowTitle[] = "TD1_2回目!!";

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

// 衝撃波構造体
struct ShockWave
{
    float x;
    float y;
    float radius;
    float alpha;
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

    Novice::Initialize(kWindowTitle, 1280, 720);

    Controller::Initialize();

    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    int bokasiTexture = Novice::LoadTexture("./Resources/bokasi.png");
    int hosiTexture = Novice::LoadTexture("./Resources/hosi.png");

    // ボスのデータ
    float bossX = 640.0f;
    float bossY = 180.0f;
    float bossRadius = 60.0f;
    float bossSpeedX = 4.0f;
    int bossTrailTimer = 0;

    int bossState = 0;
    int bossAttackTimer = 180;
    float bossAuraRadius = 0.0f;
    float bossAuraAlpha = 0.0f;
    float bossChargeVelocityX = 0.0f;
    float bossChargeVelocityY = 0.0f;
    float bossRotationAngle = 0.0f;
    int bossSpinTimer = 0;

    // プレイヤーのデータ
    float playerX = 640.0f;
    float playerY = 550.0f;
    float playerRadius = 30.0f;
    float playerAngle = -1.5708f;

    float speed = 8.0f;
    float dashDistance = 200.0f;

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

    AfterImage afterImages[kMaxAfterImages];

    // ボス残像発生タイマー
    int afterImageTimer = 0;

    // 火花パーティクル設定
    const int kMaxSparks = 200;
    Spark sparks[kMaxSparks];

    // 衝撃波設定
    const int kMaxShockWaves = 20;
    ShockWave shockWaves[kMaxShockWaves];

    // 放射状ヒットエフェクト設定
    const int kMaxHitBursts = 20;
    HitBurst hitBursts[kMaxHitBursts];

    // ボス爆発エフェクト設定
    const int kMaxBossExplosions = 100;
    BossExplosion bossExplosions[kMaxBossExplosions];

    // ボス爆発パーティクル設定
    const int kMaxBossExplosionParticles = 300;
    BossExplosionParticle bossExplosionParticles[kMaxBossExplosionParticles];

    // 爆弾の設定
    const int kMaxBombs = 30;
    Bomb bombs[kMaxBombs];
    int bombDropTimer = 0;

    // グリッチエフェクトの設定
    const int kMaxGlitchParticles = 200;
    GlitchParticle glitchParticles[kMaxGlitchParticles];

    // 画面シェイク
    int screenShakeTimer = 0;
    int vibrationTimer = 0;
    int screenShakeX = 0;
    int screenShakeY = 0;

    // 初期化
    for (int shockInitIndex = 0; shockInitIndex < kMaxShockWaves; shockInitIndex++) {
        shockWaves[shockInitIndex].isExist = 0;
    }

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
    BoxData boxes[kMaxBoxes];
    int boxCount = 0;

    Diamond diamonds[kMaxBoxes];

    BlasterBeam beams[kMaxBoxes];
    LaserBeam lasers[kMaxBoxes];

    const int kMaxLines = 100;
    DashLine lines[kMaxLines];
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
        lasers[boxInitIndex].timer = 0;
        lasers[boxInitIndex].sparkMade = 0;
        lasers[boxInitIndex].shakeMade = 0;
    }

    for (int lineInitIndex = 0; lineInitIndex < kMaxLines; lineInitIndex++) {
        lines[lineInitIndex].isExist = 0;
    }

    int isAttacking = 0;
    int attackTimer = 0;
    int currentAttackIndex = 0;

    int attackType = 1;

    while (Novice::ProcessMessage() == 0) {

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

        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        // --- 1. ボスの移動・突進処理 ---

        if (bossState == 0) {

            bossX += bossSpeedX;

            if (bossX - bossRadius <= 0.0f ||
                bossX + bossRadius >= 1280.0f) {
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
                bossX + bossRadius > 1280.0f ||
                bossY - bossRadius < 0.0f ||
                bossY + bossRadius > 720.0f ||
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
        if (dashCooldown == 0 && !isAttacking &&
            (Controller::IsPressed(kPadButton10) ||
                (keys[DIK_SPACE] && !preKeys[DIK_SPACE])) &&
            dashCount < kMaxDashes) {

            float startX = playerX;
            float startY = playerY;

            float dashDirX = cosf(playerAngle);
            float dashDirY = sinf(playerAngle);

            dashStartX = startX;
            dashStartY = startY;

            dashTargetX = startX + dashDirX * dashDistance;
            dashTargetY = startY + dashDirY * dashDistance;

            if (dashTargetX < playerRadius) dashTargetX = playerRadius;
            if (dashTargetX > 1280.0f - playerRadius) dashTargetX = 1280.0f - playerRadius;
            if (dashTargetY < playerRadius) dashTargetY = playerRadius;
            if (dashTargetY > 720.0f - playerRadius) dashTargetY = 720.0f - playerRadius;

            dashMoveTimer = kDashMoveFrames;
            dashStunTimer = kDashStunMax;

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
                isAttacking = 1;
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
                            lasers[currentAttackIndex].timer = 0;
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

        // 衝撃波の更新処理
        for (int shockUpdateIndex = 0; shockUpdateIndex < kMaxShockWaves; shockUpdateIndex++) {
            if (shockWaves[shockUpdateIndex].isExist) {
                shockWaves[shockUpdateIndex].radius += 5.0f;
                shockWaves[shockUpdateIndex].alpha -= 0.025f;

                if (shockWaves[shockUpdateIndex].alpha <= 0.0f) {
                    shockWaves[shockUpdateIndex].isExist = 0;
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

                                float sparkAngle = (float)p / 25.0f * 6.28318f;
                                float particleSpeed = 3.0f + (float)(rand() % 70) / 10.0f;

                                sparks[sparkSpawnIndex].x = bossX;
                                sparks[sparkSpawnIndex].y = bossY;

                                sparks[sparkSpawnIndex].velocityX = cosf(sparkAngle) * particleSpeed;
                                sparks[sparkSpawnIndex].velocityY = sinf(sparkAngle) * particleSpeed;

                                sparks[sparkSpawnIndex].life = 40;
                                sparks[sparkSpawnIndex].isExist = 1;

                                break;
                            }
                        }
                    }
                }

                if (beams[beamUpdateIndex].x < -50 || beams[beamUpdateIndex].x > 1330 ||
                    beams[beamUpdateIndex].y < -50 || beams[beamUpdateIndex].y > 770) {
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

                                float sparkAngle = (float)p / 25.0f * 6.28318f;
                                float particleSpeed = 3.0f + (float)(rand() % 70) / 10.0f;

                                sparks[laserSparkIndex].x = bossX;
                                sparks[laserSparkIndex].y = bossY;

                                sparks[laserSparkIndex].velocityX = cosf(sparkAngle) * particleSpeed;
                                sparks[laserSparkIndex].velocityY = sinf(sparkAngle) * particleSpeed;

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
        if (playerX + playerRadius > 1280.0f) playerX = 1280.0f - playerRadius;
        if (playerY - playerRadius < 0.0f) playerY = playerRadius;
        if (playerY + playerRadius > 720.0f) playerY = 720.0f - playerRadius;

        // =========================
        // 描画処理
        // =========================

        Novice::DrawBox(0, 0, 1280, 720, 0, BLACK, kFillModeSolid);

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

                    // 爆弾の外側
                    Novice::DrawEllipse(
                        (int)x,
                        (int)y,
                        23,
                        23,
                        0.0f,
                        0x555555FF,
                        kFillModeSolid
                    );

                    // 爆弾の本体
                    Novice::DrawEllipse(
                        (int)x,
                        (int)y,
                        18,
                        18,
                        0.0f,
                        0x171717FF,
                        kFillModeSolid
                    );

                    // ひび割れ
                    Novice::DrawLine(
                        (int)x - 13, (int)y - 10,
                        (int)x - 4, (int)y - 3,
                        0xFF5500FF
                    );

                    Novice::DrawLine(
                        (int)x - 4, (int)y - 3,
                        (int)x + 2, (int)y - 11,
                        0xFFAA00FF
                    );

                    Novice::DrawLine(
                        (int)x + 2, (int)y - 11,
                        (int)x + 7, (int)y - 3,
                        0xFF5500FF
                    );

                    Novice::DrawLine(
                        (int)x + 7, (int)y - 3,
                        (int)x + 1, (int)y + 5,
                        0xFFAA00FF
                    );

                    Novice::DrawLine(
                        (int)x + 1, (int)y + 5,
                        (int)x + 9, (int)y + 12,
                        0xFF5500FF
                    );

                    Novice::DrawLine(
                        (int)x + 1, (int)y + 5,
                        (int)x - 9, (int)y + 11,
                        0xFFAA00FF
                    );

                    // ひびの先端から飛び出す光
                    float lightLength = 22.0f;

                    if (flash == 0) {
                        lightLength = 38.0f;
                    }

                    // 左上のひびから出る光
                    Novice::DrawLine(
                        (int)x - 4,
                        (int)y - 3,
                        (int)((int)x - 4 - (int)lightLength * 0.5f),
                        (int)y - 3 - (int)lightLength,
                        WHITE
                    );

                    Novice::DrawLine(
                        (int)x - 4,
                        (int)y - 3,
                        (int)x - 4 - (int)lightLength,
                        (int)y - 3 - (int)lightLength,
                        0xFFAA00FF
                    );

                    Novice::DrawLine(
                        (int)x - 4,
                        (int)y - 3,
                        (int)(x - 4 - lightLength * 0.8f),
                        (int)(y - 3 - lightLength * 0.35f),
                        0xFF6600FF
                    );

                    // 右上のひびから出る光jya
                    Novice::DrawLine(
                        (int)x + 7,
                        (int)y - 3,
                        (int)x + 7 + (int)lightLength,
                        (int)y - 3 - (int)lightLength,
                        WHITE
                    );

                    Novice::DrawLine(
                        (int)x + 7,
                        (int)y - 3,
                        (int)x + 7 + (int)lightLength,
                        (int)((int)y - 3 - (int)lightLength * 0.4f),
                        0xFFAA00FF
                    );

                    Novice::DrawLine(
                        (int)x + 7,
                        (int)y - 3,
                        (int)(x + 7 + lightLength * 0.4f),
                        (int)(y - 3 - lightLength * 0.85f),
                        0xFF6600FF
                    );

                    // 下側のひびから出る光
                    Novice::DrawLine(
                        (int)x + 1,
                        (int)y + 5,
                        (int)x + 1 + (int)lightLength,
                        (int)y + 5 + (int)lightLength,
                        WHITE
                    );

                    Novice::DrawLine(
                        (int)x + 1,
                        (int)y + 5,
                        (int)((int)x - (int)lightLength * 0.3f),
                        (int)y + 5 + (int)lightLength,
                        0xFFAA00FF
                    );

                    Novice::DrawLine(
                        (int)x + 1,
                        (int)y + 5,
                        (int)(x + 1 + lightLength * 0.8f),
                        (int)(y + 5 + lightLength * 0.45f),
                        0xFF6600FF
                    );

                    int lightSize = 2;
                    if (flash == 0) {
                        lightSize = 4;
                    }

                    Novice::DrawEllipse(
                        (int)(x - 4 - lightLength * 0.5f),
                        (int)(y - 3 - lightLength),
                        lightSize,
                        lightSize,
                        0.0f,
                        WHITE,
                        kFillModeSolid
                    );

                    Novice::DrawEllipse(
                        (int)(x + 7 + lightLength),
                        (int)(y - 3 - lightLength),
                        lightSize,
                        lightSize,
                        0.0f,
                        WHITE,
                        kFillModeSolid
                    );

                    Novice::DrawEllipse(
                        (int)(x + 1 + lightLength),
                        (int)(y + 5 + lightLength),
                        lightSize,
                        lightSize,
                        0.0f,
                        WHITE,
                        kFillModeSolid
                    );

                    // ひびの中心の光
                    int centerLightSize = 5;
                    if (flash == 0) {
                        centerLightSize = 8;
                    }

                    Novice::DrawEllipse(
                        (int)x,
                        (int)y,
                        centerLightSize,
                        centerLightSize,
                        0.0f,
                        WHITE,
                        kFillModeSolid
                    );

                    Novice::DrawEllipse(
                        (int)x,
                        (int)y,
                        13,
                        13,
                        0.0f,
                        0xFF6600FF,
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

                    // 爆発の輪郭を一瞬だけ表示
                    if (bombs[bombDrawIndex].life > 5) {
                        int glitchRadius = (12 - bombs[bombDrawIndex].life) * 16 + 35;
                        Novice::DrawEllipse(
                            (int)x,
                            (int)y,
                            glitchRadius,
                            glitchRadius,
                            0.0f,
                            0x00FFFFFF,
                            kFillModeWireFrame
                        );
                    }
                }
                else {

                    float ratio = (float)bombs[bombDrawIndex].life / 25.0f;
                    int radius = (int)((25.0f - bombs[bombDrawIndex].life) * 5.0f + 10.0f);
                    float pulse = sinf((float)bombs[bombDrawIndex].life * 0.7f);

                    for (int smokeIndex = 0; smokeIndex < 9; smokeIndex++) {

                        float smokeAngle = (float)smokeIndex * 6.28318f / 9.0f;
                        float smokeDistance = radius * 0.58f;
                        int smokeSize = (int)(7.0f + (1.0f - ratio) * 7.0f);

                        Novice::DrawEllipse(
                            (int)(x + cosf(smokeAngle) * smokeDistance),
                            (int)(y + sinf(smokeAngle) * smokeDistance),
                            smokeSize,
                            smokeSize,
                            0.0f,
                            0x555555FF,
                            kFillModeSolid
                        );
                    }

                    // 爆発の外側の光
                    for (int flameIndex = 0; flameIndex < 18; flameIndex++) {

                        float angle1 = (float)flameIndex * 6.28318f / 18.0f;
                        float angle2 = (float)(flameIndex + 1) * 6.28318f / 18.0f;

                        float spike1 = 0.72f;
                        float spike2 = 0.72f;
                        if (flameIndex % 2 == 0) {
                            spike1 = 1.18f;
                        }
                        if ((flameIndex + 1) % 2 == 0) {
                            spike2 = 1.18f;
                        }

                        float outerRadius1 = radius * spike1 * (1.0f + pulse * 0.04f);
                        float outerRadius2 = radius * spike2 * (1.0f + pulse * 0.04f);

                        int point1X = (int)(x + cosf(angle1) * outerRadius1);
                        int point1Y = (int)(y + sinf(angle1) * outerRadius1);
                        int point2X = (int)(x + cosf(angle2) * outerRadius2);
                        int point2Y = (int)(y + sinf(angle2) * outerRadius2);

                        Novice::DrawTriangle(
                            (int)x,
                            (int)y,
                            point1X,
                            point1Y,
                            point2X,
                            point2Y,
                            0xCC2200FF,
                            kFillModeSolid
                        );
                    }

                    // 爆発の中間の光
                    for (int flameInnerIndex = 0; flameInnerIndex < 16; flameInnerIndex++) {

                        float angle1 = (float)flameInnerIndex * 6.28318f / 16.0f;
                        float angle2 = (float)(flameInnerIndex + 1) * 6.28318f / 16.0f;

                        float spike1 = 0.78f;
                        float spike2 = 0.78f;
                        if (flameInnerIndex % 2 == 0) {
                            spike1 = 1.12f;
                        }
                        if ((flameInnerIndex + 1) % 2 == 0) {
                            spike2 = 1.12f;
                        }

                        float innerRadius1 = radius * 0.78f * spike1;
                        float innerRadius2 = radius * 0.78f * spike2;

                        Novice::DrawTriangle(
                            (int)x,
                            (int)y,
                            (int)(x + cosf(angle1) * innerRadius1),
                            (int)(y + sinf(angle1) * innerRadius1),
                            (int)(x + cosf(angle2) * innerRadius2),
                            (int)(y + sinf(angle2) * innerRadius2),
                            0xFF6600FF,
                            kFillModeSolid
                        );
                    }

                    for (int flameCoreIndex = 0; flameCoreIndex < 14; flameCoreIndex++) {

                        float angle1 = (float)flameCoreIndex * 6.28318f / 14.0f;
                        float angle2 = (float)(flameCoreIndex + 1) * 6.28318f / 14.0f;

                        float coreScale1 = 0.82f;
                        float coreScale2 = 0.82f;
                        if (flameCoreIndex % 2 == 0) {
                            coreScale1 = 1.1f;
                        }
                        if ((flameCoreIndex + 1) % 2 == 0) {
                            coreScale2 = 1.1f;
                        }
                        float coreRadius1 = radius * 0.48f * coreScale1;
                        float coreRadius2 = radius * 0.48f * coreScale2;

                        Novice::DrawTriangle(
                            (int)x,
                            (int)y,
                            (int)(x + cosf(angle1) * coreRadius1),
                            (int)(y + sinf(angle1) * coreRadius1),
                            (int)(x + cosf(angle2) * coreRadius2),
                            (int)(y + sinf(angle2) * coreRadius2),
                            0xFFCC00FF,
                            kFillModeSolid
                        );
                    }

                    // 爆発の中心
                    Novice::DrawEllipse(
                        (int)x,
                        (int)y,
                        radius / 4,
                        radius / 4,
                        0.0f,
                        WHITE,
                        kFillModeSolid
                    );

                    // 爆発から飛び散る光
                    for (int rayIndex = 0; rayIndex < 20; rayIndex++) {

                        float angle = (float)rayIndex * 6.28318f / 20.0f;

                        float innerRadius = radius * 0.65f;
                        float outerRadius = radius + 35.0f * ratio;

                        Novice::DrawLine(
                            (int)(x + cosf(angle) * innerRadius),
                            (int)(y + sinf(angle) * innerRadius),
                            (int)(x + cosf(angle) * outerRadius),
                            (int)(y + sinf(angle) * outerRadius),
                            WHITE
                        );
                    }

                    for (int emberIndex = 0; emberIndex < 12; emberIndex++) {

                        float emberAngle = (float)emberIndex * 6.28318f / 12.0f + (float)bombs[bombDrawIndex].life * 0.08f;
                        float emberDistance = radius * (1.05f + (float)(emberIndex % 3) * 0.12f);
                        int emberSize = 2;
                        if (emberIndex % 3 == 0) {
                            emberSize = 4;
                        }

                        unsigned int emberColor = 0xFFAA00FF;
                        if (emberIndex % 2 == 0) {
                            emberColor = WHITE;
                        }

                        Novice::DrawEllipse(
                            (int)(x + cosf(emberAngle) * emberDistance),
                            (int)(y + sinf(emberAngle) * emberDistance),
                            emberSize,
                            emberSize,
                            0.0f,
                            emberColor,
                            kFillModeSolid
                        );
                    }

                    // 爆発の輪
                    Novice::DrawEllipse(
                        (int)x,
                        (int)y,
                        radius + 20,
                        radius + 20,
                        0.0f,
                        0xFFAA00FF,
                        kFillModeWireFrame
                    );
                }
            }
        }

        // グリッチパーティクル描画
        for (int glitchDrawIndex = 0;
            glitchDrawIndex < kMaxGlitchParticles;
            glitchDrawIndex++) {

            if (glitchParticles[glitchDrawIndex].isExist) {

                Novice::DrawBox(
                    (int)glitchParticles[glitchDrawIndex].x + screenShakeX,
                    (int)glitchParticles[glitchDrawIndex].y + screenShakeY,
                    glitchParticles[glitchDrawIndex].width,
                    glitchParticles[glitchDrawIndex].height,
                    0.0f,
                    glitchParticles[glitchDrawIndex].color,
                    kFillModeSolid
                );
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