#include <stddef.h>
#include "terrain.h"
#include "raymath.h"
#include "stb_ds.h"
// Modified version of RayLib heighmap generation
Mesh terrain_init(Image height_image, game_terrain_map_t *terrain_map)
{
#define GRAY_VALUE(c) ((float)(c.r + c.g + c.b) / 3.0f)

  Mesh mesh = {0};

  int mapX = height_image.width;
  int mapZ = height_image.height;

  const float y_scale = 32.0f / 256.0f;

  Color *pixels = LoadImageColors(height_image);

  // NOTE: One vertex per pixel
  mesh.triangleCount =
      (mapX - 1) * (mapZ - 1) * 2; // One quad every four pixels

  mesh.vertexCount = mesh.triangleCount * 3;

  mesh.vertices = RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
  mesh.normals = RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
  mesh.texcoords = RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
  mesh.colors = NULL;

  int vCounter = 0;  // Used to count vertices float by float
  int tcCounter = 0; // Used to count texcoords float by float
  int nCounter = 0;  // Used to count normals float by float

  Vector3 vA = {0};
  Vector3 vB = {0};
  Vector3 vC = {0};
  Vector3 vN = {0};

  for (int z = 0; z < mapZ - 1; z++)
  {
    for (int x = 0; x < mapX - 1; x++)
    {
      // Fill vertices array with data
      //----------------------------------------------------------

      // one triangle - 3 vertex
      mesh.vertices[vCounter] = (float)x;
      mesh.vertices[vCounter + 1] = GRAY_VALUE(pixels[x + z * mapX]) * y_scale;
      mesh.vertices[vCounter + 2] = (float)z;

      mesh.vertices[vCounter + 3] = (float)x;
      mesh.vertices[vCounter + 4] =
          GRAY_VALUE(pixels[x + (z + 1) * mapX]) * y_scale;
      mesh.vertices[vCounter + 5] = (float)(z + 1);

      mesh.vertices[vCounter + 6] = (float)(x + 1);
      mesh.vertices[vCounter + 7] =
          GRAY_VALUE(pixels[(x + 1) + z * mapX]) * y_scale;
      mesh.vertices[vCounter + 8] = (float)z;
      
      // populate heightMap
      terrain_map->value[x * terrain_map->max_width + z] = GRAY_VALUE(pixels[x + z * mapX]) * y_scale;
      terrain_map->value[x * terrain_map->max_width + (z + 1)] = GRAY_VALUE(pixels[x + (z + 1) * mapX]) * y_scale;
      terrain_map->value[(x + 1) * terrain_map->max_width + z] = GRAY_VALUE(pixels[(x + 1) + z * mapX]) * y_scale;
      terrain_map->value[(x + 1) * terrain_map->max_width + (z + 1)] = GRAY_VALUE(pixels[(x + 1) + (z + 1) * mapX]) * y_scale;

      // Another triangle - 3 vertex
      mesh.vertices[vCounter + 9] = mesh.vertices[vCounter + 6];
      mesh.vertices[vCounter + 10] = mesh.vertices[vCounter + 7];
      mesh.vertices[vCounter + 11] = mesh.vertices[vCounter + 8];

      mesh.vertices[vCounter + 12] = mesh.vertices[vCounter + 3];
      mesh.vertices[vCounter + 13] = mesh.vertices[vCounter + 4];
      mesh.vertices[vCounter + 14] = mesh.vertices[vCounter + 5];

      mesh.vertices[vCounter + 15] = (float)(x + 1);
      mesh.vertices[vCounter + 16] = GRAY_VALUE(pixels[(x + 1) + (z + 1) * mapX]) * y_scale;
      mesh.vertices[vCounter + 17] = (float)(z + 1);
      vCounter += 18; // 6 vertex, 18 floats

      // Fill texcoords array with data
      //--------------------------------------------------------------
      mesh.texcoords[tcCounter] = (float)x / (mapX - 1);
      mesh.texcoords[tcCounter + 1] = (float)z / (mapZ - 1);

      mesh.texcoords[tcCounter + 2] = (float)x / (mapX - 1);
      mesh.texcoords[tcCounter + 3] = (float)(z + 1) / (mapZ - 1);

      mesh.texcoords[tcCounter + 4] = (float)(x + 1) / (mapX - 1);
      mesh.texcoords[tcCounter + 5] = (float)z / (mapZ - 1);

      mesh.texcoords[tcCounter + 6] = mesh.texcoords[tcCounter + 4];
      mesh.texcoords[tcCounter + 7] = mesh.texcoords[tcCounter + 5];

      mesh.texcoords[tcCounter + 8] = mesh.texcoords[tcCounter + 2];
      mesh.texcoords[tcCounter + 9] = mesh.texcoords[tcCounter + 3];

      mesh.texcoords[tcCounter + 10] = (float)(x + 1) / (mapX - 1);
      mesh.texcoords[tcCounter + 11] = (float)(z + 1) / (mapZ - 1);
      tcCounter += 12; // 6 texcoords, 12 floats

      // Fill normals array with data
      //--------------------------------------------------------------
      #if 1
      for (int i = 0; i < 18; i += 9)
      {
        vA.x = mesh.vertices[nCounter + i];
        vA.y = mesh.vertices[nCounter + i + 1];
        vA.z = mesh.vertices[nCounter + i + 2];

        vB.x = mesh.vertices[nCounter + i + 3];
        vB.y = mesh.vertices[nCounter + i + 4];
        vB.z = mesh.vertices[nCounter + i + 5];

        vC.x = mesh.vertices[nCounter + i + 6];
        vC.y = mesh.vertices[nCounter + i + 7];
        vC.z = mesh.vertices[nCounter + i + 8];

        vN = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(vB, vA),
                                                  Vector3Subtract(vC, vA)));

        mesh.normals[nCounter + i] = vN.x;
        mesh.normals[nCounter + i + 1] = vN.y;
        mesh.normals[nCounter + i + 2] = vN.z;

        mesh.normals[nCounter + i + 3] = vN.x;
        mesh.normals[nCounter + i + 4] = vN.y;
        mesh.normals[nCounter + i + 5] = vN.z;

        mesh.normals[nCounter + i + 6] = vN.x;
        mesh.normals[nCounter + i + 7] = vN.y;
        mesh.normals[nCounter + i + 8] = vN.z;
      }
      #endif
      nCounter += 18; // 6 vertex, 18 floats
    }
  }
  #if 0
  typedef struct {
      Vector3 v1;
      Vector3 v2;
      Vector3 v3;
    } Triangle;
  Triangle *triList = NULL;
  Vector3 *normalList = NULL;
  const float smoothAngle = 90.f;
  for (int i = 0; i < mesh.vertexCount; i++)
  {
    Vector3 target_vertex = (Vector3){mesh.vertices[i * 3], mesh.vertices[i * 3 + 1], mesh.vertices[i * 3 + 2]};
    // add triangle that shares vertex to list
    // only get adjacent triangles
    int row = i / mapX / 3;
    int col = i % mapX;
    int n = row * mapX + (col * 3);

    for (int j = i ; j < mesh.triangleCount && j <= n; j++)
    {
      int index1 = j * 3 * 3;
      int index2 = index1 + 3;
      int index3 = index2 + 3;
      Vector3 vA = (Vector3){mesh.vertices[index1], mesh.vertices[index1 + 1], mesh.vertices[index1 + 2]};
      Vector3 vB = (Vector3){mesh.vertices[index2], mesh.vertices[index2 + 1], mesh.vertices[index2 + 2]};
      Vector3 vC = (Vector3){mesh.vertices[index3], mesh.vertices[index3 + 1], mesh.vertices[index3 + 2]};
      // does triangle share the target vertex?
      // can't use indices, need to compare each vertex manually
      if (Vector3Equals(target_vertex, vA) || Vector3Equals(target_vertex, vB) || Vector3Equals(target_vertex, vC))
      {
        arrput(triList, ((Triangle){vA, vB, vC}));
      }
    }
    // go through every triangle in the list and calculate its normal
    for (int j = 0; j < arrlen(triList); j++)
    {
      Triangle tri = triList[j];
      Vector3 normal = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(tri.v2, tri.v1), Vector3Subtract(tri.v3, tri.v1)));
      arrput(normalList, normal);
    }
    Vector3 source_normal = normalList[0];
    Vector3 dest_normal = source_normal;
    for (int j = 1; j < arrlen(normalList); j++)
    {
      for (int n = j + 1; n < arrlen(normalList); n++)
      {
        Vector3 target_normal = normalList[n];
        if (Vector3DotProduct(source_normal, target_normal) < cosf(smoothAngle * DEG2RAD))
        {
          dest_normal = Vector3Add(dest_normal, target_normal);
        }
      }
    }
    Vector3Normalize(dest_normal);
    // add mesh normals here
    mesh.normals[i * 3] = dest_normal.x;
    mesh.normals[i * 3 + 1] = dest_normal.y;
    mesh.normals[i * 3 + 2] = dest_normal.z;
    arrsetlen(triList, 0);
    arrsetlen(normalList, 0);
  }
  arrfree(triList);
  arrfree(normalList);
  #endif

  UnloadImageColors(pixels); // Unload pixels color data

  // Upload vertex data to GPU (static mesh)
  UploadMesh(&mesh, false);

  return mesh;
}

Vector3 terrain_convert_from_world_pos(Vector3 world_pos, game_terrain_map_t *terrain_map)
{
  return (Vector3){.x = world_pos.x + (terrain_map->max_width / 2.0f),
                   .y = world_pos.y,
                   .z = world_pos.z + (terrain_map->max_height / 2.0f)};
}

Vector3 terrain_convert_to_world_pos(Vector3 terrain_pos, game_terrain_map_t *terrain_map)
{
  return (Vector3){.x = terrain_pos.x - (terrain_map->max_width / 2.0f),
                   .y = terrain_pos.y,
                   .z = terrain_pos.z - (terrain_map->max_height / 2.0f)};
}

float terrain_get_adjusted_y(Vector3 world_pos, game_terrain_map_t *terrain_map)
{
  Vector3 terrain_pos = terrain_convert_from_world_pos(world_pos, terrain_map);
  int index_x = floor(terrain_pos.x);
  int index_z = floor(terrain_pos.z);
  if (index_x >= terrain_map->max_width || index_z >= terrain_map->max_height)
  {
    // we are out of bounds
    return 0.0f;
  }
  float xCoord = (terrain_pos.x - index_x);
  float zCoord = (terrain_pos.z - index_z);
  Vector3 a, b, c;    // three vectors constructed around oldPos
  Vector3 barycenter; // u, v, w calculated from a, b, c
  float answer;
  if (xCoord <= 1 - zCoord)
  {
    a = (Vector3){0, terrain_map->value[index_x * terrain_map->max_width + index_z], 0}; // maxWidth used as a stride offset
    b = (Vector3){1, terrain_map->value[(index_x + 1) * terrain_map->max_width + index_z], 0};
    c = (Vector3){0, terrain_map->value[index_x * terrain_map->max_width + (index_z + 1)], 1};
  }
  else
  {
    a = (Vector3){1, terrain_map->value[(index_x + 1) * terrain_map->max_width + (index_z)], 0};
    b = (Vector3){1, terrain_map->value[(index_x + 1) * terrain_map->max_width + (index_z + 1)], 1};
    c = (Vector3){0, terrain_map->value[(index_x) * terrain_map->max_width + (index_z + 1)], 1};
  }
  barycenter = Vector3Barycenter((Vector3){xCoord, terrain_pos.y, zCoord}, a, b, c);
  answer = barycenter.x * a.y + barycenter.y * b.y + barycenter.z * c.y;

  return answer;
}

// Get collision info between ray and terrain
// sourced from  https://iquilezles.org/articles/terrainmarching/
Vector3 terrain_get_ray(Ray ray, game_terrain_map_t *terrain_map, float z_near, float z_far)
{
  // calculations will be done using terrain coordinates until testing collision
  Vector3 ray_pos = terrain_convert_from_world_pos(ray.position, terrain_map);

  // change in direction in 3 axes
  float t_delta = 1.f;
  Vector3 p;
  bool intersection = false;
  for (float t = z_near; t < z_far; t += t_delta)
  {
    p = Vector3Add(ray_pos, Vector3Scale(ray.direction, t));
    // check to ensure no segfaults
    if (p.x < 0 || p.z < 0 || p.x > terrain_map->max_width - 1 || p.z > terrain_map->max_height - 1) {
      return (Vector3){0};
    }
    float heightY = terrain_map->value[(int)floor(p.x) * terrain_map->max_width + (int)floor(p.z)];
    if (p.y < heightY)
    {
      if (!intersection)
      {
        t -= 1.f;
        t_delta = 0.1f;
        intersection = true;
      }
      else 
      {
        return terrain_convert_to_world_pos(p, terrain_map);
      }
    }
  }
  return (Vector3){0};
}