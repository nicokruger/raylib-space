#ifndef MAIN
#include "raylib.h"
#include "raylibextras/physac.h"
#include "vendor/flecs/flecs.h"
#include "raymath.h"
#endif

struct HudInfo {
  float health;
  float maxHealth;
};

struct Position {
  float x, y;
  float rotation;
};

struct Dying {
  float time;
};

struct Velocity {
  float x, y;
};

struct Follower {
  flecs::entity follow;
};

struct Shooter {
  float cooldown;
  float cooldown_max;
  float speed;
  float lifetime;
};
struct Shot {
  float x, y;
  float length;
  float angle;
  float speed;
  float lifetime;
};

struct Chmmr {
  float radius;
  float rotateSpeed;
  float hp;
  float angle;
  float size;
  flecs::entity parent;
};

struct sRectangle {
  int color;
};

struct sTriangle {
  Color color;
  float size;
};

struct sCircle {
  Color color;
  float size;
};

struct PlayerControl {
  int bla;
  float maxFore;
  float force;
  float maxVel;
  float turn;

  float health;
  float maxHealth;

};

struct PhysicsBodyComponent {
  float density;
  float size;
  PhysicsBody body;
};

struct CameraFollow {
  float offsetX;
  float offsetY;
};

struct FighterWave {
  float time;
  float nextCooldown;
  int numFighters;
  float radius;
  flecs::entity player;
};

struct GravityWell {
  float density;
  float size;
};

struct HudInfo *init_ecs(Camera2D *camera);

void run_ecs();
