# Explicit external geometry code

80 original code files from 12 source sets are stored here, with the upstream paths retained inside each directory. Files are unchanged; sources.tsv records repository, exact revision, original path, Git blob identity and size. PROVENANCE.md and upstream license files identify the authors and redistribution terms.

The [complete upstream source roots](../../sources/external/README.md) add 15 pinned Git submodules. The copied Curved Spaces files are bound to the official archive in archive-sources.tsv; the other copied files remain bound to upstream commits in sources.tsv.

## Start with the operation you want to inspect

| Source set | Actual code to open | Operation |
| --- | --- | --- |
| Tim Hutton | [index.html](crochet-simulator/index.html) | Parse crochet patterns, connect stitches and relax the surface. |
| RogueViz | [rug.cpp](rogueviz/rug.cpp), [hyperpoint.cpp](rogueviz/hyperpoint.cpp) | Realize a rug and perform intrinsic point/isometry operations. |
| CirclePack | [HypPacker.java](circlepack/src/main/java/rePack/HypPacker.java), [HyperbolicMath.java](circlepack/src/main/java/geometry/HyperbolicMath.java) | Adjust circle radii/angle sums and compute hyperbolic geometry. |
| SnapPy | [hyperboloid_utilities.py](snappy/src/snappy/raytracing/hyperboloid_utilities.py), [hyperboloid_navigation.py](snappy/src/snappy/raytracing/hyperboloid_navigation.py), [ideal_raytracing_data.py](snappy/src/snappy/raytracing/ideal_raytracing_data.py) | Lorentz transformations, camera movement and tetrahedral ray data. |
| Thurston geometries | [part1.glsl](thurston-geometries/src/geometries/hyp/geometry/shaders/part1.glsl), [part2.glsl](thurston-geometries/src/geometries/hyp/geometry/shaders/part2.glsl), [mapping.glsl](thurston-geometries/src/geometries/hyp/cameras/native/shaders/mapping.glsl) | Point/vector definitions, explicit hyperbolic geodesic flow and camera-ray mapping. |
| Geometry Center / Knot-complement | [recenter.c](knot-complement/refactor/c/recenter.c), [dgdraw.c](knot-complement/upstream/geometry-center/geomview/src/lib/gprim/discgrp/dgdraw.c), [dgdirdom.c](knot-complement/upstream/geometry-center/geomview/src/lib/gprim/discgrp/dgdirdom.c), [Flythrough main.c](knot-complement/upstream/geometry-center/geomview/src/bin/flythrough/main.c) | Compare whole-camera recentering, Dirichlet neighbors and historical playback. |
| Geometry Central | [intrinsic_triangulation.cpp](geometry-central/src/surface/intrinsic_triangulation.cpp), [upstream tests](geometry-central/test/src/intrinsic_triangulation_test.cpp) | Intrinsic triangulation operations, refinement and Delaunay comparisons. |
| Flipper | [lamination.py](flipper/flipper/kernel/lamination.py), [moves.py](flipper/flipper/kernel/moves.py), [encoding tests](flipper/tests/encoding.py) | Curve/lamination operations and mapping-class examples. |
| Hyperbolic-Honeycombs | [Circle-Packing-2D.frag](hyperbolic-honeycombs/src/Circle-Packing-2D.frag), [DERaytracer.frag](hyperbolic-honeycombs/src/Hyperbolic-Honeycombs-DERaytracer.frag) | Explicit packing and honeycomb raytracing shader code. |
| Regina | [example3.cpp](regina/engine/triangulation/example3.cpp), [triangulation.py](regina/examples/triangulation.py) | Construct example three-manifold triangulations; query topology and normal surfaces. | 
| Curved Spaces | [Matrices](<curved-spaces/04 Curved Spaces/Source - common/C code/CurvedSpacesMatrices.c>), [Dirichlet domains](<curved-spaces/04 Curved Spaces/Source - common/C code/CurvedSpacesDirichlet.c>), [simulation](<curved-spaces/04 Curved Spaces/Source - common/C code/CurvedSpacesSimulation.c>) | Original Weeks common C/Metal kernel and shared utilities, with original GPL notices. |
| Stitch Meshing (2018) | [stitchmeshing.cpp](stitch-meshing/src/core/stitchmeshing.cpp), [dual.cpp](stitch-meshing/src/core/dual.cpp) | Original stitch-mesh construction and topological operations; original LGPL notices retained. |

These are selected original source files. Their upstream packages, shader include dependencies, test frameworks, data assets and browser dependencies are not fully copied. The original languages are retained for study; none becomes a selected application implementation or new build dependency merely by appearing here.

All external source execution, host compilation, numerical acceptance, GPU behavior and physical-device behavior remain NOT_RUN. Source retrieval and exact-byte/commit checks were completed on 14 October 2026. Copied upstream tests are examples of upstream checks. The Knot-complement comparison retains its known finite-value/NaN validation gaps.

## Remaining source boundaries

| Source | Current state |
| --- | --- |
| Surface Evolver | Complete historical source mirror is pinned in sources/external; official 2.70 archive was retrieved and hashed. The mirror is not relabeled as that release, and no blanket redistribution grant for the official archive is invented. |
| Existing ricci_flow fork | Complete original fork is pinned in sources/external. Its original notices remain authoritative; no new redistribution license is assigned. |
| Cem Yuksel stitch-mesh work | Actual 2018 Stitch Meshing core source is copied, and the complete repository plus the separate KnittableStitchMesh browser repository are pinned. The distinct 2012 Stitch Meshes system's source release remains unlocated. |
| Jeff Weeks, Curved Spaces | Recovered: 44 original common C/Metal and shared utility/header code files, original GPL/TermsOfUse, and archive provenance are present. |
| Casselman PostScript packages | Author source locations are known; a separate redistribution grant for the selected packages remains unresolved. |
| Earlier unnamed PostScript and D-language engines | Exact project identities remain unresolved. |
| Trettel homogeneous-spaces candidate | Complete original repository is pinned in sources/external; its original ownership and notices are preserved without assigning a new grant. |

The existing catalogue retains the physical constructions, literature, missing source identities and proposed studies. It should be read alongside these code files. No substitute source is labeled as the missing original.
