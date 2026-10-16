#include "flower_camera.h"
#include <math.h>
static FlowerPoint plus(FlowerPoint a,FlowerPoint b) { return (FlowerPoint){a.x+b.x,a.y+b.y,a.z+b.z}; }
static FlowerPoint times(FlowerPoint a,float b) { return (FlowerPoint){a.x*b,a.y*b,a.z*b}; }
static float scalar(FlowerPoint a,FlowerPoint b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
static FlowerPoint outer(FlowerPoint a,FlowerPoint b) { return (FlowerPoint){a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
static FlowerPoint unit(FlowerPoint a) { float length=sqrtf(scalar(a,a)); return length>0?times(a,1/length):(FlowerPoint){0,1,0}; }
static FlowerPoint turn(FlowerPoint a,FlowerPoint axis,float angle) {
    return plus(plus(times(a,cosf(angle)),times(outer(axis,a),sinf(angle))),times(axis,scalar(axis,a)*(1-cosf(angle))));
}
void flower_camera_turn(FlowerCamera *camera,float yaw,float pitch,float roll) {
    FlowerPoint forward=unit(plus(camera->target,times(camera->position,-1))), up=unit(camera->up);
    forward=turn(forward,up,yaw);
    FlowerPoint right=unit(outer(forward,up));
    forward=turn(forward,right,pitch); up=turn(up,right,pitch); up=turn(up,forward,roll);
    right=unit(outer(forward,up)); up=unit(outer(right,forward));
    camera->up=up; camera->target=plus(camera->position,unit(forward));
}
void flower_camera_move(FlowerCamera *camera,float rightward,float forwardward,float upward) {
    FlowerPoint forward=unit(plus(camera->target,times(camera->position,-1)));
    FlowerPoint right=unit(outer(forward,camera->up));
    FlowerPoint movement=plus(plus(times(right,rightward),times(forward,forwardward)),times(camera->up,upward));
    camera->position=plus(camera->position,movement); camera->target=plus(camera->target,movement);
}
