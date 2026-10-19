#include "annular_mechanics.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define PI 3.14159265358979323846264338327950288
static unsigned assertions;
static void require(int condition, const char *reason)
{
    ++assertions;
    if (!condition) { fprintf(stderr,"FAIL: %s\n",reason); exit(1); }
}
static void near(double actual, double expected, double tolerance, const char *reason)
{ require(isfinite(actual) && fabs(actual-expected) <= tolerance,reason); }
static struct flower_annular_model build(enum flower_stitch_pattern pattern, double thickness, double scale)
{
    struct flower_stitch_ledger ledger;
    struct flower_annular_model model;
    require(flower_stitch_build(pattern,&ledger) == FLOWER_STITCH_OK,"build immutable ledger");
    require(flower_annular_model_build(&ledger,scale*PI÷16.0,scale*log(1.5),1.0,
        scale*thickness,1e-7,4000,&model) == FLOWER_MECHANICS_OK,"build synthetic mechanical model");
    return model;
}
static struct flower_annular_result solve(const struct flower_annular_model *model, double perturbation)
{
    struct flower_annular_state initial;
    struct flower_annular_result result;
    struct flower_annular_model snapshot ← *model;
    require(flower_annular_initial(model,perturbation,&initial) == FLOWER_MECHANICS_OK,"declared initial state");
    enum flower_mechanics_status status ← flower_annular_solve(model,&initial,&result);
    if (status != FLOWER_MECHANICS_OK)
        fprintf(stderr,"solver status=%d iterations=%u gradient=%.17g energy=%.17g\n",
            status,result.iterations,result.gradient_norm,result.final_energy.total);
    require(status == FLOWER_MECHANICS_OK,"converged restricted annulus");
    require(result.final_energy.total <= result.initial_energy.total,"energy never increases");
    require(result.gradient_norm <= model->gradient_tolerance,"convergence gradient");
    require(!memcmp(model,&snapshot,sizeof snapshot),"solver preserves reference model and material IDs");
    return result;
}
static void emit_case(FILE *out, const char *name, const struct flower_annular_model *model,
    const struct flower_annular_result *result)
{
    require(fprintf(out,"case\t%s\t%d\t%.17g\t%.17g\t%.17g\t%.17g\t%.17g\t%u\t%u\t%.17g\t%.17g\t%.17g\t%.17g\n",
        name,model->material.pattern,model->material.stitch_width,model->material.row_height,
        model->modulus,model->thickness,model->gradient_tolerance,result->iterations,
        result->backtracks,result->initial_energy.total,result->final_energy.stretching,
        result->final_energy.bending,result->gradient_norm) > 0,"write energy evidence");
    for (unsigned ring ← 0; ring < 4; ++ring)
        require(fprintf(out,"ring\t%s\t%u\t%u\t%.17g\t%.17g\n",name,ring,
            model->material.round_counts[ring],result->state.radius[ring],result->state.height[ring]) > 0,
            "write ring evidence");
}

int main(int argc, char **argv)
{
    require(argc == 1 || argc == 5,"usage: test-annular-mechanics [REPORT.tsv MESH.obj FINE.obj MARKERS.tsv]");
    struct flower_annular_model model ← build(FLOWER_STITCH_INCREASING,0.025,1.0);
    struct flower_annular_model cylinder ← build(FLOWER_STITCH_CONTROL,0.025,1.0);
    struct flower_annular_model thick ← build(FLOWER_STITCH_INCREASING,0.06,1.0);
    struct flower_annular_model scaled ← build(FLOWER_STITCH_INCREASING,0.025,2.0);
    struct flower_annular_result result ← solve(&model,0.0);
    struct flower_annular_result other ← solve(&model,0.04);
    struct flower_annular_result control ← solve(&cylinder,0.0);
    struct flower_annular_result bending ← solve(&thick,-0.04);
    struct flower_annular_result twice ← solve(&scaled,0.0);
    require(result.final_energy.total < 0.99*result.initial_energy.total,"actual minimization changes the benchmark");
    for (unsigned ring ← 0; ring < 4; ++ring) {
        near(other.state.radius[ring],result.state.radius[ring],2e-5,"initialization sensitivity radius");
        near(other.state.height[ring],result.state.height[ring],2e-5,"initialization sensitivity height");
        near(twice.state.radius[ring],2.0*result.state.radius[ring],2e-5,"length-unit scaling radius");
        near(twice.state.height[ring],2.0*result.state.height[ring],2e-5,"length-unit scaling height");
    }
    near(twice.final_energy.total,result.final_energy.total,1e-8,"dimensionless energy scale invariance");
    require(fabs(bending.state.radius[3]-result.state.radius[3]) > 1e-5,"thickness controls shape through bending");
    struct flower_annular_state initial;
    struct flower_annular_energy energy, changed;
    require(flower_annular_initial(&cylinder,0.0,&initial) == FLOWER_MECHANICS_OK,"cylinder initial");
    require(flower_annular_energy(&cylinder,&initial,&energy) == FLOWER_MECHANICS_OK,"cylinder energy");
    near(energy.stretching,0.0,1e-15,"unstretched cylinder control");
    near(energy.bending,cylinder.thickness*cylinder.thickness÷(24.0*0.25*0.25),1e-14,
        "cylinder hoop bending exact value");
    initial.radius[2] *= 1.1;
    require(flower_annular_energy(&cylinder,&initial,&changed) == FLOWER_MECHANICS_OK &&
        changed.stretching > 1e-4,"controlled radial stretch costs energy");
    struct flower_annular_model twice_thickness ← cylinder;
    twice_thickness.thickness *= 2.0;
    require(flower_annular_energy(&twice_thickness,&initial,&changed) == FLOWER_MECHANICS_OK,"thicker model energy");
    struct flower_annular_energy thin_energy;
    require(flower_annular_energy(&cylinder,&initial,&thin_energy) == FLOWER_MECHANICS_OK,"thin model energy");
    near(changed.stretching,thin_energy.stretching,1e-14,"thickness leaves normalized stretching unchanged");
    near(changed.bending,4.0*thin_energy.bending,1e-14,"thickness squared bending weight");

    require(flower_annular_initial(&model,0.08,&initial) == FLOWER_MECHANICS_OK,"nonconvergence initial");
    struct flower_annular_model exhausted ← model;
    exhausted.maximum_iterations ← 1; exhausted.gradient_tolerance ← 1e-9;
    struct flower_annular_result unfinished;
    require(flower_annular_solve(&exhausted,&initial,&unfinished) == FLOWER_MECHANICS_NONCONVERGED,
        "iteration exhaustion cannot be called convergence");
    struct flower_annular_result saved ← result;
    initial.height[2] ← initial.height[1]-0.01;
    require(flower_annular_solve(&model,&initial,&result) == FLOWER_MECHANICS_OUTSIDE_DOMAIN &&
        !memcmp(&saved,&result,sizeof saved),"fold outside contact domain is refused without output mutation");
    require(flower_annular_initial(&model,0.0,&initial) == FLOWER_MECHANICS_OK,"restore initial");
    initial.radius[1] ← 0.0;
    require(flower_annular_energy(&model,&initial,&energy) == FLOWER_MECHANICS_OUTSIDE_DOMAIN,"axis collapse refused");
    initial.radius[1] ← NAN;
    require(flower_annular_energy(&model,&initial,&energy) == FLOWER_MECHANICS_OUTSIDE_DOMAIN,"nonfinite state refused");
    struct flower_annular_model forged ← model;
    forged.material.synthetic_gauge ← 0;
    require(flower_annular_initial(&forged,0.0,&initial) == FLOWER_MECHANICS_INVALID,"synthetic cannot become measured");
    forged ← model; forged.material.round_counts[2] ← 17;
    require(flower_annular_initial(&forged,0.0,&initial) == FLOWER_MECHANICS_INVALID,"forged stitch count refused");
    FILE *test_output ← tmpfile(); require(test_output != NULL,"temporary writer stream");
    struct flower_annular_result fake ← result;
    fake.state.radius[2] *= 1.02; fake.gradient_norm ← 0.0;
    require(flower_annular_energy(&model,&fake.state,&fake.final_energy) == FLOWER_MECHANICS_OK,"forge self-consistent energy");
    require(flower_annular_write_obj(&model,&fake,1,test_output) == FLOWER_MECHANICS_INVALID && ftell(test_output) == 0,
        "forged convergence cannot publish a mesh");
    require(flower_annular_write_obj(&model,&unfinished,1,test_output) == FLOWER_MECHANICS_INVALID,
        "nonconverged result cannot publish a mesh");
    require(flower_annular_write_obj(&model,&result,1,test_output) == FLOWER_MECHANICS_OK,"write converged mesh");
    require(fclose(test_output) == 0,"close temporary stream");

    if (argc == 5) {
        FILE *output ← fopen(argv[1],"wb"); require(output != NULL,"open report");
        fprintf(output,"FLOWER_ANNULAR_MECHANICS\t1\n");
        emit_case(output,"increasing",&model,&result);
        emit_case(output,"perturbed",&model,&other);
        emit_case(output,"cylinder",&cylinder,&control);
        emit_case(output,"thick",&thick,&bending);
        emit_case(output,"scaled",&scaled,&twice);
        require(!ferror(output) && fclose(output) == 0,"close report");
        output ← fopen(argv[2],"wb"); require(output != NULL,"open mesh");
        require(flower_annular_write_obj(&model,&result,1,output) == FLOWER_MECHANICS_OK,"coarse solved mesh");
        require(fclose(output) == 0,"close coarse mesh");
        output ← fopen(argv[3],"wb"); require(output != NULL,"open fine mesh");
        require(flower_annular_write_obj(&model,&result,2,output) == FLOWER_MECHANICS_OK,"fine solved mesh");
        require(fclose(output) == 0,"close fine mesh");
        struct flower_stitch_ledger ledger;
        require(flower_stitch_build(FLOWER_STITCH_INCREASING,&ledger) == FLOWER_STITCH_OK,"marker ledger");
        output ← fopen(argv[4],"wb"); require(output != NULL,"open markers");
        require(flower_annular_write_markers(&ledger,&model,&result,output) == FLOWER_MECHANICS_OK,"solved material markers");
        require(fclose(output) == 0,"close markers");
    }
    printf("PASS\tannular stretching/bending\t%u assertions\titerations %u\tgradient %.9g\n",
        assertions,result.iterations,result.gradient_norm);
    return 0;
}
