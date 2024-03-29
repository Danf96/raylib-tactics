#pragma once

#include "raylib.h"
#include "stdint.h"
// Based on Jeff M's Raylib extras camera, modified following methods from Game Engine Architecture
// https://github.com/raylib-extras/extras-c/tree/main/cameras

typedef enum
{
  MOVE_FRONT = 0,
  MOVE_BACK,
  MOVE_RIGHT,
  MOVE_LEFT,
  MOVE_UP,
  MOVE_DOWN,
  ROTATE_RIGHT,
  ROTATE_LEFT,
  ROTATE_UP,
  ROTATE_DOWN,
  MODIFIER_ADD,
  MODIFIER_ATTACK,
  LAST_CONTROL
} game_camera_controls;

typedef enum
{
  LEFT_CLICK = 1,
  RIGHT_CLICK,
  LEFT_CLICK_ADD,
  LEFT_CLICK_ATTACK,
  LEFT_CLICK_ADD_GROUP,
  LEFT_CLICK_GROUP,
} game_input_event_type;

typedef struct 
{
  game_input_event_type event_type;
  union 
  {
    Ray mouse_ray;
    Rectangle mouse_rect;
  };
} game_input_event_t;

typedef struct game_camera_t
{
  int controls_keys[LAST_CONTROL];

  uint8_t mouse_states;       // current frame's button states
  uint8_t prev_mouse_states;  // last frame's states
  uint8_t mouse_downs;        // 1 = button pressed this frame
  uint8_t mouse_ups;          // 1 = button released this frame
  Vector2 mouse_old_pos;

  game_input_event_t *input_events;

  // the speed in units/second to move
  Vector3 move_speed;
  Vector2 rotation_speed;

  float mouse_sens;

  float click_timer;

  float min_view_y;
  float max_view_y;

  Vector3 camera_pos;

  float camera_pullback_dist;

  Camera ray_view_cam;

  Vector2 fov;

  Vector2 view_angles;

  bool focused;

  double near_plane;
  double far_plane;

} game_camera_t;

typedef struct game_terrain_map_t game_terrain_map_t;

void game_camera_init(game_camera_t *camera, float fov_y, Vector3 position, game_terrain_map_t *terrain_map);

Vector3 game_camera_get_world_pos(game_camera_t *camera);

// Vector3 game_camera_get_terrain_pos(game_camera_t *camera);

Rectangle game_camera_get_mouse_rect(Vector2 pos_old, Vector2 pos_new);

void game_camera_set_pos(game_camera_t *camera, Vector3 position);

Ray game_camera_get_view_ray(game_camera_t *camera);

void game_camera_update(game_camera_t *camera, game_terrain_map_t *terrain_map);

void game_camera_begin_mode_3d(game_camera_t *camera);

void game_camera_end_mode_3d(void);

void game_camera_input_clear(game_camera_t *camera);

typedef struct shadow_camera_t
{
  Camera ray_view_cam;
  Matrix projection;
  Matrix view;
  double near_plane;
  double far_plane;

} shadow_camera_t;

void shadow_camera_init(shadow_camera_t *camera, Vector3 position, float fovy, float near_plane, float far_plane, CameraProjection projection_type);
void shadow_camera_begin_mode_3d(shadow_camera_t *camera);
void shadow_camera_end_mode_3d(void);

