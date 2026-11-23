#include "flower_inspection.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, \
    "inspection FAIL line %d: %s\n", __LINE__, #c); exit(1); } } while(0)
static Flower flower, before;

static const double tau = 6.2831853071795864769;

static double edge_length(FlowerPoint a, FlowerPoint b) {
    return hypot(hypot((double)a.x-b.x,(double)a.y-b.y),(double)a.z-b.z);
}

static double stored_target(int first, int second) {
    for (int i=0;i<flower.skin.link_count;++i) {
        FlowerLink edge=flower.skin.link[i];
        if (!((first==edge.first && second==edge.second) ||
              (first==edge.second && second==edge.first))) continue;
        double tangent=1+(flower.skin.growth[first]+flower.skin.growth[second])/2.;
        double radial=1+(flower.skin.radial_growth[first]+flower.skin.radial_growth[second])/2.;
        return edge.length*hypot(tangent*sqrt(edge.tangent_squared),
                                  radial*sqrt(1.-edge.tangent_squared));
    }
    CHECK(0); /* No material edge with these endpoints. */
    return 0;
}

static void check_metric(void) {
    FlowerNodeMetric metric, untouched={.spoke={123,456}};
    CHECK(!flower_inspection_metric(&flower,-1,&untouched));
    CHECK(untouched.spoke.preferred==123);
    double preferred_total=0, realized_total=0, largest_difference=0;
    for (int node=0; node<flower.net.node_count; ++node) {
        int ring=node/FLOWER_SLICES, sector=node%FLOWER_SLICES;
        double phase=tau*sector/FLOWER_SLICES, radius=1+0.25*ring;
        /* Independent realized fixture: nested regular polygons in a plane. */
        flower.skin.position[node]=(FlowerPoint){(float)(radius*cos(phase)),0,
                                                (float)(radius*sin(phase))};
        flower.skin.growth[node]=(float)(0.15*(1+cos(phase))*ring/FLOWER_RINGS);
        flower.skin.radial_growth[node]=0.05f;
    }
    for (int node=0; node<flower.net.node_count; ++node) {
        CHECK(flower_inspection_metric(&flower,node,&metric));
        int ring=node/FLOWER_SLICES;
        double expected_turn=ring==0 ? -tau/FLOWER_SLICES :
                             ring==FLOWER_RINGS ? tau/FLOWER_SLICES : 0;
        CHECK(metric.boundary==(ring==0 || ring==FLOWER_RINGS));
        CHECK(fabs(metric.angle_defect.realized-expected_turn)<1e-5);
        CHECK(fabs(metric.spoke.realized-0.25)<1e-6);
        CHECK(fabs(metric.round.realized-2*(1+0.25*ring)*sin(tau/(2*FLOWER_SLICES)))<1e-6);
        largest_difference=fmax(largest_difference,fabs(metric.angle_defect.preferred-expected_turn));
        preferred_total+=metric.angle_defect.preferred;
        realized_total+=metric.angle_defect.realized;
    }
    CHECK(largest_difference>0.01);
    CHECK(fabs(preferred_total)<1e-8 && fabs(realized_total)<1e-8);
    /* Cross-check reference reconstruction against the actual solver links.
     * This includes both diagonals and links that span two sectors or rings. */
    for (int i=0; i<flower.skin.link_count; ++i) {
        FlowerLink link=flower.skin.link[i];
        double tangent=1+(flower.skin.growth[link.first]+flower.skin.growth[link.second])/2.;
        double radial=1+(flower.skin.radial_growth[link.first]+flower.skin.radial_growth[link.second])/2.;
        double expected=link.length*hypot(tangent*sqrt(link.tangent_squared),
                                          radial*sqrt(1.-link.tangent_squared));
        CHECK(fabs(expected-flower_preferred_edge_length(&flower.skin,link.first,link.second))<1e-6);
    }
    const int node=8*FLOWER_SLICES+FLOWER_SLICES-1, next=8*FLOWER_SLICES;
    CHECK(flower_inspection_metric(&flower,node,&metric));
    CHECK(metric.round.preferred!=metric.round.realized);
    CHECK(fabs(metric.round.realized-edge_length(flower.skin.position[node],flower.skin.position[next]))<1e-7);
    /* Independently walk the six faces at the seam and use the cosine law
     * on cached spring targets; no chart, face accessor or live length call. */
    const int neighbors[]={next,node+FLOWER_SLICES,node+FLOWER_SLICES-1,
                           node-1,node-FLOWER_SLICES,next-FLOWER_SLICES};
    double expected_defect=tau;
    for (int i=0;i<6;++i) {
        int first=neighbors[i], second=neighbors[(i+1)%6];
        double a=stored_target(node,first), b=stored_target(node,second);
        double c=stored_target(first,second);
        expected_defect-=acos((a*a+b*b-c*c)/(2*a*b));
    }
    CHECK(fabs(expected_defect-metric.angle_defect.preferred)<1e-6);
    CHECK(fabs(stored_target(node,next)-metric.round.preferred)<1e-6);
    CHECK(fabs(stored_target(node,node+FLOWER_SLICES)-metric.spoke.preferred)<1e-6);
    /* A collapsed realized face must not erase a still-defined target. */
    flower.skin.position[node]=flower.skin.position[next];
    CHECK(flower_inspection_metric(&flower,node,&metric));
    CHECK(metric.round.realized==0 && isnan(metric.angle_defect.realized));
    CHECK(isfinite(metric.angle_defect.preferred));
    flower_init(&flower);
    FlowerHold hold;
    flower_hold_init(&hold);
    FlowerHit hit={.hit=true,.radial=0.5f,.angle=0,.node=node};
    CHECK(flower_hold_begin(&hold,0,true,true,hit));
    CHECK(flower_hold_frame(&flower,&hold,0,true,true,hit,FLOWER_STEP));
    CHECK(flower_inspection_metric(&flower,node,&metric));
    CHECK(isfinite(metric.angle_defect.realized) && isfinite(metric.angle_defect.preferred));
    CHECK(fabs(metric.round.preferred-metric.round.realized)>1e-6);
    CHECK(fabs(metric.angle_defect.preferred-metric.angle_defect.realized)>1e-6);
    before=flower;
    flower_hold_release(&hold,0);
    CHECK(!flower_hold_frame(&flower,&hold,0,false,true,hit,FLOWER_STEP));
    CHECK(memcmp(&before,&flower,sizeof flower)==0);
}

int main(void) {
    flower_init(&flower);
    FlowerInspectionSample a = {0}, untouched = {.node=UINT16_MAX};
    CHECK(flower_inspection_sample(&flower, 96+54, 96, &a));
    CHECK(a.node == 8*FLOWER_SLICES && a.slice == 0);
    CHECK(a.growth == 0 && a.reference_stretch == 0);
    CHECK(!flower_inspection_sample(&flower, 96, 96, &untouched));
    CHECK(untouched.node == UINT16_MAX);
    CHECK(!flower_inspection_sample(&flower, NAN, 96, &untouched));
    CHECK(!flower_inspection_sample(NULL, 130, 96, &untouched));
    CHECK(flower_inspection_sample(&flower, 96+53, 95.5, &a));
    CHECK(a.slice == 0);  /* periodic seam roundoff, not a new node */
    uint32_t initial=flower_inspection_color(&flower,150,96,FLOWER_INSPECTION_GROWTH,0);
    CHECK(initial != flower_inspection_color(&flower,96,96,FLOWER_INSPECTION_GROWTH,0));
    CHECK(flower_inspection_max_growth(&flower) == 0);
    flower.skin.growth[8*FLOWER_SLICES] = 0.5f;
    CHECK(fabsf(flower_inspection_max_growth(&flower)-0.5f) < 1e-6f);
    CHECK(flower_inspection_sample(&flower, 150, 96, &a));
    CHECK(fabsf(a.growth-0.5f) < 1e-6f);
    CHECK(initial != flower_inspection_color(&flower,150,96,FLOWER_INSPECTION_GROWTH,0.5f));
    int node = a.node;
    int adjacent = flower.net.neighbor[node][0];
    FlowerPoint p=flower.skin.position[adjacent];
    flower.skin.position[adjacent]=(FlowerPoint){p.x*1.3f,p.y*1.3f,p.z*1.3f};
    CHECK(flower_inspection_sample(&flower,150,96,&a));
    CHECK(fabsf(a.reference_stretch) > 1e-5f);
    before=flower;
    uint32_t strain=flower_inspection_color(&flower,150,96,FLOWER_INSPECTION_STRETCH,0.5f);
    CHECK(strain != initial);
    FlowerMesh low={0}, high={0};
    CHECK(flower_mesh_build(&flower,1,&low));
    CHECK(flower_mesh_build(&flower,2,&high));
    CHECK(flower_inspection_sample(&flower,150,96,&a));
    CHECK(strain == flower_inspection_color(&flower,150,96,FLOWER_INSPECTION_STRETCH,0.5f));
    flower_mesh_free(&low); flower_mesh_free(&high);
    CHECK(memcmp(&before,&flower,sizeof(flower)) == 0);
    /* Corrupt topology must refuse, not read outside bounds. */
    flower.net.neighbor[node][0] = UINT16_MAX;
    CHECK(!flower_inspection_sample(&flower,150,96,&a));
    flower_init(&flower);
    check_metric();
    puts("PASS 1088 retained-Net nodes, both rims, preferred and realized angles and lengths, Gauss-Bonnet, collapsed face, hold/release");
    puts("PASS material grid, growth, stretched links, invalid input, source and density invariance");
    return 0;
}
