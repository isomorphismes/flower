# External geometry examples for Flower

Flower's intended direction follows Daina Taimiņa and the Institute For
Figuring: physical mathematical construction, experimental making, manipulable
surfaces and rich mathematical/artistic exhibitions. The current spring-sheet
prototype remains far short of that standard.

A Flower or Coral surface is realized in ordinary three-dimensional space.
Its intrinsic material distances may describe negative curvature; the surrounding
space, camera and viewing rays remain Euclidean. A Poincaré-disk picture does
not replace this construction.

## Examples to carry into Flower

| Priority | Proposed example | What must remain authoritative |
| --- | --- | --- |
| First | Hutton's cylinder versus an increasing Taimiņa-style crochet patch | Stitch order, parent attachments, increase counts and gauge; distinguish new stitches from changing preferred lengths on the existing fixed net. |
| First | Ferguson/Weeks sewn pentagonal blanket beside crochet and cardboard controls | Panel identities and curved seams; which geometry is prescribed and how material stretch changes the realization. |
| First | IFF-style branching tube, folded sheet or lace construction | The actual construction and material choices, rather than a decorative preset labeled “coral.” |
| Next | Yuksel stitch mesh → yarn curves → relaxation | Keep stitches, yarn and display triangles distinct. Preserve identities during picking and viewing. |
| Next | CirclePack / discrete Ricci flow metric → independently measured realization | Reference edge lengths, discrete curvature convention, boundary choices and stretch/degeneracy reports. |
| Next | Surface Evolver / non-Euclidean plate comparison | Declared energy, constraints, thickness and convergence; no claim of calibrated yarn mechanics from a generic minimizer. |
| Comparison | RogueViz rug view beside an intrinsic geodesic view | Coordinate picture, intrinsic metric and realized surface stay distinct; close folds in space do not become material neighbors. |

Continue the existing
[SUN FLOWER-STITCH-01 assignment](https://github.com/isomorphisms/flexible-pipes/issues/121)
rather than duplicating it. Relevant retained implementation:
[Net → Skin → Mesh](https://github.com/isomorphismes/flower/blob/aa74149c17e1154c54b8db7d7ef3ef441614f37c/docs/net-model.md).
That fixed annulus does not yet implement stitch insertion or calibrated
self-contact/material behavior.

## Explicit code

The repository now contains 80 original source files from twelve source sets.
Start with [the code index](../examples/external/README.md) and
[the exact source manifest](../examples/external/sources.tsv),
[the official archive manifest](../examples/external/archive-sources.tsv), and
[15 pinned complete source repositories](../sources/external/README.md).
These include explicit crochet relaxation, hyperbolic geodesic flow, Lorentz
camera transformations, circle packing, intrinsic triangulations, laminations
and example three-manifold constructions. Their execution remains unverified.

## External sources and examples

These are source references and study examples, checked on 14 October 2026.
The “local study” column proposes work; it does not report an implemented
demonstration, an executed external program, or a physical model. Links preserve
the authors' attribution. Selected original source files, licenses and exact provenance now live in
[explicit code examples](../examples/external/README.md). Photographs and book
chapters are not copied.

### Physical construction and realization

| Reference | External example | Local study |
| --- | --- | --- |
| [Henderson and Daina Taimiņa, Crocheting the Hyperbolic Plane](https://pi.math.cornell.edu/~dtaimina/crochet/hplane.htm) | Crochet with repeated increases; paper annuli and polyhedral constructions provide comparisons. | Record stitch parents, increases, row spacing and gauge; compare marked intrinsic lengths with a finite realization in ordinary three-dimensional space. |
| [Institute For Figuring, hyperbolic crochet interview](https://www.theiff.org/publications/cab16-crochet.html) and [Crochet Coral Reef](https://www.theiff.org/exhibits/reef.html) | Manipulable crochet models and varied coral-like forms. | Keep construction, material choices and exhibition practice visible. Branching, tubes, folds and lace require explicit constructions; appearance alone does not establish constant curvature. |
| [Jeff Weeks, How to Sew a Hyperbolic Blanket](https://www.geometrygames.org/HyperbolicBlanket/) | Helaman Ferguson's fleece design, explained by Weeks: 51 curved-edge pentagons, with four meeting at a vertex. | Compare sewn seams with crochet increases and a straight-sided cardboard control. Distinguish curvature concentrated at vertices or seams from a material that stretches across its interior. |
| [Tim Hutton, Crochet Simulator source](https://github.com/timhutton/crochet-simulator) and [live demo](https://timhutton.github.io/crochet-simulator/) | Cylinder, flat spiral disc, granny square and a negative-curvature surface. | Trace one pattern through stitch connectivity and numerical relaxation. Use a no-increase cylinder as a control and compare against an increasing pattern. |
| [Cem Yuksel and collaborators, Stitch Meshes](https://www.cemyuksel.com/research/stitchmeshes/) | Stitch layouts, yarn curves and yarn-level relaxation; the author page also collects later knittable and machine-knitting work. | Compare surface samples, stitch identities and yarn geometry as different objects. Knitting is a related construction, not automatically the same connectivity as crochet. |
| [Ken Brakke, Surface Evolver](https://www.kenbrakke.com/evolver/evolver.html) and [example collection](https://www.kenbrakke.com/evolver/examples/default.htm) | Constrained surfaces and energy minimization; catenoid and volume-constrained examples. | Inspect how geometry, energy, constraints, refinement and convergence are specified separately. A minimized surface energy is not by itself a calibrated yarn or crystal growth law. |
| [Efrati, Sharon and Kupferman, Elastic theory of unconstrained non-Euclidean plates](https://arxiv.org/abs/0810.2411) | A reference geometry can compete with stretching and bending in a realized plate. | State reference metric, thickness, boundary conditions and energy independently. Test the declared material model rather than inferring it from a ruffled picture. |

### Intrinsic spaces, quotients and viewing

| Reference | External example | Local study |
| --- | --- | --- |
| [HyperRogue / RogueViz](https://www.roguetemple.com/z/hyper/rogueviz.php) and [source](https://github.com/zenorogue/hyperrogue) | Hyperbolic tessellations, quotient spaces, geodesic inner views and the simulated “hypersian rug” model. | Compare an intrinsic navigable world with an extrinsic rug view while retaining the same marked mathematical data. Read algorithms and interactions without adopting its C++ implementation as a dependency. |
| [Geometry Center archive in Knot-complement](https://github.com/isomorphismes/knot-complement/tree/d0918cc2cf58fa0d01dac73cffab95686b2aab1e/upstream/geometry-center) | Not Knot, Flythrough, Maniview, Geomview discrete-group/Dirichlet-domain code, historical figure-eight and Borromean data. | Follow camera recentering and group transformations; distinguish precomputed path playback from ray traversal. Keep historical scene identities and coordinate conventions explicit. |
| [Jeff Weeks, Curved Spaces](https://www.geometrygames.org/CurvedSpaces/) | Flight through multiply connected spaces with repeated images of objects. | Compare fundamental-domain seams, full camera frames and repeated objects. Ordinary Euclidean crystal repetition and hyperbolic quotient repetition have different geometry. |
| [SnapPy / SnapPea source](https://github.com/3-manifolds/SnapPy) and [pinned raytracing source](https://github.com/3-manifolds/SnapPy/tree/c3da53c2b3105489b4d71531c4f781c92b237753/src/snappy/raytracing) | Hyperbolic three-manifold data and ideal-tetrahedron raytracing, including the figure-eight complement m004. | Inspect shapes, face pairings, cusp data and tangent/frame transformations. Shape-equation agreement does not alone implement navigation or prove a physical realization. |
| [Coulon, Matsumoto, Segerman and Trettel, Ray-marching Thurston geometries](https://arxiv.org/abs/2010.15801) and [implementation](https://github.com/henryseg/non-euclidean_VR) | Geometry-dependent geodesic rendering in Thurston geometries. | Compare ray propagation, distance bounds, intersections and lighting; bending a ray in an intrinsic world is a different operation from viewing a folded Euclidean sheet. |

### Discrete geometry and mathematical illustration

| Reference | External example | Local study |
| --- | --- | --- |
| [Ken Stephenson, CirclePack](https://www.circlepack.com/software.html) and [author's circle-packing site](https://www.circlepack.com/) | Circle packings and discrete conformal geometry. | Compare adjacency, circle radii, discrete metric data and a conformal picture. A packing drawing does not determine a physical sheet or a material growth law. |
| [Bill Casselman, Mathematical Illustrations](https://personal.math.ubc.ca/~cass/graphics/manual/) | PostScript construction procedures, nonlinear transformations, three-dimensional surfaces and regular polyhedra. | Make a construction authoritative and derive its views from it. Keep camera/projection changes separate from a change in the object. |
| George Francis, A Topological Picturebook; [related preserved Geometry Center material](https://github.com/isomorphismes/knot-complement/tree/d0918cc2cf58fa0d01dac73cffab95686b2aab1e/upstream/geometry-center) | The mathematical illustration tradition discussed alongside Hatcher and Casselman. | Study a construction and its transformation before designing an animation vocabulary. The archive link is related material, not a digital copy of Francis's book. |
| [Regina](https://regina-normal.github.io/) | Low-dimensional topology and triangulations. | Use topology/combinatorial checks where appropriate; do not treat them as a reference metric, shell mechanics or an automatic embedding solver. |
| [Hatcher interactive-illustration work](https://github.com/isomorphisms/hatcher/tree/008533e1226bdabb365dd9df98a2357d0d145b05/movie-language/TYPED-PS-MOVIES-01) | The local typed PostScript movie design separates mathematical constructions, realizations, dependent display and checked motion. | Let one accepted construction drive linked views and slider motion. This is adjacent design work, not an external engine or an implemented dependency. |

## Retained categories whose exact earlier source identity was not recovered

The earlier engine comparison also named the seven categories below without
recoverable project URLs. Every category remains here. A verified representative
is a candidate added during this collection, not a claim that its identity matches
the earlier unnamed reference.

| Earlier category | Candidate or related source | Concrete study and unresolved boundary |
| --- | --- | --- |
| Spherical/hyperbolic three-dimensional PostScript engine | Casselman's manual above is a related illustration reference. | Describe the same construction through spherical and hyperbolic models. The particular previously mentioned engine remains UNRESOLVED; Casselman's Euclidean 3D package is not asserted to be it. |
| Intrinsic triangulations / discrete differential geometry | [Geometry Central source](https://github.com/nmwsharp/geometry-central), [intrinsic-triangulation documentation](https://geometry-central.net/surface/intrinsic_triangulations/basics/) — candidate representative. | Compare intrinsic edge lengths and connectivity with an embedded display mesh. Track marked material identity through any flip/refinement. The earlier unnamed project identity remains UNRESOLVED. |
| Discrete Ricci flow | [Existing ricci_flow fork](https://github.com/isomorphisms/ricci_flow), [parent](https://github.com/shizuo-kaji/ricci_flow), [original upstream](https://github.com/hchapman/ricci-flow) — verified existing related work. | Follow prescribed discrete curvature → edge lengths → a separate three-dimensional realization. Check boundary curvature and global compatibility; do not confuse integrated angle defect with curvature per unit area. The old unnamed item's identity remains UNRESOLVED. |
| Hyperbolic geodesic/isometry reference | [Steve Trettel, homogeneous-spaces](https://stevejtrettel.site/software/homogeneous-spaces/) — candidate representative. | Compare points, isometries, geodesics and coordinate-model changes; use the closed-form constant-curvature path as a numerical check. This is not asserted to be the earlier unnamed project. |
| Mapping-class / lamination tool | [Mark Bell, flipper](https://github.com/MarkCBell/flipper), [documentation](https://flipper.readthedocs.io/en/latest/) — candidate representative. | Follow a marked curve or lamination through a surface transformation. Keep topology and intrinsic geometry distinct from spatial coordinates. The earlier tool identity remains UNRESOLVED. |
| Coxeter groups / tilings / honeycombs | [Existing Coxeter work](https://github.com/walnut-burgundy/coxeter), [existing Hyperbolic-Honeycombs fork](https://github.com/isomorphismes/Hyperbolic-Honeycombs), [upstream](https://github.com/neozhaoliang/Hyperbolic-Honeycombs). | Compare reflection-generated face families and hyperbolic honeycomb repetition. A reflection group alone does not establish a crystal's point/space group or a physical material law. The exact earlier unnamed source remains UNRESOLVED. |
| D-language symbolic surface engine | No exact source recovered. | Retain the proposed example: a declared parametric/implicit surface, its derivatives and normals, and dependent rendering. Resolve project name, implementation language and license before copying or evaluating it. Do not silently substitute a Python/C++ engine. |

## Evidence and adoption

This catalogue records names, source links and proposed study examples. External
program execution, numerical comparisons, mobile behavior and handmade models
are NOT_RUN by this source-collection change. Where an author page could not be
fetched, a retained author link is a lead rather than a verified live download.
In particular, CirclePack's author pages were returned by search but direct
fetch timed out.

Before adapting source, bind the exact revision, inspect its license and identify
the mathematical operation being transferred. New runtime dependencies and a
shared renderer require their own concrete need and acceptance. A picture,
source catalogue, package build or topological check cannot supply missing
mathematical or physical evidence.
