#ifndef FLOWER_DIAGRAM_H
#define FLOWER_DIAGRAM_H
#include "flower_subpixel.h"
#include "flower_inspection.h"
#include "raylib.h"

typedef struct {
    Texture2D texture;
    bool enabled, dirty;
    int model, view, selected_node;
    uint64_t drawn_steps;
    unsigned rebuilds;
} FlowerDiagram;

void flower_diagram_init(FlowerDiagram *diagram);
/* Cycle OFF -> GROWTH -> REFERENCE STRETCH -> OFF. */
void flower_diagram_toggle(FlowerDiagram *diagram);
void flower_diagram_cycle_sampling(FlowerDiagram *diagram);
bool flower_diagram_select(FlowerDiagram *diagram, const Flower *flower,
                           int viewport_width, double screen_x, double screen_y);
bool flower_diagram_update(FlowerDiagram *diagram, const Flower *flower);
void flower_diagram_draw(const FlowerDiagram *diagram, const Flower *flower);
void flower_diagram_free(FlowerDiagram *diagram);
#endif
