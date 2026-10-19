# Flower Net → Skin → Mesh

## Carrier and identity

`FlowerNet` is a marked, finite quadrilateral annulus: 16 radial bands × 64
periodic angular sectors, 1,088 stable nodes and 1,024 cells. Node ID is
`ring * slices + slice`; ring zero is the inner boundary and ring sixteen is
the outer boundary. A cell has four ordered material IDs. Angular slice 64
is slice zero, with an unwrapped local chart at the seam. Reciprocal cell-edge
incidence is tested on a small annulus. No positions, spring laws, Lua objects,
raylib types or GPU buffers belong to the Net.

The material graph uses the four radial/angular cell-edge neighbors. Diagonal
springs and weak two-hop springs belong to mechanics, not this adjacency.
Hop distance means shortest path in **this chosen finite graph**; it is not
invariant under a future change of material discretization.

## Intrinsic state and realization

`FlowerSkin` holds reference and realized positions, accumulated tangential
and radial intrinsic growth at marked nodes, fixed spring constraints and the
simulation step count. Link rest lengths and their reference tangential
fractions are material geometric data. Preferred link length is generated
from endpoint growth rather than stored independently:

`rest_length * sqrt(tangent_scale² * tangent_fraction + radial_scale² * (1 - tangent_fraction))`

Each scale is one plus the mean accumulated growth at its two endpoints.
Rigidly moving the realization changes no intrinsic field. Deformation is an
ordered eight-pass spring relaxation with the inner boundary pinned. Positions,
intrinsic fields, constraints and step count suffice to resume deterministically
on the same floating-point implementation; no velocity or hidden solver history
exists. Cross-architecture bitwise reproducibility is not asserted. Process-death
serialization is not implemented.

## Native stroke and Lua policy

Only `flower_hold_frame_policy` can reach the private simulation step. It
checks focus, elapsed-time validity, capture, cancellation, pointer ownership,
down state and valid selected material before stepping. Paused/off-surface
time is discarded, release discards fractional time, and stalls cancel. A new
stroke requires a fresh focused press after all contacts have lifted.

`policy/default.lua` is embedded at build time, executed at startup, and calls
`flower.set_growth(rate, hop_radius, falloff, edge_bias, radial_fraction)`.
The validated result directly controls the actual native numerical growth law.
The VM receives a temporary policy only, no Flower, hold state, clock, native
pointer or step API. No standard libraries are opened. Failed scripts roll back
all parameter changes; 256 KiB allocation and 100,000-instruction budgets bound
configuration. Tests change the policy while paused and after release, then
prove the entire retained Flower unchanged. Script mutation cannot transfer
simulation-time authority.

Picking snaps the hit to one marked node. BFS finds each node at distance
`d <= k` once. Weight is `(1 - d/(k+1))^falloff`; the tangential growth increment
is `rate * fixed_dt * weight * (1-edge_bias + edge_bias*material_radius²)`.
Radial growth receives that increment times `radial_fraction`. Intrinsic scales
are capped at 1.5 accumulated growth; the pinned inner ring receives none.
Mechanical deformation can spread beyond this intrinsic support. Coincident
or nearby folds cannot create material neighbors.

## Replaceable drawing and picking

`flower_mesh.c` derives a CPU drawing cache without raylib. Each material quad
uses the prototype's two triangles. Density 1 emits those 2,048 triangles;
density 2 splits each into four coplanar triangles, emitting 8,192. Both retain
the same piecewise planar realization. Interpolated normals and every drawing
vertex are caches. `main.c` alone allocates/uploads raylib Mesh objects and
handles input/platform integration. The Mesh button (or desktop T) replaces
only those drawing caches; its press cannot start growth.

This separates **display tessellation** from the authoritative Net/Skin. The
simulation still samples a fixed material Net; no claim of simulation-resolution
independence or new smooth geometry is made. More triangles do not create
material, growth, bending state or topological neighbors.

Picking intersects actual triangles of the selected drawing density. A hit
maps through its cached canonical parent triangle into an unwrapped material
chart. Nearest marked node uses squared chart distance; distances tied within
1e-6 choose the lower stable ID, including the seam. Ray intersections use
double arithmetic and a 2e-6 barycentric shared-edge tolerance. Coincident ray
hits within 1e-6 ray parameter choose the lower parent-face ID. Parent-chart
reconstruction avoids density-dependent interpolated material rounding.
These tolerances assume the current unit-scale model. Grazing/degenerate
triangles and extreme coordinates are not qualified as general robust predicates.

## Camera and architectural evidence

The native camera module owns translation, look and roll only. Its turn operation
preserves position; no player body exists. The raylib adapter copies camera
coordinates into it and back. Tests execute the actual camera operations while
retained Net/Skin remains bitwise frozen.

Crystal PR #7 was refreshed at `0ed094920683cc0eae12e7c077dd347ea3f4e9b5`.
Its working C/Lua/raylib APK path informed a small compiled-in Lua policy boundary,
SDK component installation, AGP 8.7.3 / Gradle 8.9 and NativeActivity packaging.
Its temporary `CrystalNet` embeds positions and drawing concerns more closely
than its conceptual design; Flower deliberately separates these. Crystal's
auto-spin and orientation-dependent external shapes are not Flower growth laws.
Its raylib 6.0 `--wrap=fopen` linker flag is also version-specific: Flower's
pinned 5.5 implements `android_fopen` directly and needs no linker wrapping.
No shared source library is introduced.

The GT3M notes pay for explicit material identity, finite incidence, bounded hops,
seam compatibility and replaceable drawing here. Developing maps, holonomy,
Teichmüller/Weil–Petersson theory, mapping classes, laminations, train tracks and
circle-pattern solvers remain unimplemented. This is a spring-sheet experiment,
not a botanical or validated elastic-shell model; self-collision and adaptive
material remeshing remain absent.
