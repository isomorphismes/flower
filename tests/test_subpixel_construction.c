#include "flower_subpixel.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "subpixel construction FAIL line %d: %s\n",__LINE__,#condition); \
    exit(1); } } while (0)

enum { W=160, H=160, STRIDE=164 };
static uint64_t words[STRIDE*H], prior[STRIDE*H];
static unsigned count_bits(uint64_t x) {
    unsigned n=0;
    while (x) { x &= x-1; ++n; }
    return n;
}
static void independent_diagonal(void) {
    uint64_t cells[3*3] = {0};
    cp_coverage_plane plane = { cells, 9, 3, 3, 3, 39, 34 };
    cp_curve_patch patch;
    CHECK(cp_patch_from_segment(&patch,
        (cp_vec2){30,30}, (cp_vec2){50,40}, 0.52) == CP_OK);
    CHECK(cp_coverage_plane_accumulate_3x3(&plane,&patch,40,35)==CP_OK);
    for (int row=0;row<3;row++) for(int col=0;col<3;col++) {
        uint64_t oracle = 0;
        for (int sy=0;sy<8;sy++) for(int sx=0;sx<8;sx++) {
            double x=39+col+(sx+0.5)/8.0-0.5;
            double y=34+row+(sy+0.5)/8.0-0.5;
            double distance=fabs(20*(y-35)-10*(x-40))/sqrt(500.0);
            if (distance<=0.52) oracle |= UINT64_C(1) << (sy*8+sx);
        }
        CHECK(cells[row*3+col]==oracle);
    }
    /* Different tilt, wrong row stride, wrong bit orientation are detectable. */
    CHECK(cells[0] != cells[2]);
}
static void independent_alpha(void) {
    uint64_t one[1] = {UINT64_C(1)};
    cp_coverage_plane p = {one,1,1,1,1,0,0};
    uint32_t destination[3] = {UINT32_C(0xffffffff),UINT32_C(0xfeedbeef),
                                UINT32_C(0x11223344)};
    rough_framebuffer framebuffer = {destination,1,1,3};
    CHECK(flower_subpixel_blend(&framebuffer,&p,UINT32_C(0xff000000),1.0));
    CHECK(destination[0]==UINT32_C(0xfffbfbfb));
    CHECK(destination[1]==UINT32_C(0xfeedbeef));
    CHECK(destination[2]==UINT32_C(0x11223344));
    one[0]=UINT64_C(0xffffffff);
    destination[0]=UINT32_C(0xffffffff);
    CHECK(flower_subpixel_blend(&framebuffer,&p,UINT32_C(0xff000000),1.0));
    CHECK(destination[0]==UINT32_C(0xff808080));
    framebuffer.stride_pixels=0;
    CHECK(!flower_subpixel_blend(&framebuffer,&p,UINT32_C(0xff000000),1.0));
    CHECK(destination[0]==UINT32_C(0xff808080));
}
static void integrated_round(void) {
    FlowerNet net;
    CHECK(flower_net_init(&net,16,64));
    FlowerMaterialRound round;
    CHECK(flower_material_round(&net,8,80,80,50,&round));
    cp_coverage_plane plane = {words,STRIDE*H,STRIDE,W,H,0,0};
    for (int model=3;model<=5;model+=2) {
        memset(words,0,sizeof words);
        for(int y=0;y<H;y++)
            for(int x=W;x<STRIDE;x++)
                words[y*STRIDE+x]=UINT64_C(0x8badf00d8badf00d);
        FlowerDiagramStats stats={0};
        CHECK(flower_subpixel_round(&round,model,&plane,&stats));
        CHECK(stats.curves==64 && stats.patches==64*9);
        CHECK(stats.maximum_sample_evaluations==
            (size_t)(64*9*model*model*64));
        unsigned fractional=0;
        for(int y=0;y<H;y++) for(int x=0;x<STRIDE;x++) {
            if(x>=W) CHECK(words[y*STRIDE+x]==UINT64_C(0x8badf00d8badf00d));
            else {
                unsigned count=count_bits(words[y*STRIDE+x]);
                if(count>0 && count<64) ++fractional;
            }
        }
        CHECK(fractional>80 && stats.occupied_pixels>100);
        memcpy(prior,words,sizeof prior);
        CHECK(flower_subpixel_round(&round,model,&plane,0));
        CHECK(memcmp(words,prior,sizeof words)==0);
        round.segment[17].control[1].x=NAN;
        CHECK(!flower_subpixel_round(&round,model,&plane,0));
        CHECK(memcmp(words,prior,sizeof words)==0);
        CHECK(flower_material_round(&net,8,80,80,50,&round));
    }
    cp_coverage_plane invalid=plane;
    invalid.stride_words=W-1;
    CHECK(!flower_subpixel_round(&round,3,&invalid,0));
    CHECK(memcmp(words,prior,sizeof words)==0);
    FlowerMaterialRadial radial;
    CHECK(flower_material_radial(&net,0,80,80,18,4,&radial));
    CHECK(flower_subpixel_radial(&radial,5,&plane,0));
    unsigned nonempty=0;
    for(int y=0;y<H;y++) for(int x=0;x<W;x++)
        nonempty += words[y*STRIDE+x] != 0;
    CHECK(nonempty>100);
}
int main(void) {
    independent_diagonal();
    independent_alpha();
    integrated_round();
    puts("PASS independent 8x8 diagonal masks, alpha, padded stride, repeated union, 3x3 and 5x5");
    return 0;
}
