#include "flower.h"
#include "flower_policy.h"
#include "flower_camera.h"
#include "flower_default_policy.h"
#include "raylib.h"
#include "rlgl.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static Flower flower;
static FlowerHold hold;
static FlowerMesh display;
static FlowerPolicy policy;
static int display_density=1;
static bool platform_active=true;
#if defined(PLATFORM_ANDROID)
#include <android_native_app_glue.h>
/* Export present at the pinned raylib 5.5 commit; not assumed across versions. */
extern struct android_app *GetAndroidApp(void);
static void (*previous_command)(struct android_app *,int32_t);
static int32_t (*previous_input)(struct android_app *,AInputEvent *);
static void on_command(struct android_app *app,int32_t command) {
    if (command==APP_CMD_LOST_FOCUS || command==APP_CMD_PAUSE || command==APP_CMD_STOP || command==APP_CMD_TERM_WINDOW) {
        platform_active=false; flower_hold_cancel(&hold);
    }
    if (previous_command) previous_command(app,command);
    if (command==APP_CMD_GAINED_FOCUS) platform_active=true;
}
static int32_t on_input(struct android_app *app,AInputEvent *event) {
    if (AInputEvent_getType(event)==AINPUT_EVENT_TYPE_MOTION) {
        int action=AMotionEvent_getAction(event), kind=action&AMOTION_EVENT_ACTION_MASK;
        if (kind==AMOTION_EVENT_ACTION_CANCEL) flower_hold_cancel(&hold);
        if (kind==AMOTION_EVENT_ACTION_UP || kind==AMOTION_EVENT_ACTION_POINTER_UP) {
            int index=(action&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)>>AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            flower_hold_release(&hold,AMotionEvent_getPointerId(event,index));
        }
    }
    return previous_input? previous_input(app,event):0;
}
static void lifecycle_install(void) {
    struct android_app *app=GetAndroidApp();
    previous_command=app->onAppCmd; previous_input=app->onInputEvent;
    app->onAppCmd=on_command; app->onInputEvent=on_input;
}
#else
static void lifecycle_install(void) {}
#endif

static Vector3 times(Vector3 a,float b) { return (Vector3){a.x*b,a.y*b,a.z*b}; }
static float scalar(Vector3 a,Vector3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
static Vector3 unit(Vector3 a) { float length=sqrtf(scalar(a,a)); return length>0?times(a,1/length):(Vector3){0,1,0}; }
static FlowerPoint point_from_vector(Vector3 p) { return (FlowerPoint){p.x,p.y,p.z}; }
static Vector3 vector_from_point(FlowerPoint p) { return (Vector3){p.x,p.y,p.z}; }
static FlowerCamera camera_state(Camera3D camera) {
    return (FlowerCamera){point_from_vector(camera.position),point_from_vector(camera.target),point_from_vector(camera.up)};
}
static void camera_store(Camera3D *camera,FlowerCamera state) {
    camera->position=vector_from_point(state.position); camera->target=vector_from_point(state.target); camera->up=vector_from_point(state.up);
}
static void camera_turn(Camera3D *camera,float yaw,float pitch,float roll) {
    FlowerCamera state=camera_state(*camera); flower_camera_turn(&state,yaw,pitch,roll); camera_store(camera,state);
}
static void camera_move(Camera3D *camera,float rightward,float forwardward,float upward) {
    FlowerCamera state=camera_state(*camera); flower_camera_move(&state,rightward,forwardward,upward); camera_store(camera,state);
}
static FlowerHit pick(Vector2 point,Camera3D camera) {
    Ray ray=GetScreenToWorldRay(point,camera);
    return flower_mesh_pick(&display,(FlowerPoint){ray.position.x,ray.position.y,ray.position.z},
                      (FlowerPoint){ray.direction.x,ray.direction.y,ray.direction.z});
}

/* One fixed pigment. Brightness is two-sided normal-based inspection lighting.
   Colors are a render cache, not material growth state. No wall-clock uniforms. */
static void mesh_refresh(Mesh *mesh,bool uploaded) {
    flower_mesh_free(&display);
    if (!flower_mesh_build(&flower,display_density,&display)) {
        TraceLog(LOG_FATAL,"Flower display allocation failed"); exit(1);
    }
    Vector3 light=unit((Vector3){-0.3f,0.9f,0.5f});
    for (int index=0;index<display.vertex_count;index++) {
        FlowerPoint position=display.position[index], normal=display.normal[index];
        mesh->vertices[3*index]=position.x; mesh->vertices[3*index+1]=position.y; mesh->vertices[3*index+2]=position.z;
        mesh->normals[3*index]=normal.x; mesh->normals[3*index+1]=normal.y; mesh->normals[3*index+2]=normal.z;
        float brightness=0.24f+0.76f*fabsf(normal.x*light.x+normal.y*light.y+normal.z*light.z);
        mesh->colors[4*index]=(unsigned char)(224*brightness);
        mesh->colors[4*index+1]=(unsigned char)(100*brightness);
        mesh->colors[4*index+2]=(unsigned char)(139*brightness);
        mesh->colors[4*index+3]=255;
    }
    if (uploaded) {
        UpdateMeshBuffer(*mesh,0,mesh->vertices,display.vertex_count*3*(int)sizeof(float),0);
        UpdateMeshBuffer(*mesh,2,mesh->normals,display.vertex_count*3*(int)sizeof(float),0);
        UpdateMeshBuffer(*mesh,3,mesh->colors,display.vertex_count*4,0);
    }
}
static Mesh mesh_create(void) {
    Mesh mesh={0}; mesh.triangleCount=flower.net.cell_count*2*display_density*display_density;
    mesh.vertexCount=mesh.triangleCount*3;
    mesh.vertices=MemAlloc(mesh.vertexCount*3*sizeof(float));
    mesh.normals=MemAlloc(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords=MemAlloc(mesh.vertexCount*2*sizeof(float));
    mesh.colors=MemAlloc(mesh.vertexCount*4);
    if (!mesh.vertices || !mesh.normals || !mesh.texcoords || !mesh.colors) {
        TraceLog(LOG_FATAL,"Flower mesh allocation failed"); exit(1);
    }
    memset(mesh.texcoords,0,mesh.vertexCount*2*sizeof(float));
    mesh_refresh(&mesh,false); UploadMesh(&mesh,true); return mesh;
}

#if defined(PLATFORM_ANDROID)
enum { UNUSED,GROW,LOOK,MOVE,LIFT,LOWER,ROLL_LEFT,ROLL_RIGHT,IGNORE,DENSITY };
typedef struct { int id,role; bool seen; Vector2 start,last; } Contact;
static Contact contacts[8];
static Rectangle pad(int role) {
    float width=(float)GetScreenWidth(),height=(float)GetScreenHeight();
    if (role==MOVE) return (Rectangle){0,height*0.75f,width*0.41f,height*0.25f};
    if (role==LOOK) return (Rectangle){width*0.59f,height*0.75f,width*0.41f,height*0.25f};
    return (Rectangle){width*0.42f,height*(0.75f+0.0625f*(role-LIFT)),width*0.16f,height*0.06f};
}
static int role_at(Vector2 point) {
    if (CheckCollisionPointRec(point,(Rectangle){GetScreenWidth()-140,12,130,56})) return DENSITY;
    for (int role=LOOK;role<=ROLL_RIGHT;role++) if (CheckCollisionPointRec(point,pad(role))) return role;
    return GROW;
}
static float clamp(float value) { return fmaxf(-1,fminf(1,value)); }
static bool touch_frame(Camera3D *camera,float camera_seconds,double growth_seconds,bool focused) {
    int count=GetTouchPointCount();
    if (count>8) { flower_hold_cancel(&hold); return false; }
    for (int index=0;index<8;index++) contacts[index].seen=false;
    if (!focused) { flower_hold_cancel(&hold); memset(contacts,0,sizeof(contacts)); return false; }
    /* Reconcile contact IDs, never array indices. A new press owns its initial role. */
    for (int index=0;index<count;index++) {
        int id=GetTouchPointId(index), slot=-1;
        Vector2 point=GetTouchPosition(index);
        for (int candidate=0;candidate<8;candidate++) if (contacts[candidate].role && contacts[candidate].id==id) slot=candidate;
        bool fresh=slot<0;
        if (fresh) {
            for (int candidate=0;candidate<8;candidate++) if (!contacts[candidate].role) { slot=candidate; break; }
            if (slot<0) { flower_hold_cancel(&hold); continue; }
            contacts[slot]=(Contact){.id=id,.role=role_at(point),.start=point,.last=point};
            if (contacts[slot].role==DENSITY) display_density=3-display_density;
            if (contacts[slot].role==GROW && !flower_hold_begin(&hold,id,true,true,pick(point,*camera))) contacts[slot].role=IGNORE;
        }
        Contact *contact=&contacts[slot]; contact->seen=true;
        if (contact->role==LOOK) camera_turn(camera,-(point.x-contact->last.x)*0.004f,-(point.y-contact->last.y)*0.004f,0);
        if (contact->role==MOVE) camera_move(camera,clamp((point.x-contact->start.x)/70)*camera_seconds,
                                            -clamp((point.y-contact->start.y)/70)*camera_seconds,0);
        if (contact->role==LIFT || contact->role==LOWER) camera_move(camera,0,0,(contact->role==LIFT?1:-1)*camera_seconds);
        if (contact->role==ROLL_LEFT || contact->role==ROLL_RIGHT) camera_turn(camera,0,0,(contact->role==ROLL_LEFT?1:-1)*camera_seconds);
        contact->last=point;
        /* Do not charge elapsed time from before the press to the first held frame. */
        if (fresh && contact->role==GROW) growth_seconds=0;
    }
    for (int index=0;index<8;index++) if (contacts[index].role && !contacts[index].seen) {
        flower_hold_release(&hold,contacts[index].id); contacts[index].role=UNUSED;
    }
    if (count==0) { flower_hold_all_released(&hold); return false; }
    bool changed=false;
    for (int index=0;index<8;index++) if (contacts[index].role==GROW && contacts[index].seen) {
        FlowerHit hit=role_at(contacts[index].last)==GROW?pick(contacts[index].last,*camera):(FlowerHit){0};
        changed=flower_hold_frame_policy(&flower,&hold,contacts[index].id,true,true,hit,growth_seconds,&policy)||changed;
    }
    return changed;
}
static void draw_touch_controls(void) {
    const char *names[]={"","","LOOK","FLY","UP","DOWN","ROLL <","ROLL >"};
    for (int role=LOOK;role<=ROLL_RIGHT;role++) {
        Rectangle rectangle=pad(role);
        DrawRectangleRec(rectangle,(Color){29,32,40,230});
        DrawRectangleLinesEx(rectangle,1,(Color){100,103,112,255});
        int size=role>=LIFT?14:20;
        DrawText(names[role],(int)rectangle.x+8,(int)rectangle.y+8,size,LIGHTGRAY);
    }
}
#endif

int main(int argc,char **argv) {
    (void)argc; (void)argv;
    flower_init(&flower); flower_hold_init(&hold);
    policy=flower_policy_default(); char policy_error[256];
    if (!flower_policy_lua(&policy,flower_default_policy,policy_error,sizeof(policy_error))) {
        fprintf(stderr,"Flower policy failed: %s\n",policy_error); return 1;
    }
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(576,960,"Flower - hold a region to grow");
    SetTargetFPS(60); lifecycle_install();
    Camera3D camera={.position={0,2.8f,3.2f},.target={0,0,0},.up={0,1,0},.fovy=50,.projection=CAMERA_PERSPECTIVE};
    camera_turn(&camera,0,0,0);
    Mesh mesh=mesh_create(); Material material=LoadMaterialDefault();
    while (!WindowShouldClose()) {
        double seconds=GetFrameTime();
        bool focused=platform_active && IsWindowFocused();
        float camera_seconds=isfinite(seconds)?(float)fmax(0,fmin(seconds,0.05)):0;
        bool changed=false;
#if defined(PLATFORM_ANDROID)
        changed=touch_frame(&camera,camera_seconds,seconds,focused);
#else
        if (!focused) flower_hold_cancel(&hold);
        if (focused) {
            float speed=camera_seconds*(IsKeyDown(KEY_LEFT_SHIFT)?3:1);
            camera_move(&camera,(IsKeyDown(KEY_D)-IsKeyDown(KEY_A))*speed,
                        (IsKeyDown(KEY_W)-IsKeyDown(KEY_S))*speed,(IsKeyDown(KEY_E)-IsKeyDown(KEY_Q))*speed);
            camera_turn(&camera,0,0,(IsKeyDown(KEY_Z)-IsKeyDown(KEY_X))*camera_seconds);
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                Vector2 delta=GetMouseDelta(); camera_turn(&camera,-delta.x*0.004f,-delta.y*0.004f,0);
            }
            camera_move(&camera,0,GetMouseWheelMove()*0.25f,0);
        }
        bool down=IsMouseButtonDown(MOUSE_BUTTON_LEFT), pressed=IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        FlowerHit hit=pick(GetMousePosition(),camera);
        bool density_control=CheckCollisionPointRec(GetMousePosition(),(Rectangle){GetScreenWidth()-140,12,130,56});
        if (density_control) hit=(FlowerHit){0};
        if (pressed && focused && density_control) display_density=3-display_density;
        if (!down && focused) flower_hold_all_released(&hold);
        if (pressed) flower_hold_begin(&hold,-2,true,focused,hit);
        changed=flower_hold_frame_policy(&flower,&hold,-2,down,focused,hit,pressed?0:seconds,&policy);
        if (IsKeyPressed(KEY_T)) display_density=3-display_density;
#endif
        if (display.density!=display_density) { UnloadMesh(mesh); mesh=mesh_create(); }
        else if (changed) mesh_refresh(&mesh,true);
        BeginDrawing(); ClearBackground((Color){14,16,22,255});
        BeginMode3D(camera); rlDisableBackfaceCulling();
        DrawMesh(mesh,material,(Matrix){.m0=1,.m5=1,.m10=1,.m15=1});
        rlEnableBackfaceCulling(); EndMode3D();
        DrawText(changed?"GROWING":"PAUSED",18,18,24,LIGHTGRAY);
        DrawText("Hold a flower region. Release to freeze.",18,48,16,LIGHTGRAY);
        DrawRectangle(GetScreenWidth()-140,12,130,56,(Color){29,32,40,230});
        DrawText(display_density==1?"MESH 1x":"MESH 2x",GetScreenWidth()-132,30,18,LIGHTGRAY);
#if defined(PLATFORM_ANDROID)
        draw_touch_controls();
#else
        DrawText("WASD: fly  Q/E: down/up  right drag: look  Z/X: roll  T: mesh",18,GetScreenHeight()-28,15,LIGHTGRAY);
#endif
        EndDrawing();
    }
    flower_mesh_free(&display); UnloadMaterial(material); UnloadMesh(mesh); CloseWindow(); return 0;
}
