#ifndef FLOWER_H
#define FLOWER_H
#include <stdbool.h>
#include <stdint.h>

#define FLOWER_RINGS 16
#define FLOWER_SLICES 64
#define FLOWER_VERTICES ((FLOWER_RINGS + 1)*FLOWER_SLICES)
#define FLOWER_TRIANGLES (2*FLOWER_RINGS*FLOWER_SLICES)
#define FLOWER_MAX_EDGES ((6*FLOWER_RINGS + 2)*FLOWER_SLICES)
#define FLOWER_STEP (1.0/120.0)
#define FLOWER_FRAME_LIMIT 0.25
#define FLOWER_CATCHUP_LIMIT 0.05

typedef struct { float x, y, z; } FlowerPoint;
typedef struct { uint16_t first, second; float length, tangent_squared, stiffness; } FlowerLink;
/* Stable material IDs and quadrilateral incidence; no embedding or renderer. */
typedef struct {
    int rings, slices, node_count, cell_count;
    uint16_t cell[FLOWER_RINGS*FLOWER_SLICES][4];
    uint16_t neighbor[FLOWER_VERTICES][4];
    uint8_t degree[FLOWER_VERTICES], boundary[FLOWER_VERTICES];
    uint16_t ring[FLOWER_VERTICES], slice[FLOWER_VERTICES];
} FlowerNet;
/* Intrinsic fields and deformation state on the fixed material budget. */
typedef struct {
    FlowerPoint position[FLOWER_VERTICES], reference[FLOWER_VERTICES];
    float growth[FLOWER_VERTICES], radial_growth[FLOWER_VERTICES];
    FlowerLink link[FLOWER_MAX_EDGES];
    int link_count;
    uint64_t steps;
} FlowerSkin;
typedef struct { FlowerNet net; FlowerSkin skin; } Flower;
typedef struct { float rate, falloff, edge_bias, radial_fraction; int hop_radius; } FlowerPolicy;
typedef struct { bool hit; float radial, angle, distance; FlowerPoint point; uint16_t node; } FlowerHit;
typedef struct {
    bool captured, blocked;
    int owner;
    double remainder;
} FlowerHold;

void flower_init(Flower *flower);
bool flower_net_init(FlowerNet *net, int rings, int slices);
bool flower_net_hops(const FlowerNet *net, int seed, int radius, int16_t *distance);
FlowerPolicy flower_policy_default(void);
bool flower_policy_valid(FlowerPolicy policy);
void flower_hold_init(FlowerHold *hold);
void flower_hold_cancel(FlowerHold *hold);
/* A fresh press is mandatory. A held pointer cannot inherit another's stroke. */
bool flower_hold_begin(FlowerHold *hold, int pointer, bool fresh_press, bool focused, FlowerHit hit);
void flower_hold_release(FlowerHold *hold, int pointer);
void flower_hold_all_released(FlowerHold *hold);
/* Release, missed surface, focus loss and invalid/stalled time never step the flower. */
bool flower_hold_frame(Flower *flower, FlowerHold *hold, int pointer, bool down,
                       bool focused, FlowerHit hit, double frame_seconds);
bool flower_hold_frame_policy(Flower *flower, FlowerHold *hold, int pointer, bool down,
                              bool focused, FlowerHit hit, double seconds, const FlowerPolicy *policy);
FlowerHit flower_pick(const Flower *flower, FlowerPoint origin, FlowerPoint direction);
void flower_normals(const Flower *flower, FlowerPoint *normals);

/* CPU drawing cache; testable without raylib. */
typedef struct {
    FlowerPoint position[3], normal[3];
    uint16_t node[3];
    float radial[3], angular[3];
} FlowerSurfaceTriangle;
typedef struct {
    int density, vertex_count, triangle_count, surface_count, rings, slices;
    FlowerPoint *position, *normal;
    uint16_t *parent;
    FlowerSurfaceTriangle *surface;
} FlowerMesh;
bool flower_mesh_build(const Flower *flower, int density, FlowerMesh *mesh);
void flower_mesh_free(FlowerMesh *mesh);
FlowerHit flower_mesh_pick(const FlowerMesh *mesh, FlowerPoint origin, FlowerPoint direction);
#endif
