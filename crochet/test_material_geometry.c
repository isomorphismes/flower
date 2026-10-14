#include "material_geometry.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI 3.14159265358979323846264338327950288
static unsigned checks;
static FILE *report;
static void require(int condition, const char *label)
{
    ++checks;
    if (!condition) { fprintf(stderr,"FAIL: %s\n",label); exit(1); }
}
static void near(double actual, double expected, double tolerance, const char *label)
{
    require(isfinite(actual) && isfinite(expected) && isfinite(tolerance) &&
        tolerance >= 0.0 && fabs(actual-expected) <= tolerance,label);
}
static int same_markers(const struct flower_material_marker *a,
    const struct flower_material_marker *b, unsigned count)
{
    for (unsigned i ← 0; i < count; ++i)
        if (a[i].material_id != b[i].material_id || a[i].round != b[i].round ||
            a[i].index != b[i].index || a[i].parent != b[i].parent ||
            a[i].parent_kind != b[i].parent_kind || a[i].row_coordinate != b[i].row_coordinate ||
            a[i].angle != b[i].angle || a[i].position.x != b[i].position.x ||
            a[i].position.y != b[i].position.y || a[i].position.z != b[i].position.z)
            return 0;
    return 1;
}
static struct flower_metric_diagnostics measure(struct flower_metric m, unsigned r, unsigned a)
{
    struct flower_metric_diagnostics d;
    require(flower_metric_measure(&m,r,a,&d) == FLOWER_GEOMETRY_OK,"metric measurement succeeds");
    return d;
}
static void scalar(const char *key, double value)
{ if (fprintf(report,"%s\t%.17g\n",key,value) < 0) exit(2); }
static int finish_output(FILE *output, enum flower_geometry_status status)
{
    int failed ← status != FLOWER_GEOMETRY_OK || ferror(output);
    if (fclose(output) != 0) failed ← 1;
    return failed ? 2 : 0;
}
static void diagnostics(const char *name, struct flower_metric_diagnostics d)
{
    char key[128];
#define EMIT(suffix,value) do { if (snprintf(key,sizeof(key),"%s.%s",name,suffix) < 0) exit(2); scalar(key,value); } while (0)
    EMIT("chi",d.euler_characteristic);
    EMIT("vertices",d.vertices); EMIT("edges",d.edges); EMIT("faces",d.faces);
    EMIT("boundary_components",d.boundary_components);
    EMIT("defect",d.interior_angle_defect); EMIT("boundary",d.boundary_turning);
    EMIT("gb",d.gauss_bonnet_residual); EMIT("flat_area",d.reference_flat_area);
    EMIT("smooth_area",d.intended_smooth_area); EMIT("curvature_integral",d.intended_curvature_integral);
    EMIT("minimum_stretch",d.minimum_stretch); EMIT("maximum_stretch",d.maximum_stretch);
    EMIT("factor",d.distortion_factor); EMIT("edge_discrepancy",d.maximum_relative_edge_discrepancy);
#undef EMIT
}

int main(int argc, char **argv)
{
    require(argc == 1 || (argc == 3 && (strcmp(argv[1],"--emit") == 0 ||
        strcmp(argv[1],"--mesh") == 0 || strcmp(argv[1],"--mesh-fine") == 0 ||
        strcmp(argv[1],"--markers") == 0 || strcmp(argv[1],"--material-edges") == 0)),"known mode and explicit output path");
    if (argc == 3) { report ← fopen(argv[2],"wb"); require(report != NULL,"open output path"); }
    struct flower_stitch_ledger ledger, control;
    require(flower_stitch_build(FLOWER_STITCH_INCREASING,&ledger) == FLOWER_STITCH_OK,"build increasing ledger");
    require(flower_stitch_build(FLOWER_STITCH_CONTROL,&control) == FLOWER_STITCH_OK,"build control ledger");
    struct flower_material_geometry g, cylinder;
    require(flower_material_build(&ledger,PI÷16.0,log(1.5),&g) == FLOWER_GEOMETRY_OK,"synthetic annulus");
    require(flower_material_build(&control,PI÷16.0,log(1.5),&cylinder) == FLOWER_GEOMETRY_OK,"synthetic cylinder");
    if (argc == 3 && strcmp(argv[1],"--mesh") == 0)
        return finish_output(report,flower_material_write_obj(&g,1,report));
    if (argc == 3 && strcmp(argv[1],"--mesh-fine") == 0)
        return finish_output(report,flower_material_write_obj(&g,2,report));
    if (argc == 3 && strcmp(argv[1],"--markers") == 0)
        return finish_output(report,flower_material_write_markers(&ledger,&g,report));
    if (argc == 3 && strcmp(argv[1],"--material-edges") == 0)
        return finish_output(report,flower_material_write_edges(&ledger,&g,report));

    struct flower_metric flat_disk ← {FLOWER_FLAT_POLAR,0.0,1.0,0.0,1.5};
    struct flower_metric flat_annulus ← {FLOWER_FLAT_POLAR,0.0,1.0,0.25,1.5};
    struct flower_metric hyper ← {FLOWER_HYPERBOLIC_POLAR,1.0,1.0,0.25,1.5};
    struct flower_metric hyper4 ← {FLOWER_HYPERBOLIC_POLAR,4.0,1.0,0.0,1.5};
    struct flower_metric_diagnostics fd ← measure(flat_disk,16,64);
    struct flower_metric_diagnostics fa ← measure(flat_annulus,16,64);
    struct flower_metric_diagnostics hc ← measure(hyper,16,64), hf ← measure(hyper,32,128);
    struct flower_metric_diagnostics ec ← measure(g.metric,3,216), ef ← measure(g.metric,6,432);
    struct flower_metric_diagnostics cc ← measure(cylinder.metric,3,216);
    near(fd.euler_characteristic,1,0,"disk Euler characteristic");
    near(fd.interior_angle_defect,0,1e-8,"flat disk interior curvature");
    near(fd.boundary_turning,2.0*PI,1e-8,"flat disk boundary turning");
    near(fd.gauss_bonnet_residual,0,1e-8,"disk Gauss-Bonnet");
    near(fd.minimum_stretch,1,1e-9,"flat disk minimum stretch");
    near(fd.maximum_stretch,1,1e-9,"flat disk maximum stretch");
    near(fa.euler_characteristic,0,0,"annulus Euler characteristic");
    near(fa.interior_angle_defect,0,1e-8,"flat annulus interior curvature");
    near(fa.boundary_turning,0,1e-8,"flat annulus signed boundary turns cancel");
    near(fa.gauss_bonnet_residual,0,1e-8,"flat annulus Gauss-Bonnet");
    require(hc.interior_angle_defect < -1.0 && hc.boundary_turning > 1.0,"negative target curvature needs positive boundary term");
    near(hc.gauss_bonnet_residual,0,1e-8,"hyperbolic coarse Gauss-Bonnet");
    near(hf.gauss_bonnet_residual,0,1e-8,"hyperbolic fine Gauss-Bonnet");
    require(fabs(hf.interior_angle_defect-hf.intended_curvature_integral) <
        fabs(hc.interior_angle_defect-hc.intended_curvature_integral),"polar intrinsic curvature convergence");
    require(hf.minimum_stretch < 0.8 && hf.maximum_relative_edge_discrepancy > 0.2,
        "flat R3 placement must report hyperbolic target distortion");
    require(fabs(hf.reference_flat_area-hf.intended_smooth_area) <
        fabs(hc.reference_flat_area-hc.intended_smooth_area),"polar PL area convergence");
    near(ec.gauss_bonnet_residual,0,1e-8,"exponential annulus Gauss-Bonnet");
    near(ef.gauss_bonnet_residual,0,1e-8,"refined exponential annulus Gauss-Bonnet");
    require(ec.distortion_factor > 1.000001,"smooth exact isometry does not certify chord mesh");
    require(ef.distortion_factor < ec.distortion_factor,"revolution mesh distortion converges");
    require(fabs(ef.reference_flat_area-ef.intended_smooth_area) <
        fabs(ec.reference_flat_area-ec.intended_smooth_area),"revolution reference PL area converges");
    near(cc.interior_angle_defect,0,1e-8,"cylinder target is intrinsically flat");
    near(cc.boundary_turning,0,1e-8,"cylinder boundary turns");
    near(cc.minimum_stretch,sin(PI÷216.0)÷(PI÷216.0),1e-8,"cylinder circumferential chord error");
    near(cc.maximum_stretch,1,1e-8,"cylinder meridional stretch");

    double radius, derivative, length, radial4;
    require(flower_metric_radius(&hyper4,1.0,&radius,&derivative) == FLOWER_GEOMETRY_OK,"polar kappa4 radius");
    near(radius,sinh(2.0)÷2.0,1e-12,"sqrt curvature radius factor");
    near(derivative,cosh(2.0),1e-12,"polar derivative factor");
    double radius4 ← radius;
    require(flower_reference_edge(&hyper4,0.25,0.4,1.25,0.4,&radial4) == FLOWER_GEOMETRY_OK,"radial H2 distance");
    near(radial4,1.0,1e-12,"radial coordinate is intrinsic length");
    require(flower_reference_edge(&g.metric,0.0,0.0,g.row_height,0.0,&length) == FLOWER_GEOMETRY_OK,"annular meridian length");
    near(length,g.row_height,1e-12,"row height only for same angle");
    struct flower_material_marker markers[65], unchanged[65];
    require(flower_material_markers(&ledger,&g,markers,65) == FLOWER_GEOMETRY_OK,"material markers");
    memcpy(unchanged,markers,sizeof(markers));
    const double radii[4] ← {0.25,0.375,0.5625,0.84375};
    for (unsigned r ← 0; r < 4; ++r) {
        require(flower_metric_radius(&g.metric,(double)r*g.row_height,&radius,&derivative) == FLOWER_GEOMETRY_OK,"row radius");
        near(radius,radii[r],1e-12,"synthetic exact row radius");
        near(2.0*PI*radius,(double)ledger.rounds[r].count*g.stitch_width,1e-12,"count-width circumference equality");
        near(derivative*derivative+(1.0-derivative*derivative),1,1e-14,"analytic revolution pullback uu");
    }
    for (unsigned id ← 0; id < 65; ++id) {
        require(markers[id].material_id == ledger.stitches[id].id,"marker material ID");
        require(markers[id].parent == ledger.stitches[id].parent &&
            markers[id].parent_kind == ledger.stitches[id].parent_kind,"marker parent role");
        if (id >= 8) require(markers[id].position.z > markers[ledger.stitches[id].parent].position.z,
            "supported realization has strictly increasing row height");
    }
    struct flower_material_geometry scaled;
    require(flower_material_build(&ledger,PI÷8.0,2.0*log(1.5),&scaled) == FLOWER_GEOMETRY_OK,"uniform gauge scaling");
    struct flower_material_marker twice[65];
    require(flower_material_markers(&ledger,&scaled,twice,65) == FLOWER_GEOMETRY_OK,"scaled markers");
    for (unsigned id ← 0; id < 65; ++id) {
        near(twice[id].position.x,2.0*markers[id].position.x,1e-12,"length unit scaling x");
        near(twice[id].position.y,2.0*markers[id].position.y,1e-12,"length unit scaling y");
        near(twice[id].position.z,2.0*markers[id].position.z,1e-12,"length unit scaling z");
    }
    near(scaled.metric.curvature_magnitude,g.metric.curvature_magnitude÷4.0,1e-12,"curvature inverse length squared");

    const double reference[3] ← {1.0,1.0,sqrt(2.0)};
    struct flower_vector p[3] ← {{0,0,0},{2,0,0},{0,0.5,0}};
    struct flower_stretches stretch;
    require(flower_triangle_stretch(reference,p,&stretch) == FLOWER_GEOMETRY_OK,"anisotropic triangle");
    near(stretch.minimum,0.5,1e-12,"known small singular stretch");
    near(stretch.maximum,2,1e-12,"known large singular stretch");
    near(stretch.factor,2,1e-12,"two-sided distortion factor");
    near(stretch.anisotropy,4,1e-12,"anisotropy differs from factor");
    p[0] ← (struct flower_vector){3,-2,7}; p[1] ← (struct flower_vector){3,0,7}; p[2] ← (struct flower_vector){3,-2,7.5};
    require(flower_triangle_stretch(reference,p,&stretch) == FLOWER_GEOMETRY_OK,"rigidly moved triangle");
    near(stretch.minimum,0.5,1e-12,"rigid invariance minimum");
    near(stretch.maximum,2,1e-12,"rigid invariance maximum");
    struct flower_stretches known_triangle ← stretch;
    /* A flat material hinge may look folded while each face remains isometric.
       Stretch says nothing about global intersection/contact of such faces. */
    struct flower_vector folded[3] ← {{0,0,0},{1,0,0},{0,-0.5,sqrt(3.0)÷2.0}};
    require(flower_triangle_stretch(reference,folded,&stretch) == FLOWER_GEOMETRY_OK,"folded flat face");
    near(stretch.minimum,1,1e-12,"fold cannot create intrinsic curvature minimum");
    near(stretch.maximum,1,1e-12,"fold cannot create intrinsic curvature maximum");

    struct flower_metric invalid ← hyper;
    invalid.curvature_magnitude ← -1.0;
    require(flower_metric_validate(&invalid) == FLOWER_GEOMETRY_INVALID,"negative curvature magnitude invalid");
    invalid ← hyper; invalid.last_row ← NAN;
    require(flower_metric_validate(&invalid) == FLOWER_GEOMETRY_INVALID,"nonfinite metric invalid");
    length ← 7;
    require(flower_reference_edge(&hyper,NAN,0,1,0,&length) == FLOWER_GEOMETRY_INVALID && length == 7,
        "bad edge leaves output unchanged");
    require(flower_reference_edge(&hyper,0.25,0,1,4,&length) == FLOWER_GEOMETRY_INVALID,"ambiguous long angular lift invalid");
    const double degenerate[3] ← {1,1,2};
    struct flower_stretches saved ← stretch;
    require(flower_triangle_stretch(degenerate,p,&stretch) == FLOWER_GEOMETRY_DEGENERATE &&
        memcmp(&saved,&stretch,sizeof(stretch)) == 0,"degenerate reference rejected without output mutation");
    p[2] ← p[1];
    require(flower_triangle_stretch(reference,p,&stretch) == FLOWER_GEOMETRY_DEGENERATE,"collapsed realized triangle rejected");
    p[2].x ← NAN;
    require(flower_triangle_stretch(reference,p,&stretch) == FLOWER_GEOMETRY_INVALID,"NaN triangle rejected");
    struct flower_material_geometry bad_g ← g;
    bad_g.round_counts[2] ← 17;
    require(flower_material_markers(&ledger,&bad_g,markers,65) == FLOWER_GEOMETRY_INVALID &&
        memcmp(markers,unchanged,sizeof(markers)) == 0,"count mutant rejected without marker mutation");
    bad_g ← g; bad_g.synthetic_gauge ← 0;
    require(flower_material_markers(&ledger,&bad_g,markers,65) == FLOWER_GEOMETRY_INVALID,"cannot claim measured calibration");
    require(flower_material_markers(&ledger,&g,markers,64) == FLOWER_GEOMETRY_CAPACITY,"insufficient marker capacity");
    require(flower_material_build(&ledger,0.5,log(1.5),&bad_g) == FLOWER_GEOMETRY_RANGE,"unsupported revolution radius rejected");
    invalid ← g.metric; invalid.last_row ← 4.0*log(1.5);
    struct flower_vector sentinel ← {11,12,13};
    require(flower_reference_realization(&invalid,invalid.last_row,0,&sentinel) == FLOWER_GEOMETRY_RANGE &&
        sentinel.x == 11 && sentinel.y == 12 && sentinel.z == 13,"continuation beyond revolution family refused");
    FILE *display ← tmpfile(); require(display != NULL,"temporary display stream");
    require(flower_material_write_obj(&g,1,display) == FLOWER_GEOMETRY_OK,"coarse display export");
    require(flower_material_write_obj(&g,2,display) == FLOWER_GEOMETRY_OK,"fine display export");
    require(fclose(display) == 0,"display stream close");
    require(flower_material_markers(&ledger,&g,markers,65) == FLOWER_GEOMETRY_OK &&
        same_markers(markers,unchanged,65),"display density cannot mutate authoritative markers");
    require(flower_metric_measure(&hyper,65,128,&hf) == FLOWER_GEOMETRY_INVALID,"bounded refinement limit");
    require(flower_triangle_stretch(NULL,p,&stretch) == FLOWER_GEOMETRY_INVALID,"null reference rejected");

    if (argc == 3) {
        scalar("format",1); scalar("synthetic_gauge",g.synthetic_gauge);
        scalar("stitches",ledger.stitch_count); scalar("increases",ledger.increase_count);
        scalar("stitch_width",g.stitch_width); scalar("row_height",g.row_height);
        scalar("hyperbolic_polar_4.radius1",radius4);
        scalar("hyperbolic_polar_4.circumference1",2.0*PI*radius4);
        scalar("hyperbolic_polar_4.radial_edge",radial4);
        for (unsigned r ← 0; r < 4; ++r) {
            char key[64];
            require(flower_metric_radius(&g.metric,(double)r*g.row_height,&radius,&derivative) == FLOWER_GEOMETRY_OK,"emitted radius");
            snprintf(key,sizeof(key),"exponential.radius%u",r); scalar(key,radius);
        }
        scalar("triangle.minimum_stretch",known_triangle.minimum); scalar("triangle.maximum_stretch",known_triangle.maximum);
        scalar("triangle.factor",known_triangle.factor); scalar("triangle.anisotropy",known_triangle.anisotropy);
        diagnostics("flat_disk",fd); diagnostics("flat_annulus",fa);
        diagnostics("hyperbolic_annulus.coarse",hc); diagnostics("hyperbolic_annulus.fine",hf);
        diagnostics("exponential.coarse",ec); diagnostics("exponential.fine",ef);
        diagnostics("cylinder",cc);
    }
    fprintf(stderr,"Flower material geometry: PASS (%u assertions)\n",checks);
    return argc == 3 ? finish_output(report,FLOWER_GEOMETRY_OK) : 0;
}
