/* Host diagnostic: a chart of Flower's fixed 64-sector Net, NOT a counted
 * crochet stitch ledger, a GPU screenshot, or an occlusion-aware 3D overlay.
 * Output: nearest-binary illustration | quadratic 3x3 | cubic 5x5.
 * Built only against the corrected, source-pinned subpixel checkout. */
#include "flower_subpixel.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { W=192, H=192, OUTPUT_W=3*W };
static uint64_t mask3[W*H], mask5[W*H];
static uint32_t color3[W*H], color5[W*H];
static FlowerNet net;
static FlowerMaterialRound round_path;
static FlowerMaterialRadial radial_path;

static int render(int method, cp_coverage_plane *plane,
                  uint32_t *pixels, FlowerDiagramStats *stats) {
    if (!flower_subpixel_round(&round_path,method,plane,stats) ||
        !flower_subpixel_radial(&radial_path,method,plane,0))
        return 0;
    for(size_t i=0;i<(size_t)W*H;i++) pixels[i]=UINT32_C(0xffffffff);
    rough_framebuffer target={pixels,W,H,W};
    return flower_subpixel_blend(&target,plane,UINT32_C(0xff192e44),1.0);
}
static int ppm(FILE *output) {
    if (fprintf(output,"P6\n%d %d\n255\n",OUTPUT_W,H)<0) return 0;
    for(int y=0;y<H;y++) for(int x=0;x<OUTPUT_W;x++) {
        uint32_t pixel;
        if(x<W) {
            /* Deliberately crude threshold, not an earlier Flower renderer. */
            unsigned n=0;
            uint64_t bits=mask3[y*W+x];
            while(bits) { bits&=bits-1; ++n; }
            pixel=n>=32?UINT32_C(0xff192e44):UINT32_C(0xffffffff);
        } else if(x<2*W) pixel=color3[y*W+x-W];
        else pixel=color5[y*W+x-2*W];
        unsigned char rgb[3]={(unsigned char)(pixel>>16),
                              (unsigned char)(pixel>>8),(unsigned char)pixel};
        if(fwrite(rgb,1,3,output)!=3) return 0;
    }
    return !ferror(output);
}
int main(int argc, char **argv) {
    if(argc!=2) {
        fputs("usage: flower-construction-preview OUTPUT.ppm\n",stderr);
        return 2;
    }
    if(!flower_net_init(&net,FLOWER_RINGS,FLOWER_SLICES) ||
       !flower_material_round(&net,8,96,96,55,&round_path) ||
       !flower_material_radial(&net,0,96,96,20,4.375,&radial_path))
        return 1;
    cp_coverage_plane plane3={mask3,W*H,W,W,H,0,0};
    cp_coverage_plane plane5={mask5,W*H,W,W,H,0,0};
    FlowerDiagramStats stats3,stats5;
    if(!render(3,&plane3,color3,&stats3) ||
       !render(5,&plane5,color5,&stats5)) return 1;
    FILE *output=fopen(argv[1],"wb");
    if(!output) { perror(argv[1]); return 1; }
    int ok=ppm(output);
    if(fclose(output)!=0) ok=0;
    if(!ok) { fputs("failed to write image\n",stderr); return 1; }
    printf("3x3 round %u patches, <=%zu sample predicates; 5x5 round %u patches, <=%zu\n",
           stats3.patches,stats3.maximum_sample_evaluations,
           stats5.patches,stats5.maximum_sample_evaluations);
    printf("persistent mask+color storage %zu bytes, %u x %u pixels, no per-frame work while disabled\n",
           sizeof(mask3)+sizeof(mask5)+sizeof(color3)+sizeof(color5),W,H);
    return 0;
}
