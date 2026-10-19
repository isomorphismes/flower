# A counted crochet construction

This small joined-round construction can be followed with a hook and yarn.
It starts with a ring of eight chains and makes four rounds of single crochet.
Every increase inserts an actual second child into a named parent stitch.
The increasing sample has 8 → 12 → 18 → 27 counted stitches; the control has
8 → 8 → 8 → 8. The first sample has 65 counted single crochets and 19 increases.
The control has 32 counted single crochets and no increases.

The construction is inspired by Daina Taimina's physical hyperbolic crochet and
the Institute For Figuring's experimental Crochet Coral Reef. It provides a
small reproducible object to make and compare. It does not establish measured
Gaussian curvature, yarn mechanics or a unique three-dimensional shape. The
code has not made or examined a handmade object.

## Make the two samples

Use smooth, light-colored yarn, a matching hook and a removable stitch marker.
Use the same yarn and hook for both samples. Keep the tension even; record yarn,
hook size and gauge rather than assigning a universal length to a stitch.
The terms below are US: `sc` is single crochet, `ch` is chain and `sl st` is
slip stitch. An increase is two sc into one parent. Both children are counted.

1. Chain eight and label the chains F0 through F7 in making order. Join with
   a slip stitch into F0 without twisting the chain. The eight foundation
   chains and this join are not counted single crochets.
2. Chain one for height. Work one sc into F0 and mark this first sc. Continue
   with one sc in each remaining foundation chain, using the same single loop
   of each chain. Join with a slip stitch into the marked first sc. There are
   eight sc, labelled S0 through S7. Do not turn.
3. For each of the next three increasing rounds, chain one and work around the
   preceding round under both top loops of each sc. Alternate one sc in the
   next parent, then two sc in the next parent. Begin with the first parent
   after each join and stop after the last parent. The previous round has an
   even count on each of these three rounds, so the pairs divide exactly.
   Join into the first sc of the new round and do not turn. The counts are
   twelve, eighteen and twenty-seven.
4. For the control, replace step 3 by one sc in every parent. Each round has
   eight sc. Use the same four-round convention.
5. Neither the lifting chain nor the terminal slip stitch is counted as a sc
   or used as a parent in the next round. Fasten off and weave in the ends;
   keep the initial marker for comparison.

The generated instructions name every parent and child, so no increase is left
implicit in a rendering or floating growth field. The compact repeat above is
the human summary of those exact instructions. The sample deliberately stops
at the odd count 27; extending the repeat another round would require an explicit
choice about the remaining parent and would change this finite contract.

## Material ledger and drawing boundary

`stitch_ledger.h` declares the isolated ledger. A counted stitch carries its
stable ID, round, position in that round, parent role and ID, child ordinal,
increase flag, and previous/next counted neighbors in its cyclic round. A
foundation chain has a separate role, even when its small integer index equals
a counted stitch ID. The height-chain event is L followed by its round number;
the joining event is J followed by its round number. They are round properties,
not material sc IDs. The foundation joining event is JF. All events explicitly
count as zero sc. The construction has no turns.

The public records are forgeable C carriers. `flower_stitch_validate` checks
them before instructions, serialization or drawing. Bad counts, missing
increases, reused IDs, broken parents, wrong seam targets and counted lifting
chains are rejected. The parser-free builder accepts only the two bounded
patterns; this is not a universal crochet notation or an arbitrary pattern
interpreter. A failure leaves the builder's destination unchanged.

This incidence records crochet parenthood and cyclic neighbors. It does not
encode every yarn crossing, over/under relation, strand thickness or loop shape.
It is not a triangle mesh and it supplies no metric. Foundation and round joins
describe an annular construction with a hole; no center disk has been filled in.

`stitch_diagram.c` reads the checked ledger and makes an unwrapped incidence
diagram. Blue segments join parents and children; the additional child of each
increase is marked `+`; dashed seams close the cyclic rows. Its page coordinates
are a drawing cache. They have no metric or shape meaning. The two supported
display scales change the SVG page dimensions and leave all stable IDs,
incidences and printable instructions unchanged. No GPU or camera is needed.

## Relation to Flower's retained Net → Skin → Mesh work

Flower main at `e237192a4450963b298d65851cf03ae7dd3b3f7b` contains only its
project README. There is no root AGENTS.md or STYLE.md at that exact main.
The closed, unmerged Net prototype at
`aa74149c17e1154c54b8db7d7ef3ef441614f37c` is a separate retained reference;
this change does not revive it as accepted application code.

That reference Net has 16 bands and 64 periodic sectors, giving 1,088 fixed
nodes and 1,024 quadrilateral cells. Its growth modifies preferred material
distances without inserting nodes or changing adjacency. Our increasing stitch
ledger has different round cardinalities and genuinely inserts children. There
is no adjacency-preserving one-to-one identification with that fixed Net.

A later adapter can associate retained stitch IDs with a chosen material chart
and triangulate the intervening annular bands. That adapter must retain the
original ledger, record its interpolation choices and keep rendering density
separate from stitch count. A triangulated representative still does not prove
that yarn loop geometry or contact has been modeled. No changes to the existing
hold-growth, picking or pointer semantics are required by this isolated sample.

## Build and output contract

The maintained source uses the owned ICK C path, with its Unicode assignment
and division operators. Consumer builds must use the pinned ICK identified by
the parent execution receipt. Do not relabel a system compiler or an Idriç
generated-C path as that compiler. The Idriç-first attempt and concrete compiler
acquisition failure are preserved under
`examples/autogenerated/stitch-count/`; the C reference is separate evidence.

The executable takes six arguments including its name:
`flower-stitch increasing|control LEDGER.tsv INSTRUCTIONS.txt DIAGRAM.svg 1|2`.
It writes deterministic output to explicit, distinct paths. I/O failures cause
a nonzero exit; already written outputs are not promised transactional rollback.

Compile `crochet/stitch_ledger.c`, `crochet/stitch_diagram.c` and `crochet/main.c`
for the CLI. Replace main.c by `crochet/test_stitch.c` for its native tests.
Use C17 and `-O2 -Wall -Wextra -Werror`. Neither path depends on a renderer,
native window, GPU, external library or floating arithmetic.

The TSV is versioned by `FLOWER_STITCH_LEDGER` and `1`. It then contains:

| Record | Ordered fields after the record name |
| --- | --- |
| pattern | increasing or control |
| foundation | chain count; joining chain index; counted sc contribution |
| summary | round count; counted stitch total; increase total |
| round | round index; first material ID; count; increases; lifting chain count; counted lifting sc; join target material ID; turn count |
| stitch | material ID; round; index within round; parent role (chain/stitch); parent ID; child ordinal; increase flag; previous; next |
| end | no fields |

All rounds precede all material stitch records. Every ID and index is zero-based.
The foundation F0..F7 is given by its fixed range; sc IDs are S0..S64 or S0..S31.

The default native test runs both patterns, incidence checks, unchanged-output
checks, deterministic reconstruction, all ten malformed-ledger classes, and
both display scales. Its `--reject NAME` modes deliberately damage a valid
construction and demand validity, so a correct validator exits 1 with the
specific name. Names are `missing-increase`, `duplicate-stitch`, `seam-misjoin`,
`counted-chain`, `broken-parent`, `wrong-adjacency`, `extra-stitch`,
`foundation-misjoin`, `counted-foundation` and `turned-round`. Independent AI-CI
must also parse the instructions, reconstruct the material ledger and compare
both outputs; the implementation's own validation is not the independent oracle.

## Mathematical and physical evidence

The increasing construction specifies a 3:2 count ratio between these rounds.
Inferring a continuum metric additionally requires a measured or chosen stitch
width and row height, a material chart and a limit/approximation interpretation.
Equal counts in the control do not prove a flat planar annulus. Stitch tension,
anisotropy, yarn thickness, folding, bending and contact can change the realized
object. Distance in this incidence graph is a graph distance; it is not a
measured path length through fabric, a Riemannian distance, or a page distance.

Any later three-dimensional realization is viewed with ordinary Euclidean rays.
Neither this ledger nor its diagram changes the geometry of surrounding space.
Host correctness, GPU execution, A1/C67 device execution and a handmade sample
must retain separate evidence. This bounded integer ledger is portable in
principle; no device execution has been established here.

Primary sources read for this construction:

- [David W. Henderson and Daina Taimina, Crocheting the Hyperbolic Plane](https://pi.math.cornell.edu/~dtaimina/crochet/hplane.htm),
  published in Mathematical Intelligencer 23(2), 2001, pp.17–28. The source
  connects repeated increases, annular constructions and tactile geometry.
- [Institute For Figuring / Crochet Coral Reef, Hyperbolic Space](https://crochetcoralreef.org/artscience/hyperbolicspace/)
  and its linked [How to Crochet Hyperbolic Space brochure](https://crochetcoralreef.org/wp-content/uploads/2026/08/Reef-How-To-FINAL2020.pdf).
  Its experimental changes to growth patterns explain why attractive organic
  shapes and a prescribed constant-curvature model require different claims.
- [Institute For Figuring, the Crochet Coral Reef](https://www.theiff.org/exhibits/reef.html).

The present eight-chain joined-round pattern is our explicit finite test
construction, not a claim to reproduce a particular Taimina specimen or an
attribution of our exact recipe to her.
