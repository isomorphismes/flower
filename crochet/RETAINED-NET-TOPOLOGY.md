# Regina checks the actual retained Net and drawing caches

The accepted counted crochet ledger is different from the retained fixed
16×64 FlowerNet. This host-only test consumes the real retained implementation
at f404096058cf4b6ac35b59d6b67cb72587b297d8. It does not copy that Net into
the crochet program or promote the retained application to main.

The adapter initializes the actual Flower and invokes its real
flower_mesh_build at drawing densities one and two. It checks every drawing
triangle's parent and barycentric subdivision, using material-node identities
and unordered endpoint pairs for midpoint identities. It never identifies
vertices by spatial position. A fully coincident folded cache therefore keeps
its original material topology.

The unindexed drawing buffers contain repeated corners. The exported quotient
has 1,088 vertices/2,048 faces at density one and 4,224 vertices/8,192 faces at
density two. The extra vertices are drawing-edge midpoints, not new crochet
stitches or material neighbors. Every quadrilateral side must be a reciprocal
Net adjacency; the actual 1–3 drawing diagonal must not be one. Native controls
deliberately add that diagonal and remove a boundary edge, and require refusal.

The separately pinned ai-ci topology.pi constructs a real Regina
Triangulation2 from each exported incidence and independently checks one
component, orientability, two boundary components and Euler characteristic 0.
It also executes disk, torus, Mobius, seam-reversal, boundary, pinch and
nonmanifold controls. All four normal/folded/density combinations must pass.
The foreign host package is pinned to Regina 7.4.1 and checked by the oracle.
No Regina C++ or Python runtime is added to the Android application.

This proves the declared bounded topological checks when executed successfully.
It says nothing about Gaussian curvature, yarn, physical folding, GPU rendering
or phone acceptance. The normal/folded OBJ files are diagnostic exports, not
certified geometrically embedded surfaces; the deliberately collapsed fold
exists to prove that spatial coincidence cannot create material neighbors.
The Idriç attempt and its actual acquisition failure remain alongside export.c.
