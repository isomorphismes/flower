#include "stitch_ledger.h"

/* An unwrapped incidence diagram. Distances on this page have no metric meaning. */
static unsigned page_x(unsigned index, unsigned count)
{
    return 85 + 1000 * index ÷ (count - 1);
}

enum flower_stitch_status flower_stitch_draw_svg(
    const struct flower_stitch_ledger *ledger, FILE *output, unsigned display_scale)
{
    if (!output || flower_stitch_validate(ledger) != FLOWER_STITCH_OK ||
        (display_scale != 1 && display_scale != 2)) return FLOWER_STITCH_INVALID;
    fprintf(output, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%u\" height=\"%u\" viewBox=\"0 0 1180 710\">\n",
        1180 * display_scale, 710 * display_scale);
    fprintf(output, "<title>Flower %s: stable stitch incidence</title>\n"
        "<desc>Unwrapped cyclic rows; blue links join parents to children. Red marked stitches are the second child of an increase. Dashed arcs are cyclic seams. Page distance is not material distance.</desc>\n"
        "<rect width=\"1180\" height=\"710\" fill=\"white\"/>\n"
        "<g font-family=\"sans-serif\" fill=\"#182329\">\n"
        "<text x=\"30\" y=\"30\" font-size=\"23\">Flower: %s stitch construction</text>\n"
        "<text x=\"30\" y=\"58\" font-size=\"14\">Stable material labels. Rows wrap across dashed seams. Page distances are not fabric distances.</text>\n",
        flower_stitch_pattern_name(ledger->pattern), flower_stitch_pattern_name(ledger->pattern));
    for (unsigned id ← 0; id < ledger->stitch_count; ++id) {
        const struct flower_material_stitch *stitch ← &ledger->stitches[id];
        const struct flower_stitch_round *round ← &ledger->rounds[stitch->round];
        unsigned parent_index ← stitch->parent;
        unsigned parent_count ← ledger->foundation_count;
        unsigned child_y ← 230 + 125 * stitch->round;
        if (stitch->parent_kind == FLOWER_PARENT_STITCH) {
            parent_index ← ledger->stitches[stitch->parent].index;
            parent_count ← ledger->rounds[stitch->round - 1].count;
        }
        fprintf(output, "<line data-child-id=\"%u\" x1=\"%u\" y1=\"%u\" x2=\"%u\" y2=\"%u\" stroke=\"#9eb7c4\"/>\n",
            id, page_x(parent_index, parent_count), child_y - 125,
            page_x(stitch->index, round->count), child_y);
    }
    for (unsigned index ← 0; index < ledger->foundation_count; ++index)
        fprintf(output, "<g data-foundation-id=\"%u\"><circle cx=\"%u\" cy=\"105\" r=\"6\" fill=\"#777\"/><text x=\"%u\" y=\"132\" text-anchor=\"middle\" font-size=\"13\">F%u</text></g>\n",
            index, page_x(index, ledger->foundation_count), page_x(index, ledger->foundation_count), index);
    for (unsigned round_index ← 0; round_index < ledger->round_count; ++round_index) {
        const struct flower_stitch_round *round ← &ledger->rounds[round_index];
        unsigned height ← 230 + 125 * round_index;
        fprintf(output, "<text x=\"15\" y=\"%u\" font-size=\"14\">R%u</text>\n"
            "<path data-seam-round=\"%u\" d=\"M85 %u Q585 %u 1085 %u\" fill=\"none\" stroke=\"#657880\" stroke-dasharray=\"5 4\"/>\n",
            height + 5, round_index, round_index, height, height + 76, height);
        for (unsigned index ← 0; index < round->count; ++index) {
            const struct flower_material_stitch *stitch ← &ledger->stitches[round->first + index];
            if (index + 1 < round->count)
                fprintf(output, "<line x1=\"%u\" y1=\"%u\" x2=\"%u\" y2=\"%u\" stroke=\"#243c49\"/>\n",
                    page_x(index, round->count), height, page_x(index + 1, round->count), height);
            fprintf(output, "<g data-material-id=\"%u\"><circle cx=\"%u\" cy=\"%u\" r=\"7\" fill=\"%s\"/><text x=\"%u\" y=\"%u\" text-anchor=\"middle\" font-size=\"11\">S%u%s</text></g>\n",
                stitch->id, page_x(index, round->count), height,
                stitch->increase ? "#a02d3e" : "#164b62", page_x(index, round->count), height + 23,
                stitch->id, stitch->increase ? "+" : "");
        }
    }
    fprintf(output, "<text x=\"30\" y=\"675\" font-size=\"13\">+ marks the additional child in a two-sc increase. Lifting chains and slip joins are uncounted events.</text>\n"
        "<text x=\"30\" y=\"698\" font-size=\"12\">Inspired by Daina Taimina and the Institute For Figuring. This diagram makes no shape or curvature claim.</text>\n</g></svg>\n");
    return ferror(output) ? FLOWER_STITCH_IO_ERROR : FLOWER_STITCH_OK;
}
