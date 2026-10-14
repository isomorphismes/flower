#ifndef FLOWER_STITCH_LEDGER_H
#define FLOWER_STITCH_LEDGER_H

#include <stdio.h>

/* IDs denote counted material stitches, independently of drawing coordinates. */
enum flower_stitch_pattern { FLOWER_STITCH_INCREASING, FLOWER_STITCH_CONTROL };
enum flower_parent_kind { FLOWER_PARENT_CHAIN, FLOWER_PARENT_STITCH };
enum flower_stitch_status {
    FLOWER_STITCH_OK, FLOWER_STITCH_INVALID, FLOWER_STITCH_IO_ERROR
};
enum { FLOWER_STITCH_ROUNDS = 4, FLOWER_STITCH_CAPACITY = 65 };

struct flower_material_stitch {
    unsigned id, round, index;
    enum flower_parent_kind parent_kind;
    unsigned parent, child_ordinal, increase, previous, next;
};

struct flower_stitch_round {
    unsigned first, count, increases;
    unsigned lifting_chains, lifting_chain_counted, join_to, turns;
};

struct flower_stitch_ledger {
    enum flower_stitch_pattern pattern;
    unsigned foundation_count, foundation_join_to, foundation_counted;
    unsigned round_count, stitch_count, increase_count;
    struct flower_stitch_round rounds[FLOWER_STITCH_ROUNDS];
    struct flower_material_stitch stitches[FLOWER_STITCH_CAPACITY];
};

enum flower_stitch_status flower_stitch_build(
    enum flower_stitch_pattern pattern, struct flower_stitch_ledger *ledger);
enum flower_stitch_status flower_stitch_validate(
    const struct flower_stitch_ledger *ledger);
const char *flower_stitch_pattern_name(enum flower_stitch_pattern pattern);
enum flower_stitch_status flower_stitch_write_ledger(
    const struct flower_stitch_ledger *ledger, FILE *output);
enum flower_stitch_status flower_stitch_write_instructions(
    const struct flower_stitch_ledger *ledger, FILE *output);
enum flower_stitch_status flower_stitch_draw_svg(
    const struct flower_stitch_ledger *ledger, FILE *output, unsigned display_scale);

#endif
