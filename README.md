# 🚀 Space Defender

A 2D arcade space shooter built in C using OpenGL/GLUT. Shoot down incoming asteroids, survive as long as possible, and beat your high score.

---

## 📸 Preview

> Twinkling star field · rotating planet · pulsing sun · DDA laser beams · Bresenham asteroid outlines · scaling explosion effects

---

## 🎮 Controls

| Key | Action |
|-----|--------|
| `A` / `←` | Move left |
| `D` / `→` | Move right |
| `Space` | Fire laser |
| `Enter` | Start / Restart |
| `Q` | Quit |

---

## ⚙️ Algorithms Used

| Algorithm | Applied To |
|-----------|-----------|
| DDA Line | Laser beams |
| Bresenham's Line | Asteroid outlines |
| Midpoint Circle | Sun, planet, highlights |
| 2D Transformations | Ship tilt, planet rotation, explosion scale |

---

## 🛠️ Build & Run

**Linux**
```bash
gcc space_defender.c -o space_defender -lGL -lGLU -lglut -lm
./space_defender
```

**macOS**
```bash
gcc space_defender.c -o space_defender -framework OpenGL -framework GLUT -lm
./space_defender
```

**Windows (MinGW)**
```bash
gcc space_defender.c -o space_defender.exe -lfreeglut -lopengl32 -lglu32 -lm
```

### Dependencies

- OpenGL
- GLU
- GLUT / FreeGLUT

**Install on Ubuntu/Debian:**
```bash
sudo apt install freeglut3-dev
```

---

## 🧩 Features

- Scrolling animated star field with twinkle effect
- Rotating ringed planet and pulsing sun
- Irregular asteroid shapes with rotation
- DDA-rendered laser with glowing core
- Explosion animations via scaling transformation
- HUD: score, high score, level, lives
- Menu screen and game over screen
- Progressive difficulty (speed + spawn rate)

---

## 📁 Structure

```
space_defender.c   # Single-file source — everything included
README.md
```

---