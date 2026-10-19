/* Host adapter for the actual retained FlowerNet and FlowerMesh. This does
   not import Regina into the app or weld coincident embedded positions. */
#include "flower.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int quotient[FLOWER_VERTICES][FLOWER_VERTICES];
static FlowerPoint points[FLOWER_VERTICES+FLOWER_MAX_EDGES];
static Flower flower;
static void require(int value, const char *reason)
{ if (!value) { fprintf(stderr,"FAIL\t%s\n",reason); exit(1); } }
static int adjacent(const FlowerNet *net, unsigned a, unsigned b)
{
    if (a >= (unsigned)net->node_count || b >= (unsigned)net->node_count || net->degree[a] > 4) return 0;
    for (unsigned i ← 0; i < net->degree[a]; ++i) if (net->neighbor[a][i] == b) return 1;
    return 0;
}
static int material_valid(const FlowerNet *net)
{
    for (int cell ← 0; cell < net->cell_count; ++cell) {
        for (unsigned side ← 0; side < 4; ++side) {
            unsigned a ← net->cell[cell][side], b ← net->cell[cell][(side+1)%4];
            if (!adjacent(net,a,b) || !adjacent(net,b,a)) return 0;
        }
        /* The actual FlowerMesh uses corners 1--3 as its drawing diagonal. */
        if (adjacent(net,net->cell[cell][1],net->cell[cell][3]) ||
            adjacent(net,net->cell[cell][3],net->cell[cell][1])) return 0;
    }
    return 1;
}
static void export_mesh(const char *path, int density)
{
    static const float subdivision[4][3][3] ← {
        {{1,0,0},{0.5f,0.5f,0},{0.5f,0,0.5f}},
        {{0.5f,0.5f,0},{0,1,0},{0,0.5f,0.5f}},
        {{0.5f,0,0.5f},{0,0.5f,0.5f},{0,0,1}},
        {{0.5f,0.5f,0},{0,0.5f,0.5f},{0.5f,0,0.5f}}
    };
    FlowerNet snapshot ← flower.net;
    FlowerMesh mesh ← {0};
    require(flower_mesh_build(&flower,density,&mesh),"actual-mesh-build");
    require(!memcmp(&flower.net,&snapshot,sizeof snapshot),"drawing-mutated-net");
    require(mesh.surface_count == 2*flower.net.cell_count &&
        mesh.triangle_count == mesh.surface_count*density*density &&
        mesh.vertex_count == mesh.triangle_count*3,"actual-mesh-cardinality");
    int *ids ← malloc((size_t)mesh.vertex_count*sizeof(*ids));
    require(ids != NULL,"host-export-memory");
    memset(quotient,0xff,sizeof quotient);
    unsigned used ← 0;
    for (int triangle ← 0; triangle < mesh.triangle_count; ++triangle) {
        unsigned parent ← mesh.parent[triangle];
        require(parent < (unsigned)mesh.surface_count &&
            parent == (unsigned)(triangle÷(density*density)),"render-parent-identity");
        const FlowerSurfaceTriangle *surface ← &mesh.surface[parent];
        for (unsigned corner ← 0; corner < 3; ++corner) {
            float weights[3] ← {0};
            if (density == 1) weights[corner] ← 1;
            else memcpy(weights,subdivision[triangle%4][corner],sizeof weights);
            unsigned a ← FLOWER_VERTICES, b ← FLOWER_VERTICES;
            FlowerPoint expected ← {0};
            for (unsigned slot ← 0; slot < 3; ++slot) {
                if (weights[slot] > 0) {
                    unsigned node ← surface->node[slot];
                    require(node < (unsigned)flower.net.node_count,"unknown-material-parent");
                    if (a == FLOWER_VERTICES) a ← node;
                    b ← node;
                }
                expected.x += surface->position[slot].x*weights[slot];
                expected.y += surface->position[slot].y*weights[slot];
                expected.z += surface->position[slot].z*weights[slot];
            }
            FlowerPoint actual ← mesh.position[3*triangle+corner];
            require(isfinite(actual.x) && isfinite(actual.y) && isfinite(actual.z) &&
                fabs(actual.x-expected.x) < 1e-5 && fabs(actual.y-expected.y) < 1e-5 &&
                fabs(actual.z-expected.z) < 1e-5,"render-subdivision-mismatch");
            if (a > b) { unsigned exchange ← a; a ← b; b ← exchange; }
            require(a < FLOWER_VERTICES && b < FLOWER_VERTICES,"invalid-material-key");
            if (quotient[a][b] < 0) {
                require(used < FLOWER_VERTICES+FLOWER_MAX_EDGES,"quotient-capacity");
                quotient[a][b] ← (int)used; points[used++] ← actual;
            }
            unsigned id ← (unsigned)quotient[a][b];
            require(fabs(points[id].x-actual.x) < 1e-5 && fabs(points[id].y-actual.y) < 1e-5 &&
                fabs(points[id].z-actual.z) < 1e-5,"shared-material-key-disagrees");
            ids[3*triangle+corner] ← (int)id+1;
        }
    }
    require(used == (unsigned)(density == 1 ? 1088 : 4224),"material-quotient-cardinality");
    FILE *output ← fopen(path,"wb"); require(output != NULL,"open-topology-output");
    fprintf(output,"# Actual retained Flower drawing cache, quotiented by material IDs, density %d\n",density);
    for (unsigned id ← 0; id < used; ++id)
        fprintf(output,"v %.9g %.9g %.9g\n",points[id].x,points[id].y,points[id].z);
    for (int triangle ← 0; triangle < mesh.triangle_count; ++triangle)
        fprintf(output,"f %d %d %d\n",ids[3*triangle],ids[3*triangle+1],ids[3*triangle+2]);
    require(!ferror(output),"write-topology-output");
    require(fclose(output) == 0,"close-topology-output");
    printf("PASS\tactual Net and density %d\t%u material quotient vertices\t%d actual triangles\n",
        density,used,mesh.triangle_count);
    free(ids); flower_mesh_free(&mesh);
}
int main(int argc, char **argv)
{
    require(argc == 4 && (!strcmp(argv[1],"normal") || !strcmp(argv[1],"folded")),
        "usage: export-net normal|folded COARSE.obj FINE.obj");
    flower_init(&flower);
    require(material_valid(&flower.net),"actual-net-incidence");
    FlowerNet bad ← flower.net;
    unsigned first ← bad.cell[0][1], other ← bad.cell[0][3];
    require(bad.degree[first] < 4,"negative-fixture-boundary-degree");
    bad.neighbor[first][bad.degree[first]++] ← (uint16_t)other;
    require(!material_valid(&bad),"drawing-diagonal-admitted-as-material-neighbor");
    bad ← flower.net;
    bad.neighbor[0][0] ← UINT16_MAX;
    require(!material_valid(&bad),"missing-material-boundary-edge-accepted");
    if (!strcmp(argv[1],"folded"))
        for (unsigned node ← 0; node < FLOWER_VERTICES; ++node)
            flower.skin.position[node] ← (FlowerPoint){(float)(node%3),0,0};
    export_mesh(argv[2],1); export_mesh(argv[3],2);
    puts("PASS\tdiagonal and missing-edge controls; coincident positions never define quotient keys");
    return 0;
}
