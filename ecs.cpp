#include <iostream>
#include <vector>
#include "ecs.h"

flecs::world ecs;
  std::vector<flecs::entity> theFuckingList;

void init_ecs(Camera2D *camera)
{
 ecs.system<Position, const Velocity>()
    .each([](Position& p, const Velocity& v) {
      p.x += v.x;
      p.y += v.y;
    });
 ecs.system<const Follower, Velocity, Position>()
    .each([](const Follower &f, Velocity &v, Position &p) {
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
      DrawTriangle(v1, v2, v3, triangle.color);

    });

  ecs.system<const PhysicsBodyComponent, const PlayerControl>()
    .each([](const PhysicsBodyComponent &bodyComponent, const PlayerControl &playerControl) {
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
        if (Vector2Length(playerBody->force) > playerControl.maxFore)
        {
          printf("SCALE");
          Vector2 force = Vector2Normalize(playerBody->force);
          force = Vector2Scale(force, playerControl.maxFore);
          PhysicsAddForce(playerBody, force);
        }

        if (IsKeyDown(KEY_A))
        {
          playerBody->orient -= playerControl.turn;
          //PhysicsAddTorque(playerBody, -5000.0f);
        }
        else if (IsKeyDown(KEY_D))
        {
          playerBody->orient += playerControl.turn;
          //PhysicsAddTorque(playerBody, 5000.0f);
        }

    });

  ecs.system<const CameraFollow, const PhysicsBodyComponent>()
    .each([camera](const CameraFollow &cameraFollow, const PhysicsBodyComponent &physics) {
      camera->target = physics.body->position;
    });
  ecs.system<Shooter, const PhysicsBodyComponent>()
    .iter([](flecs::iter &iter, Shooter *shooter, const PhysicsBodyComponent *physicsBody) {
        shooter->cooldown -= iter.delta_system_time();
        if (shooter->cooldown < 0) {
          shooter->cooldown = shooter->cooldown_max + shooter->cooldown;

          iter.world().defer([&iter, &shooter, &physicsBody]() {
            std::cout << "create bullet" << std::endl;
            auto bullet = iter.world().entity();

            bullet.set([physicsBody,shooter](Velocity &velocity) {
                Vector2 unit = {1,0};
              Vector2 force = Vector2Rotate(unit, physicsBody->body->orient);
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
            bullet.set([physicsBody](Position &pos)  {
                pos.x = physicsBody->body->position.x;
                pos.y = physicsBody->body->position.y;
                pos.rotation = physicsBody->body->orient;
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

  ecs.system<Position>()
    .term<PlayerControl>().oper(flecs::Not)
    .term<Shot>().oper(flecs::Not)
    .each([](flecs::entity entity, Position &position) {
      theFuckingList.push_back(entity);
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
                  //std::cout << "hit" << std::endl;
                  //iterr.entity(i).set<Dying>({0});
                  //iterr.entity(i).add_if
                }
          }
        }

        theFuckingList.clear();
        std::vector<flecs::entity> toDie;
        for (auto i :iter) {
          auto me = iter.entity(i);
          auto shot = shotList[i];
          auto mePos = mePosList[i];
          //iter.world().defer([&allPositionsQuery,&toDie,shot,me,mePos]() {
          /*
          allPositionsQuery.iter( [&toDie,shot,me,mePos](flecs::iter &iterr, const Position *otherPosList) {

              for (auto i : iterr)
              {
                flecs::entity other = iterr.entity(i);
              }
          //});
              });
              */
        }


        //ecs.defer([&toDie]() {
            //std::cout << "defer stuff lols" << std::endl;
          //for (auto ent : toDie) {
            //std::cout << "delete shot " << ent << std::endl;
            //ent.destruct();
            //std::cout << "deleted shot" << std::endl;
          //}
        //});
        //ecs.defer_begin();
        //for (auto die : toDie) {
          //die.destruct();
          //die.add<Dying>();
        //}
        //ecs.defer_end();
    });



  auto entity1 = ecs.entity()
    .set([](Position& p, Velocity& v) {
      p = {10, 20};
      v = {1, 2};
    });

  auto player = ecs.entity();
  player.set([](Position& p, PhysicsBodyComponent& physics, PlayerControl &playerControl, sTriangle &triangle, CameraFollow &cameraFollow, Shooter &shooter) {
    p = {200, 200};
    cameraFollow = {20,20};
    physics.density = 1.0f;
    physics.size = 1.0f;
    playerControl.force = 4.0f;
    playerControl.turn = 0.1f;
    playerControl.maxVel = 0.1f;
    playerControl.maxFore = 10.0f;
    triangle.color = ORANGE;
    triangle.size = 1.0f;
    shooter.cooldown = 0.1f;
    shooter.cooldown_max = 0.1f;
    shooter.speed = 5.0f;
    shooter.lifetime = 0.3f;
  });

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
