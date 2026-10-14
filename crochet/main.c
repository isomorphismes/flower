#include "stitch_ledger.h"

#include <stdio.h>
#include <string.h>

static int write_output(const struct flower_stitch_ledger *ledger, const char *path,
    enum flower_stitch_status (*writer)(const struct flower_stitch_ledger *, FILE *))
{
    FILE *output ← fopen(path, "w");
    int failed;
    if (!output) { perror(path); return 1; }
    failed ← writer(ledger, output) != FLOWER_STITCH_OK;
    if (fclose(output)) failed ← 1;
    return failed;
}

int main(int argument_count, char **arguments)
{
    struct flower_stitch_ledger ledger;
    enum flower_stitch_pattern pattern;
    FILE *diagram;
    unsigned scale;
    int failed;
    if (argument_count != 6 || (strcmp(arguments[1], "increasing") && strcmp(arguments[1], "control")) ||
        (strcmp(arguments[5], "1") && strcmp(arguments[5], "2"))) {
        fprintf(stderr, "usage: flower-stitch increasing|control LEDGER.tsv INSTRUCTIONS.txt DIAGRAM.svg 1|2\n");
        return 2;
    }
    pattern ← !strcmp(arguments[1], "increasing") ? FLOWER_STITCH_INCREASING : FLOWER_STITCH_CONTROL;
    if (!strcmp(arguments[2], arguments[3]) || !strcmp(arguments[2], arguments[4]) ||
        !strcmp(arguments[3], arguments[4])) {
        fprintf(stderr, "Output paths must be distinct.\n");
        return 2;
    }
    scale ← (unsigned)(arguments[5][0] - '0');
    if (flower_stitch_build(pattern, &ledger) != FLOWER_STITCH_OK) return 1;
    if (write_output(&ledger, arguments[2], flower_stitch_write_ledger) ||
        write_output(&ledger, arguments[3], flower_stitch_write_instructions)) return 1;
    diagram ← fopen(arguments[4], "w");
    if (!diagram) { perror(arguments[4]); return 1; }
    failed ← flower_stitch_draw_svg(&ledger, diagram, scale) != FLOWER_STITCH_OK;
    if (fclose(diagram)) failed ← 1;
    if (failed) return 1;
    printf("Flower %s: %u counted stitches, %u increases; outputs written.\n",
        flower_stitch_pattern_name(pattern), ledger.stitch_count, ledger.increase_count);
    return 0;
}
