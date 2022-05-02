#ifndef MAIN
#include "raylib.h"
#include "raylibextras/physac.h"
#include "vendor/flecs/flecs.h"
#include "raymath.h"
#endif

struct Position {
  float x, y;
};

struct Velocity {
  float x, y;
};

struct Shooter {
  float cooldown;
  float cooldown_max;
};
struct Shot {
  float x, y;
  float length;
  float angle;
  float speed;
  float lifetime;
};
struct sRectangle {
  int color;
};

struct sTriangle {
  Color color;
};

struct PlayerControl {
  int bla;
  float maxFore;
  float force;
  float maxVel;
  float turn;
};

struct PhysicsBodyComponent {
  float density;
  PhysicsBody body;
};

struct CameraFollow {
  float offsetX;
  float offsetY;
};

void init_ecs(Camera2D *camera);

void run_ecs();
