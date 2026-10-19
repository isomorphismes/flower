#ifndef FLOWER_DIAGRAM_H
#define FLOWER_DIAGRAM_H

#include "flower_subpixel.h"
#include "raylib.h"

typedef struct {
    Texture2D texture;
    bool enabled, dirty;
    int model; /* 3x3 or 5x5; never stored on Net or Skin */
    unsigned rebuilds;
} FlowerDiagram;

void flower_diagram_init(FlowerDiagram *diagram);
void flower_diagram_toggle(FlowerDiagram *diagram);
void flower_diagram_cycle_sampling(FlowerDiagram *diagram);
bool flower_diagram_update(FlowerDiagram *diagram, const FlowerNet *net);
void flower_diagram_draw(const FlowerDiagram *diagram);
void flower_diagram_free(FlowerDiagram *diagram);
#endif
