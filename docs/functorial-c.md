# Functorial C / ICK qualification — 2026-10-06

**FUNCTORIAL + ICK BLOCKED:** current ICK cannot parse Bionic nullability and
Android availability annotations. No automatic compiler fallback was added.

| Conception | Native representation and operations | Interaction / derivation |
|---|---|---|
| Material net | `FlowerNet`, `flower_net_init`, `flower_net_hops` | Stable material node identity, never display-triangle adjacency |
| Intrinsic sheet / realization | `FlowerSkin`, `grow_intrinsic_geometry`, `preferred_link_length`, `relax_skin` | `simulation_step` composes BFS region, intrinsic growth, ordered relaxation |
| Policy map | `FlowerPolicy`, `flower_policy_valid`, Lua transaction validation | Lua changes bounded parameters; the native hold gate owns stepping |
| Drawing mesh | `FlowerMesh`, `flower_mesh_build`, `flower_pick` | Derived density and ray queries do not mutate Net/Skin |
| View | `FlowerCamera`, camera operations | Camera inspection preserves material state |

The existing Net → Skin → Mesh architecture is retained. Private growth and
relaxation operations now state the fixed-step algorithm; spring arithmetic is
owned by `relax_material_link`, rather than remaining inline in that algorithm.
Iteration count, ordering, timestep, material IDs, and numerical formulas are
unchanged. `ANDROID_ARM_MODE=arm` is maintained in CMake; an explicit Thumb
request fails instead of silently changing the A1 instruction state.

Executed: both CMake core/architecture tests using real pinned Lua
`dca7e57c16c524c8616144ed294fe598947a029f` from the selected symbolic Lua fork;
all eight compiling mutants rejected
by runtime assertions; CMake Android native library with actual pinned raylib
`c1ab645ca298a2801097931d1079b10ff7eb9df8`, API 24, A32/softfp ELF32 ARM.
Native diagnostics used NDK r27c explicitly; the existing hosted packaging lane
retains its r26d pin, package and stable signer. Those are different receipts.
The fork pin and its three-argument `lua_newstate` API were reconciled with the
live branch before rerunning both host suites, mutations and the native build.

The actual ICK probe of `src/flower.c` at API 24 failed on Bionic annotations,
using the current source-built GCC 17 ICK compiler. The exact fixtures and
terminal-failure compiler step live in `dilapidated-shed/ick`, under
`qualification/android-boundary`. NDK headers, raylib, Lua and Android linking
are retained. No new APK or physical-device acceptance is claimed here.
