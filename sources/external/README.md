# Complete upstream source roots

These 15 Git submodules bind the complete original repositories to immutable commits. GitHub's directory entries open the pinned upstream source trees. They retain the original owners and notices; no new redistribution license is assigned. No module is wired into an application build or selected as a runtime dependency.

| Source root | Pinned original repository |
| --- | --- |
| crochet-simulator | [timhutton/crochet-simulator at b6e1ba1341f9](https://github.com/timhutton/crochet-simulator/tree/b6e1ba1341f9258c7581ae1ebf02d5c56ff22c5b) |
| rogueviz | [zenorogue/hyperrogue at c7c9a2993b43](https://github.com/zenorogue/hyperrogue/tree/c7c9a2993b43ee2f4bbac422e565d18fdd98f886) |
| snappy | [3-manifolds/SnapPy at c3da53c2b310](https://github.com/3-manifolds/SnapPy/tree/c3da53c2b3105489b4d71531c4f781c92b237753) |
| thurston-geometries | [henryseg/non-euclidean_VR at 5e382dc7bc1e](https://github.com/henryseg/non-euclidean_VR/tree/5e382dc7bc1e785ba225761d10f6a9f480e48556) |
| geometry-central | [nmwsharp/geometry-central at 019669ddabda](https://github.com/nmwsharp/geometry-central/tree/019669ddabda05e0f71fa3587cfb3c1dadf19cb8) |
| flipper | [MarkCBell/flipper at 318ae9133b91](https://github.com/MarkCBell/flipper/tree/318ae9133b91628fed6f6ebb674aa61e3ee6d7a7) |
| hyperbolic-honeycombs | [isomorphismes/Hyperbolic-Honeycombs at f2df32b97961](https://github.com/isomorphismes/Hyperbolic-Honeycombs/tree/f2df32b979613dda51584c95ea73e4e51e458257) |
| knot-complement | [isomorphismes/knot-complement at d0918cc2cf58](https://github.com/isomorphismes/knot-complement/tree/d0918cc2cf58fa0d01dac73cffab95686b2aab1e) |
| circlepack | [kensmath/CirclePack at 83dad4412252](https://github.com/kensmath/CirclePack/tree/83dad441225201785ca51607842f3ed8b2a8f129) |
| regina | [regina-normal/regina at e3a5c7c0e058](https://github.com/regina-normal/regina/tree/e3a5c7c0e0581706d9e5531b750160b322489950) |
| surface-evolver | [kashif/evolver at 311714617a63](https://github.com/kashif/evolver/tree/311714617a630afabf72411bff184f3c42b4aa58) |
| ricci-flow | [isomorphisms/ricci_flow at 8b875a7c4e32](https://github.com/isomorphisms/ricci_flow/tree/8b875a7c4e3230802f81988e3f92c221aceb5141) |
| homogeneous-spaces | [stevejtrettel/homogeneous-spaces at 3551ceb2db79](https://github.com/stevejtrettel/homogeneous-spaces/tree/3551ceb2db79a932db4c40602ba78650f1e59125) |
| stitch-meshing | [kuiwuchn/stitchMeshing at ec792353eaa4](https://github.com/kuiwuchn/stitchMeshing/tree/ec792353eaa45f15ec9453aad62576cca5f7b3ec) |
| knittable-stitch-mesh | [kuiwuchn/KnittableStitchMesh at bab21f94313d](https://github.com/kuiwuchn/KnittableStitchMesh/tree/bab21f94313d361d6195e3e08dc09a3a7e73e519) |

See pins.tsv and the root .gitmodules for the exact checkout identities. Existing copied examples remain in [examples/external](../../examples/external/README.md). The complete official Curved Spaces archive is separately identified by digest there; its common geometry sources and shared utilities are copied unchanged.

The Surface Evolver module points to a historical mirror at 311714617a630afabf72411bff184f3c42b4aa58. The author’s current official evolver-2.70.tar.gz was also retrieved, with SHA256 be7d60d9eb690f1fb124e2ecc12237a7a35be1935bfa5e92be3fe912207a3b6b. This module is not relabeled as that official release. No blanket redistribution license for the official archive is inferred from “free of charge”.

The existing ricci_flow and homogeneous-spaces repositories have no redistribution notice established by this collection. Their original source roots are pinned without vendoring their code or inventing a grant.

Stitch Meshing is the LGPL SIGGRAPH 2018 implementation. KnittableStitchMesh supplies a separate browser source/visualization repository. Neither is represented as the recovered source of the distinct 2012 Stitch Meshes system. Nested modules, external libraries, solver dependencies and runtime behavior still need separate preparation and acceptance.

Verification: all 15 commit objects were resolved in their original repositories. The five new source repositories were also fetched locally and their pinned commit objects checked. Copied-file identities are checked separately. No engine execution, compilation, GPU, device or independent isomorphisms/ai-ci acceptance is claimed.
