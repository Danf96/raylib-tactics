#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "camera.h"
#include "scene.h"
#include "skybox.h"
#include "terrain.h"
#include "models.h"

#define screenWidth 1280
#define screenHeight 720

char *mouse_strings[] = {"LMB", "RMB", "MMB"};

int main(void)
{
  //--------------------------------------------------------------------------
  // Initialization
  //--------------------------------------------------------------------------
  InitWindow(screenWidth, screenHeight, "raylib - demo");

// textures, models, and shaders
#if 0
  Texture2D bill = LoadTexture("../resources/billboard.png");
  Shader alphaDiscard = LoadShader(NULL, "../shaders/alphaDiscard.fs");
  GenTextureMipmaps(&bill);
  SetTextureWrap(bill, TEXTURE_WRAP_CLAMP);
  SetTextureFilter(bill, TEXTURE_FILTER_ANISOTROPIC_16X);
#endif

  Model skybox = skybox_init("../resources/ACID");

  // terrain
  Image disc_map = LoadImage("../resources/discmap.BMP");
  Texture2D color_map = LoadTexture("../resources/colormap.BMP");
  game_terrain_map_t terrain_map = (game_terrain_map_t){.max_width = disc_map.width, .max_height = disc_map.height};
  terrain_map.value = MemAlloc(sizeof(*terrain_map.value) * disc_map.width * disc_map.height);
  if (!terrain_map.value)
  {
    TraceLog(LOG_ERROR, "Failed to allocate heightmap memory.");
    return EXIT_FAILURE;
  }
  Mesh terrain_mesh = terrain_init(disc_map, &terrain_map);
  Material terrain_material = LoadMaterialDefault();
  terrain_material.maps[MATERIAL_MAP_DIFFUSE].texture = color_map;

  GenTextureMipmaps(&color_map);
  SetTextureWrap(color_map, TEXTURE_WRAP_REPEAT);
  SetTextureFilter(color_map, TEXTURE_FILTER_TRILINEAR);
  SetTextureFilter(color_map, TEXTURE_FILTER_ANISOTROPIC_16X);

  UnloadImage(disc_map);

  // Terrain Matrix;
  Matrix terrain_matrix = MatrixTranslate(-(terrain_map.max_width / 2.0f), 0.0f, -(terrain_map.max_height / 2.0f));

  // Entity List

  game_entity_t *entities = NULL;
  game_entity_create_t new_ent = (game_entity_create_t){
      .scale = (Vector3){1.0f, 1.0f, 1.0f},
      .position = (Vector2){.x = 0, .y = 0.f},
      .offset_y = 0.0f,
      .dimensions = (Vector3){1, 4, 1},
      .dimensions_offset = (Vector3){0, 2, 0},
      .model_path = "../resources/robot.glb",
      .model_anims_path = "../resources/robot.glb",
      .move_speed = 0.1f,
      .attack_radius = 5.f,
      .attack_damage = 25.f,
      .attack_cooldown_max = 1.75f,
      .hit_points = 100.f,
      .team = GAME_TEAM_PLAYER,
      .type = GAME_ENT_TYPE_ACTOR};
  entities = entity_add(entities, &new_ent);
  new_ent.position.x = 0;
  new_ent.position.y = 10;
  new_ent.team = GAME_TEAM_AI;

  entities = entity_add(entities, &new_ent);

  short selected[GAME_MAX_SELECTED];
  memset(selected, -1, sizeof selected);

  SetTargetFPS(200);

  game_camera_t camera = {};
  game_camera_init(&camera, 45.0f, (Vector3){0, 0, 0}, &terrain_map);

  float sim_accumulator = 0;
  float sim_dt = 1.f / 60.f; // how many times a second calculations should be made

  //--------------------------------------------------------------------------
  // Main game loop
  //--------------------------------------------------------------------------
  while (!WindowShouldClose())
  {
    // Detect window close button or ESC key
    //----------------------------------------------------------------------
    // Update
    //----------------------------------------------------------------------
    game_camera_update(&camera, &terrain_map);
    sim_accumulator += GetFrameTime();
    while (sim_accumulator >= sim_dt)
    {
      entity_update_all(&camera, entities, &terrain_map, selected, sim_dt);
      sim_accumulator -= sim_dt;
    }

    //----------------------------------------------------------------------
    // Draw
    //----------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    game_camera_begin_mode_3d(&camera);

    // Draw Terrain
    DrawMesh(terrain_mesh, terrain_material, terrain_matrix);

    // TODO: pass model, view, and perspective matrices to shaders when drawing now (will require custom DrawMesh)
    // load and bind new shaders for animations
    // consolidate model transform matrix and entity matrix (can just add pointer/handle to model for entities now)
    // find way to update animations
    // as of right now, entities will own models, their animations, and anim counts

    // draw entities
    for (size_t i = 0; i < arrlen(entities); i++)
    {
      game_entity_t *ent = &entities[i];
      if (ent->type == GAME_ENT_TYPE_ACTOR)
      {
        entity_draw_actor(&ent->model);
      }
    }
    // draw selection boxes
    for (size_t i = 0; i < GAME_MAX_SELECTED; i++)
    {
      if (selected[i] >= 0)
      {
        short selected_id = selected[i];
        game_entity_t *ent = &entities[selected_id]; // only works right now, will not work if deleting entities is added since selectedId may not necessarily map to an index
        DrawCubeWires(Vector3Add(ent->dimensions_offset, Vector3Transform(Vector3Zero(), ent->model.transform)), ent->dimensions.x, ent->dimensions.y, ent->dimensions.z, MAGENTA);
        DrawCircle3D(ent->position, ent->attack_radius, (Vector3){1, 0, 0}, 90, RED);
      }
    }

    DrawSphere(camera.camera_pos, 0.25f, RED);

#if 0
    BeginShaderMode(alphaDiscard);

    EndShaderMode();
#endif



    // skybox, to be drawn last
    DrawModel(skybox, (Vector3){0, 0, 0}, 1.0f, GREEN);

    game_camera_end_mode_3d();

    DrawFPS(10, 10);
    DrawText(TextFormat("%.4f\n%.4f\n%05.4f",
                        camera.camera_pos.x,
                        camera.camera_pos.y,
                        camera.camera_pos.z),
             10, 30, 20, WHITE);
    if (camera.is_button_pressed)
    {
      DrawText(TextFormat("%s", mouse_strings[camera.mouse_button]), 100, 100, 20, GRAY);
    }

    EndDrawing();
  }

  //--------------------------------------------------------------------------
  // De-Initialization
  //--------------------------------------------------------------------------

  UnloadShader(skybox.materials[0].shader);
  UnloadTexture(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
  UnloadModel(skybox);

  // arrfree(meshes);
  // arrfree(mats);
  UnloadMesh(terrain_mesh);
  UnloadTexture(terrain_material.maps[MATERIAL_MAP_DIFFUSE].texture);
  UnloadMaterial(terrain_material);
  MemFree(terrain_map.value);

  // Free entities here
  entity_unload_all(entities);

  MemFree(terrain_map.value);

  CloseWindow();

  return 0;
}
