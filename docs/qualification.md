# Flower qualification — 2026-10-05

## Executed native diagnostics

The original 15 core regression groups remain, adapted to separate Net/Skin.
They execute actual core code with `-O2 -DNDEBUG -Wall -Wextra -Werror -pedantic`:
starts paused; fresh valid press; intrinsic growth and changed realization;
full-state release freeze; no pointer handoff; owner identity; discarded
off-surface time; focus loss; cancel; fractional/paused-time discard; invalid
time cancellation; invalid picks; equal fixed steps at 60/120 Hz; two-sided
triangle picking and center hole; finite unit normals.

The additional architectural executable uses the same strict host compiler
flags and real Lua 5.4.8 at `6e22fedb74cf0c9b6656e9fce8b7331db847c605`:

- Exact BFS shells on a 2-band/4-sector fixture, angular seam, both boundaries,
  reciprocal cell-edge incidence and invalid seeds.
- Complete expected intrinsic increments, including a multiply-reached node,
  folded coincident distant material, unchanged Net and changed realization.
- Rigidly rotated/translated positions preserve intrinsic state and increments.
- Release freezes the full retained Net/Skin across 500 attempted frames.
- Two drawing densities preserve full state/hops and the same canonical faces;
  1,465 vertex/edge/interior ray cases plus background, hole and explicit seam
  tie cases map through both display meshes to the same material nodes.
- Actual camera translation/look/roll operations preserve retained Net/Skin;
  turning the head preserves camera position.
- Real Lua changes permitted held-growth parameters; closed native gate and
  release prevent evolution. Missing step APIs, parameter validation, rollback,
  instruction and memory budgets are executed.

Eight compiling deliberately broken implementations are rejected by runtime
checks: ignored release, ignored owner identity, ignored focus loss, disconnected
simulation, Euclidean folded-nearness brush, a render diagonal admitted as
material adjacency, Lua-policy bypass of the native gate, and repeated-path
growth multiplication. Compilation failure is not accepted as mutation evidence.

Both executables also passed local ASan/UBSan with the Lua library instrumented.
LeakSanitizer cannot inspect process threads in this execution container, so
local execution used `ASAN_OPTIONS=detect_leaks=0`. The hosted workflow retains
the ordinary full sanitizer configuration; its result is a separate evidence
layer. This does not establish leak checking locally.

## Android and packaging boundaries

Crystal's refreshed successful run is
https://github.com/isomorphisms/crystal/actions/runs/37306490183
at source `0ed094920683cc0eae12e7c077dd347ea3f4e9b5`. Flower's previous Android
failure occurred in `android-actions/setup-android`, before NDK compilation.
Flower now uses the runner's installed SDK directly, installs explicit SDK/NDK
components, and separately builds the real raylib/Lua native host and packages
its own NativeActivity APK with a stable public test signer. The product keeps
raylib pinned at 5.5 rather than silently inheriting Crystal's different renderer
revision. The first updated Android run compiled every source and exposed a
final-link failure from copying raylib 6.0's `--wrap=fopen` flag. Inspecting the
pinned 5.5 `utils.c` showed direct `android_fopen` implementation; the mismatched
flag was removed. No source library or build-infrastructure dependency was extracted.

Hosted Android compilation/package/signature status must be bound to the exact
published source commit and workflow, not inferred from Crystal or host tests.
No physical MIRO A1 execution has occurred in this task. Rendered input,
Android callback integration, actual multitouch release/cancel/focus behavior,
GPU behavior, performance, package replacement and installation remain UNKNOWN
until exercised on the exact APK and device. No prior Flower installation is
assumed. The APK is a test artifact, not a release.
