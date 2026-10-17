# SUN FLOWER-SUBPIXEL-01: material construction chart (FP #124)

Origin: https://github.com/isomorphisms/flexible-pipes/issues/124
Prerequisite: https://github.com/isomorphisms/flexible-pipes/issues/120
Retained Flower baseline: prototype/raylib-hold-growth at
aa74149c17e1154c54b8db7d7ef3ef441614f37c.
Reusable coverage candidate: rough.framebuffer PR #13 at
49aa03cdeabbc57dbac758e0a9470579c3af0015 (old names, **not** yet usable).

## Boundaries

- A stable Net marks a round by 64 consecutive material IDs at a selected
  ring. The periodic final edge returns to the same first material ID, not a
  duplicated seam node. A radial marker traverses actual Net edges.
  A fixed circular 2D chart assigns cubic controls analytically from these
  IDs; it never reads world XY, Skin, springs, camera or display Mesh.
- This is **not** the variable-count 8→12→18→27 crochet ledger in Flower PR #5.
  The 64-sector Net is a discretization of the prototype material, not a
  counted physical crochet pattern. No metric, curvature, crochet-equivalence
  or handmade-tension claim is warranted.
- The optional adapter consumes only corrected paths:
  subpixel/include/subpixel/{curve_patch,coverage_plane}.h and
  src/subpixel_framebuffer.h. It calls cp_* curve constructors and the renamed
  rough_blend_subpixel_coverage symbol. No legacy alias is accepted. SUN #120
  must publish and qualify corrected source before integration.
- Each cubic arc or radial segment is sampled at 9 local points. Both the
  quadratic 3×3 and cubic 5×5 methods consume the same material mark.
  Per-sample OR union applies to one mask per ink/color. Straight-alpha
  composition happens once per completed shape, never per overlapping patch.
  Existing framebuffer has no depth, so ONLY FLAT DIAGRAMS are permitted:
  never superimpose this layer through occluded 3D folds.
- Limits: up to 64 curves per call, 9 patches each, 256×256 pixels per plane,
  radii no greater than 96 pixels. Exporter uses 192×192 panels and 884,736
  bytes of static mask+color storage with no per-frame allocation. The 64-arc
  round has upper bounds of 331,776 sample predicates for 3×3 and 921,600
  for 5×5, plus the radial marker. These are NOT latency measurements.
  Default Flower application retains the fast path; no diagram work per frame.
- Local 3×3/5×5 patches do not prove global curve accuracy or exact circle
  rendering. The 2D circle is itself a cubic approximation.

## Source and tests

- src/flower_construction.[ch] is display-neutral, independent of graphics.
  tests/test_construction.c checks actual Net IDs, seam, exact radial edges,
  circular controls, refusal without writes, a deliberately damaged edge,
  folded Skin, and both Flower drawing densities.
- src/flower_subpixel.[ch] is the separate optional consumer.
  tests/test_subpixel_construction.c independently calculates the 9 × 64
  midpoint samples of a tilted segment. It checks alpha for 1/64 and 32/64
  occupancy, padded stride, valid invalid/refusal, repeat-union idempotence,
  and actual 3×3/5×5 invocation. Tests must be COMPILED AND RUN against the
  corrected source; merely writing them is not acceptance.
- examples/flower_construction_preview.c generates three P6 image panels:
  binary illustration thresholded from the quadratic mask (NOT an earlier
  Flower screenshot), true 3×3 coverage, and true 5×5 coverage.
  This is native CPU output, not Android/GPU output.

After #120 publishes a corrected, immutable source commit, configure:

    cmake -S . -B build/core -DFLOWER_CORE_ONLY=ON \
      -DFLOWER_SUBPIXEL_SOURCE=/absolute/path/to/rough.framebuffer \
      -DFLOWER_SUBPIXEL_EXPECTED_SHA=FULL_40_HEX_COMMIT
    cmake --build build/core --parallel 2
    ctest --test-dir build/core --output-on-failure
    cmake --build build/core --target construction_preview_ppm

The checkout must have corrected API paths and match the exact pinned Git
commit; any mismatch is fatal. Even a host CMake pass is not the required
ICK/Ike qualification. The Flower baseline has an unresolved ICK/Bionic
Android compiler gap. No stock-host-compiler substitution may be reported
as ICK acceptance.

## Evidence and unresolved work

- Stable material coordinates: candidate implemented; CI result must be read.
- Real 3×3/5×5: consumer code prepared, BLOCKED by FP #120 rename.
- Numeric oracle and negative controls: tests authored; source run, independent
  ai-ci reconstruction and compiler policy acceptance are NOT YET evidenced.
- PPM: producer written, no independently inspected output or artifact yet.
- Physical A1/C67, NativeActivity and raylib 5.5 on-device UI: NOT TESTED;
  belongs to FP #125. Flower PR #3 lighting/rings remains separate.
- Signed APK, Cat Food delivery, Kitchen/Ike receipts: NOT DONE.
- 3D occlusion, true metric/crochet construction: NOT CLAIMED.
- No merge, issue closure, deployment, publishing or dispatch authority.

Bounded continuations: E1 qualify active source rename in FP #120; E2 pin
that source and ICK/Ike, compile actual tests and PPM, then independently
reconstruct masks/alpha and reject deliberately broken diagonal, stride,
overlap and receipt mutants in ai-ci; E3 only then add a raylib 5.5 flat
overlay toggle and produce a signed APK candidate. A1/C67 physical signoff
remains the separate FP #125 gate.
