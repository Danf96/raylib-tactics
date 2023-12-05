#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "raylib.h"
#include "raymath.h"

#include "terrain.h"



typedef enum {
  ENT_TYPE_EMPTY = 0,
  ENT_TYPE_ACTOR,
} EntityType;

typedef struct {
  short id;
  Vector3 scale;
  Vector3 position;
  Vector3 velocity;
  Vector3 rotation;
  Matrix worldMatrix;
  short int typeHandle;
  short int materialHandle;
  EntityType type;
  bool isDirty;
} Entity;

typedef struct {
  Vector3 scale;
  Vector3 position;
  Vector3 rotation;
  short int typeHandle;
  short int materialHandle;
  EntityType type;
} EntityCreate;

typedef struct {
  size_t capacity;
  size_t size;
  Entity *entities;
} EntityList;


extern Vector2 terrainOffset;

int AddEntity(EntityList *entityList, EntityCreate *entityCreate);

EntityList CreateEntityList(size_t capacity);
void UpdateDirtyEntities(EntityList *entityList, TerrainMap *terrainMap);

