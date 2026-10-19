#ifndef FLOWER_ANNULAR_MECHANICS_H
#define FLOWER_ANNULAR_MECHANICS_H

#include "material_geometry.h"

/* A deliberately restricted finite model: four circular coaxial rings,
   connected by straight meridian segments. The inner ring is fixed and the
   other six radius/height values are free. No arbitrary cloth/yarn is hidden
   behind this interface. Synthetic reference dimensions are immutable. */
enum flower_mechanics_status {
    FLOWER_MECHANICS_OK, FLOWER_MECHANICS_INVALID,
    FLOWER_MECHANICS_OUTSIDE_DOMAIN, FLOWER_MECHANICS_NONCONVERGED,
    FLOWER_MECHANICS_IO_ERROR
};
struct flower_annular_model {
    struct flower_material_geometry material;
    double modulus, thickness;
    double gradient_tolerance;
    unsigned maximum_iterations;
};
struct flower_annular_state {
    double radius[FLOWER_STITCH_ROUNDS], height[FLOWER_STITCH_ROUNDS];
};
struct flower_annular_energy { double stretching, bending, total; };
struct flower_annular_result {
    struct flower_annular_state state;
    struct flower_annular_energy initial_energy, final_energy;
    double gradient_norm;
    unsigned iterations, backtracks;
    enum flower_mechanics_status status;
};

enum flower_mechanics_status flower_annular_model_build(
    const struct flower_stitch_ledger *, double width, double row_height,
    double modulus, double thickness, double tolerance, unsigned iterations,
    struct flower_annular_model *);
enum flower_mechanics_status flower_annular_initial(
    const struct flower_annular_model *, double perturbation,
    struct flower_annular_state *);
enum flower_mechanics_status flower_annular_energy(
    const struct flower_annular_model *, const struct flower_annular_state *,
    struct flower_annular_energy *);
/* The returned result is inspectable on nonconvergence. Invalid input leaves
   it unchanged. Success means finite-model stationarity, not yarn calibration. */
enum flower_mechanics_status flower_annular_solve(
    const struct flower_annular_model *, const struct flower_annular_state *,
    struct flower_annular_result *);
enum flower_mechanics_status flower_annular_write_obj(
    const struct flower_annular_model *, const struct flower_annular_result *,
    unsigned display_density, FILE *);
enum flower_mechanics_status flower_annular_write_markers(
    const struct flower_stitch_ledger *, const struct flower_annular_model *,
    const struct flower_annular_result *, FILE *);

#endif
