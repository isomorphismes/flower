/* Material-attached polar chart, independent of the 3D Mesh/camera.
 * 3x3 and 5x5 compare the same chosen construction curve; all rings and
 * sectors, growth and reference-length stretch are drawn from live state.
 * This is a descriptive diagnostic, NOT geometric curvature certification. */
#include "flower_diagram.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum { DIAGRAM_SIDE=FLOWER_DIAGRAM_SIDE, DIAGRAM_PIXELS=DIAGRAM_SIDE*DIAGRAM_SIDE };
enum { DIAGRAM_TEMP_BYTES=DIAGRAM_PIXELS*(sizeof(uint64_t)+sizeof(uint32_t)) };
#define TAU 6.2831853071795864769

static int left_at(int width) {
    int x=width-FLOWER_DIAGRAM_SIDE-18;
    return x<12?12:x;
}
void flower_diagram_init(FlowerDiagram *diagram) {
    if (!diagram) return;
    *diagram=(FlowerDiagram){.model=FLOWER_CURVE_3X3,
                             .selected_node=-1,.drawn_steps=UINT64_MAX};
}
void flower_diagram_free(FlowerDiagram *diagram) {
    if (!diagram) return;
    if (diagram->texture.id) UnloadTexture(diagram->texture);
    flower_diagram_init(diagram);
}
void flower_diagram_toggle(FlowerDiagram *diagram) {
    if (!diagram) return;
    if (!diagram->enabled) {
        diagram->enabled=true;
        diagram->view=FLOWER_INSPECTION_GROWTH;
    } else if (diagram->view==FLOWER_INSPECTION_GROWTH) {
        diagram->view=FLOWER_INSPECTION_STRETCH;
    } else {
        diagram->enabled=false;
        diagram->view=0;
        if (diagram->texture.id) UnloadTexture(diagram->texture);
        diagram->texture=(Texture2D){0};
    }
    diagram->dirty=diagram->enabled;
}
void flower_diagram_cycle_sampling(FlowerDiagram *diagram) {
    if (!diagram) return;
    diagram->model=diagram->model==FLOWER_CURVE_3X3?
                   FLOWER_CURVE_5X5:FLOWER_CURVE_3X3;
    if (diagram->enabled) diagram->dirty=true;
}
bool flower_diagram_select(FlowerDiagram *diagram, const Flower *flower,
                           int viewport_width, double screen_x, double screen_y) {
    if (!diagram || !diagram->enabled || !flower ||
        !flower_construction_pane_contains(true,viewport_width,screen_x,screen_y))
        return false;
    FlowerInspectionSample sample;
    if (!flower_inspection_sample(flower,screen_x-left_at(viewport_width),
                                 screen_y-FLOWER_DIAGRAM_TOP,&sample))
        return false;
    diagram->selected_node=sample.node;
    return true;
}
bool flower_diagram_update(FlowerDiagram *diagram, const Flower *flower) {
    if (!diagram || !diagram->enabled) return true;
    if (!flower) return false;
    if (diagram->drawn_steps!=flower->skin.steps) diagram->dirty=true;
    if (!diagram->dirty) return true;
    double start=GetTime(); /* read-only telemetry, never a simulation clock */
    uint64_t *occupied=calloc(DIAGRAM_PIXELS,sizeof(*occupied));
    uint32_t *rgba=malloc(DIAGRAM_PIXELS*sizeof(*rgba));
    if (!occupied || !rgba) { free(occupied); free(rgba); return false; }
    float peak=flower_inspection_max_growth(flower);
    for (int y=0;y<DIAGRAM_SIDE;y++) for (int x=0;x<DIAGRAM_SIDE;x++) {
        rgba[y*DIAGRAM_SIDE+x]=flower_inspection_color(
             flower,x+0.5,y+0.5,diagram->view,peak);
    }
    /* Highlight one real Net round and radial path using the existing pinned
     * subpixel implementation. The many fine grid lines come from Net IDs. */
    FlowerMaterialRound round;
    FlowerMaterialRadial radial;
    FlowerDiagramStats circle_stats={0},spoke_stats={0};
    cp_coverage_plane plane={.occupied=occupied,.capacity_words=DIAGRAM_PIXELS,
         .stride_words=DIAGRAM_SIDE,.width=DIAGRAM_SIDE,.height=DIAGRAM_SIDE,
         .origin_x=0,.origin_y=0};
    bool ready=flower_material_round(&flower->net,8,96,96,55,&round) &&
               flower_material_radial(&flower->net,0,96,96,20,4.375,&radial) &&
               flower_subpixel_round(&round,diagram->model,&plane,&circle_stats) &&
               flower_subpixel_radial(&radial,diagram->model,&plane,&spoke_stats);
    if (ready) {
        rough_framebuffer target={.pixels=rgba,.width=DIAGRAM_SIDE,
            .height=DIAGRAM_SIDE,.stride_pixels=DIAGRAM_SIDE};
        ready=flower_subpixel_blend(&target,&plane,UINT32_C(0xff15253b),0.8);
    }
    if (ready) {
        /* Convert AARRGGBB words to contiguous RGBA bytes without relying
         * on the native CPU byte order. */
        unsigned char *bytes=(unsigned char *)rgba;
        for (int i=0;i<DIAGRAM_PIXELS;i++) {
            uint32_t argb=rgba[i];
            bytes[4*i]=(unsigned char)(argb>>16);
            bytes[4*i+1]=(unsigned char)(argb>>8);
            bytes[4*i+2]=(unsigned char)argb;
            bytes[4*i+3]=(unsigned char)(argb>>24);
        }
        if (!diagram->texture.id) {
            Image image={.data=rgba,.width=DIAGRAM_SIDE,.height=DIAGRAM_SIDE,
                         .mipmaps=1,.format=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
            diagram->texture=LoadTextureFromImage(image);
        } else UpdateTexture(diagram->texture,rgba);
        ready=diagram->texture.id!=0;
    }
    if (ready) {
        ++diagram->rebuilds;
        diagram->dirty=false;
        diagram->drawn_steps=flower->skin.steps;
        TraceLog(LOG_INFO,"Flower real-Net chart: steps=%llu view=%d model=%dx%d "
            "rebuilds=%u patches=%u temporary=%zu elapsed_ms=%.3f",
            (unsigned long long)diagram->drawn_steps,diagram->view,
            diagram->model,diagram->model,diagram->rebuilds,
            circle_stats.patches+spoke_stats.patches,
            (size_t)DIAGRAM_TEMP_BYTES,(GetTime()-start)*1000.0);
    }
    free(occupied);free(rgba);
    return ready;
}
void flower_diagram_draw(const FlowerDiagram *diagram, const Flower *flower) {
    if (!diagram || !diagram->enabled || !diagram->texture.id || !flower) return;
    int x=left_at(GetScreenWidth()), y=FLOWER_DIAGRAM_TOP;
    DrawTexture(diagram->texture,x,y,WHITE);
    DrawRectangleLines(x-1,y-1,DIAGRAM_SIDE+2,DIAGRAM_SIDE+2,GRAY);
    DrawRectangle(x,y+DIAGRAM_SIDE+2,DIAGRAM_SIDE,45,(Color){29,32,40,245});
    DrawText(diagram->view==FLOWER_INSPECTION_GROWTH?
             "NET: intrinsic growth":"NET: reference stretch",
             x+3,y+DIAGRAM_SIDE+5,13,LIGHTGRAY);
    if (diagram->selected_node>=0 &&
        diagram->selected_node<flower->net.node_count) {
        int id=diagram->selected_node, ring=flower->net.ring[id],
            slice=flower->net.slice[id];
        double radius=20.0+68.0*ring/flower->net.rings;
        double theta=TAU*slice/flower->net.slices;
        DrawCircle(x+96+(int)lround(radius*cos(theta)),
                   y+96+(int)lround(radius*sin(theta)),4,YELLOW);
        FlowerInspectionSample sample;
        if (flower_inspection_sample(flower,96+radius*cos(theta),
                                     96+radius*sin(theta),&sample))
            DrawText(TextFormat("ID %d G %.2f dL/L %.2f",id,
                      sample.growth,sample.reference_stretch),
                     x+3,y+DIAGRAM_SIDE+24,12,LIGHTGRAY);
    } else DrawText(TextFormat("peak growth %.3f | %dx%d",
                     flower_inspection_max_growth(flower),
                     diagram->model,diagram->model),
                  x+3,y+DIAGRAM_SIDE+24,12,LIGHTGRAY);
}
