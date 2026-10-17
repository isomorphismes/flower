/* Optional NativeActivity/raylib 5.5 2D pane, NOT surface decoration.
 * One display-neutral material chart is built only when toggled on or when
 * its sampling model changes. All temporary CPU buffers are caller-owned.
 */
#include "flower_diagram.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum { DIAGRAM_SIDE = 192, DIAGRAM_PIXELS = DIAGRAM_SIDE * DIAGRAM_SIDE };
enum {
    DIAGRAM_TEMP_BYTES = DIAGRAM_PIXELS * (sizeof(uint64_t) + sizeof(uint32_t))
};

void flower_diagram_init(FlowerDiagram *diagram) {
    if (!diagram) return;
    *diagram = (FlowerDiagram){.model = FLOWER_CURVE_3X3};
}
void flower_diagram_free(FlowerDiagram *diagram) {
    if (!diagram) return;
    if (diagram->texture.id) UnloadTexture(diagram->texture);
    flower_diagram_init(diagram);
}
void flower_diagram_toggle(FlowerDiagram *diagram) {
    if (!diagram) return;
    if (diagram->enabled) {
        if (diagram->texture.id) UnloadTexture(diagram->texture);
        diagram->texture = (Texture2D){0};
        diagram->enabled = false;
        diagram->dirty = false;
    } else {
        diagram->enabled = true;
        diagram->dirty = true;
    }
}
void flower_diagram_cycle_sampling(FlowerDiagram *diagram) {
    if (!diagram) return;
    diagram->model = diagram->model == FLOWER_CURVE_3X3
        ? FLOWER_CURVE_5X5 : FLOWER_CURVE_3X3;
    if (diagram->enabled) diagram->dirty = true;
}

/* Same ink on one plane: masks are unioned before straight-alpha composition.
 * All construction coordinates come from Net IDs, never from folded Skin. */
bool flower_diagram_update(FlowerDiagram *diagram, const FlowerNet *net) {
    if (!diagram || !diagram->enabled || !diagram->dirty) return true;
    if (!net) return false;

    double start = GetTime(); /* telemetry only; never modifies growth */
    uint64_t *occupied = calloc(DIAGRAM_PIXELS, sizeof(*occupied));
    uint32_t *rgba = malloc(DIAGRAM_PIXELS * sizeof(*rgba));
    if (!occupied || !rgba) {
        free(occupied); free(rgba);
        return false;
    }
    FlowerMaterialRound round;
    FlowerMaterialRadial radial;
    FlowerDiagramStats circle_stats = {0}, spoke_stats = {0};
    cp_coverage_plane plane = {
        .occupied = occupied, .capacity_words = DIAGRAM_PIXELS,
        .stride_words = DIAGRAM_SIDE, .width = DIAGRAM_SIDE,
        .height = DIAGRAM_SIDE, .origin_x = 0, .origin_y = 0
    };
    bool ready = flower_material_round(net, 8, 96, 96, 55, &round) &&
        flower_material_radial(net, 0, 96, 96, 20, 4.375, &radial) &&
        flower_subpixel_round(&round, diagram->model, &plane, &circle_stats) &&
        flower_subpixel_radial(&radial, diagram->model, &plane, &spoke_stats);
    if (ready) {
        for (size_t i = 0; i < DIAGRAM_PIXELS; ++i)
            rgba[i] = UINT32_C(0xffffffff);
        rough_framebuffer target = {
            .pixels = rgba, .width = DIAGRAM_SIDE, .height = DIAGRAM_SIDE,
            .stride_pixels = DIAGRAM_SIDE
        };
        ready = flower_subpixel_blend(&target, &plane,
                                     UINT32_C(0xff192e44), 1.0);
    }
    if (ready) {
        /* Explicit RGBA bytes independent of CPU endian/order of 0xAARRGGBB. */
        unsigned char *bytes = (unsigned char *)rgba;
        for (size_t i = 0; i < DIAGRAM_PIXELS; ++i) {
            uint32_t argb = rgba[i];
            bytes[4*i + 0] = (unsigned char)(argb >> 16);
            bytes[4*i + 1] = (unsigned char)(argb >> 8);
            bytes[4*i + 2] = (unsigned char)argb;
            bytes[4*i + 3] = (unsigned char)(argb >> 24);
        }
        if (!diagram->texture.id) {
            Image image = {
                .data = rgba, .width = DIAGRAM_SIDE,
                .height = DIAGRAM_SIDE, .mipmaps = 1,
                .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
            };
            diagram->texture = LoadTextureFromImage(image);
        } else {
            UpdateTexture(diagram->texture, rgba);
        }
        ready = diagram->texture.id != 0;
    }
    if (ready) {
        ++diagram->rebuilds;
        diagram->dirty = false;
        TraceLog(LOG_INFO,
            "Flower flat 2D subpixel %dx%d: rebuilt=%u patches=%u "
            "max_subsamples=%zu transient_bytes=%zu elapsed_ms=%.3f",
            diagram->model, diagram->model, diagram->rebuilds,
            circle_stats.patches + spoke_stats.patches,
            circle_stats.maximum_sample_evaluations +
                spoke_stats.maximum_sample_evaluations,
            (size_t)DIAGRAM_TEMP_BYTES, (GetTime()-start)*1000.0);
    }
    free(occupied);
    free(rgba);
    return ready;
}
void flower_diagram_draw(const FlowerDiagram *diagram) {
    if (!diagram || !diagram->enabled || !diagram->texture.id) return;
    int x = GetScreenWidth() - DIAGRAM_SIDE - 18;
    if (x < 12) x = 12;
    int y = 88;
    DrawTexture(diagram->texture, x, y, WHITE);
    DrawRectangleLines(x-1, y-1, DIAGRAM_SIDE+2, DIAGRAM_SIDE+2, GRAY);
    DrawText(diagram->model == FLOWER_CURVE_3X3
                 ? "NET round 8 - flat 2D / 3x3"
                 : "NET round 8 - flat 2D / 5x5",
             x, y+DIAGRAM_SIDE+6, 13, LIGHTGRAY);
}
