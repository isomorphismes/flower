#include "flower_subpixel.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum { SAMPLES_PER_CURVE = 9, MAX_DIAGRAM_SIDE = 256 };
static bool valid_curve(const FlowerMaterialCubic *curve) {
    if (curve->first_node >= FLOWER_VERTICES ||
        curve->last_node >= FLOWER_VERTICES) return false;
    for (int c = 0; c < 4; ++c)
        if (!isfinite(curve->control[c].x) ||
            !isfinite(curve->control[c].y) ||
            fabs(curve->control[c].x) > 8192 ||
            fabs(curve->control[c].y) > 8192)
            return false;
    return true;
}
static bool patch_at(const FlowerMaterialCubic *curve, int index, int model,
                     cp_curve_patch *patch) {
    cp_vec2 controls[4];
    for (int c = 0; c < 4; ++c)
        controls[c] = (cp_vec2){curve->control[c].x, curve->control[c].y};
    double t = (double)index / (SAMPLES_PER_CURVE - 1);
    cp_status status = model == FLOWER_CURVE_3X3
        ? cp_patch_from_cubic(patch, controls, t, 0.52, 1.5)
        : cp_patch_from_cubic_jet3(patch, controls, t, 0.52, 2.5);
    return status == CP_OK;
}
static bool run(const FlowerMaterialCubic *curves, int count, int model,
                cp_coverage_plane *plane, FlowerDiagramStats *stats) {
    if (!curves || count < 1 || count > FLOWER_SLICES ||
        (model != FLOWER_CURVE_3X3 && model != FLOWER_CURVE_5X5) ||
        cp_coverage_plane_validate(plane) != CP_OK ||
        plane->width > MAX_DIAGRAM_SIDE || plane->height > MAX_DIAGRAM_SIDE)
        return false;
    /* Preflight all input geometry before changing caller-owned mask. */
    for (int i = 0; i < count; ++i) {
        if (!valid_curve(&curves[i])) return false;
        for (int step = 0; step < SAMPLES_PER_CURVE; ++step) {
            cp_curve_patch patch;
            if (!patch_at(&curves[i], step, model, &patch)) return false;
        }
    }
    for (int i = 0; i < count; ++i)
        for (int step = 0; step < SAMPLES_PER_CURVE; ++step) {
            cp_curve_patch patch;
            if (!patch_at(&curves[i], step, model, &patch)) return false;
            int center_x = (int)floor(patch.origin.x + 0.5);
            int center_y = (int)floor(patch.origin.y + 0.5);
            cp_status status = model == FLOWER_CURVE_3X3
                ? cp_coverage_plane_accumulate_3x3(plane, &patch, center_x, center_y)
                : cp_coverage_plane_accumulate_5x5(plane, &patch, center_x, center_y);
            if (status != CP_OK) return false;
        }
    if (stats) {
        FlowerDiagramStats result = {0};
        result.curves = (unsigned)count;
        result.patches = (unsigned)(count * SAMPLES_PER_CURVE);
        /* Maximum midpoint occupancy predicates, not a measured CPU time. */
        result.maximum_sample_evaluations = (size_t)result.patches *
            (size_t)model * (size_t)model * 64;
        for (unsigned y = 0; y < plane->height; ++y)
            for (unsigned x = 0; x < plane->width; ++x)
                if (plane->occupied[(size_t)y * plane->stride_words + x])
                    ++result.occupied_pixels;
        *stats = result;
    }
    return true;
}
bool flower_subpixel_round(const FlowerMaterialRound *round, int model,
                           cp_coverage_plane *plane, FlowerDiagramStats *stats) {
    if (!round || round->count < 3 || round->count > FLOWER_SLICES)
        return false;
    return run(round->segment, round->count, model, plane, stats);
}
bool flower_subpixel_radial(const FlowerMaterialRadial *radial, int model,
                            cp_coverage_plane *plane, FlowerDiagramStats *stats) {
    if (!radial || radial->count < 1 || radial->count > FLOWER_RINGS)
        return false;
    return run(radial->segment, radial->count, model, plane, stats);
}
bool flower_subpixel_blend(rough_framebuffer *target,
                            const cp_coverage_plane *coverage,
                            uint32_t color, double opacity) {
    return rough_blend_subpixel_coverage(target, coverage, color, opacity) == 0;
}
