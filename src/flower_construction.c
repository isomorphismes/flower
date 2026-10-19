#include "flower_construction.h"

#include <math.h>
#include <string.h>

#define FLOWER_TAU 6.283185307179586476925286766559

bool flower_construction_pane_contains(bool visible, int viewport_width,
                                      double pointer_x, double pointer_y) {
    if (!visible || viewport_width < 1 || !isfinite(pointer_x) || !isfinite(pointer_y))
        return false;
    int left = viewport_width - FLOWER_DIAGRAM_SIDE - 18;
    if (left < 12) left = 12;
    return pointer_x >= left - 1 && pointer_x <= left + FLOWER_DIAGRAM_SIDE + 1 &&
        pointer_y >= FLOWER_DIAGRAM_TOP - 1 &&
        pointer_y <= FLOWER_DIAGRAM_TOP + FLOWER_DIAGRAM_SIDE + 24;
}

static bool valid_net(const FlowerNet *net) {
    return net && net->rings >= 1 && net->rings <= FLOWER_RINGS &&
        net->slices >= 3 && net->slices <= FLOWER_SLICES &&
        net->node_count == (net->rings + 1) * net->slices &&
        net->cell_count == net->rings * net->slices;
}
static bool has_material_edge(const FlowerNet *net, int first, int last) {
    if (first < 0 || first >= net->node_count ||
        last < 0 || last >= net->node_count || net->degree[first] > 4)
        return false;
    for (unsigned i = 0; i < net->degree[first]; ++i)
        if (net->neighbor[first][i] == last) return true;
    return false;
}
static FlowerDiagramPoint point(double center_x, double center_y,
                                double radius, double angle) {
    return (FlowerDiagramPoint){center_x + radius * cos(angle),
                                 center_y + radius * sin(angle)};
}
static FlowerDiagramPoint sum(FlowerDiagramPoint a, FlowerDiagramPoint b) {
    return (FlowerDiagramPoint){a.x + b.x, a.y + b.y};
}
static FlowerDiagramPoint diff(FlowerDiagramPoint a, FlowerDiagramPoint b) {
    return (FlowerDiagramPoint){a.x - b.x, a.y - b.y};
}
static FlowerDiagramPoint times(FlowerDiagramPoint a, double k) {
    return (FlowerDiagramPoint){a.x * k, a.y * k};
}
bool flower_material_round(const FlowerNet *net, int ring,
                           double center_x, double center_y, double radius,
                           FlowerMaterialRound *out) {
    if (!out || !valid_net(net) || ring < 0 || ring > net->rings ||
        !isfinite(center_x) || !isfinite(center_y) ||
        !isfinite(radius) || radius <= 0 || radius > 96 ||
        fabs(center_x) > 4096 || fabs(center_y) > 4096)
        return false;

    FlowerMaterialRound result = {0};
    result.ring = ring;
    result.count = net->slices;
    const double step = FLOWER_TAU / net->slices;
    const double tangent_scale = (4.0 / 3.0) * tan(step / 4.0);
    for (int slice = 0; slice < net->slices; ++slice) {
        int first = ring * net->slices + slice;
        int last = ring * net->slices + (slice + 1) % net->slices;
        if (net->ring[first] != ring || net->slice[first] != slice ||
            net->ring[last] != ring ||
            !has_material_edge(net, first, last))
            return false;
        double a = step * slice, b = step * (slice + 1);
        FlowerMaterialCubic *curve = &result.segment[slice];
        curve->first_node = (uint16_t)first;
        curve->last_node = (uint16_t)last;
        curve->control[0] = point(center_x, center_y, radius, a);
        curve->control[3] = point(center_x, center_y, radius, b);
        FlowerDiagramPoint tangent_a = {-sin(a), cos(a)};
        FlowerDiagramPoint tangent_b = {-sin(b), cos(b)};
        curve->control[1] = sum(curve->control[0],
                                times(tangent_a, tangent_scale * radius));
        curve->control[2] = diff(curve->control[3],
                                 times(tangent_b, tangent_scale * radius));
    }
    *out = result;
    return true;
}
bool flower_material_radial(const FlowerNet *net, int slice,
                            double center_x, double center_y,
                            double inner_radius, double step_radius,
                            FlowerMaterialRadial *out) {
    if (!out || !valid_net(net) || slice < 0 || slice >= net->slices ||
        !isfinite(center_x) || !isfinite(center_y) ||
        !isfinite(inner_radius) || !isfinite(step_radius) ||
        inner_radius <= 0 || step_radius <= 0 ||
        inner_radius + net->rings * step_radius > 96 ||
        fabs(center_x) > 4096 || fabs(center_y) > 4096)
        return false;
    FlowerMaterialRadial result = {0};
    result.slice = slice;
    result.count = net->rings;
    double theta = FLOWER_TAU * slice / net->slices;
    for (int ring = 0; ring < net->rings; ++ring) {
        int first = ring * net->slices + slice;
        int last = (ring + 1) * net->slices + slice;
        if (net->ring[first] != ring || net->slice[first] != slice ||
            net->ring[last] != ring + 1 || net->slice[last] != slice ||
            !has_material_edge(net, first, last))
            return false;
        FlowerMaterialCubic *curve = &result.segment[ring];
        curve->first_node = (uint16_t)first;
        curve->last_node = (uint16_t)last;
        curve->control[0] = point(center_x, center_y,
                                   inner_radius + ring * step_radius, theta);
        curve->control[3] = point(center_x, center_y,
                                   inner_radius + (ring + 1) * step_radius, theta);
        FlowerDiagramPoint direction =
            times(diff(curve->control[3], curve->control[0]), 1.0 / 3.0);
        curve->control[1] = sum(curve->control[0], direction);
        curve->control[2] = diff(curve->control[3], direction);
    }
    *out = result;
    return true;
}
