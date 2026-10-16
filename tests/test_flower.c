#include "flower.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Explicit checks survive NDEBUG; an assertion-free build must not silently pass. */
#define CHECK(test) do { if (!(test)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#test); exit(1); } } while (0)
static Flower flower, saved, reference;
static FlowerHold hold;
static const FlowerHit hit={.hit=true,.radial=0.9f,.angle=0,.node=14*FLOWER_SLICES};
static void reset(void) { flower_init(&flower); flower_hold_init(&hold); }
static void press(int id) { CHECK(flower_hold_begin(&hold,id,true,true,hit)); }
static void frame(int id, bool down, double seconds) { flower_hold_frame(&flower,&hold,id,down,true,hit,seconds); }
static void frozen(void) { CHECK(memcmp(&flower,&saved,sizeof(flower))==0); }

int main(void) {
    reset(); saved=flower;
    for (int i=0;i<500;i++) frame(7,false,1.0/60);
    frozen(); puts("PASS starts paused");

    CHECK(!flower_hold_begin(&hold,7,false,true,hit));
    CHECK(!flower_hold_begin(&hold,7,true,false,hit));
    CHECK(!flower_hold_begin(&hold,7,true,true,(FlowerHit){0}));
    frame(7,true,1.0/60); frozen(); puts("PASS no fresh valid press means no growth");

    press(7); frame(7,true,1.0/60); CHECK(flower.skin.steps==2);
    CHECK(flower.skin.growth[14*FLOWER_SLICES]>0);
    CHECK(flower.skin.growth[14*FLOWER_SLICES+FLOWER_SLICES/2]==0);
    CHECK(memcmp(flower.skin.position,saved.skin.position,sizeof(flower.skin.position))!=0);
    CHECK(memcmp(flower.skin.position,saved.skin.position,FLOWER_SLICES*sizeof(FlowerPoint))==0);
    CHECK(memcmp(&flower.net,&saved.net,sizeof(flower.net))==0);
    puts("PASS held region gains preferred length; geometry changes, root and topology stay fixed");

    saved=flower; frame(7,false,1.0/60);
    for (int i=0;i<500;i++) frame(7,false,1.0/60);
    frozen(); puts("PASS release freezes entire material and geometry state bit-for-bit");

    CHECK(!flower_hold_begin(&hold,8,true,true,hit));
    frame(8,true,1.0/60); frozen();
    flower_hold_all_released(&hold); press(8); frame(8,true,1.0/60);
    CHECK(flower.skin.steps==saved.skin.steps+2); puts("PASS no pointer handoff; release and fresh press resume");

    saved=flower; frame(9,true,1.0/60); flower_hold_release(&hold,9); frozen();
    CHECK(hold.captured && hold.owner==8); puts("PASS unrelated finger cannot grow or release captured stroke");

    flower_hold_frame(&flower,&hold,8,true,true,(FlowerHit){0},1.0/60); frozen();
    frame(8,true,1.0/60); CHECK(flower.skin.steps==saved.skin.steps+2); puts("PASS off-surface time does not accumulate");

    saved=flower;
    flower_hold_frame(&flower,&hold,8,true,false,hit,1.0/60);
    frame(8,true,1.0/60); frozen();
    CHECK(!flower_hold_begin(&hold,8,true,true,hit));
    flower_hold_all_released(&hold); press(8); frame(8,true,1.0/60);
    CHECK(flower.skin.steps==saved.skin.steps+2); puts("PASS focus loss cancels and requires a fresh stroke");

    saved=flower; flower_hold_cancel(&hold); frame(8,true,1.0/60); frozen();
    puts("PASS explicit touch cancellation freezes state");

    reset(); press(1); frame(1,true,1.0/240); CHECK(flower.skin.steps==0);
    frame(1,false,1.0/60); saved=flower;
    for (int i=0;i<1000;i++) frame(1,false,1.0/60);
    frozen(); flower_hold_all_released(&hold); press(1); frame(1,true,1.0/240);
    CHECK(flower.skin.steps==0); frame(1,true,1.0/240); CHECK(flower.skin.steps==1);
    puts("PASS release discards fractional time; no paused-time catch-up");

    const double bad_times[]={NAN,INFINITY,-1,0.5,120};
    for (unsigned i=0;i<sizeof(bad_times)/sizeof(bad_times[0]);i++) {
        reset(); press(1); saved=flower; frame(1,true,bad_times[i]); frozen();
        frame(1,true,1.0/60); frozen(); CHECK(hold.blocked);
    }
    puts("PASS invalid or stalled frame cancels rather than growing a backlog");

    reset(); press(1); saved=flower;
    FlowerHit bad=hit; bad.radial=NAN;
    flower_hold_frame(&flower,&hold,1,true,true,bad,1.0/60); frozen();
    puts("PASS invalid surface coordinates cannot mutate the flower");

    reset(); press(1); for (int i=0;i<60;i++) frame(1,true,1.0/60); reference=flower;
    reset(); press(1); for (int i=0;i<120;i++) frame(1,true,1.0/120);
    CHECK(memcmp(&flower,&reference,sizeof(flower))==0);
    puts("PASS 60-Hz and 120-Hz holds execute identical fixed steps");

    reset(); FlowerHit picked=flower_pick(&flower,(FlowerPoint){1.2f,3,0},(FlowerPoint){0,-1,0});
    CHECK(picked.hit && picked.radial>0 && picked.radial<1);
    CHECK(flower_pick(&flower,(FlowerPoint){1.2f,-3,0},(FlowerPoint){0,1,0}).hit);
    CHECK(!flower_pick(&flower,(FlowerPoint){5,3,0},(FlowerPoint){0,-1,0}).hit);
    CHECK(!flower_pick(&flower,(FlowerPoint){0,3,0},(FlowerPoint){0,-1,0}).hit);
    puts("PASS actual triangle picking works on both sides and refuses empty space");

    FlowerPoint normals[FLOWER_VERTICES]; flower_normals(&flower,normals);
    for (int i=0;i<FLOWER_VERTICES;i++) {
        CHECK(isfinite(normals[i].x) && isfinite(normals[i].y) && isfinite(normals[i].z));
        CHECK(fabsf(normals[i].x*normals[i].x+normals[i].y*normals[i].y+normals[i].z*normals[i].z-1)<1e-4f);
    }
    puts("PASS finite unit normals");
    printf("PASS all core regressions; %d vertices, %d triangles, %d links\n",FLOWER_VERTICES,FLOWER_TRIANGLES,flower.skin.link_count);
    return 0;
}
