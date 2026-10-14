#ifndef FLOWER_MATERIAL_GEOMETRY_H
#define FLOWER_MATERIAL_GEOMETRY_H

#include "stitch_ledger.h"

/* This host reference is deliberately bounded; these values are forgeable,
   so public operations validate them. No camera or render density is input
   to the material definition. Lengths use one caller-declared unit. */
enum flower_geometry_status {
    FLOWER_GEOMETRY_OK, FLOWER_GEOMETRY_INVALID,
    FLOWER_GEOMETRY_DEGENERATE, FLOWER_GEOMETRY_RANGE,
    FLOWER_GEOMETRY_CAPACITY, FLOWER_GEOMETRY_IO_ERROR
};
enum flower_metric_kind {
    FLOWER_FLAT_POLAR, FLOWER_HYPERBOLIC_POLAR,
    FLOWER_EXPONENTIAL_ANNULUS, FLOWER_CYLINDER
};
struct flower_metric {
    enum flower_metric_kind kind;
    double curvature_magnitude, radius_scale;
    double first_row, last_row;
};
struct flower_vector { double x, y, z; };
struct flower_stretches { double minimum, maximum, factor, anisotropy; };
struct flower_metric_diagnostics {
    unsigned vertices, edges, faces, boundary_components;
    int euler_characteristic;
    double interior_angle_defect, boundary_turning, gauss_bonnet_residual;
    double reference_flat_area, intended_smooth_area;
    double intended_curvature_integral;
    double minimum_stretch, maximum_stretch, distortion_factor;
    double maximum_relative_edge_discrepancy;
};
struct flower_material_geometry {
    enum flower_stitch_pattern pattern;
    unsigned round_counts[FLOWER_STITCH_ROUNDS];
    double stitch_width, row_height;
    unsigned synthetic_gauge;
    struct flower_metric metric;
};
struct flower_material_marker {
    unsigned material_id, round, index, parent;
    enum flower_parent_kind parent_kind;
    double row_coordinate, angle;
    struct flower_vector position;
};

enum flower_geometry_status flower_metric_validate(const struct flower_metric *);
enum flower_geometry_status flower_metric_radius(const struct flower_metric *, double row,
    double *radius, double *derivative);
/* Local developed edge, with supplied unwrapped angles. This is not a
   shortest-path routine constrained to a finite band or around a quotient. */
enum flower_geometry_status flower_reference_edge(const struct flower_metric *,
    double first_row, double first_angle, double second_row, double second_angle,
    double *length);
enum flower_geometry_status flower_triangle_stretch(const double reference_edges[3],
    const struct flower_vector realized[3], struct flower_stretches *);
/* Explicit flat placement for polar fixtures; exact revolution for the two
   annular families. Polar hyperbolic output intentionally has nonunit stretch. */
enum flower_geometry_status flower_reference_realization(const struct flower_metric *,
    double row, double angle, struct flower_vector *);
enum flower_geometry_status flower_metric_measure(const struct flower_metric *,
    unsigned radial_segments, unsigned angular_segments, struct flower_metric_diagnostics *);
enum flower_geometry_status flower_material_build(const struct flower_stitch_ledger *,
    double stitch_width, double row_height, struct flower_material_geometry *);
enum flower_geometry_status flower_material_markers(const struct flower_stitch_ledger *,
    const struct flower_material_geometry *, struct flower_material_marker *, unsigned capacity);
enum flower_geometry_status flower_material_write_markers(const struct flower_stitch_ledger *,
    const struct flower_material_geometry *, FILE *);
enum flower_geometry_status flower_material_write_edges(const struct flower_stitch_ledger *,
    const struct flower_material_geometry *, FILE *);
/* Density changes the sampled display only; marker IDs remain in the ledger. */
enum flower_geometry_status flower_material_write_obj(const struct flower_material_geometry *,
    unsigned density, FILE *);

#endif
