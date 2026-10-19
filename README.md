# Flower: hold a material region to grow

Flower starts paused. A fresh press on the flower captures one pointer and
grows a bounded **topological material neighborhood**. Release, cancel, focus
or lifecycle loss freezes growth and all relaxation immediately. Camera flight,
look and roll do not advance the flower. Paused time never catches up; another
already-down finger cannot inherit a stroke.

The native architecture is [Net → Skin → Mesh](docs/net-model.md):

- Net: a marked quadrilateral annulus with radial/angular adjacency, periodic
  seam and center opening; no embedded positions or raylib objects.
- Skin: preferred intrinsic tangential/radial growth, reference and realized
  positions, spring constraints and deterministic continuation state.
- Mesh: replaceable CPU/raylib drawing caches. The MESH button switches between
  2,048 and 8,192 display triangles without changing Net/Skin or hop distance.
- Lua: [a real growth policy](policy/default.lua), embedded at build time;
  validated rate, hop radius, falloff and directional/edge bias govern the native
  growth calculation. Lua receives neither native state nor simulation time.

Android has separate FLY, LOOK, UP, DOWN and ROLL pads. Desktop diagnostic
inspection uses WASD, Q/E, right-drag, Z/X and T for display density. The camera
has no player body. One fixed rose pigment and derived two-sided lighting remain.

## Build and evidence

The product route is pinned Android NDK `26.3.11579264`, `armeabi-v7a`, API 24;
raylib 5.5 source `c1ab645ca298a2801097931d1079b10ff7eb9df8`; Lua 5.4.8 source
`6e22fedb74cf0c9b6656e9fce8b7331db847c605`. The initial repository had no
qualified Ick/raylib/NativeActivity integration, which remains the explicit Ick
integration gap. No phone/tablet compilation is required.

The workflow separately runs native core/Lua tests, runtime mutation rejection,
sanitizers, actual NDK compilation and signed test APK packaging. Packaging uses
Crystal's demonstrated Java 17 / Gradle 8.9 / AGP 8.7.3 NativeActivity route.
Flower's identity is `org.isomorphisms.flower`, with a stable public development
test signer described in [android/README.md](android/README.md). This is not a
production key. An APK build does not prove MIRO A1 installation or behavior.

Core-only CMake qualification (`FLOWER_CORE_ONLY=ON`) needs no raylib/window.
`scripts/test.sh` retains the original core-only regressions; CMake/CTest also
runs the architectural and real-Lua checks. The adapted mutation runner uses
the pinned Lua checkout via `FLOWER_LUA_SOURCE`.

See [qualification](docs/qualification.md) for executed evidence and boundaries.
The fixed-Net spring-sheet approximation is not a calibrated botanical/shell
model. Display density is independent, but simulation discretization is fixed.
No adaptive material remeshing, collision, thickness, process-death persistence,
advanced GT3M machinery or shared Crystal/Flower library has been added.

Design input: [GT3M notes](docs/gt3m-flower-notes.md).
