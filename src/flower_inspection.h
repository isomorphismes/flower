#ifndef FLOWER_INSPECTION_H
#define FLOWER_INSPECTION_H
#include "flower.h"
#include <stdint.h>

/* A fixed material chart. These are Net IDs, not projected mesh pixels.
 * Stretch compares realized edge lengths to their reference lengths;
 * it does NOT claim to be intrinsic curvature or shell strain energy. */
typedef struct {
    uint16_t node;
    uint16_t ring, slice;
    float growth, radial_growth, reference_stretch;
} FlowerInspectionSample;

typedef struct { double preferred, realized; } FlowerMetricPair;
typedef struct {
    FlowerMetricPair round, spoke, angle_defect;
    bool boundary;
} FlowerNodeMetric;

/* Forward round edge and outward spoke (inward on the outer rim).
 * Angle defect and boundary turning are integrated angles in radians.
 * A triangle-inequality failure leaves the corresponding angle as NaN. */
bool flower_inspection_metric(const Flower *flower, int node, FlowerNodeMetric *out);

enum { FLOWER_INSPECTION_GROWTH = 1, FLOWER_INSPECTION_STRETCH = 2 };

bool flower_inspection_sample(const Flower *flower, double x, double y,
                              FlowerInspectionSample *out);
float flower_inspection_max_growth(const Flower *flower);
/* ARGB pixel for a 192x192 material chart, with all ring/sector boundaries. */
uint32_t flower_inspection_color(const Flower *flower, double x, double y,
                                 int view, float peak_growth);
#endif
