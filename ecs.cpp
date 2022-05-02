#include <iostream>
#include "ecs.h"

flecs::world ecs;

void init_ecs(Camera2D *camera)
{
 ecs.system<Position, const Velocity>()
    .each([](Position& p, const Velocity& v) {
      p.x += v.x;
      p.y += v.y;
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
          std::cout << "shoot" << std::endl;
          shooter->cooldown = shooter->cooldown_max + shooter->cooldown;

          auto bullet = iter.world().entity();

          bullet.set([physicsBody,shooter](Velocity &velocity) {
              Vector2 unit = {1,0};
            Vector2 force = Vector2Rotate(unit, physicsBody->body->orient);
            Vector2 maxVel = Vector2Scale(Vector2Normalize(force),shooter->speed);
            velocity.x = maxVel.x;
            velocity.y = maxVel.y;
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

        }
        
     });



  auto entity1 = ecs.entity()
    .set([](Position& p, Velocity& v) {
      p = {10, 20};
      v = {1, 2};
    });
  auto bullet = ecs.entity();
  bullet.set([](Shot& s) {
    s.x = 0;
    s.y = 0;
    s.length = 10;
    s.angle = 0;
    s.speed = 10;
    s.lifetime = 0;
  });
  bullet.set([](Position& p) {
    p.x = 0;
    p.y = 0;
  });
  //bullet.set([](Velocity& v) {
    //v.x = 0;
    //v.y = -5;
  //});
  bullet.set([](sTriangle& r) {
      r.color = GREEN;
  });
  bullet.set([](PhysicsBodyComponent& p) {
    p.density = 99;
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
    shooter.cooldown = 1.0f;
    shooter.cooldown_max = 0.1f;
    shooter.speed = 20.0f;
    shooter.lifetime = 5.0f;
  });


}

void run_ecs()
{
  ecs.progress(0);
}
