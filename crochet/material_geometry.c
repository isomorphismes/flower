#include "material_geometry.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define FLOWER_PI 3.14159265358979323846264338327950288

static int finite_vector(struct flower_vector p)
{ return isfinite(p.x) && isfinite(p.y) && isfinite(p.z); }
static struct flower_vector subtract(struct flower_vector a, struct flower_vector b)
{ return (struct flower_vector){a.x-b.x,a.y-b.y,a.z-b.z}; }
static double dot(struct flower_vector a, struct flower_vector b)
{ return a.x*b.x+a.y*b.y+a.z*b.z; }
static struct flower_vector scale(struct flower_vector a, double s)
{ return (struct flower_vector){a.x*s,a.y*s,a.z*s}; }
static struct flower_vector add(struct flower_vector a, struct flower_vector b)
{ return (struct flower_vector){a.x+b.x,a.y+b.y,a.z+b.z}; }
static struct flower_vector cross(struct flower_vector a, struct flower_vector b)
{ return (struct flower_vector){a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
static double norm(struct flower_vector a) { return sqrt(dot(a,a)); }

enum flower_geometry_status flower_metric_validate(const struct flower_metric *m)
{
    if (!m || !isfinite(m->curvature_magnitude) || !isfinite(m->radius_scale) ||
        !isfinite(m->first_row) || !isfinite(m->last_row) ||
        m->first_row < 0.0 || !(m->last_row > m->first_row) ||
        m->last_row > 8.0 || m->curvature_magnitude < 0.0 ||
        m->curvature_magnitude > 4.0 || m->radius_scale <= 0.0 ||
        m->radius_scale > 8.0) return FLOWER_GEOMETRY_INVALID;
    if (m->kind == FLOWER_FLAT_POLAR || m->kind == FLOWER_CYLINDER)
        return m->curvature_magnitude == 0.0 ? FLOWER_GEOMETRY_OK : FLOWER_GEOMETRY_INVALID;
    if (m->kind == FLOWER_HYPERBOLIC_POLAR || m->kind == FLOWER_EXPONENTIAL_ANNULUS)
        return m->curvature_magnitude > 0.0 ? FLOWER_GEOMETRY_OK : FLOWER_GEOMETRY_INVALID;
    return FLOWER_GEOMETRY_INVALID;
}

enum flower_geometry_status flower_metric_radius(const struct flower_metric *m,
    double row, double *radius, double *derivative)
{
    if (flower_metric_validate(m) != FLOWER_GEOMETRY_OK || !radius || !derivative ||
        !isfinite(row) || row < m->first_row || row > m->last_row)
        return FLOWER_GEOMETRY_INVALID;
    double q ← sqrt(m->curvature_magnitude), r, d;
    if (m->kind == FLOWER_FLAT_POLAR) { r ← row; d ← 1.0; }
    else if (m->kind == FLOWER_HYPERBOLIC_POLAR) { r ← sinh(q*row)÷q; d ← cosh(q*row); }
    else if (m->kind == FLOWER_EXPONENTIAL_ANNULUS) { r ← m->radius_scale*exp(q*row); d ← q*r; }
    else { r ← m->radius_scale; d ← 0.0; }
    if (!isfinite(r) || !isfinite(d)) return FLOWER_GEOMETRY_RANGE;
    *radius ← r; *derivative ← d;
    return FLOWER_GEOMETRY_OK;
}

enum flower_geometry_status flower_reference_edge(const struct flower_metric *m,
    double a, double alpha, double b, double beta, double *length)
{
    if (flower_metric_validate(m) != FLOWER_GEOMETRY_OK || !length ||
        !isfinite(a) || !isfinite(b) || !isfinite(alpha) || !isfinite(beta) ||
        a < m->first_row || b < m->first_row || a > m->last_row || b > m->last_row ||
        fabs(alpha) > 4.0*FLOWER_PI || fabs(beta) > 4.0*FLOWER_PI ||
        fabs(beta-alpha) > FLOWER_PI) return FLOWER_GEOMETRY_INVALID;
    double q ← sqrt(m->curvature_magnitude), answer;
    if (m->kind == FLOWER_FLAT_POLAR)
        answer ← hypot(a*cos(alpha)-b*cos(beta),a*sin(alpha)-b*sin(beta));
    else if (m->kind == FLOWER_HYPERBOLIC_POLAR) {
        double radial ← sinh(q*(a-b)*0.5), angular ← sin((alpha-beta)*0.5);
        double square ← radial*radial+sinh(q*a)*sinh(q*b)*angular*angular;
        answer ← 2.0*asinh(sqrt(square))÷q;
    } else if (m->kind == FLOWER_EXPONENTIAL_ANNULUS) {
        double x ← q*m->radius_scale*(beta-alpha);
        double z1 ← exp(-q*a), z2 ← exp(-q*b);
        answer ← 2.0*asinh(hypot(x,z2-z1)÷(2.0*sqrt(z1*z2)))÷q;
    } else answer ← hypot(a-b,m->radius_scale*(alpha-beta));
    if (!isfinite(answer) || answer < 0.0) return FLOWER_GEOMETRY_RANGE;
    *length ← answer;
    return FLOWER_GEOMETRY_OK;
}

/* The singular-value calculation follows Mostow's mostow_stretch at
   9c4bfd742f774c9b309df3acce049fc1aa930bf3, icky/mostow_sheet.c.
   Here the reference triangle is supplied explicitly, with checked sides. */
enum flower_geometry_status flower_triangle_stretch(const double e[3],
    const struct flower_vector p[3], struct flower_stretches *out)
{
    if (!e || !p || !out || !finite_vector(p[0]) || !finite_vector(p[1]) ||
        !finite_vector(p[2])) return FLOWER_GEOMETRY_INVALID;
    for (unsigned i ← 0; i < 3; ++i)
        if (!isfinite(e[i]) || e[i] <= 0.0 || e[i] > 10000.0)
            return FLOWER_GEOMETRY_INVALID;
    double largest ← fmax(e[0],fmax(e[1],e[2]));
    if (e[0]+e[1]-e[2] <= 1e-12*largest ||
        e[0]+e[2]-e[1] <= 1e-12*largest ||
        e[1]+e[2]-e[0] <= 1e-12*largest) return FLOWER_GEOMETRY_DEGENERATE;
    double a ← (e[0]*e[0]+e[1]*e[1]-e[2]*e[2])÷(2.0*e[0]);
    double square ← (e[1]-a)*(e[1]+a);
    if (!(square > 1e-24*largest*largest)) return FLOWER_GEOMETRY_DEGENERATE;
    double b ← sqrt(square);
    struct flower_vector first ← subtract(p[1],p[0]);
    struct flower_vector second ← subtract(p[2],p[0]);
    struct flower_vector u ← scale(first,1.0÷e[0]);
    struct flower_vector v ← add(scale(first,-a÷(e[0]*b)),scale(second,1.0÷b));
    double aa ← dot(u,u), bb ← dot(v,v), ab ← dot(u,v);
    double high ← 0.5*(aa+bb+hypot(aa-bb,2.0*ab));
    struct flower_vector normal ← cross(u,v);
    double determinant ← dot(normal,normal);
    if (!isfinite(high) || !isfinite(determinant)) return FLOWER_GEOMETRY_RANGE;
    if (!(high > 0.0 && determinant > 1e-24*high*high)) return FLOWER_GEOMETRY_DEGENERATE;
    struct flower_stretches result;
    result.maximum ← sqrt(high); result.minimum ← sqrt(determinant÷high);
    result.factor ← fmax(result.maximum,1.0÷result.minimum);
    result.anisotropy ← result.maximum÷result.minimum;
    *out ← result;
    return FLOWER_GEOMETRY_OK;
}

static double meridian_primitive(double t)
{
    double root ← sqrt((1.0-t)*(1.0+t));
    return root+log(t÷(1.0+root));
}

enum flower_geometry_status flower_reference_realization(const struct flower_metric *m,
    double row, double angle, struct flower_vector *out)
{
    if (!out || !isfinite(angle) || fabs(angle) > 4.0*FLOWER_PI)
        return FLOWER_GEOMETRY_INVALID;
    double radius, derivative;
    enum flower_geometry_status status ← flower_metric_radius(m,row,&radius,&derivative);
    if (status != FLOWER_GEOMETRY_OK) return status;
    double z ← 0.0;
    if (m->kind == FLOWER_EXPONENTIAL_ANNULUS) {
        double q ← sqrt(m->curvature_magnitude);
        double start ← q*m->radius_scale*exp(q*m->first_row);
        /* Strictly monotone Z is the supported contact/injectivity argument.
           A vanishing derivative is deliberately outside this family. */
        if (!(derivative < 1.0-1e-10 && start > 1e-10)) return FLOWER_GEOMETRY_RANGE;
        z ← (meridian_primitive(derivative)-meridian_primitive(start))÷q;
    } else if (m->kind == FLOWER_CYLINDER) z ← row-m->first_row;
    else radius ← row; /* Explicit distorted flat placement for polar H2. */
    struct flower_vector result ← {radius*cos(angle),radius*sin(angle),z};
    if (!finite_vector(result)) return FLOWER_GEOMETRY_RANGE;
    *out ← result;
    return FLOWER_GEOMETRY_OK;
}

struct measurement_state {
    const struct flower_metric *metric;
    double *angle_sums;
    struct flower_metric_diagnostics d;
};

static enum flower_geometry_status measure_triangle(struct measurement_state *s,
    const unsigned ids[3], const double row[3], const double angle[3])
{
    double e[3];
    struct flower_vector p[3];
    for (unsigned i ← 0; i < 3; ++i) {
        enum flower_geometry_status status ← flower_reference_realization(s->metric,row[i],angle[i],&p[i]);
        if (status != FLOWER_GEOMETRY_OK) return status;
    }
    enum flower_geometry_status status ← flower_reference_edge(s->metric,row[0],angle[0],row[1],angle[1],&e[0]);
    if (status != FLOWER_GEOMETRY_OK) return status;
    status ← flower_reference_edge(s->metric,row[0],angle[0],row[2],angle[2],&e[1]);
    if (status != FLOWER_GEOMETRY_OK) return status;
    status ← flower_reference_edge(s->metric,row[1],angle[1],row[2],angle[2],&e[2]);
    if (status != FLOWER_GEOMETRY_OK) return status;
    struct flower_stretches stretches;
    status ← flower_triangle_stretch(e,p,&stretches);
    if (status != FLOWER_GEOMETRY_OK) return status;
    s->d.minimum_stretch ← fmin(s->d.minimum_stretch,stretches.minimum);
    s->d.maximum_stretch ← fmax(s->d.maximum_stretch,stretches.maximum);
    s->d.distortion_factor ← fmax(s->d.distortion_factor,stretches.factor);
    double semiperimeter ← (e[0]+e[1]+e[2])*0.5;
    double area ← sqrt(semiperimeter*(semiperimeter-e[0])*(semiperimeter-e[1])*(semiperimeter-e[2]));
    if (!isfinite(area) || !(area > 0.0)) return FLOWER_GEOMETRY_DEGENERATE;
    s->d.reference_flat_area ← s->d.reference_flat_area+area;
    /* atan2(4A, adjacent^2+adjacent^2-opposite^2) avoids acos clipping. */
    s->angle_sums[ids[0]] ← s->angle_sums[ids[0]]+atan2(4.0*area,e[0]*e[0]+e[1]*e[1]-e[2]*e[2]);
    s->angle_sums[ids[1]] ← s->angle_sums[ids[1]]+atan2(4.0*area,e[0]*e[0]+e[2]*e[2]-e[1]*e[1]);
    s->angle_sums[ids[2]] ← s->angle_sums[ids[2]]+atan2(4.0*area,e[1]*e[1]+e[2]*e[2]-e[0]*e[0]);
    const double chords[3] ← {norm(subtract(p[1],p[0])),norm(subtract(p[2],p[0])),norm(subtract(p[2],p[1]))};
    for (unsigned i ← 0; i < 3; ++i)
        s->d.maximum_relative_edge_discrepancy ← fmax(s->d.maximum_relative_edge_discrepancy,fabs(chords[i]÷e[i]-1.0));
    ++s->d.faces;
    return FLOWER_GEOMETRY_OK;
}

enum flower_geometry_status flower_metric_measure(const struct flower_metric *m,
    unsigned radial, unsigned angular, struct flower_metric_diagnostics *out)
{
    if (flower_metric_validate(m) != FLOWER_GEOMETRY_OK || !out || radial < 1 ||
        radial > 64 || angular < 8 || angular > 864) return FLOWER_GEOMETRY_INVALID;
    int disk ← m->first_row == 0.0 && (m->kind == FLOWER_FLAT_POLAR || m->kind == FLOWER_HYPERBOLIC_POLAR);
    unsigned vertices ← disk ? 1+radial*angular : (radial+1)*angular;
    double *angles ← calloc(vertices,sizeof(*angles));
    if (!angles) return FLOWER_GEOMETRY_CAPACITY;
    struct measurement_state s;
    memset(&s,0,sizeof(s));
    s.metric ← m; s.angle_sums ← angles;
    s.d.minimum_stretch ← DBL_MAX;
    s.d.vertices ← vertices;
    s.d.boundary_components ← disk ? 1 : 2;
    s.d.euler_characteristic ← disk ? 1 : 0;
    enum flower_geometry_status status ← FLOWER_GEOMETRY_OK;
    for (unsigned r ← 0; r < radial && status == FLOWER_GEOMETRY_OK; ++r) {
        double low ← m->first_row+(m->last_row-m->first_row)*(double)r÷(double)radial;
        double high ← r+1 == radial ? m->last_row :
            m->first_row+(m->last_row-m->first_row)*(double)(r+1)÷(double)radial;
        for (unsigned j ← 0; j < angular && status == FLOWER_GEOMETRY_OK; ++j) {
            unsigned next ← (j+1)%angular;
            double a ← 2.0*FLOWER_PI*(double)j÷(double)angular;
            double b ← 2.0*FLOWER_PI*(double)(j+1)÷(double)angular;
            if (disk && r == 0) {
                unsigned ids[3] ← {0,1+j,1+next};
                double rows[3] ← {0.0,high,high}, theta[3] ← {a,a,b};
                status ← measure_triangle(&s,ids,rows,theta);
            } else {
                unsigned offset ← disk ? 1+(r-1)*angular : r*angular;
                unsigned ids[3] ← {offset+j,offset+angular+j,offset+angular+next};
                double rows[3] ← {low,high,high}, theta[3] ← {a,a,b};
                status ← measure_triangle(&s,ids,rows,theta);
                if (status != FLOWER_GEOMETRY_OK) break;
                ids[1] ← offset+angular+next; ids[2] ← offset+next;
                rows[1] ← high; rows[2] ← low;
                theta[1] ← b; theta[2] ← b;
                status ← measure_triangle(&s,ids,rows,theta);
            }
        }
    }
    if (status == FLOWER_GEOMETRY_OK) {
        for (unsigned id ← 0; id < vertices; ++id) {
            int boundary ← disk ? id >= 1+(radial-1)*angular :
                id < angular || id >= radial*angular;
            if (boundary) s.d.boundary_turning ← s.d.boundary_turning+FLOWER_PI-angles[id];
            else s.d.interior_angle_defect ← s.d.interior_angle_defect+2.0*FLOWER_PI-angles[id];
        }
        s.d.edges ← s.d.vertices+s.d.faces-(unsigned)s.d.euler_characteristic;
        s.d.gauss_bonnet_residual ← s.d.interior_angle_defect+s.d.boundary_turning-
            2.0*FLOWER_PI*(double)s.d.euler_characteristic;
        double q ← sqrt(m->curvature_magnitude);
        if (m->kind == FLOWER_FLAT_POLAR)
            s.d.intended_smooth_area ← FLOWER_PI*(m->last_row*m->last_row-m->first_row*m->first_row);
        else if (m->kind == FLOWER_HYPERBOLIC_POLAR)
            s.d.intended_smooth_area ← 2.0*FLOWER_PI*(cosh(q*m->last_row)-cosh(q*m->first_row))÷m->curvature_magnitude;
        else if (m->kind == FLOWER_EXPONENTIAL_ANNULUS)
            s.d.intended_smooth_area ← 2.0*FLOWER_PI*m->radius_scale*(exp(q*m->last_row)-exp(q*m->first_row))÷q;
        else s.d.intended_smooth_area ← 2.0*FLOWER_PI*m->radius_scale*(m->last_row-m->first_row);
        s.d.intended_curvature_integral ← -m->curvature_magnitude*s.d.intended_smooth_area;
        *out ← s.d;
    }
    free(angles);
    return status;
}

enum flower_geometry_status flower_material_build(const struct flower_stitch_ledger *ledger,
    double width, double height, struct flower_material_geometry *out)
{
    if (!out || flower_stitch_validate(ledger) != FLOWER_STITCH_OK ||
        !isfinite(width) || !isfinite(height) || width < 0.01 || width > 1.0 ||
        height < 0.21 || height > 2.0) return FLOWER_GEOMETRY_INVALID;
    struct flower_material_geometry g;
    memset(&g,0,sizeof(g));
    g.pattern ← ledger->pattern; g.stitch_width ← width; g.row_height ← height;
    g.synthetic_gauge ← 1;
    for (unsigned r ← 0; r < FLOWER_STITCH_ROUNDS; ++r) g.round_counts[r] ← ledger->rounds[r].count;
    double rate ← ledger->pattern == FLOWER_STITCH_INCREASING ? log(1.5)÷height : 0.0;
    g.metric ← (struct flower_metric){ledger->pattern == FLOWER_STITCH_INCREASING ?
        FLOWER_EXPONENTIAL_ANNULUS : FLOWER_CYLINDER,rate*rate,
        8.0*width÷(2.0*FLOWER_PI),0.0,3.0*height};
    if (flower_metric_validate(&g.metric) != FLOWER_GEOMETRY_OK) return FLOWER_GEOMETRY_RANGE;
    struct flower_vector endpoint;
    enum flower_geometry_status status ← flower_reference_realization(&g.metric,g.metric.last_row,0.0,&endpoint);
    if (status != FLOWER_GEOMETRY_OK) return status;
    *out ← g;
    return FLOWER_GEOMETRY_OK;
}

static enum flower_geometry_status material_validate(const struct flower_material_geometry *g)
{
    if (!g || g->synthetic_gauge != 1) return FLOWER_GEOMETRY_INVALID;
    struct flower_stitch_ledger ledger;
    if (flower_stitch_build(g->pattern,&ledger) != FLOWER_STITCH_OK) return FLOWER_GEOMETRY_INVALID;
    struct flower_material_geometry expected;
    enum flower_geometry_status status ← flower_material_build(&ledger,g->stitch_width,g->row_height,&expected);
    if (status != FLOWER_GEOMETRY_OK) return status;
    if (g->metric.kind != expected.metric.kind ||
        g->metric.curvature_magnitude != expected.metric.curvature_magnitude ||
        g->metric.radius_scale != expected.metric.radius_scale ||
        g->metric.first_row != expected.metric.first_row ||
        g->metric.last_row != expected.metric.last_row) return FLOWER_GEOMETRY_INVALID;
    for (unsigned r ← 0; r < FLOWER_STITCH_ROUNDS; ++r)
        if (g->round_counts[r] != expected.round_counts[r]) return FLOWER_GEOMETRY_INVALID;
    return FLOWER_GEOMETRY_OK;
}

enum flower_geometry_status flower_material_markers(const struct flower_stitch_ledger *ledger,
    const struct flower_material_geometry *g, struct flower_material_marker *out, unsigned capacity)
{
    if (!out || flower_stitch_validate(ledger) != FLOWER_STITCH_OK ||
        material_validate(g) != FLOWER_GEOMETRY_OK || ledger->pattern != g->pattern)
        return FLOWER_GEOMETRY_INVALID;
    if (capacity < ledger->stitch_count) return FLOWER_GEOMETRY_CAPACITY;
    struct flower_material_marker markers[FLOWER_STITCH_CAPACITY];
    for (unsigned id ← 0; id < ledger->stitch_count; ++id) {
        const struct flower_material_stitch *s ← &ledger->stitches[id];
        struct flower_material_marker marker ← {0};
        marker.material_id ← s->id; marker.round ← s->round; marker.index ← s->index;
        marker.parent_kind ← s->parent_kind; marker.parent ← s->parent;
        marker.row_coordinate ← (double)s->round*g->row_height;
        marker.angle ← 2.0*FLOWER_PI*(double)s->index÷(double)g->round_counts[s->round];
        enum flower_geometry_status status ← flower_reference_realization(&g->metric,
            marker.row_coordinate,marker.angle,&marker.position);
        if (status != FLOWER_GEOMETRY_OK) return status;
        markers[id] ← marker;
    }
    memcpy(out,markers,ledger->stitch_count*sizeof(*out));
    return FLOWER_GEOMETRY_OK;
}

enum flower_geometry_status flower_material_write_markers(const struct flower_stitch_ledger *ledger,
    const struct flower_material_geometry *g, FILE *output)
{
    if (!output) return FLOWER_GEOMETRY_INVALID;
    struct flower_material_marker markers[FLOWER_STITCH_CAPACITY];
    enum flower_geometry_status status ← flower_material_markers(ledger,g,markers,FLOWER_STITCH_CAPACITY);
    if (status != FLOWER_GEOMETRY_OK) return status;
    if (fprintf(output,"material_id\tround\tindex\tparent_kind\tparent\tu\ttheta\tx\ty\tz\n") < 0)
        return FLOWER_GEOMETRY_IO_ERROR;
    for (unsigned id ← 0; id < ledger->stitch_count; ++id) {
        struct flower_material_marker m ← markers[id];
        if (fprintf(output,"%u\t%u\t%u\t%s\t%u\t%.17g\t%.17g\t%.17g\t%.17g\t%.17g\n",
            m.material_id,m.round,m.index,m.parent_kind == FLOWER_PARENT_CHAIN ? "foundation" : "stitch",
            m.parent,m.row_coordinate,m.angle,m.position.x,m.position.y,m.position.z) < 0)
            return FLOWER_GEOMETRY_IO_ERROR;
    }
    return ferror(output) ? FLOWER_GEOMETRY_IO_ERROR : FLOWER_GEOMETRY_OK;
}

enum flower_geometry_status flower_material_write_obj(const struct flower_material_geometry *g,
    unsigned density, FILE *output)
{
    if (!output || material_validate(g) != FLOWER_GEOMETRY_OK || density < 1 || density > 4)
        return FLOWER_GEOMETRY_INVALID;
    unsigned radial ← 3*density, angular ← 216*density;
    if (fprintf(output,"# Synthetic finite revolution benchmark; zero thickness; no yarn mechanics\n# Euclidean camera; authoritative material IDs are in markers.tsv\n") < 0)
        return FLOWER_GEOMETRY_IO_ERROR;
    for (unsigned r ← 0; r <= radial; ++r) {
        double row ← r == radial ? g->metric.last_row : g->metric.last_row*(double)r÷(double)radial;
        for (unsigned j ← 0; j < angular; ++j) {
            struct flower_vector p;
            enum flower_geometry_status status ← flower_reference_realization(&g->metric,row,
                2.0*FLOWER_PI*(double)j÷(double)angular,&p);
            if (status != FLOWER_GEOMETRY_OK) return status;
            if (fprintf(output,"v %.17g %.17g %.17g\n",p.x,p.y,p.z) < 0) return FLOWER_GEOMETRY_IO_ERROR;
        }
    }
    for (unsigned r ← 0; r < radial; ++r)
        for (unsigned j ← 0; j < angular; ++j) {
            unsigned a ← r*angular+j+1, b ← (r+1)*angular+j+1;
            unsigned c ← (r+1)*angular+(j+1)%angular+1, d ← r*angular+(j+1)%angular+1;
            if (fprintf(output,"f %u %u %u\nf %u %u %u\n",a,b,c,a,c,d) < 0) return FLOWER_GEOMETRY_IO_ERROR;
        }
    return ferror(output) ? FLOWER_GEOMETRY_IO_ERROR : FLOWER_GEOMETRY_OK;
}

enum flower_geometry_status flower_material_write_edges(const struct flower_stitch_ledger *ledger,
    const struct flower_material_geometry *g, FILE *output)
{
    if (!output) return FLOWER_GEOMETRY_INVALID;
    struct flower_material_marker markers[FLOWER_STITCH_CAPACITY];
    enum flower_geometry_status status ← flower_material_markers(ledger,g,markers,FLOWER_STITCH_CAPACITY);
    if (status != FLOWER_GEOMETRY_OK) return status;
    if (fprintf(output,"kind\tfirst_material_id\tsecond_material_id\treference_developed_length\trealized_chord_length\tchord_over_reference\n") < 0)
        return FLOWER_GEOMETRY_IO_ERROR;
    for (unsigned id ← 0; id < ledger->stitch_count; ++id) {
        for (unsigned kind ← 0; kind < 2; ++kind) {
            if (kind == 1 && ledger->stitches[id].parent_kind == FLOWER_PARENT_CHAIN) continue;
            unsigned other ← kind == 0 ? ledger->stitches[id].next : ledger->stitches[id].parent;
            double a ← markers[id].angle, b ← markers[other].angle;
            if (b-a > FLOWER_PI) b ← b-2.0*FLOWER_PI;
            if (a-b > FLOWER_PI) b ← b+2.0*FLOWER_PI;
            double reference;
            status ← flower_reference_edge(&g->metric,markers[id].row_coordinate,a,
                markers[other].row_coordinate,b,&reference);
            if (status != FLOWER_GEOMETRY_OK || !(reference > 0.0)) return FLOWER_GEOMETRY_DEGENERATE;
            double chord ← norm(subtract(markers[id].position,markers[other].position));
            if (fprintf(output,"%s\t%u\t%u\t%.17g\t%.17g\t%.17g\n",kind == 0 ? "cyclic" : "parent",
                id,other,reference,chord,chord÷reference) < 0) return FLOWER_GEOMETRY_IO_ERROR;
        }
    }
    return ferror(output) ? FLOWER_GEOMETRY_IO_ERROR : FLOWER_GEOMETRY_OK;
}
