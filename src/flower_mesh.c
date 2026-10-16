#include "flower.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static FlowerPoint add(FlowerPoint a,FlowerPoint b) { return (FlowerPoint){a.x+b.x,a.y+b.y,a.z+b.z}; }
static FlowerPoint sub(FlowerPoint a,FlowerPoint b) { return (FlowerPoint){a.x-b.x,a.y-b.y,a.z-b.z}; }
static FlowerPoint scale(FlowerPoint a,float s) { return (FlowerPoint){a.x*s,a.y*s,a.z*s}; }
static FlowerPoint cross(FlowerPoint a,FlowerPoint b) { return (FlowerPoint){a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
static FlowerPoint unit(FlowerPoint a) {
    float length=sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);
    return length>1e-8f?scale(a,1/length):(FlowerPoint){0,1,0};
}
static void surface_triangle(const Flower *flower,int cell,int half,FlowerSurfaceTriangle *out) {
    const int order[2][3]={{0,1,3},{1,2,3}};
    int ring=cell/flower->net.slices, slice=cell%flower->net.slices;
    const int dr[4]={0,0,1,1}, ds[4]={0,1,1,0};
    for (int i=0;i<3;i++) {
        int corner=order[half][i], node=flower->net.cell[cell][corner];
        out->node[i]=(uint16_t)node; out->position[i]=flower->skin.position[node];
        out->radial[i]=(float)(ring+dr[corner])/flower->net.rings;
        /* Unwrapped cell chart makes the angular seam ordinary local material. */
        out->angular[i]=(float)(slice+ds[corner]);
    }
}
void flower_normals(const Flower *flower,FlowerPoint *normals) {
    memset(normals,0,sizeof(*normals)*FLOWER_VERTICES);
    for (int cell=0;cell<flower->net.cell_count;cell++) for (int half=0;half<2;half++) {
        FlowerSurfaceTriangle t; surface_triangle(flower,cell,half,&t);
        FlowerPoint normal=cross(sub(t.position[1],t.position[0]),sub(t.position[2],t.position[0]));
        for (int i=0;i<3;i++) normals[t.node[i]]=add(normals[t.node[i]],normal);
    }
    for (int i=0;i<flower->net.node_count;i++) normals[i]=unit(normals[i]);
}
void flower_mesh_free(FlowerMesh *mesh) {
    free(mesh->position); free(mesh->normal); free(mesh->parent); free(mesh->surface);
    memset(mesh,0,sizeof(*mesh));
}
static FlowerPoint blend(const FlowerPoint *p,float u,float v) {
    return add(scale(p[0],1-u-v),add(scale(p[1],u),scale(p[2],v)));
}
bool flower_mesh_build(const Flower *flower,int density,FlowerMesh *mesh) {
    if (density!=1 && density!=2) return false;
    FlowerMesh out={.density=density,.surface_count=flower->net.cell_count*2,
                    .rings=flower->net.rings,.slices=flower->net.slices};
    out.triangle_count=out.surface_count*density*density; out.vertex_count=out.triangle_count*3;
    out.position=malloc((size_t)out.vertex_count*sizeof(*out.position));
    out.normal=malloc((size_t)out.vertex_count*sizeof(*out.normal));
    out.parent=malloc((size_t)out.triangle_count*sizeof(*out.parent));
    out.surface=calloc((size_t)out.surface_count,sizeof(*out.surface));
    if (!out.position || !out.normal || !out.parent || !out.surface) { flower_mesh_free(&out); return false; }
    FlowerPoint normals[FLOWER_VERTICES]; flower_normals(flower,normals);
    int emitted=0;
    const float low[1][3][2]={{{0,0},{1,0},{0,1}}};
    const float high[4][3][2]={{{0,0},{0.5f,0},{0,0.5f}},{{0.5f,0},{1,0},{0.5f,0.5f}},
                              {{0,0.5f},{0.5f,0.5f},{0,1}},{{0.5f,0},{0.5f,0.5f},{0,0.5f}}};
    for (int face=0;face<out.surface_count;face++) {
        FlowerSurfaceTriangle *t=&out.surface[face]; surface_triangle(flower,face/2,face%2,t);
        for (int v=0;v<3;v++) t->normal[v]=normals[t->node[v]];
        for (int part=0;part<density*density;part++) {
            out.parent[emitted]=(uint16_t)face;
            for (int v=0;v<3;v++) {
                const float *weight=density==1?low[part][v]:high[part][v];
                int id=emitted*3+v;
                out.position[id]=blend(t->position,weight[0],weight[1]);
                out.normal[id]=unit(blend(t->normal,weight[0],weight[1]));
            }
            emitted++;
        }
    }
    /* Caller explicitly frees/replaces its old cache; authoritative Flower is const. */
    *mesh=out; return true;
}
/* Double arithmetic avoids density-dependent edge rounding; a small barycentric
   tolerance includes shared edges. Parent face order resolves coincident hits. */
static bool intersect(const FlowerPoint *p,FlowerPoint origin,FlowerPoint direction,double *distance,double *u,double *v) {
    double a[3]={p[1].x-p[0].x,p[1].y-p[0].y,p[1].z-p[0].z};
    double b[3]={p[2].x-p[0].x,p[2].y-p[0].y,p[2].z-p[0].z};
    double d[3]={direction.x,direction.y,direction.z}, f[3]={origin.x-p[0].x,origin.y-p[0].y,origin.z-p[0].z};
    double h[3]={d[1]*b[2]-d[2]*b[1],d[2]*b[0]-d[0]*b[2],d[0]*b[1]-d[1]*b[0]};
    double det=a[0]*h[0]+a[1]*h[1]+a[2]*h[2];
    if (fabs(det)<1e-12) return false;
    *u=(f[0]*h[0]+f[1]*h[1]+f[2]*h[2])/det;
    double q[3]={f[1]*a[2]-f[2]*a[1],f[2]*a[0]-f[0]*a[2],f[0]*a[1]-f[1]*a[0]};
    *v=(d[0]*q[0]+d[1]*q[1]+d[2]*q[2])/det;
    *distance=(b[0]*q[0]+b[1]*q[1]+b[2]*q[2])/det;
    return *u>=-2e-6 && *v>=-2e-6 && *u+*v<=1+2e-6 && *distance>0 && isfinite(*distance);
}
static FlowerHit material_hit(const FlowerSurfaceTriangle *face,int rings,int slices,FlowerPoint origin,FlowerPoint direction) {
    double distance,u,v;
    if (!intersect(face->position,origin,direction,&distance,&u,&v)) return (FlowerHit){0};
    double radial=(1-u-v)*face->radial[0]+u*face->radial[1]+v*face->radial[2];
    double angular=(1-u-v)*face->angular[0]+u*face->angular[1]+v*face->angular[2];
    radial=fmax(0,fmin(1,radial)); angular=fmax(0,fmin(slices,angular));
    double material_ring=radial*rings, best=DBL_MAX; int node=0;
    int first_ring=(int)floor(material_ring), first_slice=(int)floor(angular);
    /* Nearest marked node in this local chart; equal distances (1e-6) use
       the lower stable ID, including the periodic seam. Not a growth brush. */
    for (int r=first_ring;r<=first_ring+1 && r<=rings;r++) for (int s=first_slice;s<=first_slice+1;s++) {
        double delta_ring=r-material_ring, delta_slice=s-angular;
        double squared=delta_ring*delta_ring+delta_slice*delta_slice;
        int candidate=r*slices+s%slices;
        if (squared<best-1e-6 || (fabs(squared-best)<=1e-6 && candidate<node)) { best=squared; node=candidate; }
    }
    return (FlowerHit){.hit=true,.radial=(float)radial,.angle=(float)(angular*6.283185307179586/slices),
        .distance=(float)distance,.point=add(origin,scale(direction,(float)distance)),.node=(uint16_t)node};
}
FlowerHit flower_mesh_pick(const FlowerMesh *mesh,FlowerPoint origin,FlowerPoint direction) {
    double best=DBL_MAX; int face=-1;
    for (int i=0;i<mesh->triangle_count;i++) {
        double distance,u,v;
        if (!intersect(mesh->position+i*3,origin,direction,&distance,&u,&v)) continue;
        int parent=mesh->parent[i];
        if (distance<best-1e-6 || (fabs(distance-best)<=1e-6 && (face<0 || parent<face))) { best=distance; face=parent; }
    }
    if (face<0) return (FlowerHit){0};
    return material_hit(&mesh->surface[face],mesh->rings,mesh->slices,origin,direction);
}
FlowerHit flower_pick(const Flower *flower,FlowerPoint origin,FlowerPoint direction) {
    FlowerHit best={.distance=FLT_MAX};
    for (int face=0;face<flower->net.cell_count*2;face++) {
        FlowerSurfaceTriangle t; surface_triangle(flower,face/2,face%2,&t);
        FlowerHit hit=material_hit(&t,flower->net.rings,flower->net.slices,origin,direction);
        if (hit.hit && hit.distance<best.distance-1e-6f) best=hit;
    }
    return best;
}
