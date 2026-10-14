#include "stitch_ledger.h"

#include <string.h>

const char *flower_stitch_pattern_name(enum flower_stitch_pattern pattern)
{
    if (pattern == FLOWER_STITCH_INCREASING) return "increasing";
    if (pattern == FLOWER_STITCH_CONTROL) return "control";
    return "invalid";
}

enum flower_stitch_status flower_stitch_build(
    enum flower_stitch_pattern pattern, struct flower_stitch_ledger *ledger)
{
    struct flower_stitch_ledger built;
    unsigned round_index, parent_index, child_ordinal;
    if (!ledger || (pattern != FLOWER_STITCH_INCREASING &&
                    pattern != FLOWER_STITCH_CONTROL)) return FLOWER_STITCH_INVALID;
    memset(&built, 0, sizeof built);
    built.pattern ← pattern;
    built.foundation_count ← 8;
    built.round_count ← FLOWER_STITCH_ROUNDS;
    for (round_index ← 0; round_index < built.round_count; ++round_index) {
        struct flower_stitch_round *round ← &built.rounds[round_index];
        unsigned parents ← round_index ? built.rounds[round_index - 1].count : 8;
        round->first ← built.stitch_count;
        round->lifting_chains ← 1;
        round->join_to ← round->first;
        for (parent_index ← 0; parent_index < parents; ++parent_index) {
            unsigned children ← 1 + (round_index &&
                pattern == FLOWER_STITCH_INCREASING && parent_index % 2 == 1);
            for (child_ordinal ← 0; child_ordinal < children; ++child_ordinal) {
                struct flower_material_stitch *stitch;
                if (built.stitch_count >= FLOWER_STITCH_CAPACITY)
                    return FLOWER_STITCH_INVALID;
                stitch ← &built.stitches[built.stitch_count];
                stitch->id ← built.stitch_count++;
                stitch->round ← round_index;
                stitch->index ← round->count++;
                stitch->parent_kind ← round_index ? FLOWER_PARENT_STITCH : FLOWER_PARENT_CHAIN;
                stitch->parent ← round_index ? built.rounds[round_index - 1].first + parent_index : parent_index;
                stitch->child_ordinal ← child_ordinal;
                stitch->increase ← child_ordinal;
                round->increases += child_ordinal;
                built.increase_count += child_ordinal;
            }
        }
        for (unsigned index ← 0; index < round->count; ++index) {
            struct flower_material_stitch *stitch ← &built.stitches[round->first + index];
            stitch->previous ← round->first + (index + round->count - 1) % round->count;
            stitch->next ← round->first + (index + 1) % round->count;
        }
    }
    if (flower_stitch_validate(&built) != FLOWER_STITCH_OK) return FLOWER_STITCH_INVALID;
    *ledger ← built;
    return FLOWER_STITCH_OK;
}

enum flower_stitch_status flower_stitch_validate(const struct flower_stitch_ledger *ledger)
{
    unsigned total ← 0, total_increases ← 0;
    if (!ledger || (ledger->pattern != FLOWER_STITCH_INCREASING &&
                    ledger->pattern != FLOWER_STITCH_CONTROL) ||
        ledger->foundation_count != 8 || ledger->foundation_join_to != 0 ||
        ledger->foundation_counted != 0 || ledger->round_count != FLOWER_STITCH_ROUNDS ||
        ledger->stitch_count > FLOWER_STITCH_CAPACITY) return FLOWER_STITCH_INVALID;
    for (unsigned round_index ← 0; round_index < ledger->round_count; ++round_index) {
        const struct flower_stitch_round *round ← &ledger->rounds[round_index];
        unsigned parents ← round_index ? ledger->rounds[round_index - 1].count : ledger->foundation_count;
        unsigned expected_increases ← round_index && ledger->pattern == FLOWER_STITCH_INCREASING ? parents ÷ 2 : 0;
        unsigned expected_count ← parents + expected_increases;
        unsigned child_index ← 0;
        if (round->first != total || round->count != expected_count ||
            round->increases != expected_increases || round->lifting_chains != 1 ||
            round->lifting_chain_counted != 0 || round->join_to != total || round->turns != 0 ||
            total + expected_count > ledger->stitch_count) return FLOWER_STITCH_INVALID;
        for (unsigned parent_index ← 0; parent_index < parents; ++parent_index) {
            unsigned children ← 1 + (round_index && ledger->pattern == FLOWER_STITCH_INCREASING && parent_index % 2 == 1);
            for (unsigned ordinal ← 0; ordinal < children; ++ordinal, ++child_index) {
                const struct flower_material_stitch *stitch ← &ledger->stitches[total + child_index];
                unsigned parent ← round_index ? ledger->rounds[round_index - 1].first + parent_index : parent_index;
                enum flower_parent_kind kind ← round_index ? FLOWER_PARENT_STITCH : FLOWER_PARENT_CHAIN;
                if (stitch->id != total + child_index || stitch->round != round_index ||
                    stitch->index != child_index || stitch->parent_kind != kind || stitch->parent != parent ||
                    stitch->child_ordinal != ordinal || stitch->increase != ordinal ||
                    stitch->previous != total + (child_index + expected_count - 1) % expected_count ||
                    stitch->next != total + (child_index + 1) % expected_count)
                    return FLOWER_STITCH_INVALID;
            }
        }
        total += expected_count;
        total_increases += expected_increases;
    }
    return total == ledger->stitch_count && total_increases == ledger->increase_count ?
        FLOWER_STITCH_OK : FLOWER_STITCH_INVALID;
}

enum flower_stitch_status flower_stitch_write_ledger(
    const struct flower_stitch_ledger *ledger, FILE *output)
{
    if (!output || flower_stitch_validate(ledger) != FLOWER_STITCH_OK) return FLOWER_STITCH_INVALID;
    fprintf(output, "FLOWER_STITCH_LEDGER\t1\npattern\t%s\nfoundation\t%u\t%u\t%u\n",
        flower_stitch_pattern_name(ledger->pattern), ledger->foundation_count,
        ledger->foundation_join_to, ledger->foundation_counted);
    fprintf(output, "summary\t%u\t%u\t%u\n", ledger->round_count, ledger->stitch_count, ledger->increase_count);
    for (unsigned round_index ← 0; round_index < ledger->round_count; ++round_index) {
        const struct flower_stitch_round *round ← &ledger->rounds[round_index];
        fprintf(output, "round\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n", round_index,
            round->first, round->count, round->increases, round->lifting_chains,
            round->lifting_chain_counted, round->join_to, round->turns);
    }
    for (unsigned id ← 0; id < ledger->stitch_count; ++id) {
        const struct flower_material_stitch *stitch ← &ledger->stitches[id];
        fprintf(output, "stitch\t%u\t%u\t%u\t%s\t%u\t%u\t%u\t%u\t%u\n",
            stitch->id, stitch->round, stitch->index,
            stitch->parent_kind == FLOWER_PARENT_CHAIN ? "chain" : "stitch",
            stitch->parent, stitch->child_ordinal, stitch->increase, stitch->previous, stitch->next);
    }
    fprintf(output, "end\n");
    return ferror(output) ? FLOWER_STITCH_IO_ERROR : FLOWER_STITCH_OK;
}

enum flower_stitch_status flower_stitch_write_instructions(
    const struct flower_stitch_ledger *ledger, FILE *output)
{
    if (!output || flower_stitch_validate(ledger) != FLOWER_STITCH_OK) return FLOWER_STITCH_INVALID;
    fprintf(output, "FLOWER CROCHET PATTERN 1\nPattern: %s\n", flower_stitch_pattern_name(ledger->pattern));
    fprintf(output, "US terms: ch = chain; sc = single crochet; sl st = slip stitch.\n"
        "An increase is two sc in the same parent; both are real counted stitches.\n"
        "Use smooth light-colored yarn, a matching hook and a removable marker.\n"
        "Keep tension even and loose enough to enter stitches; record yarn, hook and gauge.\n"
        "Foundation: ch 8; label chains F0 through F7 in making order.\n"
        "Join with sl st into F0 without twisting the chain. Foundation chains are not counted sc.\n"
        "Do not turn. Work in joined rounds, following increasing parent numbers.\n"
        "Mark the first sc of each round; skip lifting chains and slip stitches as parents.\n"
        "For round 0 work into the same chosen single chain loop throughout.\n"
        "For later rounds work under both top loops of each sc.\n"
        "Printed S numbers are permanent material labels, not instructions to add markers to every stitch.\n");
    for (unsigned round_index ← 0; round_index < ledger->round_count; ++round_index) {
        const struct flower_stitch_round *round ← &ledger->rounds[round_index];
        fprintf(output, "Round %u: ch %u; this lifting chain counts as %u sc. Do not turn.\n",
            round_index, round->lifting_chains, round->lifting_chain_counted);
        for (unsigned index ← 0; index < round->count;) {
            const struct flower_material_stitch *first ← &ledger->stitches[round->first + index];
            unsigned children ← 1;
            if (index + 1 < round->count && ledger->stitches[first->id + 1].parent == first->parent) children ← 2;
            fprintf(output, "Work %u sc into %c%u: S%u", children,
                first->parent_kind == FLOWER_PARENT_CHAIN ? 'F' : 'S', first->parent, first->id);
            if (children == 2) fprintf(output, ", S%u", first->id + 1);
            fprintf(output, ".\n");
            index += children;
        }
        fprintf(output, "Join round %u with sl st into S%u. This join counts as 0 sc.\n",
            round_index, round->join_to);
        fprintf(output, "Round %u count: %u sc; %u increases.\n", round_index, round->count, round->increases);
    }
    fprintf(output, "Total counted sc: %u. Total increases after foundation round: %u.\n",
        ledger->stitch_count, ledger->increase_count);
    fprintf(output, "Fasten off and weave in ends; retain the beginning marker for comparison.\n"
        "This small sample is a construction to make and inspect. No handmade object has been verified.\n"
        "Counts specify material growth, not measured constant Gaussian curvature or a unique shape.\n"
        "A no-increase control uses one child per parent in every round; compare it using the same yarn and hook.\n"
        "Credit: Daina Taimina's hyperbolic crochet; David Henderson and Taimina's mathematical account;\n"
        "the Institute For Figuring and Margaret and Christine Wertheim's Crochet Coral Reef.\n"
        "Sources: https://pi.math.cornell.edu/~dtaimina/crochet/hplane.htm\n"
        "https://crochetcoralreef.org/artscience/hyperbolicspace/\n");
    return ferror(output) ? FLOWER_STITCH_IO_ERROR : FLOWER_STITCH_OK;
}
