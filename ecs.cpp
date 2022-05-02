#include <iostream>
#include <vector>
#include "ecs.h"

flecs::world *currentEcs = new flecs::world();
flecs::world &ecs = *currentEcs;
std::vector<flecs::entity> theFuckingList;
std::vector<flecs::entity> theFuckingList2;
flecs::entity playerEntity;
HudInfo hudInfo;
int numChmmr = 1;
float distChmmr = 100.0f;
float chmmrSpeed = 3.14f / 3.0f;

void create_chmmr(const flecs::world &world);
HudInfo *init_ecs(Camera2D *camera)
{
  // lol shitty pre frame hack
  ecs.system<Position>()
    .term<PlayerControl>().oper(flecs::Not)
    .term<Shot>().oper(flecs::Not)
    .term<Chmmr>().oper(flecs::Not)
    .term<GravityWell>().oper(flecs::Not)
    .each([](flecs::entity entity, Position &position) {
      theFuckingList.push_back(entity);
      theFuckingList2.push_back(entity);
    });

  ecs.system<UpgradeChmmr>()
    .iter([] (flecs::iter iter, UpgradeChmmr *upgradeChmmr) {
        for (auto i : iter) {
          iter.entity(i).remove<UpgradeChmmr>();
        }
        iter.world().delete_with<Chmmr>();

        if (upgradeChmmr[0].type == 1) {
          numChmmr++;
          create_chmmr(iter.world());
        } else {
          chmmrSpeed *= 1.3f;
          create_chmmr(iter.world());
        }

    });

 ecs.system<Position, const Velocity>()
    .each([](Position& p, const Velocity& v) {
      p.x += v.x;
      p.y += v.y;
    });
 ecs.system<const Follower, Velocity, const Position>()
    .each([](const Follower &f, Velocity &v, const Position &p) {
        auto flecsEntity = ecs.entity(f.follow);
        auto followPosition = flecsEntity.get<Position>();

        Vector2 vToFollow = Vector2Subtract(
            (Vector2) { followPosition->x, followPosition->y },
            (Vector2) { p.x, p.y }
        );
        Vector2 maxVel = Vector2Scale(Vector2Normalize(vToFollow),2.0f);
        //float angle = Vector2Angle(
            //(Vector2) { 1.0f, 0.0f },
            //Vector2Normalize(maxVel)
        //);

        v.x = maxVel.x;
        v.y = maxVel.y;
        //p->rotation = angle;

    });

 ecs.system<PhysicsBodyComponent, const Velocity>()
    .each([](PhysicsBodyComponent& p, const Velocity& v) {
      p.body->force = (Vector2){v.x,v.y};
    });
  ecs.system<const sRectangle, const Position>()
    .each([](const sRectangle &r, const Position& p) {
      DrawRectangle(p.x,p.y, 50, 50, GREEN);
    });
  ecs.observer<PhysicsBodyComponent,Position>()
    .event(flecs::OnSet)
    .each([](flecs::entity e, PhysicsBodyComponent& p, Position &pp) {
        const Position *posi = e.get<Position>();
        std::cout << "set physics: " << p.density << std::endl;
        std::cout << "pp " << pp.x << " " << pp.y << std::endl;
        p.body = CreatePhysicsBodyRectangle((Vector2){ pp.x, pp.y }, 40 * p.size, 40 * p.size, p.density);
        p.body->useGravity = false;
    });

  ecs.system<const PhysicsBodyComponent, Position>()
    .each([](const PhysicsBodyComponent &pc, Position& p) {
        p.x = pc.body->position.x;
        p.y = pc.body->position.y;
        p.rotation = pc.body->orient;
    });

  ecs.system<const Position, const sTriangle>()
    .each([](const Position &position, const sTriangle &triangle) {
      Vector2 v1, v2, v3;
      float playerAngle = position.rotation;
      v1 = Vector2Rotate((Vector2){
        40.0f * triangle.size,
        0.0f
      }, playerAngle);
      v2 = Vector2Rotate((Vector2){
        - 40.0f * triangle.size,
        - 40.0f * triangle.size,
        //0,0
      }, playerAngle);
      v3 = Vector2Rotate((Vector2){
        -40.0f * triangle.size,
        40.0f * triangle.size
        //500,500
      }, playerAngle);
      DrawTriangle(
          (Vector2){
          v1.x + position.x,
          v1.y + position.y},
          (Vector2){v2.x + position.x,
          v2.y + position.y},
          (Vector2){v3.x + position.x,
          v3.y + position.y},
          triangle.color);

    });
  ecs.system<const Position, const sCircle>()
    .each([](const Position &position, const sCircle &circle) {
      DrawCircle(position.x, position.y, circle.size, circle.color);
    });


  ecs.system<const PhysicsBodyComponent, PlayerControl>()
    .each([](flecs::entity player, const PhysicsBodyComponent &bodyComponent, PlayerControl &playerControl) {
        hudInfo.health = playerControl.health;
        hudInfo.maxHealth = playerControl.maxHealth;
        hudInfo.score = playerControl.score;
        hudInfo.maxScore = playerControl.maxScore;
        hudInfo.level = playerControl.level;
        hudInfo.scoreMultiplier = playerControl.scoreMultiplier;

        auto playerBody = bodyComponent.body;
        Vector2 p = playerBody->position;
        float orientation = playerBody->orient;
        if (IsKeyDown(KEY_W))
        {
          float orientation = playerBody->orient;
          Vector2 force = { cos(orientation) * playerControl.force, sin(orientation) * playerControl.force };
          PhysicsAddForce(playerBody, force);

        }
        else if (IsKeyDown(KEY_S))
        {
          float orientation = playerBody->orient;
          Vector2 force = { cos(orientation) * -playerControl.force, sin(orientation) * -playerControl.force };
          PhysicsAddForce(playerBody, force);
        }

        if (Vector2Length(playerBody->velocity) > playerControl.maxVel)
        {
          Vector2 maxVel = Vector2Scale(Vector2Normalize(playerBody->velocity),playerControl.maxVel);
          playerBody->velocity = maxVel;
        }
        /*
        if (Vector2Length(playerBody->force) > playerControl.maxFore)
        {
          Vector2 force = Vector2Normalize(playerBody->force);
          force = Vector2Scale(force, playerControl.maxFore);
          PhysicsAddForce(playerBody, force);
        }
        */

        if (IsKeyDown(KEY_A))
        {
          playerBody->orient -= playerControl.turn;
          // playerBody->orient = fmod(playerBody->orient, 2 * PI);
          // clamp to closest 16th of a circle
          //PhysicsAddTorque(playerBody, -5000.0f);
        }
        else if (IsKeyDown(KEY_D))
        {
          playerBody->orient += playerControl.turn;
          //PhysicsAddTorque(playerBody, 5000.0f);
        }

        for (auto ent : theFuckingList2) {
          auto position = ent.get<Position>();
          float distance = Vector2Distance(playerBody->position, Vector2({position->x,position->y}));
          if (distance < 30.0f) {
            playerControl.health -= 1;
            ent.destruct();
            if (playerControl.health <= 0) {
              playerControl.health = 0;
              hudInfo.state = GUI_STATE_GAMEOVER;
            }
          }
        }

        if (hudInfo.doUpgrade) {
          //(*hudInfo.toUpgrade)(player);
          hudInfo.toUpgrade(player);
          hudInfo.doUpgrade = false;
        }


    });

  ecs.system<const CameraFollow, const PhysicsBodyComponent>()
    .each([camera](const CameraFollow &cameraFollow, const PhysicsBodyComponent &physics) {
      camera->target = physics.body->position;
      //camera->rotation = -3.14/2.0f +RAD2DEG*physics.body->orient;
    });

  ecs.system<Shooter, const PhysicsBodyComponent>()
    .iter([](flecs::iter &iter, Shooter *shooter, const PhysicsBodyComponent *physicsBody) {
        shooter->cooldown -= iter.delta_system_time();
        if (shooter->cooldown < 0) {
          shooter->cooldown = shooter->cooldown_max + shooter->cooldown;

          iter.world().defer([&iter, &shooter, &physicsBody]() {
            auto bullet = iter.world().entity();

            float angle = physicsBody->body->orient;
            // find closest enemy
            /*
            float closestDistance = 0;
            for (auto ent : theFuckingList2) {
              auto position = ent.get<Position>();
              float distance = Vector2Distance(physicsBody->body->position, Vector2({position->x,position->y}));
              if (distance < closestDistance) {
                closestDistance = distance;
                Vector2 diff = Vector2Subtract(Vector2({position->x,position->y}), physicsBody->body->position);
                angle = Vector2Angle(
                      (Vector2) { 1.0f, 0.0f },
                      diff);
              }
            }
            */
            bullet.set([physicsBody,shooter,angle](Velocity &velocity) {
              Vector2 unit = {1,0};
              Vector2 force = Vector2Rotate(unit, angle);
              Vector2 maxVel = Vector2Scale(Vector2Normalize(force),shooter->speed);
              maxVel.x += physicsBody->body->velocity.x;
              maxVel.y += physicsBody->body->velocity.y;
              velocity.x = maxVel.x;
              velocity.y = maxVel.y;
            });

            bullet.set([shooter](Shot &shot) {
                shot.speed = shooter->speed;
                shot.lifetime = shooter->lifetime;
                shot.length = 20;
            });

            bullet.set([](sTriangle& t) {
                t.color = BLUE;
                t.size = 0.2f;
            });
            bullet.set([physicsBody,angle](Position &pos)  {
                pos.x = physicsBody->body->position.x;
                pos.y = physicsBody->body->position.y;
                pos.rotation = angle;
            });
            
          });

        }

     });

  ecs.system<Shot>()
    .iter([](flecs::iter &iter, Shot *shot) {
      auto const dt = ecs.delta_time();
      for (auto i : iter) {
        shot[i].lifetime -= dt;
        if (shot[i].lifetime < 0) {
          auto ent = iter.entity(i);
          ent.destruct();
        } else {
        }
      }
    });

  ecs.system<Shot,Position,sTriangle>()
    .iter([](flecs::iter &iter, Shot *shotList, const Position *mePosList, sTriangle *triangleList) {
        for (auto i :iter) {
          auto me = iter.entity(i);
          auto shot = shotList[i];
          auto mePos = mePosList[i];
          auto meTriangle = triangleList[i];
          for (auto other : theFuckingList) {
                if (other == me) continue;
                auto otherPos = other.get<Position>();
                auto dx = fabs(mePos.x - otherPos->x);
                auto dy = fabs(mePos.y - otherPos->y);

                if (dx < shot.length && dy < shot.length) {
                  other.destruct();

                  std::cout << "shot length1" << shot.length << std::endl;
                  shot.length -= 10.0f;
                  std::cout << "shot length2" << shot.length << std::endl;
                  if (shot.length <= 0) {
                    me.destruct();
                  }
                  meTriangle.size *= 0.5f;
                  meTriangle.color = BLACK;
                  me.set<sTriangle>(meTriangle);
                  me.modified<sTriangle>();
                  me.modified<Shot>();

                  PlayerControl newPlayerControl = *playerEntity.get<PlayerControl>();
                  newPlayerControl.score += 1 * (newPlayerControl.scoreMultiplier);
                  if (newPlayerControl.score >= newPlayerControl.maxScore) {
                    newPlayerControl.maxScore += 5;
                    newPlayerControl.score = 0.0f;
                    hudInfo.state = GUI_STATE_UPGRADE;

                    hudInfo.upgrades[0].name = "SHOT CD";
                    hudInfo.upgrades[0].execute = [](flecs::entity player) {
                      std::cout << "upgrade SHOT CD" << std::endl;
                      Shooter shooter = *playerEntity.get<Shooter>();
                      //auto bullet = iter.world().entity();
                      shooter.cooldown_max *= 0.75f;
                      player.set<Shooter>(shooter);
                    };

                    hudInfo.upgrades[1].name = "SHOT SPD";
                    hudInfo.upgrades[1].execute = [](flecs::entity player) {
                      std::cout << "upgrade SHOT SPD" << std::endl;
                      Shooter shooter = *playerEntity.get<Shooter>();
                      //auto bullet = iter.world().entity();
                      shooter.speed *= 1.5f;
                      player.set<Shooter>(shooter);

                    };

                    hudInfo.upgrades[2].name = "CHMMR SPD";
                    hudInfo.upgrades[2].execute = [](flecs::entity player) {
                      player.set<UpgradeChmmr>({0});
                    };
                    hudInfo.upgrades[3].name = "CHMMR CNT";
                    hudInfo.upgrades[3].execute = [](flecs::entity player) {
                      player.set<UpgradeChmmr>({1});
                    };
                  }
                  iter.world().defer([iter,newPlayerControl]() {
                    playerEntity.mut(iter).set<PlayerControl>(newPlayerControl);
                  });

                }
          }
        }
    });

  ecs.system<Position,Chmmr>()
    .each([](flecs::entity entity, Position &pos, Chmmr &chmmr) {
      auto flecsEntity = ecs.entity(chmmr.parent);
      auto parentPos = flecsEntity.get<Position>();
      //std::cout << "parentPos " << parentPos->x << " " << parentPos->y << std::endl;

      chmmr.angle += chmmr.rotateSpeed * ecs.delta_time();
      pos.x = parentPos->x + cos(chmmr.angle) * chmmr.radius;
      pos.y = parentPos->y + sin(chmmr.angle) * chmmr.radius;
      //std::cout << "position " << pos.x << " " << pos.y << std::endl;

      entity.set<Chmmr>(chmmr);
      entity.set<Position>(pos);

      for (auto other : theFuckingList2) {
        auto otherPos = other.get<Position>();
        auto dist = sqrt(pow(pos.x - otherPos->x,2) + pow(pos.y - otherPos->y,2));
        if (dist < chmmr.size + 40) {
          other.destruct();
          // increase score
          PlayerControl newPlayerControl = *playerEntity.get<PlayerControl>();
          newPlayerControl.score += 1 * (newPlayerControl.scoreMultiplier);
        }
      }

    });

  ecs.system<FighterWave>()
    .iter([](flecs::iter &iter, FighterWave *waveList) {

        for (auto i : iter) {
          auto wave = waveList[i];
          wave.time -= ecs.delta_time();
          auto player = wave.player;

          if (wave.time < 0) {
            iter.world().defer([&]() {
              auto playerPos = player.get<Position>();
              std::cout << "create wave" << std::endl;
              int num = wave.numFighters;
              float dist = 500.0f;

              for (int i = 0; i < num; i++) {
              float angle = (float)i / (float)num * 2.0f * PI;
              auto fighter = iter.world().entity();
              fighter.set([playerPos,player,angle,dist](Position& p, Velocity& v, sTriangle& t, Follower &follow) {
                  p = {
                    playerPos->x + cos(angle) * dist,
                    playerPos->y + sin(angle) * dist
                  };
                  v = {0, 0};
                  t.color = RED;
                  t.size = 0.5f;

                  follow.follow = player;
                  });
              }

              wave.time = wave.nextCooldown;
              wave.nextCooldown *= 0.9f;
              wave.numFighters++;
            });
          }
          iter.entity(i).set<FighterWave>(wave);

        }
    });

  // draw a circle
  ecs.system<const Position,const GravityWell,const sCircle>()
    .each([](flecs::entity entity, const Position &p, const GravityWell &gw, const sCircle &circle) {
        DrawCircle(p.x, p.y, gw.size, circle.color);
    });

  // apply a force from the gravity well to the player
  ecs.system<const Position,GravityWell>()
    .each([](flecs::entity entity, const Position &p, const GravityWell &gw) {
        for (auto ent : theFuckingList2) {
          float dist = sqrt(pow(p.x - ent.get<Position>()->x,2) + pow(p.y - ent.get<Position>()->y,2));

          // work out affect on the entity
          float force = (gw.size * gw.density) / (dist * dist);

          // work out the angle
          float angle = atan2(p.y - ent.get<Position>()->y, p.x - ent.get<Position>()->x);

          // add the force to the velocity
          auto velocity = ent.get<Velocity>();
          if (velocity == nullptr) {
            continue;
          }
          //std::cout << "velocity " << velocity->x << " " << velocity->y << std::endl;
          Velocity newVelocity = {
            velocity->x + cos(angle) * force * 5,
            velocity->y + sin(angle) * force * 5
          };
          ent.set<Velocity>(newVelocity);
          //std::cout << "velocity after " << newVelocity.x << " " << newVelocity.y << std::endl;
          ent.modified<Velocity>();

        }
    });

  ecs.system<const Position,GravityWell>()
    .each([](flecs::entity entity, const Position &p, const GravityWell &gw) {
        auto ent = playerEntity;
        float dist = sqrt(pow(p.x - ent.get<Position>()->x,2) + pow(p.y - ent.get<Position>()->y,2));
        if (dist < 50.0f) {
        dist = 50.0f;
        }

        // work out affect on the entity
        float force = (gw.size * gw.density) / (dist * dist);

        // work out the angle
        float angle = atan2(p.y - ent.get<Position>()->y, p.x - ent.get<Position>()->x);

        // add the force to the velocity
        Vector2 newForce = {
          cos(angle) * force * 3,
          sin(angle) * force * 3
        };

        /*
        float maxForce = 2.0f;
        if (Vector2Length(newForce) > maxForce) {
          newForce = Vector2Scale(Vector2Normalize(newForce), maxForce);
        }
        */

        PhysicsAddForce(playerEntity.get<PhysicsBodyComponent>()->body, newForce);

    });


  // hack post-frame
  ecs.system<PlayerControl>()
    .each([](PlayerControl &c) {
        theFuckingList.clear();
        theFuckingList2.clear();
      });

  /*
  ecs.system<PhysicsBodyComponent &>()
    .each([](PhysicsBodyComponent &physics) {
        auto pos = physics.body->position;
        // wrap around solar system -100 to 100
        if (pos.x < -100) {
          pos.x = 100;
        std::cout << "WRAP! " << std::endl;
        PhysicsShatter(physics.body, (Vector2){0,0}, 100.0f);
        }
        if (pos.x > 100) {
          pos.x = -100;
        std::cout << "WRAP! " << std::endl;
        PhysicsShatter(physics.body, (Vector2){0,0}, 100.0f);
        }
        if (pos.y < -100) {
          pos.y = 100;
        std::cout << "WRAP! " << std::endl;
        PhysicsShatter(physics.body, (Vector2){0,0}, 100.0f);
        }
        if (pos.y > 100) {
          pos.y = -100;
        std::cout << "WRAP! " << std::endl;
        PhysicsShatter(physics.body, (Vector2){0,0}, 100.0f);
        }
    });
    */

  setup_scene();

  return &hudInfo;
}

void setup_scene()
{
  int numPlanets = 100;
  /*
  for (int i = 0; i < numPlanets; i++) {
    auto planet = ecs.entity();
    planet.set<Position>({
      (float)rand() / (float)RAND_MAX * 5000.0f - 2500.0f,
      (float)rand() / (float)RAND_MAX * 5000.0f - 2500.0f
      });
    float size = (float)rand() / (float)RAND_MAX * 200.0f + 30.f;
    planet.set<GravityWell>({
      (float)rand() / (float)RAND_MAX * 200.0f + 30.f,
      size
    });
    planet.set<sCircle>({
      BLUE,
      size
    });
  }
  */
  int numCols = 10;
  for (int i = 0; i < 20; i++) {
    auto planet = ecs.entity();
    //space evenly across
    float x = (float)i / (float)numCols * 5000.0f - 2500.0f;
    // odd / even rows
    float y = (i % 2 == 0) ? -300.0f : 300.0f;
    planet.set<Position>({x,y});
    planet.set<GravityWell>({100,100});
    planet.set<sCircle>({BLUE,100});
  }
  /*
  auto planet1 = ecs.entity();
  planet1.set<Position>({0,0});
  planet1.set<GravityWell>({100,100});
  planet1.set<sCircle>({BLUE,100});
  */

  //auto planet2 = ecs.entity();
  //planet2.set<Position>({400,400});
  //planet2.set<GravityWell>({100,100});
  //planet2.set<sCircle>({BLUE,100});

  auto player = ecs.entity();
  playerEntity = player;
  player.set([](Position& p, PhysicsBodyComponent& physics, PlayerControl &playerControl, sTriangle &triangle, CameraFollow &cameraFollow, Shooter &shooter) {
    p = {200, 200};
    cameraFollow = {20,20};
    physics.density = 0.2f;
    physics.size = 1.0f;
    playerControl.force = 2.4f;
    playerControl.turn = 0.1f;
    playerControl.maxVel = 0.1f;
    playerControl.maxFore = 10.0f;
    playerControl.health = 5;
    playerControl.maxHealth = 5;

    playerControl.score = 0.0f;
    playerControl.maxScore = 1.0f;
    playerControl.level = 1;
    playerControl.scoreMultiplier = 1.0f;

    triangle.color = ORANGE;
    triangle.size = 1.0f;
    shooter.cooldown = 0.1f;
    shooter.cooldown_max = 0.5f;
    shooter.speed = 5.0f;
    shooter.lifetime = 1.0f;
  });

  auto wave = ecs.entity();
  wave.set([player](FighterWave &f) {
    f.numFighters = 4;
    f.time = 1.0f;
    f.nextCooldown = 7.0f;
    f.player = player;
  });

  /*
  int num = 5;
  float dist = 100.0f;
  for (int i = 0; i < num; i++) {
    float angle = (float)i / (float)num * 2.0f * PI;
    auto fighter = ecs.entity();
    fighter.set([player,angle,dist](Position& p, Velocity& v, sTriangle& t, Follower &follow) {
      p = {600 + cos(angle) * dist, sin(angle) * dist+100};
      v = {0, 0};
      t.color = RED;
      t.size = 0.5f;

      follow.follow = player;
    });
  }
  */

  // chmmrs
  create_chmmr(ecs);
}

void create_chmmr(const flecs::world &world)
{
  for (int i = 0; i < numChmmr; i++) {
    float angle = (float)i / (float)numChmmr * 2.0f * PI;
    auto chmmr = world.entity();
    chmmr.set([angle](Position& p, sCircle &c, Chmmr &chmmr) {
      p = {600 + cos(angle) * distChmmr, sin(angle) * distChmmr+100};
      c.color = ORANGE;

      c.size = 20.0f;
      chmmr.size = 20.0f;

      chmmr.parent = playerEntity;
      chmmr.radius = distChmmr;
      chmmr.angle = angle;
      chmmr.rotateSpeed = chmmrSpeed;
    });
 }
}
void reset_ecs()
{
  ecs.delete_with<Position>();
  ecs.delete_with<FighterWave>();
}


void run_ecs()
{
  ecs.set_automerge(true);
  //ecs.frame_begin();
  //ecs.staging_begin();
  //ecs.defer_begin();
  ecs.progress();
  //ecs.staging_end();
  //ecs.frame_end();
  //ecs.merge();
  //ecs.defer_end();
}
