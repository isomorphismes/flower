# C67 Flower inspection — retained prototype (source candidate)

Issue: https://github.com/isomorphisms/flexible-pipes/issues/125
Base: `prototype/raylib-hold-growth`, not `main`.
The physical MIRO C67 screenshots show the original Flower 3D scene, NET
circle/spoke, curve 3×3 and 5×5 labels, and mesh-density control working.
The cramped header and little visible information are observed defects.

## Changed behavior

* `NET OFF → NET GROW → NET STR → NET OFF` now uses **the live FlowerNet
  material IDs** (17 rounds, 64 sectors) and `FlowerSkin` data. It draws
  the complete marked ring/sector chart; colors show the local **target
  intrinsic growth** or mean **actual adjacent edge length divided by
  reference edge length, minus 1**. This latter quantity is *not* intrinsic
  curvature and is not a validated strain-energy model.
* Tapping or dragging **inside the opaque NET pane** selects a material ID.
  Selection appears on the chart, as a yellow 3D point, and as an ID/growth/
  reference-stretch caption. The pane owns pointer input. It may not grow
  the underlying flower even when another pointer is held.
* `WIRE` reveals actual **display triangles**, unlike the
  17×64 material chart. `MESH 1x/2x` changes the drawing mesh while
  retaining the same underlying Skin; wireframe makes the difference
  observable. Its depth and offset must be checked on physical hardware.
* `EDGE OFF/3x3/5x5` optionally filters the **rendered 3D silhouette**,
  with a bounded x4 lens on pixels near the upper center. This is a
  screen-space **3×3/5×5 pixel neighborhood filter**, *not* 3×3/5×5
  geometric supersampling, not MSAA, not a depth-aware edge resolver,
  and not the independent rough.framebuffer cubic samplers. Strong
  differences, ghosting on folded overlaps and performance must be
  inspected; no image-quality superiority is asserted. When off, the
  application renders straight to screen with no render-texture pass.
* The title/instructions use a dedicated 70px row and four 50px buttons
  below it. The NET and EDGE lens are positioned below the header.
  Mobile fly/look/roll pads occupy the bottom 16% instead of 25%.

## Source/acceptance boundaries

* The live chart is pure portable C with a separate test executable. The
  original growth/lifecycle, Lua parameters, raylib 5.5 pin, fixed Net,
  3D Skin, material hit-picking, stable developer signer and C67 ARM64
  package ID are unchanged. Do not mutate simulated state inside chart
  drawing, wireframe or shader code.
* Gate on exact-head host regression, construction and independent subpixel
  oracles, then C67 ARM64 APK build + signer/ABI checks. Compiling GLSL
  text into an APK does **not** prove its Android runtime shader compiles.
* C67 physical test still required for touch capture, hold/release,
  selected ID, distortion reading, wire visibility and depth, 3x3/5x5
  comparison (with/without folds), renderer fallback, pause/resume and
  FPS/memory. A1 ARM32 remains separate.
* The full ICK/Bionic implementation is separately blocked. These Android
  NDK candidate stages do not cure that boundary.
* Deliver **a directly installable APK or one pasteable Termux command,
  never make the user unpack a ZIP**; do not ask to uninstall the existing
  signed package.

## Suggested device checks

1. Hold/release the surface; ensure PAUSED for 10 seconds.
2. Tap NET twice and drag the chart to select distinct real material IDs;
   the 3D shape must not change from pane touches.
3. Turn WIRE ON; switch MESH 1x/2x. The triangle count changes, not the
   intrinsic growth field.
4. Cycle EDGE OFF/3x3/5x5 and inspect the pixel x4 inset and real 3D
   boundary. Compare stationary camera/geometry only.
5. Background/resume, then retest hold and pane capture.

No physical test or independent AI-CI promotion is inferred from this
source change.
