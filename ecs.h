#ifndef MAIN
#include "raylib.h"
#include "raylibextras/physac.h"
#include "vendor/flecs/flecs.h"
#include "raymath.h"
#include <string>
#endif

#include <functional>

#define GUI_STATE_PLAYING 0
#define GUI_STATE_MENU 1
#define GUI_STATE_GAMEOVER 2
#define GUI_STATE_UPGRADE 3

struct UpgradeChmmr {
  int type;
};

struct Upgrade {
  std::string name;
  std::string description;
  std::function <void(flecs::entity player)> execute;
};

struct HudInfo {
  float health;
  float maxHealth;
  int state;

  float score;
  float maxScore;
  float scoreMultiplier;
  int level;

  Upgrade upgrades[4];

  bool doUpgrade;
  std::function <void(flecs::entity player)> toUpgrade;

  int enemiesCount;
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

  float score;
  float maxScore;
  float scoreMultiplier;
  int level;
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
void setup_scene();
void reset_ecs();
