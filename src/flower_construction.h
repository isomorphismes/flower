#ifndef FLOWER_CONSTRUCTION_H
#define FLOWER_CONSTRUCTION_H

#include "flower.h"

/* Display-neutral, material-attached construction marks. The IDs refer
 * exclusively to FlowerNet; neither FlowerSkin nor FlowerMesh is consulted.
 * Pixel coordinates are an explicit chart chosen by the caller, NOT world XY. */
typedef struct { double x, y; } FlowerDiagramPoint;
typedef struct {
    uint16_t first_node, last_node;
    FlowerDiagramPoint control[4]; /* cubic Bezier in the caller's flat chart */
} FlowerMaterialCubic;
typedef struct {
    int ring, count;
    FlowerMaterialCubic segment[FLOWER_SLICES];
} FlowerMaterialRound;
typedef struct {
    int slice, count;
    FlowerMaterialCubic segment[FLOWER_RINGS];
} FlowerMaterialRadial;

/* On invalid input *out is untouched. A marked round is closed at the seam:
 * its last segment ends at the first material ID, never at a new graph node.
 * Pixel-space circular arcs are display conventions, not intrinsic distances. */
bool flower_material_round(const FlowerNet *net, int ring,
                           double center_x, double center_y, double radius,
                           FlowerMaterialRound *out);
/* One open radial construction path through actual Net edges. */
bool flower_material_radial(const FlowerNet *net, int slice,
                            double center_x, double center_y,
                            double inner_radius, double step_radius,
                            FlowerMaterialRadial *out);
#endif
