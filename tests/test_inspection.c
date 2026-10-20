#include "flower_inspection.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, \
    "inspection FAIL line %d: %s\n", __LINE__, #c); exit(1); } } while(0)
static Flower flower, before;

int main(void) {
    flower_init(&flower);
    FlowerInspectionSample a = {0}, untouched = {.node=UINT16_MAX};
    CHECK(flower_inspection_sample(&flower, 96+54, 96, &a));
    CHECK(a.node == 8*FLOWER_SLICES && a.slice == 0);
    CHECK(a.growth == 0 && a.reference_stretch == 0);
    CHECK(!flower_inspection_sample(&flower, 96, 96, &untouched));
    CHECK(untouched.node == UINT16_MAX);
    CHECK(!flower_inspection_sample(&flower, NAN, 96, &untouched));
    CHECK(!flower_inspection_sample(NULL, 130, 96, &untouched));
    CHECK(flower_inspection_sample(&flower, 96+53, 95.5, &a));
    CHECK(a.slice == 0);  /* periodic seam roundoff, not a new node */
    uint32_t initial=flower_inspection_color(&flower,150,96,FLOWER_INSPECTION_GROWTH,0);
    CHECK(initial != flower_inspection_color(&flower,96,96,FLOWER_INSPECTION_GROWTH,0));
    CHECK(flower_inspection_max_growth(&flower) == 0);
    flower.skin.growth[8*FLOWER_SLICES] = 0.5f;
    CHECK(fabsf(flower_inspection_max_growth(&flower)-0.5f) < 1e-6f);
    CHECK(flower_inspection_sample(&flower, 150, 96, &a));
    CHECK(fabsf(a.growth-0.5f) < 1e-6f);
    CHECK(initial != flower_inspection_color(&flower,150,96,FLOWER_INSPECTION_GROWTH,0.5f));
    int node = a.node;
    int adjacent = flower.net.neighbor[node][0];
    FlowerPoint p=flower.skin.position[adjacent];
    flower.skin.position[adjacent]=(FlowerPoint){p.x*1.3f,p.y*1.3f,p.z*1.3f};
    CHECK(flower_inspection_sample(&flower,150,96,&a));
    CHECK(fabsf(a.reference_stretch) > 1e-5f);
    before=flower;
    uint32_t strain=flower_inspection_color(&flower,150,96,FLOWER_INSPECTION_STRETCH,0.5f);
    CHECK(strain != initial);
    FlowerMesh low={0}, high={0};
    CHECK(flower_mesh_build(&flower,1,&low));
    CHECK(flower_mesh_build(&flower,2,&high));
    CHECK(flower_inspection_sample(&flower,150,96,&a));
    CHECK(strain == flower_inspection_color(&flower,150,96,FLOWER_INSPECTION_STRETCH,0.5f));
    flower_mesh_free(&low); flower_mesh_free(&high);
    CHECK(memcmp(&before,&flower,sizeof(flower)) == 0);
    /* Corrupt topology must refuse, not read outside bounds. */
    flower.net.neighbor[node][0] = UINT16_MAX;
    CHECK(!flower_inspection_sample(&flower,150,96,&a));
    puts("PASS material grid, growth, stretched links, invalid input, source and density invariance");
    return 0;
}
