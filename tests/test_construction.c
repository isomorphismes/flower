#include "flower_construction.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "construction FAIL line %d: %s\n", __LINE__, #condition); \
    exit(1); } } while (0)

static Flower flower, before;
static FlowerMaterialRound round_a, round_b, untouched;
static FlowerMaterialRadial radial;
static double separation(FlowerDiagramPoint p, FlowerDiagramPoint q) {
    return hypot(p.x - q.x, p.y - q.y);
}
int main(void) {
    flower_init(&flower);
    before = flower;
    CHECK(flower_material_round(&flower.net, 8, 96, 96, 62, &round_a));
    CHECK(round_a.ring == 8 && round_a.count == FLOWER_SLICES);
    for (int s = 0; s < FLOWER_SLICES; ++s) {
        FlowerMaterialCubic *curve = &round_a.segment[s];
        FlowerMaterialCubic *next = &round_a.segment[(s + 1) % FLOWER_SLICES];
        CHECK(curve->first_node == 8 * FLOWER_SLICES + s);
        CHECK(curve->last_node == 8 * FLOWER_SLICES + (s + 1) % FLOWER_SLICES);
        CHECK(curve->last_node == next->first_node);
        CHECK(separation(curve->control[3], next->control[0]) < 1e-9);
        CHECK(fabs(hypot(curve->control[0].x - 96,
                         curve->control[0].y - 96) - 62) < 1e-9);
        /* Centerpoint of a cubic approximation to this small circular arc. */
        FlowerDiagramPoint middle = {0, 0};
        for (int k = 0; k < 4; ++k) {
            double coefficient = k == 0 || k == 3 ? 0.125 : 0.375;
            middle.x += coefficient * curve->control[k].x;
            middle.y += coefficient * curve->control[k].y;
        }
        CHECK(fabs(hypot(middle.x - 96, middle.y - 96) - 62) < 1e-4);
    }
    CHECK(round_a.segment[FLOWER_SLICES-1].last_node == 8*FLOWER_SLICES);
    CHECK(flower_material_radial(&flower.net, 7, 96, 96, 20, 3, &radial));
    CHECK(radial.slice == 7 && radial.count == FLOWER_RINGS);
    for (int ring = 0; ring < FLOWER_RINGS; ++ring) {
        CHECK(radial.segment[ring].first_node == ring*FLOWER_SLICES + 7);
        CHECK(radial.segment[ring].last_node == (ring+1)*FLOWER_SLICES + 7);
        if (ring)
            CHECK(separation(radial.segment[ring-1].control[3],
                             radial.segment[ring].control[0]) < 1e-9);
    }
    CHECK(memcmp(&flower, &before, sizeof flower) == 0);

    untouched = round_a;
    CHECK(!flower_material_round(&flower.net, -1, 96, 96, 62, &round_a));
    CHECK(!flower_material_round(&flower.net, 8, 96, 96, NAN, &round_a));
    CHECK(!flower_material_round(&flower.net, 8, 96, 96, 97, &round_a));
    CHECK(!flower_material_round(&flower.net, 8, INFINITY, 96, 62, &round_a));
    CHECK(!flower_material_radial(&flower.net, -1, 96, 96, 20, 3, &radial));
    CHECK(!flower_material_radial(&flower.net, 7, 96, 96, 20, 8, &radial));
    CHECK(memcmp(&round_a, &untouched, sizeof round_a) == 0);
    FlowerNet corrupt = flower.net;
    corrupt.neighbor[8*FLOWER_SLICES][1] = UINT16_MAX;
    CHECK(!flower_material_round(&corrupt, 8, 96, 96, 62, &round_a));
    CHECK(memcmp(&round_a, &untouched, sizeof round_a) == 0);
    CHECK(flower_material_round(&flower.net, 8, 96, 96, 62, &round_a));

    /* A 3D fold can bring unrelated nodes into contact but never adds a
     * construction edge. Neither Skin nor draw density enters the chart. */
    for (int i = 0; i < FLOWER_VERTICES; ++i) {
        flower.skin.position[i] = (FlowerPoint){(float)(i % 3), 0, 0};
        flower.skin.growth[i] = (float)i / FLOWER_VERTICES;
    }
    FlowerMesh low = {0}, high = {0};
    CHECK(flower_mesh_build(&flower, 1, &low));
    CHECK(flower_mesh_build(&flower, 2, &high));
    CHECK(flower_material_round(&flower.net, 8, 96, 96, 62, &round_b));
    CHECK(memcmp(&round_a, &round_b, sizeof round_a) == 0);
    CHECK(memcmp(&flower.net, &before.net, sizeof flower.net) == 0);
    CHECK(low.density == 1 && high.density == 2);
    flower_mesh_free(&low); flower_mesh_free(&high);
    puts("PASS stable material nodes, circular round + radial mark, seam, invalid/refusal, folded-Skin and draw-density invariance");
    return 0;
}
