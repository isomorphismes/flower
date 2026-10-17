#ifndef FLOWER_SUBPIXEL_H
#define FLOWER_SUBPIXEL_H

#include "flower_construction.h"
#include "subpixel/coverage_plane.h"
#include "subpixel_framebuffer.h"

/* Optional 2D bridge: requires the corrected subpixel checkout from FP #120.
 * Neither API owns storage. One ink/color uses ONE unioned mask. */
enum { FLOWER_CURVE_3X3 = 3, FLOWER_CURVE_5X5 = 5 };
typedef struct {
    unsigned curves, patches, occupied_pixels;
    size_t maximum_sample_evaluations;
} FlowerDiagramStats;

/* App-owned coverage plane must be cleared once per desired shape. Render
 * each segment exactly nine local neighborhoods, independent of camera,
 * shading, Skin, growth, or display triangulation. The size cap is deliberate.
 * Invalid parameters leave the plane unchanged; the source API only promises
 * atomicity for each individual local neighborhood, not for entire paths. */
bool flower_subpixel_round(const FlowerMaterialRound *round, int model,
                           cp_coverage_plane *plane, FlowerDiagramStats *stats);
bool flower_subpixel_radial(const FlowerMaterialRadial *radial, int model,
                            cp_coverage_plane *plane, FlowerDiagramStats *stats);
/* Resolve unioned coverage once (NOT once per patch) into straight-alpha
 * AARRGGBB; no depth, occlusion, or color space conversion is implied. */
bool flower_subpixel_blend(rough_framebuffer *target,
                            const cp_coverage_plane *coverage,
                            uint32_t color, double opacity);
#endif
