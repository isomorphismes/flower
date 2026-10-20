#include "flower_inspection.h"
#include <math.h>
#include <stddef.h>

#define INSPECTION_TAU 6.2831853071795864769
enum { CHART_CENTER = 96, CHART_INNER = 20, CHART_OUTER = 88 };

static double distance3(FlowerPoint a, FlowerPoint b) {
    double dx = (double)a.x - b.x;
    double dy = (double)a.y - b.y;
    double dz = (double)a.z - b.z;
    return sqrt(dx*dx + dy*dy + dz*dz);
}
static double minimum(double x, double y) { return x < y ? x : y; }
static double maximum(double x, double y) { return x > y ? x : y; }
static bool valid(const Flower *flower) {
    return flower && flower->net.rings > 0 &&
        flower->net.rings <= FLOWER_RINGS &&
        flower->net.slices >= 3 && flower->net.slices <= FLOWER_SLICES &&
        flower->net.node_count == (flower->net.rings+1)*flower->net.slices;
}
bool flower_inspection_sample(const Flower *flower, double x, double y,
                              FlowerInspectionSample *out) {
    if (!valid(flower) || !out || !isfinite(x) || !isfinite(y))
        return false;
    double dx = x - CHART_CENTER, dy = y - CHART_CENTER;
    double radius = hypot(dx, dy);
    if (radius < CHART_INNER || radius > CHART_OUTER) return false;
    double ring_fraction = (radius-CHART_INNER)*flower->net.rings/
                           (CHART_OUTER-CHART_INNER);
    double angle = atan2(dy, dx);
    if (angle < 0) angle += INSPECTION_TAU;
    double sector_fraction = angle*flower->net.slices/INSPECTION_TAU;
    int ring = (int)floor(ring_fraction + 0.5);
    int slice = (int)floor(sector_fraction + 0.5) % flower->net.slices;
    if (ring > flower->net.rings) ring = flower->net.rings;
    int node = ring*flower->net.slices + slice;
    if (node < 0 || node >= flower->net.node_count ||
        flower->net.degree[node] > 4) return false;
    double stretch = 0;
    int count = 0;
    for (unsigned j=0; j<flower->net.degree[node]; ++j) {
        int other = flower->net.neighbor[node][j];
        if (other < 0 || other >= flower->net.node_count) return false;
        double rest = distance3(flower->skin.reference[node],
                                flower->skin.reference[other]);
        double actual = distance3(flower->skin.position[node],
                                  flower->skin.position[other]);
        if (!isfinite(rest) || !isfinite(actual) || rest < 1e-8)
            return false;
        stretch += actual/rest - 1;
        count++;
    }
    FlowerInspectionSample result = {
        .node = (uint16_t)node,
        .ring = (uint16_t)ring, .slice = (uint16_t)slice,
        .growth = flower->skin.growth[node],
        .radial_growth = flower->skin.radial_growth[node],
        .reference_stretch = count ? (float)(stretch/count) : 0
    };
    if (!isfinite(result.growth) || !isfinite(result.radial_growth) ||
        !isfinite(result.reference_stretch)) return false;
    *out = result;
    return true;
}
float flower_inspection_max_growth(const Flower *flower) {
    if (!valid(flower)) return 0;
    float peak = 0;
    for (int i=0; i<flower->net.node_count; ++i) {
        float value = flower->skin.growth[i];
        if (isfinite(value) && value > peak) peak = value;
    }
    return peak;
}
static unsigned shade(double x) {
    return (unsigned)(255*maximum(0,minimum(1,x)) + 0.5);
}
uint32_t flower_inspection_color(const Flower *flower, double x, double y,
                                 int view, float peak_growth) {
    FlowerInspectionSample sample;
    if (!flower_inspection_sample(flower, x, y, &sample))
        return UINT32_C(0xfff3f4f7);
    double t = 0, red = 226, green = 228, blue = 235;
    if (view == FLOWER_INSPECTION_GROWTH) {
        t = peak_growth > 1e-7f ?
            maximum(0,minimum(1,sample.growth/peak_growth)) : 0;
        red = 226 + 10*t;
        green = 228 - 137*t;
        blue = 235 - 111*t;
    } else if (view == FLOWER_INSPECTION_STRETCH) {
        t = minimum(1, fabs(sample.reference_stretch)*6);
        if (sample.reference_stretch >= 0) {
            red = 226 + 14*t; green = 228 - 141*t; blue = 235 - 152*t;
        } else {
            red = 226 - 150*t; green = 228 - 86*t; blue = 235 - 17*t;
        }
    }
    /* Actual Net incidence: every round and every angular sector; the
     * density only changes the pixel chart, NEVER material adjacency. */
    double radius = hypot(x-CHART_CENTER,y-CHART_CENTER);
    double ring_fraction = (radius-CHART_INNER)*flower->net.rings/
                           (CHART_OUTER-CHART_INNER);
    double angle = atan2(y-CHART_CENTER,x-CHART_CENTER);
    if (angle < 0) angle += INSPECTION_TAU;
    double sector_fraction = angle*flower->net.slices/INSPECTION_TAU;
    double ring_gap = fabs(ring_fraction-floor(ring_fraction+0.5))*
                      (CHART_OUTER-CHART_INNER)/flower->net.rings;
    double sector_gap = fabs(sector_fraction-floor(sector_fraction+0.5))*
                        radius*INSPECTION_TAU/flower->net.slices;
    if (ring_gap < 0.52 || sector_gap < 0.50) {
        red = red*0.54 + 29*0.46;
        green = green*0.54 + 40*0.46;
        blue = blue*0.54 + 57*0.46;
    }
    return UINT32_C(0xff000000) |
           (shade(red/255.0) << 16) | (shade(green/255.0) << 8) |
           shade(blue/255.0);
}
