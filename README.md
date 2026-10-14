# Flower

An interactive, single-color flower. Use raylib as the initial rendering and input host.

The player holds a region of the flower to grow it. Releasing pauses the flower at its exact current shape. Camera flight, looking and rolling remain independent of growth. Paused time must never accumulate into later growth.

Growth means locally increasing preferred material distances, not adding triangles or playing a timed wave animation. The representation may combine retained data, computation and drawing caches.

## Mathematical construction references

The intended mathematical direction follows Daina Taimiņa and the Institute For Figuring's physical constructions and experimental making. See [external geometry examples](docs/external-geometry-examples.md) for crochet, sewn surfaces, geometry engines and concrete studies for Flower.

Actual upstream source files are available in [explicit code examples](examples/external/README.md), with exact revisions, original Git blob identities and licenses.

The [complete upstream source roots](sources/external/README.md) preserve 15 full source repositories at fixed commits. Official Curved Spaces core sources and original Stitch Meshing code are also copied into the example collection.
