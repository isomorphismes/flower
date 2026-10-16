#include "flower.h"
#include "flower_policy.h"
#include "flower_camera.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(test) do { if (!(test)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#test); exit(1); } } while (0)
static Flower flower,saved,other;
static FlowerHold hold;
static void reset(void) { flower_init(&flower); flower_hold_init(&hold); }
static FlowerHit location(int ring,int slice) {
    return (FlowerHit){.hit=true,.radial=(float)ring/FLOWER_RINGS,.angle=(float)slice*6.283185307f/FLOWER_SLICES,
        .node=(uint16_t)(ring*FLOWER_SLICES+slice)};
}
static void step(FlowerHit hit,FlowerPolicy *p) {
    CHECK(flower_hold_begin(&hold,7,true,true,hit));
    CHECK(flower_hold_frame_policy(&flower,&hold,7,true,true,hit,FLOWER_STEP,p));
}
static void exact_hops(void) {
    FlowerNet net; int16_t hops[FLOWER_VERTICES];
    CHECK(!flower_net_init(&net,0,4)); CHECK(!flower_net_init(&net,2,2));
    CHECK(flower_net_init(&net,2,4));
    CHECK(net.node_count==12 && net.cell_count==8);
    const int expected[12]={1,2,3,2,0,1,2,1,1,2,3,2};
    for (int radius=0;radius<=4;radius++) {
        CHECK(flower_net_hops(&net,4,radius,hops));
        for (int i=0;i<12;i++) CHECK(hops[i]==(expected[i]<=radius?expected[i]:-1));
    }
    CHECK(net.degree[0]==3 && net.degree[4]==4 && net.degree[8]==3);
    CHECK(net.boundary[0]==1 && net.boundary[4]==0 && net.boundary[8]==2);
    for (int cell=0;cell<net.cell_count;cell++) for (int corner=0;corner<4;corner++) {
        int a=net.cell[cell][corner], b=net.cell[cell][(corner+1)%4]; bool found=false;
        for (int i=0;i<net.degree[a];i++) if (net.neighbor[a][i]==b) found=true;
        CHECK(found);
    }
    CHECK(net.cell[3][1]==0 && net.cell[3][2]==4);
    CHECK(flower_net_hops(&net,0,1,hops)); CHECK(hops[3]==1 && hops[4]==1 && hops[8]==-1);
    CHECK(flower_net_hops(&net,8,1,hops)); CHECK(hops[4]==1 && hops[0]==-1);
    CHECK(!flower_net_hops(&net,-1,1,hops)); CHECK(!flower_net_hops(&net,12,1,hops));
    puts("PASS exact hop shells, periodic seam, boundaries and cell/edge compatibility");
}
static void topology_growth(void) {
    reset(); saved=flower; FlowerPolicy p={.rate=0.6f,.falloff=2,.hop_radius=2,.radial_fraction=0.25f};
    FlowerHit hit=location(8,0);
    /* Geometrically coincident, topologically distant material must not grow. */
    int distant=8*FLOWER_SLICES+32;
    flower.skin.position[distant]=flower.skin.position[hit.node];
    step(hit,&p);
    int16_t hops[FLOWER_VERTICES]; CHECK(flower_net_hops(&flower.net,hit.node,p.hop_radius,hops));
    for (int i=0;i<FLOWER_VERTICES;i++) {
        float expected=0;
        if (!(flower.net.boundary[i]&1) && hops[i]>=0) expected=p.rate*(float)FLOWER_STEP*powf(1-(float)hops[i]/3,2);
        CHECK(fabsf(flower.skin.growth[i]-expected)<1e-8f);
        CHECK(fabsf(flower.skin.radial_growth[i]-expected*0.25f)<1e-8f);
    }
    CHECK(flower.skin.growth[distant]==0);
    CHECK(flower.skin.growth[9*FLOWER_SLICES+1]>0); /* two graph paths, one increment */
    CHECK(memcmp(&flower.net,&saved.net,sizeof(flower.net))==0);
    CHECK(memcmp(flower.skin.position,saved.skin.position,sizeof(flower.skin.position))!=0);
    saved=flower; flower_hold_release(&hold,7);
    for (int i=0;i<500;i++) flower_hold_frame_policy(&flower,&hold,7,true,true,hit,0.016,&p);
    CHECK(memcmp(&flower,&saved,sizeof(flower))==0);
    puts("PASS topology-only brush; folded contact gives no shortcut; graph paths do not multiply growth; release freezes Net/Skin");

    reset(); other=flower;
    for (int i=0;i<FLOWER_VERTICES;i++) {
        FlowerPoint point=flower.skin.position[i];
        flower.skin.position[i]=(FlowerPoint){3-point.z,point.y-2,point.x+5};
    }
    CHECK(memcmp(&flower.net,&other.net,sizeof(flower.net))==0);
    CHECK(memcmp(flower.skin.growth,other.skin.growth,sizeof(flower.skin.growth))==0);
    CHECK(memcmp(flower.skin.radial_growth,other.skin.radial_growth,sizeof(flower.skin.radial_growth))==0);
    step(hit,&p); saved=flower; flower=other; flower_hold_init(&hold); step(hit,&p);
    CHECK(memcmp(flower.skin.growth,saved.skin.growth,sizeof(flower.skin.growth))==0);
    CHECK(memcmp(flower.skin.radial_growth,saved.skin.radial_growth,sizeof(flower.skin.radial_growth))==0);
    puts("PASS rigidly transformed realization does not change intrinsic growth or its increments");
}
static void compare_pick(const FlowerMesh *low,const FlowerMesh *high,FlowerPoint origin,FlowerPoint direction) {
    FlowerHit a=flower_mesh_pick(low,origin,direction),b=flower_mesh_pick(high,origin,direction);
    FlowerHit canonical=flower_pick(&flower,origin,direction);
    CHECK(a.hit==b.hit && a.hit==canonical.hit);
    if (a.hit) { CHECK(a.node==b.node && a.node==canonical.node); CHECK(fabsf(a.radial-b.radial)<1e-6f); }
}
static void meshes(void) {
    reset(); FlowerPolicy p=flower_policy_default(); step(location(8,0),&p);
    saved=flower; FlowerMesh low={0},high={0};
    CHECK(flower_mesh_build(&flower,1,&low)); CHECK(flower_mesh_build(&flower,2,&high));
    CHECK(low.triangle_count==2048 && high.triangle_count==8192);
    CHECK(memcmp(&flower,&saved,sizeof(flower))==0);
    CHECK(low.surface_count==high.surface_count);
    CHECK(memcmp(low.surface,high.surface,(size_t)low.surface_count*sizeof(*low.surface))==0);
    int16_t before[FLOWER_VERTICES],after[FLOWER_VERTICES];
    CHECK(flower_net_hops(&flower.net,8*FLOWER_SLICES,4,before));
    for (int i=0;i<low.surface_count;i+=7) {
        const FlowerSurfaceTriangle *t=&low.surface[i];
        for (int test=0;test<5;test++) {
            /* Vertices, edges (including seams), midpoint and interior points. */
            const float weight[5][3]={{1,0,0},{0,1,0},{0.5f,0.5f,0},{0,0.5f,0.5f},{0.2f,0.3f,0.5f}};
            FlowerPoint point={0};
            for (int k=0;k<3;k++) {
                point.x+=weight[test][k]*t->position[k].x; point.y+=weight[test][k]*t->position[k].y;
                point.z+=weight[test][k]*t->position[k].z;
            }
            compare_pick(&low,&high,(FlowerPoint){point.x,3,point.z},(FlowerPoint){0,-1,0});
        }
    }
    compare_pick(&low,&high,(FlowerPoint){0,3,0},(FlowerPoint){0,-1,0});
    compare_pick(&low,&high,(FlowerPoint){5,3,0},(FlowerPoint){0,-1,0});
    /* Explicit nearest-mark tie at the angular seam chooses slice zero. */
    const FlowerSurfaceTriangle *seam=&low.surface[2*(8*FLOWER_SLICES+63)];
    FlowerPoint middle={(seam->position[0].x+seam->position[1].x)*0.5f,3,
                        (seam->position[0].z+seam->position[1].z)*0.5f};
    CHECK(flower_mesh_pick(&low,middle,(FlowerPoint){0,-1,0}).node==8*FLOWER_SLICES);
    CHECK(flower_mesh_pick(&high,middle,(FlowerPoint){0,-1,0}).node==8*FLOWER_SLICES);
    CHECK(flower_net_hops(&flower.net,8*FLOWER_SLICES,4,after)); CHECK(memcmp(before,after,sizeof(before))==0);
    CHECK(memcmp(&flower,&saved,sizeof(flower))==0);
    for (int i=0;i<high.vertex_count;i++) {
        FlowerPoint n=high.normal[i]; CHECK(isfinite(n.x+n.y+n.z));
        CHECK(fabsf(n.x*n.x+n.y*n.y+n.z*n.z-1)<1e-4f);
    }
    /* Camera-only inspections use moving rays for a paused Skin. */
    flower_hold_cancel(&hold);
    FlowerCamera camera={.position={0,3,0},.target={0,0,0},.up={0,0,1}};
    FlowerPoint old_position=camera.position;
    flower_camera_turn(&camera,0.2f,0.3f,0.4f);
    CHECK(memcmp(&camera.position,&old_position,sizeof(old_position))==0);
    for (int i=0;i<100;i++) {
        flower_camera_move(&camera,0.01f,0.02f,0.01f);
        flower_camera_turn(&camera,0.01f,0.01f,0.01f);
        FlowerHit hit=flower_mesh_pick(&high,(FlowerPoint){(float)i/50,3,0},(FlowerPoint){0,-1,0});
        flower_hold_frame(&flower,&hold,7,true,true,hit,0.016);
    }
    CHECK(memcmp(&camera.position,&old_position,sizeof(old_position))!=0);
    CHECK(memcmp(&flower,&saved,sizeof(flower))==0);
    flower_mesh_free(&low); flower_mesh_free(&high);
    puts("PASS two display densities, identical authoritative state/hops, deterministic picking and paused camera inspections");
}
static void lua_policy(void) {
    reset(); saved=flower; FlowerPolicy p=flower_policy_default(),original=p; char error[256];
    CHECK(flower_policy_lua(&p,"flower.set_growth(0.6, 2, 1, 0, 0.5)",error,sizeof(error)));
    CHECK(p.rate==0.6f && p.hop_radius==2 && p.falloff==1 && p.radial_fraction==0.5f);
    FlowerHit hit=location(8,0);
    for (int i=0;i<100;i++) flower_hold_frame_policy(&flower,&hold,7,true,true,hit,FLOWER_STEP,&p);
    CHECK(memcmp(&flower,&saved,sizeof(flower))==0);
    step(hit,&p); CHECK(fabsf(flower.skin.growth[hit.node]-0.6f*(float)FLOWER_STEP)<1e-8f);
    saved=flower; flower_hold_release(&hold,7);
    CHECK(flower_policy_lua(&p,"flower.set_growth(2, 16, 0, 1, 1)",error,sizeof(error)));
    for (int i=0;i<100;i++) flower_hold_frame_policy(&flower,&hold,7,false,true,hit,FLOWER_STEP,&p);
    CHECK(memcmp(&flower,&saved,sizeof(flower))==0);
    const char *bad[]={"flower.step()","flower.grow()","os.execute('exit')","flower.set_growth(9,1,1,1,1)",
        "flower.set_growth(1,2,1,0,0); flower.step()","flower.set_growth(1,2.5,1,0,0)",
        "flower.set_growth(0/0,1,1,1,1)","while true do end","local a='x'; while true do a=a..a end"};
    for (unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);i++) {
        original=p; CHECK(!flower_policy_lua(&p,bad[i],error,sizeof(error)));
        CHECK(memcmp(&p,&original,sizeof(p))==0); CHECK(memcmp(&flower,&saved,sizeof(flower))==0);
    }
    puts("PASS real Lua policy affects held growth; no step/clock/native state API; pause gate, bounds, transactional errors and budgets");
}
int main(void) { exact_hops(); topology_growth(); meshes(); lua_policy(); puts("PASS all architecture checks"); return 0; }
