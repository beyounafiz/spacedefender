#ifdef __APPLE__
  #include <GLUT/glut.h>
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
  #include <GL/glut.h>
  #include <GL/glu.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define PI             3.14159265f
#define MAX_STARS      80
#define MAX_ASTEROIDS  8
#define MAX_LASERS     5
#define MAX_EXPLOSIONS 5
#define WINDOW_W       800
#define WINDOW_H       600

/* ============================================================
 *  STRUCTURES
 * ============================================================ */

typedef struct {
    float x, y;
    float brightness;
    float twinkleSpeed;
    float size;
} Star;

typedef struct {
    float x, y;
    float speed;
    float radius;
    int   active;
    float angle;
    float rotSpeed;
} Asteroid;

typedef struct {
    float x1, y1;
    float x2, y2;
    float speed;
    int   active;
} Laser;

typedef struct {
    float x, y;
    float scale;
    float alpha;
    int   active;
    int   timer;
} Explosion;

/* ============================================================
 *  GLOBAL VARIABLES
 * ============================================================ */

Star      stars[MAX_STARS];
Asteroid  asteroids[MAX_ASTEROIDS];
Laser     lasers[MAX_LASERS];
Explosion explosions[MAX_EXPLOSIONS];

float planetAngle = 0.0f;
float sunPulse    = 1.0f;
int   sunGrowing  = 1;

float shipX     = 400.0f;
float shipY     = 80.0f;
float shipAngle = 0.0f;
int   laserCooldown = 0;

int asteroidTimer    = 0;
int asteroidInterval = 90;

int score      = 0;
int lives      = 3;
int gameState  = 0;   /* 0=menu, 1=playing, 2=gameover */
int frameCount = 0;
int highScore  = 0;
int level      = 1;

/* ============================================================
 *  BACKGROUND
 * ============================================================ */

void drawBackground() {
    glBegin(GL_QUADS);
        glColor3f(0.0f, 0.0f, 0.08f);
        glVertex2f(0, 0);
        glVertex2f(WINDOW_W, 0);
        glColor3f(0.0f, 0.02f, 0.15f);
        glVertex2f(WINDOW_W, WINDOW_H);
        glVertex2f(0, WINDOW_H);
    glEnd();
}

void initStars() {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x            = (float)(rand() % WINDOW_W);
        stars[i].y            = (float)(rand() % WINDOW_H);
        stars[i].brightness   = 0.4f + (float)(rand() % 60) / 100.0f;
        stars[i].twinkleSpeed = 0.01f + (float)(rand() % 30) / 1000.0f;
        stars[i].size         = 1.0f + (float)(rand() % 3);
    }
}

void updateStars() {
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].brightness += stars[i].twinkleSpeed;
        if (stars[i].brightness > 1.0f || stars[i].brightness < 0.3f)
            stars[i].twinkleSpeed = -stars[i].twinkleSpeed;
    }
}

void drawStars() {
    for (int i = 0; i < MAX_STARS; i++) {
        float b = stars[i].brightness;
        glColor3f(b, b, b);
        glPointSize(stars[i].size);
        glBegin(GL_POINTS);
            glVertex2f(stars[i].x, stars[i].y);
        glEnd();
    }
    glPointSize(1.0f);
}

/* ============================================================
 *  MIDPOINT CIRCLE ALGORITHM
 * ============================================================ */

void drawCircle_Midpoint(float cx, float cy, float r,
                         float red, float green, float blue) {
    glColor3f(red, green, blue);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        int ix = (int)r, iy = 0;
        int p  = 1 - (int)r;
        glVertex2f(cx + r, cy);
        while (ix > iy) {
            iy++;
            if (p <= 0) {
                p = p + 2*iy + 1;
            } else {
                ix--;
                p = p + 2*iy - 2*ix + 1;
            }
            glVertex2f(cx + ix, cy + iy);
            glVertex2f(cx + iy, cy + ix);
            glVertex2f(cx - iy, cy + ix);
            glVertex2f(cx - ix, cy + iy);
            glVertex2f(cx - ix, cy - iy);
            glVertex2f(cx - iy, cy - ix);
            glVertex2f(cx + iy, cy - ix);
            glVertex2f(cx + ix, cy - iy);
        }
    glEnd();
}

void drawPlanet(float cx, float cy, float radius) {
    drawCircle_Midpoint(cx, cy, radius, 0.5f, 0.2f, 0.7f);
    drawCircle_Midpoint(cx - radius*0.25f, cy + radius*0.25f,
                        radius*0.35f, 0.65f, 0.35f, 0.85f);
    glColor3f(0.8f, 0.6f, 0.3f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 60; i++) {
            float ang = 2.0f * PI * i / 60.0f;
            glVertex2f(cx + radius*1.8f*cos(ang),
                       cy + radius*0.4f*sin(ang));
        }
    glEnd();
    glLineWidth(1.0f);
}

void drawSun() {
    float cx = 720.0f, cy = 540.0f;
    float r  = 55.0f * sunPulse;

    drawCircle_Midpoint(cx, cy, r*1.6f, 1.0f, 0.6f, 0.0f);
    drawCircle_Midpoint(cx, cy, r,      1.0f, 0.85f, 0.1f);
    drawCircle_Midpoint(cx, cy, r*0.55f,1.0f, 1.0f,  0.6f);

    glColor3f(1.0f, 0.75f, 0.2f);
    glLineWidth(2.0f);
    for (int i = 0; i < 8; i++) {
        float ang = (float)i * PI / 4.0f + (frameCount * 0.005f);
        float x1  = cx + (r + 5.0f)  * cos(ang);
        float y1  = cy + (r + 5.0f)  * sin(ang);
        float x2  = cx + (r + 22.0f) * cos(ang);
        float y2  = cy + (r + 22.0f) * sin(ang);
        glBegin(GL_LINES);
            glVertex2f(x1, y1);
            glVertex2f(x2, y2);
        glEnd();
    }
    glLineWidth(1.0f);
}

void drawRotatingPlanet() {
    glPushMatrix();
        glTranslatef(150.0f, 480.0f, 0.0f);
        glRotatef(planetAngle, 0, 0, 1);
        drawPlanet(0, 0, 45.0f);
    glPopMatrix();
}

/* ============================================================
 *  SPACESHIP
 * ============================================================ */

void drawSpaceship(float x, float y, float angle) {
    glPushMatrix();
        glTranslatef(x, y, 0);
        glRotatef(angle, 0, 0, 1);

        /* Main body */
        glColor3f(0.2f, 0.7f, 1.0f);
        glBegin(GL_POLYGON);
            glVertex2f(0,    28);
            glVertex2f(-14, -20);
            glVertex2f(-8,  -12);
            glVertex2f(0,   -6);
            glVertex2f(8,   -12);
            glVertex2f(14,  -20);
        glEnd();

        /* Cockpit */
        glColor3f(0.8f, 1.0f, 1.0f);
        glBegin(GL_POLYGON);
            glVertex2f(0,  18);
            glVertex2f(-5,  4);
            glVertex2f(0,   0);
            glVertex2f(5,   4);
        glEnd();

        /* Left wing */
        glColor3f(0.1f, 0.5f, 0.9f);
        glBegin(GL_POLYGON);
            glVertex2f(-8,  -12);
            glVertex2f(-26, -24);
            glVertex2f(-20,  -8);
            glVertex2f(-12,  -2);
        glEnd();

        /* Right wing */
        glBegin(GL_POLYGON);
            glVertex2f(8,   -12);
            glVertex2f(26,  -24);
            glVertex2f(20,   -8);
            glVertex2f(12,   -2);
        glEnd();

        /* Engine glow */
        glColor3f(1.0f, 0.5f + 0.3f*(float)sin(frameCount*0.2f), 0.0f);
        glBegin(GL_TRIANGLES);
            glVertex2f(-6,  -20);
            glVertex2f(6,   -20);
            glVertex2f(0,   -34 - 6*(float)sin(frameCount*0.3f));
        glEnd();

    glPopMatrix();
}

/* ============================================================
 *  DDA LINE ALGORITHM - LASER
 * ============================================================ */

void DDA_Line(float x1, float y1, float x2, float y2,
              float r, float g, float b) {
    float dx    = x2 - x1;
    float dy    = y2 - y1;
    float steps = (fabs(dx) >= fabs(dy)) ? fabs(dx) : fabs(dy);
    float xInc  = dx / steps;
    float yInc  = dy / steps;
    float x = x1, y = y1;

    glColor3f(r, g, b);
    glPointSize(2.5f);
    glBegin(GL_POINTS);
        for (int i = 0; i <= (int)steps; i++) {
            glVertex2f(x, y);
            x += xInc;
            y += yInc;
        }
    glEnd();
    glPointSize(1.0f);
}

void fireLaser() {
    if (laserCooldown > 0) return;
    for (int i = 0; i < MAX_LASERS; i++) {
        if (!lasers[i].active) {
            lasers[i].x1     = shipX;
            lasers[i].y1     = shipY + 28.0f;
            lasers[i].x2     = shipX;
            lasers[i].y2     = shipY + 28.0f;
            lasers[i].speed  = 12.0f;
            lasers[i].active = 1;
            laserCooldown    = 8;
            break;
        }
    }
}

void updateLaser() {
    if (laserCooldown > 0) laserCooldown--;
    for (int i = 0; i < MAX_LASERS; i++) {
        if (lasers[i].active) {
            lasers[i].y2 += lasers[i].speed;
            lasers[i].y1 += lasers[i].speed * 0.3f;
            if (lasers[i].y2 > WINDOW_H + 20.0f)
                lasers[i].active = 0;
        }
    }
}

void drawLaser() {
    for (int i = 0; i < MAX_LASERS; i++) {
        if (lasers[i].active) {
            DDA_Line(lasers[i].x1, lasers[i].y1,
                     lasers[i].x2, lasers[i].y2,
                     0.0f, 1.0f, 0.6f);
            DDA_Line(lasers[i].x1, lasers[i].y1,
                     lasers[i].x2, lasers[i].y2 - 3.0f,
                     0.6f, 1.0f, 0.9f);
        }
    }
}

/* ============================================================
 *  BRESENHAM'S LINE ALGORITHM - ASTEROID OUTLINES
 * ============================================================ */

void Bresenham_Line(int x1, int y1, int x2, int y2,
                    float r, float g, float b) {
    int dx  = abs(x2 - x1);
    int dy  = abs(y2 - y1);
    int sx  = (x1 < x2) ? 1 : -1;
    int sy  = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    glColor3f(r, g, b);
    glPointSize(1.5f);
    glBegin(GL_POINTS);
        while (1) {
            glVertex2i(x1, y1);
            if (x1 == x2 && y1 == y2) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x1 += sx; }
            if (e2 <  dx) { err += dx; y1 += sy; }
        }
    glEnd();
    glPointSize(1.0f);
}

void drawAsteroid(Asteroid* a) {
    if (!a->active) return;

    float pts[10][2] = {
        { 1.0f,  0.0f}, { 0.7f,  0.7f}, { 0.0f,  1.0f},
        {-0.6f,  0.8f}, {-1.0f,  0.2f}, {-0.9f, -0.6f},
        {-0.3f, -1.0f}, { 0.5f, -0.9f}, { 0.9f, -0.4f},
        { 1.0f,  0.0f}
    };

    float r    = a->radius;
    float cosA = cos(a->angle * PI / 180.0f);
    float sinA = sin(a->angle * PI / 180.0f);

    /* Filled body */
    glColor3f(0.5f, 0.4f, 0.3f);
    glBegin(GL_POLYGON);
        for (int i = 0; i < 9; i++) {
            float lx = pts[i][0] * r;
            float ly = pts[i][1] * r;
            float rx = lx*cosA - ly*sinA + a->x;
            float ry = lx*sinA + ly*cosA + a->y;
            glVertex2f(rx, ry);
        }
    glEnd();

    /* Surface highlight */
    glColor3f(0.65f, 0.55f, 0.45f);
    glBegin(GL_POLYGON);
        float cx = a->x - r*0.2f;
        float cy = a->y + r*0.2f;
        float pr = r * 0.35f;
        for (int i = 0; i < 12; i++) {
            float ang = 2.0f*PI*i/12.0f;
            glVertex2f(cx + pr*cos(ang), cy + pr*sin(ang));
        }
    glEnd();

    /* Bresenham outline */
    for (int i = 0; i < 9; i++) {
        float lx1 = pts[i][0]   * r;
        float ly1 = pts[i][1]   * r;
        float lx2 = pts[i+1][0] * r;
        float ly2 = pts[i+1][1] * r;
        float rx1 = lx1*cosA - ly1*sinA + a->x;
        float ry1 = lx1*sinA + ly1*cosA + a->y;
        float rx2 = lx2*cosA - ly2*sinA + a->x;
        float ry2 = lx2*sinA + ly2*cosA + a->y;
        Bresenham_Line((int)rx1,(int)ry1,(int)rx2,(int)ry2,
                       0.85f, 0.75f, 0.55f);
    }
}

void initAsteroids() {
    for (int i = 0; i < MAX_ASTEROIDS; i++)
        asteroids[i].active = 0;
}

void spawnAsteroid() {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].active) {
            asteroids[i].x        = 30.0f + (float)(rand() % (WINDOW_W - 60));
            asteroids[i].y        = (float)(WINDOW_H + 40);
            asteroids[i].speed    = 1.5f + (float)(rand()%20)/10.0f + level*0.3f;
            asteroids[i].radius   = 18.0f + (float)(rand() % 20);
            asteroids[i].active   = 1;
            asteroids[i].angle    = 0.0f;
            asteroids[i].rotSpeed = (float)(rand()%4+1) * (rand()%2 ? 1:-1);
            break;
        }
    }
}

void updateAsteroids() {
    asteroidTimer++;
    if (asteroidTimer >= asteroidInterval) {
        spawnAsteroid();
        asteroidTimer = 0;
        if (asteroidInterval > 35) asteroidInterval--;
    }
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) {
            asteroids[i].y     -= asteroids[i].speed;
            asteroids[i].angle += asteroids[i].rotSpeed;
            if (asteroids[i].y < -50.0f)
                asteroids[i].active = 0;
        }
    }
}

/* ============================================================
 *  EXPLOSIONS (SCALING TRANSFORMATION)
 * ============================================================ */

void initExplosions() {
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
        explosions[i].active = 0;
}

void spawnExplosion(float x, float y) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosions[i].active) {
            explosions[i].x      = x;
            explosions[i].y      = y;
            explosions[i].scale  = 0.1f;
            explosions[i].alpha  = 1.0f;
            explosions[i].active = 1;
            explosions[i].timer  = 0;
            break;
        }
    }
}

void drawExplosion(Explosion* e) {
    if (!e->active) return;

    glPushMatrix();
        glTranslatef(e->x, e->y, 0);
        glScalef(e->scale, e->scale, 1);

        glColor4f(1.0f, 0.4f, 0.0f, e->alpha);
        glBegin(GL_TRIANGLE_FAN);
            glVertex2f(0, 0);
            for (int i = 0; i <= 20; i++) {
                float ang = 2.0f * PI * i / 20.0f;
                glVertex2f(80.0f*cos(ang), 80.0f*sin(ang));
            }
        glEnd();

        glColor4f(1.0f, 1.0f, 0.5f, e->alpha);
        glBegin(GL_TRIANGLE_FAN);
            glVertex2f(0, 0);
            for (int i = 0; i <= 20; i++) {
                float ang = 2.0f * PI * i / 20.0f;
                glVertex2f(35.0f*cos(ang), 35.0f*sin(ang));
            }
        glEnd();

        glColor4f(1.0f, 0.9f, 0.2f, e->alpha * 0.8f);
        glLineWidth(1.5f);
        for (int i = 0; i < 8; i++) {
            float ang = (float)i * PI / 4.0f;
            glBegin(GL_LINES);
                glVertex2f(45.0f*cos(ang), 45.0f*sin(ang));
                glVertex2f(90.0f*cos(ang), 90.0f*sin(ang));
            glEnd();
        }
        glLineWidth(1.0f);
    glPopMatrix();
}

void updateExplosions() {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (explosions[i].active) {
            explosions[i].scale += 0.08f;
            explosions[i].alpha -= 0.045f;
            explosions[i].timer++;
            if (explosions[i].alpha <= 0.0f || explosions[i].timer > 22)
                explosions[i].active = 0;
        }
    }
}

/* ============================================================
 *  HUD + TEXT
 * ============================================================ */

void drawText(float x, float y, const char* text, float size,
              float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        if (size > 1.0f)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        else
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

void drawHUD() {
    char buf[64];

    sprintf(buf, "SCORE: %d", score);
    drawText(10, 580, buf, 1.5f, 0.3f, 1.0f, 0.5f);

    sprintf(buf, "BEST: %d", highScore);
    drawText(330, 580, buf, 1.5f, 1.0f, 0.8f, 0.2f);

    sprintf(buf, "LEVEL: %d", level);
    drawText(630, 580, buf, 1.5f, 0.5f, 0.8f, 1.0f);

    drawText(10, 558, "LIVES:", 1.0f, 1.0f, 1.0f, 1.0f);
    for (int i = 0; i < lives; i++) {
        glPushMatrix();
            glTranslatef(80.0f + i*28.0f, 562.0f, 0);
            glScalef(0.55f, 0.55f, 1.0f);
            glColor3f(0.2f, 0.7f, 1.0f);
            glBegin(GL_TRIANGLES);
                glVertex2f(0, 20);
                glVertex2f(-10, -14);
                glVertex2f(10, -14);
            glEnd();
        glPopMatrix();
    }

    glColor3f(0.3f, 0.3f, 0.5f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
        glVertex2f(0, 545);
        glVertex2f(WINDOW_W, 545);
    glEnd();
    glLineWidth(1.0f);
}

/* ============================================================
 *  MENU + GAME OVER
 * ============================================================ */

void drawMenu() {
    glColor4f(0.0f, 0.0f, 0.1f, 0.75f);
    glBegin(GL_QUADS);
        glVertex2f(150,150); glVertex2f(650,150);
        glVertex2f(650,480); glVertex2f(150,480);
    glEnd();

    glColor3f(0.3f, 0.6f, 1.0f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(150,150); glVertex2f(650,150);
        glVertex2f(650,480); glVertex2f(150,480);
    glEnd();
    glLineWidth(1.0f);

    drawText(235, 440, "S P A C E   D E F E N D E R", 2.0f, 0.3f, 0.9f, 1.0f);
    drawText(300, 400, "Arcade Space Shooter",         1.0f, 0.7f, 0.7f, 0.9f);

    drawText(285, 340, "A / D       -  Move Left / Right", 1.0f, 0.9f, 0.9f, 0.5f);
    drawText(285, 315, "SPACE       -  Fire Laser",        1.0f, 0.9f, 0.9f, 0.5f);
    drawText(285, 290, "Arrow Keys  -  Move Left / Right", 1.0f, 0.9f, 0.9f, 0.5f);
    drawText(285, 265, "Q           -  Quit",              1.0f, 0.9f, 0.9f, 0.5f);

    float pulse = 0.5f + 0.5f*(float)sin(frameCount * 0.08f);
    drawText(295, 185, "Press ENTER to Start!", 1.5f, pulse, 1.0f, pulse);
}

void drawGameOver() {
    glColor4f(0.0f, 0.0f, 0.05f, 0.85f);
    glBegin(GL_QUADS);
        glVertex2f(180,180); glVertex2f(620,180);
        glVertex2f(620,430); glVertex2f(180,430);
    glEnd();

    glColor3f(1.0f, 0.2f, 0.2f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(180,180); glVertex2f(620,180);
        glVertex2f(620,430); glVertex2f(180,430);
    glEnd();
    glLineWidth(1.0f);

    drawText(290, 390, "G A M E   O V E R", 2.0f, 1.0f, 0.2f, 0.2f);

    char buf[64];
    sprintf(buf, "Final Score  : %d", score);
    drawText(300, 340, buf, 1.5f, 1.0f, 0.8f, 0.2f);
    sprintf(buf, "Best Score   : %d", highScore);
    drawText(300, 310, buf, 1.5f, 0.5f, 1.0f, 0.5f);
    sprintf(buf, "Level Reached: %d", level);
    drawText(300, 280, buf, 1.5f, 0.5f, 0.8f, 1.0f);

    float pulse = 0.5f + 0.5f*(float)sin(frameCount * 0.1f);
    drawText(255, 225, "Press ENTER to Play Again", 1.5f, pulse, pulse, 1.0f);
    drawText(315, 198, "Press Q to Quit",           1.0f, 0.7f, 0.7f, 0.7f);
}

/* ============================================================
 *  COLLISION DETECTION
 * ============================================================ */

void checkCollisions() {
    for (int a = 0; a < MAX_ASTEROIDS; a++) {
        if (!asteroids[a].active) continue;

        /* Laser vs asteroid */
        for (int l = 0; l < MAX_LASERS; l++) {
            if (!lasers[l].active) continue;
            float dx   = lasers[l].x2 - asteroids[a].x;
            float dy   = lasers[l].y2 - asteroids[a].y;
            float dist = sqrt(dx*dx + dy*dy);
            if (dist < asteroids[a].radius) {
                asteroids[a].active = 0;
                lasers[l].active    = 0;
                spawnExplosion(asteroids[a].x, asteroids[a].y);
                score += 10;
                if (score > highScore) highScore = score;
                if (score % 50 == 0) level++;
                break;
            }
        }

        /* Asteroid reaches ship */
        if (asteroids[a].active &&
            asteroids[a].y < (shipY + 40.0f) &&
            fabs(asteroids[a].x - shipX) < (asteroids[a].radius + 20.0f)) {
            asteroids[a].active = 0;
            spawnExplosion(shipX, shipY);
            lives--;
            if (lives <= 0) gameState = 2;
        }
    }
}

/* ============================================================
 *  RESET GAME
 * ============================================================ */

void resetGame() {
    score            = 0;
    lives            = 3;
    level            = 1;
    asteroidInterval = 90;
    asteroidTimer    = 0;
    laserCooldown    = 0;
    shipX            = 400.0f;
    shipY            = 80.0f;
    shipAngle        = 0.0f;
    for (int i = 0; i < MAX_ASTEROIDS;  i++) asteroids[i].active  = 0;
    for (int i = 0; i < MAX_LASERS;     i++) lasers[i].active     = 0;
    for (int i = 0; i < MAX_EXPLOSIONS; i++) explosions[i].active = 0;
}

/* ============================================================
 *  GLUT CALLBACKS
 * ============================================================ */

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawBackground();
    drawStars();
    drawSun();
    drawRotatingPlanet();

    if (gameState == 1) {
        for (int i = 0; i < MAX_ASTEROIDS;  i++) drawAsteroid(&asteroids[i]);
        drawSpaceship(shipX, shipY, shipAngle);
        drawLaser();
        for (int i = 0; i < MAX_EXPLOSIONS; i++) drawExplosion(&explosions[i]);
        drawHUD();
    } else if (gameState == 0) {
        drawMenu();
    } else if (gameState == 2) {
        for (int i = 0; i < MAX_ASTEROIDS; i++) drawAsteroid(&asteroids[i]);
        drawSpaceship(shipX, shipY, shipAngle);
        drawGameOver();
    }

    glutSwapBuffers();
}

void timerFunc(int value) {
    frameCount++;

    updateStars();

    if (sunGrowing) { sunPulse += 0.003f; if (sunPulse > 1.08f) sunGrowing = 0; }
    else            { sunPulse -= 0.003f; if (sunPulse < 0.92f) sunGrowing = 1; }

    planetAngle += (gameState == 1) ? 0.25f : 0.15f;
    if (planetAngle > 360.0f) planetAngle -= 360.0f;

    if (gameState == 1) {
        updateLaser();
        updateAsteroids();
        updateExplosions();
        checkCollisions();
    }

    glutPostRedisplay();
    glutTimerFunc(16, timerFunc, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'a': case 'A':
            if (gameState == 1) {
                shipX -= 14.0f;
                shipAngle = 12.0f;
                if (shipX < 25.0f) shipX = 25.0f;
            }
            break;
        case 'd': case 'D':
            if (gameState == 1) {
                shipX += 14.0f;
                shipAngle = -12.0f;
                if (shipX > WINDOW_W - 25.0f) shipX = (float)(WINDOW_W - 25);
            }
            break;
        case ' ':
            if (gameState == 1) fireLaser();
            break;
        case 13: /* ENTER */
            if (gameState == 0 || gameState == 2) {
                resetGame();
                gameState = 1;
            }
            break;
        case 'q': case 'Q':
            exit(0);
            break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    if (key=='a' || key=='A' || key=='d' || key=='D')
        shipAngle = 0.0f;
}

void specialKeys(int key, int x, int y) {
    if (gameState != 1) return;
    switch (key) {
        case GLUT_KEY_LEFT:
            shipX -= 14.0f;
            shipAngle = 12.0f;
            if (shipX < 25.0f) shipX = 25.0f;
            break;
        case GLUT_KEY_RIGHT:
            shipX += 14.0f;
            shipAngle = -12.0f;
            if (shipX > WINDOW_W - 25.0f) shipX = (float)(WINDOW_W - 25);
            break;
    }
}

void specialKeysUp(int key, int x, int y) {
    shipAngle = 0.0f;
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_W, 0.0, WINDOW_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/* ============================================================
 *  MAIN
 * ===============================s============================= */

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_W, WINDOW_H);
    glutInitWindowPosition(100, 80);
    glutCreateWindow("Space Defender");

    glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    initStars();
    initAsteroids();
    initExplosions();
    for (int i = 0; i < MAX_LASERS; i++) lasers[i].active = 0;

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeysUp);
    glutTimerFunc(16, timerFunc, 0);

    glutMainLoop();
    return 0;
}
