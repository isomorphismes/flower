#ifndef FLOWER_EDGE_H
#define FLOWER_EDGE_H
#include "raylib.h"
#include <stdbool.h>

/* Optional GLES2 post-filter on the ACTUAL 3D scene, not a replacement
 * for the mathematically different 3x3/5x5 cubic subpixel sampler. */
typedef struct {
    RenderTexture2D target;
    Shader shader;
    int texel_location, radius_location;
    bool attempted, failed;
} FlowerEdge;

bool flower_edge_prepare(FlowerEdge *edge,int viewport_width,int viewport_height);
void flower_edge_draw(FlowerEdge *edge,int radius,int viewport_width,int viewport_height);
void flower_edge_free(FlowerEdge *edge);
#endif
