# Geometry Center source snapshot

Historical source retained here as an unchanged upstream reference for the fly-through project.

## Geomview

Upstream: https://github.com/geomview/geomview  
Pinned commit: `1d6759ec9771fbc6bdc71ec79b7a5a28711cd9fd`

The selected snapshot contains:

- the complete original `src/bin/flythrough/` module used for the **Not Knot** hyperbolic fly-through;
- all of that module's precomputed camera paths and dodecahedral tessellation data;
- Geomview's `src/lib/gprim/discgrp/` discrete-group / 3-manifold machinery;
- selected figure-eight-knot and Borromean-rings group descriptions;
- the original hyperbolic RenderMan shaders;
- upstream `COPYING`.

## Maniview

Upstream: https://github.com/geomview/maniview  
Pinned commit: `8c771262eec5a351bd491c58105d5c857864d009`

The complete Maniview repository is copied here. Its README describes Maniview as a Geomview module for viewing 3-D manifolds, written by Charlie Gunn at the University of Minnesota Geometry Center.

## Start reading

See [START-HERE.md](START-HERE.md).

## Policy

Do not refactor files below `upstream/geometry-center/` in place. Extract new code outside `upstream/` so the historical source remains available for exact comparison.
