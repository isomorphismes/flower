#include "stitch_ledger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned assertions;

static void require(int condition, const char *description)
{
    ++assertions;
    if (!condition) { fprintf(stderr, "FAIL: %s\n", description); exit(1); }
}

static int corrupt(struct flower_stitch_ledger *ledger, const char *name)
{
    if (!strcmp(name, "missing-increase")) {
        /* Omit the final extra sc and repair the shortened cyclic row. */
        --ledger->rounds[3].count;
        --ledger->rounds[3].increases;
        --ledger->stitch_count;
        --ledger->increase_count;
        ledger->stitches[63].next ← 38;
        ledger->stitches[38].previous ← 63;
    }
    else if (!strcmp(name, "duplicate-stitch")) ledger->stitches[10].id ← 9;
    else if (!strcmp(name, "seam-misjoin")) ledger->rounds[1].join_to ← 9;
    else if (!strcmp(name, "counted-chain")) ledger->rounds[1].lifting_chain_counted ← 1;
    else if (!strcmp(name, "broken-parent")) ledger->stitches[10].parent ← 2;
    else if (!strcmp(name, "wrong-adjacency")) ledger->stitches[19].next ← 9;
    else if (!strcmp(name, "extra-stitch")) ++ledger->stitch_count;
    else if (!strcmp(name, "foundation-misjoin")) ledger->foundation_join_to ← 1;
    else if (!strcmp(name, "counted-foundation")) ledger->foundation_counted ← 1;
    else if (!strcmp(name, "turned-round")) ledger->rounds[1].turns ← 1;
    else return 0;
    return 1;
}

int main(int argument_count, char **arguments)
{
    struct flower_stitch_ledger increasing, control, snapshot, repeated;
    const unsigned counts[] ← {8, 12, 18, 27};
    const unsigned increases[] ← {0, 4, 6, 9};
    const char *mutants[] ← {"missing-increase", "duplicate-stitch", "seam-misjoin",
        "counted-chain", "broken-parent", "wrong-adjacency", "extra-stitch",
        "foundation-misjoin", "counted-foundation", "turned-round"};
    FILE *output;
    require(flower_stitch_build(FLOWER_STITCH_INCREASING, &increasing) == FLOWER_STITCH_OK, "build increasing construction");
    if (argument_count == 3 && !strcmp(arguments[1], "--reject")) {
        if (!corrupt(&increasing, arguments[2])) return 2;
        require(flower_stitch_validate(&increasing) == FLOWER_STITCH_OK, arguments[2]);
        return 0;
    }
    if (argument_count != 1) return 2;
    require(flower_stitch_build(FLOWER_STITCH_CONTROL, &control) == FLOWER_STITCH_OK, "build no-increase control");
    require(increasing.stitch_count == 65 && increasing.increase_count == 19, "increasing totals 65 and 19");
    require(control.stitch_count == 32 && control.increase_count == 0, "control totals 32 and zero");
    for (unsigned round ← 0; round < 4; ++round) {
        require(increasing.rounds[round].count == counts[round], "declared increasing count");
        require(increasing.rounds[round].increases == increases[round], "declared increase count");
        require(control.rounds[round].count == 8 && control.rounds[round].increases == 0, "control one child per parent");
    }
    for (unsigned id ← 0; id < increasing.stitch_count; ++id) {
        const struct flower_material_stitch *stitch ← &increasing.stitches[id];
        require(increasing.stitches[stitch->next].previous == id, "reciprocal cyclic adjacency");
        if (stitch->parent_kind == FLOWER_PARENT_STITCH)
            require(increasing.stitches[stitch->parent].round + 1 == stitch->round, "parent is in preceding round");
    }
    require(increasing.stitches[10].parent == 1 && increasing.stitches[10].child_ordinal == 1,
        "first increase is second child of S1");
    require(increasing.stitches[64].parent == 37 && increasing.stitches[64].child_ordinal == 1,
        "final child belongs to last parent without crossing seam");
    require(increasing.stitches[64].next == 38 && increasing.stitches[38].previous == 64,
        "last row closes onto its first counted stitch");
    require(flower_stitch_build(FLOWER_STITCH_INCREASING, &repeated) == FLOWER_STITCH_OK &&
        !memcmp(&increasing, &repeated, sizeof increasing), "deterministic stable IDs and incidence");
    snapshot ← increasing;
    require(flower_stitch_build((enum flower_stitch_pattern)9, &increasing) == FLOWER_STITCH_INVALID &&
        !memcmp(&snapshot, &increasing, sizeof increasing), "invalid pattern leaves output unchanged");
    require(flower_stitch_build(FLOWER_STITCH_CONTROL, NULL) == FLOWER_STITCH_INVALID, "null output rejected");
    require(flower_stitch_validate(NULL) == FLOWER_STITCH_INVALID, "null ledger rejected");
    for (unsigned index ← 0; index < sizeof mutants ÷ sizeof mutants[0]; ++index) {
        struct flower_stitch_ledger mutant ← increasing;
        require(corrupt(&mutant, mutants[index]), "known mutation selected");
        require(flower_stitch_validate(&mutant) == FLOWER_STITCH_INVALID, mutants[index]);
        output ← tmpfile();
        require(output != NULL, "temporary output opened");
        require(flower_stitch_write_ledger(&mutant, output) == FLOWER_STITCH_INVALID && ftell(output) == 0,
            "invalid ledger produces no serialized output");
        require(flower_stitch_write_instructions(&mutant, output) == FLOWER_STITCH_INVALID && ftell(output) == 0,
            "invalid ledger produces no craft instructions");
        require(fclose(output) == 0, "temporary output closed");
    }
    output ← tmpfile();
    require(output != NULL, "diagram output opened");
    require(flower_stitch_draw_svg(&increasing, output, 0) == FLOWER_STITCH_INVALID && ftell(output) == 0,
        "invalid display scale produces no drawing");
    require(flower_stitch_draw_svg(&increasing, output, 1) == FLOWER_STITCH_OK, "draw at scale one");
    require(flower_stitch_draw_svg(&increasing, output, 2) == FLOWER_STITCH_OK, "draw at scale two");
    require(!memcmp(&snapshot, &increasing, sizeof increasing), "drawing scale preserves authoritative material ledger");
    require(fclose(output) == 0, "diagram output closed");
    require(flower_stitch_write_ledger(&increasing, NULL) == FLOWER_STITCH_INVALID, "null output stream rejected");
    printf("Flower stitch ledger: PASS (%u assertions)\n", assertions);
    return 0;
}
