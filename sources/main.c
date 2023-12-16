#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "camera.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "scene.h"
#include "skybox.h"
#include "terrain.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define screenWidth 1280
#define screenHeight 720

char *mouseStrings[] = {"LMB", "RMB", "MMB"};


typedef struct {
  short id;
  BoundingBox dimensions;
} BBox;

Vector2 terrainOffset = {0};
TerrainMap terrainMap = {0};

int main(void) {
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


  Model skybox = GetSkybox("../resources/ACID");

  // terrain
  Image discMap = LoadImage("../resources/discmap.BMP");
  Texture2D colorMap = LoadTexture("../resources/colormap.BMP");
  terrainMap = (TerrainMap){.maxWidth = discMap.width, .maxHeight = discMap.height};
  terrainMap.value = MemAlloc(sizeof(*terrainMap.value) * discMap.width * discMap.height);
  if (!terrainMap.value)
  {
    TraceLog(LOG_ERROR, "Failed to allocate heightmap memory.");
    return EXIT_FAILURE;
  }
  Mesh terrainMesh = GenMeshCustomHeightmap(discMap, &terrainMap);
  Material terrainMaterial = LoadMaterialDefault();
  terrainMaterial.maps[MATERIAL_MAP_DIFFUSE].texture = colorMap;
  terrainOffset = (Vector2){.x = terrainMap.maxWidth / 2.0f, .y = terrainMap.maxHeight / 2.0f};

  GenTextureMipmaps(&colorMap);
  SetTextureWrap(colorMap, TEXTURE_WRAP_REPEAT);
  SetTextureFilter(colorMap, TEXTURE_FILTER_TRILINEAR);
  SetTextureFilter(colorMap, TEXTURE_FILTER_ANISOTROPIC_16X);

  UnloadImage(discMap);

  // cube for testing
  Mesh cube = GenMeshCube(1, 1, 1);
  Mesh cone = GenMeshCone(0.5, 1, 5);

  // change to models for animations? look into bones in vertex shader
  Mesh* meshes = NULL;
  arrput(meshes, terrainMesh);
  arrput(meshes, cube);
  arrput(meshes, cone);


  Material *mats = NULL;
  arrput(mats, terrainMaterial);


  // Terrain Matrix;
  Matrix terrMatrix = MatrixTranslate(-(terrainMap.maxWidth / 2.0f), 0.0f, -(terrainMap.maxHeight / 2.0f));

  // Entity List

  Entity *entities = NULL;
  EntityCreate newEnt = (EntityCreate){.scale = (Vector3){1.0f, 1.0f, 1.0f},
                     .position = (Vector2){.x = 0, .y = 0.f},
                     .offsetY = 0.5f,
                     .dimensions = (Vector3){1, 1, 1},
                     .materialHandle = 0,
                     .typeHandle = 1,
                     .moveSpeed = 0.1f,
                     .type = ENT_TYPE_ACTOR};
  entities = AddEntity(entities, &newEnt);
  newEnt.position.x = 0;
  newEnt.position.y = 10;
  newEnt.typeHandle = 2;
  newEnt.rotation.x = 90.f * DEG2RAD;
  
  entities = AddEntity(entities, &newEnt);

  short selected[GAME_MAX_SELECTED];
  memset(selected, -1, sizeof selected);

  SetTargetFPS(200);

  RTSCamera camera;
  RTSCameraInit(&camera, 45.0f, (Vector3){0, 0, 0}, &terrainMap);

  float simAccumulator = 0;
  float simDt = 1.f / 60.f; // how many times a second calculations should be made

  //--------------------------------------------------------------------------
  // Main game loop
  //--------------------------------------------------------------------------
  while (!WindowShouldClose()) 
  {
    // Detect window close button or ESC key
    //----------------------------------------------------------------------
    // Update
    //----------------------------------------------------------------------
    RTSCameraUpdate(&camera, &terrainMap);
    simAccumulator += GetFrameTime();
    while (simAccumulator >= simDt) 
    {
      UpdateEntities(&camera, entities, &terrainMap, selected);
      simAccumulator -= simDt;
    }

    //----------------------------------------------------------------------
    // Draw
    //----------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    RTSCameraBeginMode3D(&camera);

    // Draw Terrain
    DrawMesh(meshes[0], mats[0], terrMatrix);

    // draw entities
    for (size_t i = 0; i < arrlen(entities); i++) 
    {
      Entity *ent = &entities[i];
      if (ent->type == ENT_TYPE_ACTOR) {
        DrawMesh(meshes[ent->typeHandle], mats[ent->materialHandle],
                 ent->worldMatrix);
      }
    }
    // draw selection boxes 
    for (size_t i = 0; i < GAME_MAX_SELECTED; i++)
    {
      if (selected[i] >= 0)
      {
        short selectedId = selected[i];
        Entity *ent = &entities[selectedId]; // only works right now, will not work if deleting entities is added
        DrawCubeWires(Vector3Transform(Vector3Zero() ,ent->worldMatrix), ent->dimensions.x, ent->dimensions.y, ent->dimensions.z, MAGENTA);
      }
    }
    

    DrawSphere(camera.CameraPosition, 0.25f, RED);

    #if 0
    BeginShaderMode(alphaDiscard);

    EndShaderMode();
    #endif

    // red outer circle
    // DrawCircle3D(Vector3Zero(), 2, (Vector3){1, 0, 0}, 90, RED);

    // skybox, to be drawn last
    DrawModel(skybox, (Vector3){0, 0, 0}, 1.0f, GREEN);

    RTSCameraEndMode3D();

    DrawFPS(10, 10);
    DrawText(TextFormat("%.4f\n%.4f\n%05.4f",
                        camera.CameraPosition.x + terrainOffset.x,
                        camera.CameraPosition.y,
                        camera.CameraPosition.z + terrainOffset.y),
             10, 30, 20, WHITE);
    DrawText(TextFormat("%s", mouseStrings[camera.MouseButton]), 100, 100, 20, GRAY);

    EndDrawing();
  }

  //--------------------------------------------------------------------------
  // De-Initialization
  //--------------------------------------------------------------------------

  UnloadShader(skybox.materials[0].shader);
  UnloadTexture(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
  UnloadModel(skybox);

  arrfree(meshes);
  arrfree(mats);

  UnloadTexture(terrainMaterial.maps[MATERIAL_MAP_DIFFUSE].texture);
  // Free entities here
  arrfree(entities);
  # if 0
  UnloadTexture(bill);
  #endif

  CloseWindow();

  return 0;
}
