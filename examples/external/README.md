# Explicit external geometry code

33 original code files from 10 source sets are stored here, with the upstream paths retained inside each directory. Files are unchanged; sources.tsv records repository, exact revision, original path, Git blob identity and size. PROVENANCE.md and upstream license files identify the authors and redistribution terms.

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

These are selected original source files. Their upstream packages, shader include dependencies, test frameworks, data assets and browser dependencies are not fully copied. The original languages are retained for study; none becomes a selected application implementation or new build dependency merely by appearing here.

All external source execution, host compilation, numerical acceptance, GPU behavior and physical-device behavior remain NOT_RUN. The execution environment was unavailable during this source-copy update. Copied upstream tests are examples of upstream checks, not reports that those checks ran here. The Knot-complement comparison also retains its known finite-value/NaN validation gaps.

## Sources still to recover

| Source | Exact remaining gap |
| --- | --- |
| Surface Evolver | The mirror exposes cube.fe and cat.fe, but a redistribution notice for that pinned distribution was not established. No assumed license is assigned. |
| Existing ricci_flow fork | Its source and original upstream were located, but no redistribution license was found in either inspected tree. |
| Cem Yuksel stitch-mesh work | The author page supplies papers and examples; a matching redistributable source release was not located. |
| Jeff Weeks, Curved Spaces | The author supplies a source archive; its archive bytes were not retrieved in the unavailable execution environment. |
| Casselman PostScript packages | The author's manual restricts redistribution; no separate redistribution grant for the selected packages was established. |
| Earlier unnamed PostScript and D-language engines | Exact project identities remain unresolved. |
| Trettel homogeneous-spaces candidate | The repository was found, but a source redistribution grant was not established. |

The existing catalogue retains the physical constructions, literature, missing source identities and proposed studies. It should be read alongside these code files. No substitute source is labeled as the missing original.
