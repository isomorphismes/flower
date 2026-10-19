# SUN #124 — Flower Net material chart with subpixel coverage

Authority: https://github.com/isomorphisms/flexible-pipes/issues/124
Draft: https://github.com/isomorphismes/flower/pull/6
Retained parent Flower: aa74149c17e1154c54b8db7d7ef3ef441614f37c
Source-qualified renamed Subpixel: efff5fbe71d8722d713282c478dbdb52fa280f9b
from rough.framebuffer draft PR #15, downstream of old-name PR #13.
Independent upstream source qualification: rough.framebuffer workflow
38996053521 and ai-ci workflow 38996306474. They do not independently accept
this Flower consumer.

## The mathematical/display boundary

- A true FlowerNet round consists of 64 consecutive **material IDs** at one
  selected ring. Its last edge reaches the existing first node across the
  periodic seam. One radial marking follows 16 real Net edges. No fold can
  introduce artificial neighbors. The marked paths are independent of Skin's
  realized XYZ embedding, springs, camera and display Mesh density.
- A bounded, display-neutral circular 2D chart maps these IDs into cubic
  Bezier controls; it is not derived from deformed world coordinates. The
  circular arcs themselves are approximations and do not imply a measured
  intrinsic metric. This initial visualization represents the **fixed
  64-sector Net prototype**, NOT the counted 8->12->18->27 stitch ledger
  developed separately in Flower draft PR #5.
- Two samplers consume the same cubic arcs: local quadratic 3x3 neighborhoods
  and local cubic 5x5 neighborhoods. Every pixel's mask uses 64 midpoint
  samples. Each ink/shape is OR-unioned in caller-owned occupancy before a
  SINGLE straight-alpha framebuffer blend. The local approximations do not
  prove global curve accuracy or subdivision invariance.
- The optional native raylib 5.5/NativeActivity consumer renders this in an
  **opaque separate flat 2D pane**, never through 3D folds. It is toggled by
  NET ON/OFF and sampling model controls (desktop P and O, respectively).
  Existing hold, release, Lua and picking state never enter this display
  code. Surface lighting / intrinsic rings from Flower PR #3 remain separate.
- Pointer ownership repair: the visible opaque pane and its caption block
  underlying 3D growth on Android and desktop. A shared display-neutral bounds
  calculation supplies the input exclusion and host boundary controls. Toggling
  the pane off restores the underlying grow region; physical input remains
  separately unverified.
- When disabled, no diagram allocation, raster sampling, texture update or
  extra per-frame drawing is performed (an O(1) enabled/dirty guard only).
  When shown or when the sampling mode changes, the diagram is rebuilt and
  transient buffers freed. Native temporary CPU occupancy+RGBA allocation
  is 192*192*(8+4) = 442,368 bytes; GPU RGBA texture is 147,456 bytes
  (driver overhead is unmeasured). A 64-curve round uses 576 patches; with
  the 16-segment radial mark, 720 patches total. Upper bounds: 414,720
  (3x3) or 1,152,000 (5x5) 8x8 midpoint predicates per rebuild. Raylib
  logs elapsed milliseconds, patch count and storage for an actual device to
  measure; no phone performance is asserted.
- Guardrails: <=64 curves per call, 9 local patches/curve, <=256x256
  occupancy plane, <=96px chart radius, no new framework, Java brain, C++,
  or user signing key. A caller with an invalid chart or source mask must
  fail without mixing stale pixels.

## Real sources, tests, exports and CI

- src/flower_construction.[ch]: Net mark extraction only.
- src/flower_subpixel.[ch]: exact renamed Subpixel curve/mask/blend adapter.
- src/flower_diagram.[ch]: optional raylib 5.5 CPU->RGBA flat-pane path.
- tests/test_construction.c: material seam/edge, fold, density, invalid-input
  invariance; tests/test_subpixel_construction.c: independent exact 8x8
  tilted line, fractional alpha, padded stride, double union and invalid
  refusal, both sampler paths.
- examples/flower_construction_preview.c: P6 image, left-to-right binary
  threshold illustration / true quadratic 3x3 / true cubic 5x5. Bottom row
  independently magnifies an 8x8 oblique pixel window tenfold in each panel.
  Binary panel is a deliberately crude comparator, NOT a prior Flower image.
  Exact output dimension is 576x288 and native frame content has no GUI text.

Successful host source suite:
https://github.com/isomorphismes/flower/actions/runs/39039081346
Actual optional upstream source pin was verified, then 4/4 regression
tests passed against the real renamed 3x3/5x5 source. A mismatched producer
commit was rejected. Native PPM was generated and inspected for its diagonal
coverage zoom. PPM SHA-256:
17fe75d84d5c7326c93b4815062b76ebe834d981e100dad32cf0e12629d66456
Retained artifact:
https://github.com/isomorphismes/flower/actions/runs/39039081346/artifacts/12102904376
This host source run used the explicit GCC reference diagnostic lane, NOT
pinned ICK acceptance. Source ICK/Ike and independent consumer acceptance
are handled by the separate ai-ci draft PR #279:
https://github.com/isomorphisms/ai-ci/pull/279

Successful real ARMv7 Android app compile/package at the same Flower source:
https://github.com/isomorphismes/flower/actions/runs/39039081420
Pinned NDK 26.3.11579264 API24; actual NativeActivity libflower.so now links
the optional flower_diagram_update, flower_subpixel_round and corrected
rough_blend_subpixel_coverage symbols. AGP 8.7.3 / Gradle 8.9.
Test-only APK digest:
7256bb53f40392ca35ef5ccf83cf040a633f70c6a97c27cdf303ea3969085a7d
Stable public test signer certificate SHA-256:
0cdc0345351b88296870f277a7a7ae1709f7fbc41603a97a8ad71d5931215543
Artifact: https://github.com/isomorphismes/flower/actions/runs/39039081420/artifacts/12103466984

Reproduce optional host path (after acquiring a *clean* exact Subpixel
checkout; source hash is mandatory):

    cmake -S . -B build/core -DFLOWER_CORE_ONLY=ON \
      -DFLOWER_SUBPIXEL_SOURCE=/absolute/path/to/rough.framebuffer \
      -DFLOWER_SUBPIXEL_EXPECTED_SHA=efff5fbe71d8722d713282c478dbdb52fa280f9b
    cmake --build build/core --parallel 2
    ctest --test-dir build/core --output-on-failure
    cmake --build build/core --target construction_preview_ppm

## Remaining gates

The image and APK have reproducible digests, but **physical MIRO A1/C67
installation, gestures, driver rendering, latency, and background/pause
behavior are untested**. A1 ARM32 candidate does not certify a C67 ARM64
product. Cat Food/Kitchen acquisition, FP registered dispatch and ai-ci
promotion receipts are not implied by a passing GitHub run. The optional
host source qualification does not prove ICK native-app compilation;
the baseline ICK/Bionic annotation gap remains.

SUN #124 is a draft implementation. No merge, publication, intrinsic
curvature/crochet equivalence, depth correctness or device signoff is
authorized by this artifact. Independent acceptance must run at the exact
Flower head; the separate device gate is FP #125.
