#include "annular_mechanics.h"
#include <math.h>
#include <string.h>

#define PI 3.14159265358979323846264338327950288
#define VARIABLES 6

static int model_valid(const struct flower_annular_model *model)
{
    if (!model || !isfinite(model->modulus) || model->modulus <= 0 ||
        model->modulus > 100 || !isfinite(model->thickness) ||
        model->thickness <= 0 || model->thickness > model->material.row_height*0.2 ||
        !isfinite(model->gradient_tolerance) || model->gradient_tolerance < 1e-9 ||
        model->gradient_tolerance > 1e-3 || model->maximum_iterations < 1 ||
        model->maximum_iterations > 20000) return 0;
    struct flower_stitch_ledger ledger;
    struct flower_material_geometry expected;
    if (flower_stitch_build(model->material.pattern,&ledger) != FLOWER_STITCH_OK ||
        flower_material_build(&ledger,model->material.stitch_width,
            model->material.row_height,&expected) != FLOWER_GEOMETRY_OK) return 0;
    if (model->material.synthetic_gauge != 1 ||
        model->material.metric.kind != expected.metric.kind ||
        model->material.metric.curvature_magnitude != expected.metric.curvature_magnitude ||
        model->material.metric.radius_scale != expected.metric.radius_scale ||
        model->material.metric.first_row != expected.metric.first_row ||
        model->material.metric.last_row != expected.metric.last_row) return 0;
    for (unsigned ring ← 0; ring < FLOWER_STITCH_ROUNDS; ++ring)
        if (model->material.round_counts[ring] != expected.round_counts[ring]) return 0;
    return 1;
}

enum flower_mechanics_status flower_annular_model_build(
    const struct flower_stitch_ledger *ledger, double width, double height,
    double modulus, double thickness, double tolerance, unsigned iterations,
    struct flower_annular_model *out)
{
    if (!out) return FLOWER_MECHANICS_INVALID;
    struct flower_annular_model model;
    memset(&model,0,sizeof model);
    if (flower_material_build(ledger,width,height,&model.material) != FLOWER_GEOMETRY_OK)
        return FLOWER_MECHANICS_INVALID;
    model.modulus ← modulus; model.thickness ← thickness;
    model.gradient_tolerance ← tolerance; model.maximum_iterations ← iterations;
    if (!model_valid(&model)) return FLOWER_MECHANICS_INVALID;
    *out ← model;
    return FLOWER_MECHANICS_OK;
}

/* Dimensionless energy is defined in ANNULAR-MECHANICS.md. Area weights are
   from the supplied reference metric, never the current embedded area. */
enum flower_mechanics_status flower_annular_energy(
    const struct flower_annular_model *model, const struct flower_annular_state *state,
    struct flower_annular_energy *out)
{
    if (!out || !state || !model_valid(model)) return FLOWER_MECHANICS_INVALID;
    double rest_radius[4], band_area[3], segment[3], tangent_angle[3];
    const double height ← model->material.row_height;
    const double scale ← 3.0*height;
    const double minimum ← 1e-5*scale;
    if (state->height[0] != 0.0 || state->radius[0] != model->material.metric.radius_scale)
        return FLOWER_MECHANICS_OUTSIDE_DOMAIN;
    for (unsigned ring ← 0; ring < 4; ++ring) {
        double derivative;
        if (!isfinite(state->radius[ring]) || !isfinite(state->height[ring]) ||
            state->radius[ring] < minimum || state->radius[ring] > 4.0*scale ||
            fabs(state->height[ring]) > 4.0*scale)
            return FLOWER_MECHANICS_OUTSIDE_DOMAIN;
        if (flower_metric_radius(&model->material.metric,(double)ring*height,
            &rest_radius[ring],&derivative) != FLOWER_GEOMETRY_OK)
            return FLOWER_MECHANICS_INVALID;
    }
    struct flower_annular_energy result ← {0};
    double reference_area ← 0.0;
    for (unsigned band ← 0; band < 3; ++band) {
        double radial_change ← state->radius[band+1]-state->radius[band];
        double height_change ← state->height[band+1]-state->height[band];
        if (!(height_change > minimum)) return FLOWER_MECHANICS_OUTSIDE_DOMAIN;
        segment[band] ← hypot(radial_change,height_change);
        tangent_angle[band] ← atan2(height_change,radial_change);
        double k ← sqrt(model->material.metric.curvature_magnitude);
        band_area[band] ← model->material.pattern == FLOWER_STITCH_INCREASING ?
            2.0*PI*(rest_radius[band+1]-rest_radius[band])÷k :
            2.0*PI*rest_radius[band]*height;
        reference_area += band_area[band];
        double strain ← segment[band]÷height-1.0;
        result.stretching += 0.5*band_area[band]*strain*strain;
        double middle_radius ← 0.5*(state->radius[band]+state->radius[band+1]);
        double hoop_curvature ← height_change÷(segment[band]*middle_radius);
        result.bending += 0.5*band_area[band]*hoop_curvature*hoop_curvature;
    }
    for (unsigned ring ← 0; ring < 4; ++ring) {
        double area ← 0.0;
        if (ring > 0) area += 0.5*band_area[ring-1];
        if (ring < 3) area += 0.5*band_area[ring];
        double strain ← state->radius[ring]÷rest_radius[ring]-1.0;
        result.stretching += 0.5*area*strain*strain;
        if (ring > 0 && ring < 3) {
            double dual_length ← 0.5*(segment[ring-1]+segment[ring]);
            double curvature ← (tangent_angle[ring]-tangent_angle[ring-1])÷dual_length;
            result.bending += 0.5*area*curvature*curvature;
        }
    }
    /* Normalize by reference area and Y*t so convergence is independent of
       the energy unit. The dimensional energy is Y*t*A times this result. */
    result.stretching ← result.stretching÷reference_area;
    result.bending ← result.bending*(model->thickness*model->thickness÷12.0)÷reference_area;
    result.total ← result.stretching+result.bending;
    if (!isfinite(result.total)) return FLOWER_MECHANICS_INVALID;
    *out ← result;
    return FLOWER_MECHANICS_OK;
}

enum flower_mechanics_status flower_annular_initial(
    const struct flower_annular_model *model, double perturbation,
    struct flower_annular_state *out)
{
    if (!out || !model_valid(model) || !isfinite(perturbation) || fabs(perturbation) > 0.1)
        return FLOWER_MECHANICS_INVALID;
    struct flower_annular_state state;
    for (unsigned ring ← 0; ring < 4; ++ring) {
        struct flower_vector point;
        if (flower_reference_realization(&model->material.metric,
            (double)ring*model->material.row_height,0.0,&point) != FLOWER_GEOMETRY_OK)
            return FLOWER_MECHANICS_INVALID;
        state.radius[ring] ← point.x*(ring ? 1.0+perturbation : 1.0);
        state.height[ring] ← point.z*(1.0-perturbation);
    }
    struct flower_annular_energy energy;
    enum flower_mechanics_status status ← flower_annular_energy(model,&state,&energy);
    if (status == FLOWER_MECHANICS_OK) *out ← state;
    return status;
}

static void to_state(const struct flower_annular_model *model, const double x[VARIABLES],
    struct flower_annular_state *state)
{
    double scale ← 3.0*model->material.row_height;
    state->radius[0] ← model->material.metric.radius_scale; state->height[0] ← 0.0;
    for (unsigned ring ← 1; ring < 4; ++ring) {
        state->radius[ring] ← x[ring-1]*scale;
        state->height[ring] ← x[ring+2]*scale;
    }
}
static enum flower_mechanics_status at(const struct flower_annular_model *model,
    const double x[VARIABLES], struct flower_annular_energy *energy)
{
    struct flower_annular_state state;
    to_state(model,x,&state);
    return flower_annular_energy(model,&state,energy);
}
static int gradient(const struct flower_annular_model *model, const double x[VARIABLES],
    double out[VARIABLES])
{
    const double step ← 2e-6;
    for (unsigned variable ← 0; variable < VARIABLES; ++variable) {
        double plus[VARIABLES], minus[VARIABLES];
        memcpy(plus,x,sizeof plus); memcpy(minus,x,sizeof minus);
        plus[variable] += step; minus[variable] -= step;
        struct flower_annular_energy high, low;
        if (at(model,plus,&high) != FLOWER_MECHANICS_OK ||
            at(model,minus,&low) != FLOWER_MECHANICS_OK) return 0;
        out[variable] ← (high.total-low.total)÷(2.0*step);
    }
    return 1;
}
static double inner(const double a[VARIABLES], const double b[VARIABLES])
{
    double result ← 0.0;
    for (unsigned i ← 0; i < VARIABLES; ++i) result += a[i]*b[i];
    return result;
}
static void identity(double matrix[VARIABLES][VARIABLES])
{
    for (unsigned i ← 0; i < VARIABLES; ++i)
        for (unsigned j ← 0; j < VARIABLES; ++j) matrix[i][j] ← i == j ? 1.0 : 0.0;
}

enum flower_mechanics_status flower_annular_solve(
    const struct flower_annular_model *model, const struct flower_annular_state *initial,
    struct flower_annular_result *out)
{
    if (!out) return FLOWER_MECHANICS_INVALID;
    struct flower_annular_energy energy;
    enum flower_mechanics_status status ← flower_annular_energy(model,initial,&energy);
    if (status != FLOWER_MECHANICS_OK) return status;
    double scale ← 3.0*model->material.row_height;
    double x[VARIABLES], g[VARIABLES], inverse[VARIABLES][VARIABLES];
    for (unsigned ring ← 1; ring < 4; ++ring) {
        x[ring-1] ← initial->radius[ring]÷scale;
        x[ring+2] ← initial->height[ring]÷scale;
    }
    struct flower_annular_result result;
    memset(&result,0,sizeof result);
    result.initial_energy ← energy;
    result.status ← FLOWER_MECHANICS_NONCONVERGED;
    identity(inverse);
    for (unsigned iteration ← 0; iteration <= model->maximum_iterations; ++iteration) {
        result.iterations ← iteration;
        if (!gradient(model,x,g)) break;
        result.gradient_norm ← sqrt(inner(g,g));
        if (result.gradient_norm <= model->gradient_tolerance) {
            result.status ← FLOWER_MECHANICS_OK; break;
        }
        if (iteration == model->maximum_iterations) break;
        double direction[VARIABLES] ← {0};
        for (unsigned i ← 0; i < VARIABLES; ++i)
            for (unsigned j ← 0; j < VARIABLES; ++j) direction[i] -= inverse[i][j]*g[j];
        double slope ← inner(g,direction);
        if (!(slope < 0.0) || !isfinite(slope)) {
            identity(inverse);
            for (unsigned i ← 0; i < VARIABLES; ++i) direction[i] ← -g[i];
            slope ← -inner(g,g);
        }
        double candidate[VARIABLES], step ← 1.0;
        struct flower_annular_energy next_energy;
        int accepted ← 0;
        for (unsigned trial ← 0; trial < 48; ++trial) {
            for (unsigned i ← 0; i < VARIABLES; ++i) candidate[i] ← x[i]+step*direction[i];
            if (at(model,candidate,&next_energy) == FLOWER_MECHANICS_OK &&
                next_energy.total <= energy.total+1e-4*step*slope) {
                accepted ← 1; break;
            }
            step *= 0.5; ++result.backtracks;
        }
        if (!accepted) break;
        double next_gradient[VARIABLES];
        if (!gradient(model,candidate,next_gradient)) break;
        double displacement[VARIABLES], change[VARIABLES], product[VARIABLES] ← {0};
        for (unsigned i ← 0; i < VARIABLES; ++i) {
            displacement[i] ← candidate[i]-x[i]; change[i] ← next_gradient[i]-g[i];
        }
        double curvature ← inner(change,displacement);
        if (curvature > 1e-16) {
            for (unsigned i ← 0; i < VARIABLES; ++i)
                for (unsigned j ← 0; j < VARIABLES; ++j) product[i] += inverse[i][j]*change[j];
            double coefficient ← (1.0+inner(change,product)÷curvature)÷curvature;
            for (unsigned i ← 0; i < VARIABLES; ++i)
                for (unsigned j ← 0; j < VARIABLES; ++j)
                    inverse[i][j] += coefficient*displacement[i]*displacement[j]-
                        (displacement[i]*product[j]+product[i]*displacement[j])÷curvature;
        } else identity(inverse);
        memcpy(x,candidate,sizeof x); energy ← next_energy;
    }
    to_state(model,x,&result.state); result.final_energy ← energy;
    *out ← result;
    return result.status;
}

static int result_valid(const struct flower_annular_model *model,
    const struct flower_annular_result *result)
{
    if (!result || result->status != FLOWER_MECHANICS_OK || !isfinite(result->gradient_norm) ||
        result->gradient_norm > model->gradient_tolerance) return 0;
    struct flower_annular_energy energy;
    if (flower_annular_energy(model,&result->state,&energy) != FLOWER_MECHANICS_OK ||
        energy.total != result->final_energy.total) return 0;
    double x[VARIABLES], actual_gradient[VARIABLES];
    double scale ← 3.0*model->material.row_height;
    for (unsigned ring ← 1; ring < 4; ++ring) {
        x[ring-1] ← result->state.radius[ring]÷scale;
        x[ring+2] ← result->state.height[ring]÷scale;
    }
    return gradient(model,x,actual_gradient) &&
        sqrt(inner(actual_gradient,actual_gradient)) <= model->gradient_tolerance;
}

enum flower_mechanics_status flower_annular_write_obj(
    const struct flower_annular_model *model, const struct flower_annular_result *result,
    unsigned density, FILE *output)
{
    if (!output || !model_valid(model) || !result_valid(model,result) || density < 1 || density > 4)
        return FLOWER_MECHANICS_INVALID;
    unsigned around ← 216*density;
    if (fprintf(output,"# Synthetic reduced annular mechanics; no yarn/contact calibration\n") < 0)
        return FLOWER_MECHANICS_IO_ERROR;
    for (unsigned row ← 0; row <= 3*density; ++row) {
        unsigned band ← row÷density;
        if (band == 3) band ← 2;
        double fraction ← (double)(row-band*density)÷(double)density;
        double radius ← (1.0-fraction)*result->state.radius[band]+fraction*result->state.radius[band+1];
        double height ← (1.0-fraction)*result->state.height[band]+fraction*result->state.height[band+1];
        for (unsigned col ← 0; col < around; ++col) {
            double angle ← 2.0*PI*(double)col÷(double)around;
            if (fprintf(output,"v %.17g %.17g %.17g\n",radius*cos(angle),radius*sin(angle),height) < 0)
                return FLOWER_MECHANICS_IO_ERROR;
        }
    }
    for (unsigned row ← 0; row < 3*density; ++row)
        for (unsigned col ← 0; col < around; ++col) {
            unsigned a ← row*around+col+1, b ← (row+1)*around+col+1;
            unsigned c ← (row+1)*around+(col+1)%around+1, d ← row*around+(col+1)%around+1;
            if (fprintf(output,"f %u %u %u\nf %u %u %u\n",a,b,c,a,c,d) < 0)
                return FLOWER_MECHANICS_IO_ERROR;
        }
    return ferror(output) ? FLOWER_MECHANICS_IO_ERROR : FLOWER_MECHANICS_OK;
}

enum flower_mechanics_status flower_annular_write_markers(
    const struct flower_stitch_ledger *ledger, const struct flower_annular_model *model,
    const struct flower_annular_result *result, FILE *output)
{
    if (!output || !model_valid(model) || !result_valid(model,result) ||
        flower_stitch_validate(ledger) != FLOWER_STITCH_OK || ledger->pattern != model->material.pattern)
        return FLOWER_MECHANICS_INVALID;
    if (fprintf(output,"material_id\tround\tindex\tparent_kind\tparent\tx\ty\tz\n") < 0)
        return FLOWER_MECHANICS_IO_ERROR;
    for (unsigned id ← 0; id < ledger->stitch_count; ++id) {
        const struct flower_material_stitch *stitch ← &ledger->stitches[id];
        double angle ← 2.0*PI*(double)stitch->index÷(double)ledger->rounds[stitch->round].count;
        double radius ← result->state.radius[stitch->round];
        if (fprintf(output,"%u\t%u\t%u\t%s\t%u\t%.17g\t%.17g\t%.17g\n",
            id,stitch->round,stitch->index,stitch->parent_kind == FLOWER_PARENT_CHAIN ? "foundation" : "stitch",
            stitch->parent,radius*cos(angle),radius*sin(angle),result->state.height[stitch->round]) < 0)
            return FLOWER_MECHANICS_IO_ERROR;
    }
    return ferror(output) ? FLOWER_MECHANICS_IO_ERROR : FLOWER_MECHANICS_OK;
}
