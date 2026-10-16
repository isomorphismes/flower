#ifndef FLOWER_CAMERA_H
#define FLOWER_CAMERA_H
#include "flower.h"
typedef struct { FlowerPoint position,target,up; } FlowerCamera;
void flower_camera_move(FlowerCamera *camera,float rightward,float forwardward,float upward);
void flower_camera_turn(FlowerCamera *camera,float yaw,float pitch,float roll);
#endif
