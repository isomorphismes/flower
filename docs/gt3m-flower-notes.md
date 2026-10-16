# GT3M notes for Flower

Source: William P. Thurston, *The Geometry and Topology of Three-Manifolds*,
electronic version 1.1 (March 2002), based on the 1980 Princeton notes.

Official source and chapter downloads:
https://library.slmath.org/nonmsri/gt3m/

This file is a Flower design note, not a copy of Thurston's chapters. It
paraphrases the parts we are using and points back to the source.

## Chapter 1: geometry and topology belong together

Chapter 1, section 1.1 opens with the theme that

> "topology and geometry, in dimensions up through 3, are very intricately related."

For Flower, take that separation seriously without treating it as a wall:

- topology says which local pieces belong together;
- geometry says their current intrinsic sizes, angles and preferred distances;
- an embedding says where the material currently sits in 3-space;
- rendering samples that state into whatever triangles raylib needs.

The triangle mesh is therefore not the identity of the flower. It is derived
drawing data and may be discarded and regenerated.

Direct chapter 1 PDF:
https://library.slmath.org/nonmsri/gt3m/PDF/1.pdf

## Chapter 3: local pieces and gluing data

Chapter 3 starts from a manifold as a topological space locally modelled on
Euclidean space. Thurston then describes additional geometric structure by
specifying which local homeomorphisms are allowed when open pieces are glued
together. The allowed local maps are required to behave coherently under
restriction, composition, inversion and local assembly.

Flower does not need to imitate Thurston's full (G,X)-manifold machinery.
The useful architectural idea is that the global surface can be represented
through local open patches and overlap/gluing information instead of making a
render mesh authoritative.

Direct chapter 3 PDF:
https://library.slmath.org/nonmsri/gt3m/PDF/3.pdf

## Flower's topological net

Start with a finite working cover

    U = { U_0, U_1, ..., U_(n-1) }

of local flower patches.

Store overlap as a relation

    i ~ j  iff  U_i intersects U_j.

This gives a small combinatorial net for traversal. Higher intersections can
also be recorded when needed; pairwise adjacency alone is not always enough
to reconstruct the topology.

For a selected patch i, define hop neighborhoods by the overlap relation:

    H_0(i) = { i }

    H_(k+1)(i) = H_k(i) union
                 { j : j ~ q for some q in H_k(i) }.

This is topological/material distance, not Euclidean distance in the rendered
3-D scene. Two folded petals may be millimetres apart in space while many
material hops apart.

The implementation may lay adjacent patches near one another in memory for
cache locality, but memory adjacency must not define topology.

## Growth while held

A pointer press identifies one or more local patches. While that same pointer
continues to hold the selected material region:

1. find the selected patch set;
2. expand to a bounded hop neighborhood;
3. modify intrinsic growth data in those patches;
4. update the embedding/deformation;
5. regenerate only drawing data that became stale.

On release, cancellation or loss of the owning pointer, steps 3 and 4 stop.
The flower remains at its current state. Camera motion must not advance growth.

"Adding surface area" should initially mean changing intrinsic preferred
geometry, not inserting render triangles. Mesh refinement may happen
independently when more sampling resolution is useful.

## State split

The retained flower state should eventually distinguish at least:

    TOPOLOGY
        patch identities
        overlap / gluing relations
        boundary relationships

    INTRINSIC GEOMETRY
        preferred local metric or equivalent rest-length data
        accumulated growth state

    EMBEDDING / CONFIGURATION
        enough current deformation state to continue deterministically

    RENDER CACHE
        sampled positions
        normals
        triangle indices
        GPU buffers

The first three describe the flower. The last one describes one current way
to draw it.

## First implementation consequence

Do not rewrite the current prototype merely to rename its triangle mesh
"topology." Instead, introduce the topological cover as an authoritative layer
and make the existing spring sheet / raylib mesh an adapter derived from it.

The first useful invariant is:

    changing render resolution must not change patch identity,
    hop distance, selected growth region, or accumulated intrinsic growth.

That gives us a testable boundary between topology and rendering before we
attempt a more sophisticated elastic surface model.


## Developing maps: local data can assemble a global realization

GT3M section 3.5 introduces the developing map for a geometric structure.
Starting with compatible local charts, one analytically continues a chart
through overlaps. On the universal cover this produces a map into the model
geometry. Going around loops may return with a transformation of the model;
that is the holonomy.

Flower is not currently a (G,X)-manifold, and we should not force it into that
formalism. The useful design lesson is more general:

- local patch descriptions should agree on overlaps;
- a realized surface can be assembled from those local descriptions;
- the realized 3-D coordinates are not necessarily the primary data;
- consistency around loops is a global condition which local pairwise
  compatibility alone may fail to reveal.

For Flower this suggests keeping overlap transition data explicit enough that
we can test a loop of patches for accumulated inconsistency instead of hiding
all consistency inside one render mesh.

GT3M reference: section 3.5, "The developing map."

## Chapter 5: deform a structure without changing the underlying topology

Chapter 5 studies deformation spaces of geometric structures. This is close to
the distinction Flower needs: the underlying manifold can stay fixed while its
geometric structure varies.

A particularly useful two-dimensional example is the description of a
hyperbolic surface by cutting it into pairs of pants and recording length and
twist coordinates. Thurston gives Teichmuller space of a closed genus-g
surface coordinates of the form

    (log l_1, tau_1, ..., log l_(3g-3), tau_(3g-3)).

The literal hyperbolic coordinates are not Flower's coordinates. The useful
pattern is:

    fixed topological material
        +
    a finite set of intrinsic geometric parameters
        =
    one point in a space of possible geometries.

Growth then becomes motion in a geometry-state space while patch identity and
topological adjacency remain fixed.

GT3M reference: Chapter 5, especially sections 5.1--5.3.

## Mapping classes, Teichmuller space, and Weil--Petersson geometry

Farb and Margalit's *A Primer on Mapping Class Groups* studies three objects
which should remain conceptually distinct:

1. the surface itself;
2. its mapping class group, i.e. self-homeomorphisms/diffeomorphisms modulo
   isotopy;
3. Teichmuller space, which records marked geometric/conformal structures on
   that fixed topological surface.

Quotienting Teichmuller space by the mapping class group gives moduli space:
geometries which differ only by a change of marking are identified.

The Weil--Petersson metric is a natural metric on Teichmuller space. It
measures variation of marked hyperbolic/Riemann-surface structure and is
mapping-class-group invariant, so it descends to moduli space. Mirzakhani's
work uses the associated Weil--Petersson symplectic geometry and volume.

This is relevant to Flower as architecture, not yet as mechanics.

Flower's current material is not constrained to have constant negative
curvature or even to remain in one conformal class. Therefore the
Weil--Petersson metric should not be substituted for a physical growth or
elastic energy merely because it is a beautiful metric on a space of surface
geometries.

The useful analogy is instead:

    topology / marking          -> which material point or patch is which
    intrinsic geometry state    -> how much material wants to exist locally
    reparametrization           -> a different description of the same state
    embedding in R^3            -> how that intrinsic state is currently folded
    rendering                   -> a sampled picture of the embedding.

If we later create a reduced space of admissible flower geometries, it may be
worth asking whether that space has a natural metric analogous in spirit to
Weil--Petersson. That should be derived from Flower's own deformation energy,
not imported from hyperbolic geometry without a reason.

For a simple petal modelled topologically as a disk with its boundary fixed,
the mapping class group itself carries little information. Mapping-class-group
machinery becomes more interesting if the material surface acquires marked
points, punctures, holes, nontrivial handles, or meaningful ways of permuting
distinguished regions.

Reference:
Benson Farb and Dan Margalit, *A Primer on Mapping Class Groups*, especially
Parts II and III for Teichmuller/moduli space and the Nielsen--Thurston view.

## Chapter 8: laminations and bending are closer to the flower than expected

GT3M section 8.6 treats a lamination as locally a product of a leaf direction
with a transverse local space, then equips the transverse direction with a
measure. For boundaries of hyperbolic 3-manifolds Thurston uses a transverse
bending measure: crossing the lamination accumulates turning angle.

Flower does not need a geodesic lamination immediately. But this gives a
useful representation for a surface whose deformation becomes concentrated
along families of folds:

    smooth material region
        +
    a set or lamination of preferred fold lines
        +
    a measure recording accumulated bend across them.

That is substantially richer than assigning an arbitrary height to every
vertex. It also permits a mixed representation in which large regions are
computed smoothly and sharp or repeated ruffles carry concentrated bending
data.

GT3M reference: section 8.6, "Measuring laminations."

## GT3M section 8.8: crumpled versus wrinkled surfaces

This section is unusually close to the visual problem in Flower. Thurston
draws a qualitative distinction between a crumpled sheet and a sheet that is
only wrinkled or crinkled. The surrounding discussion studies surfaces made
from locally flat pieces and limiting constructions.

For Flower the immediate lesson is that "more area" should not automatically
mean arbitrary high-frequency crumpling. We want controlled ruffling whose
local organization remains legible.

That suggests eventually recording a regularity constraint on deformation,
for example:

- bound how rapidly fold direction changes over topological hop distance;
- distinguish distributed curvature from concentrated fold/bending data;
- refine the render mesh where curvature requires it rather than where growth
  happens merely because growth happened;
- detect pathological accumulation of folds separately from ordinary ruffle
  refinement.

This can become a mechanical invariant before we know the final continuum
model.

GT3M reference: section 8.8, "Uncrumpled surfaces."

## Revised hierarchy for Flower

The working hierarchy is now:

    TOPOLOGICAL MATERIAL
        open patches
        overlaps / higher intersections
        marking and boundary data
        hop distance

    INTRINSIC GEOMETRY
        local preferred metric / area
        growth history
        perhaps reduced deformation coordinates later

    BENDING / REGULARITY
        distributed curvature
        optionally concentrated fold or lamination data

    EMBEDDING
        current realization in R^3
        enough state to continue deterministically

    RENDER ADAPTER
        tessellation
        normals
        GPU buffers
        raylib camera/draw calls

Only the last layer should care that raylib wants triangles.


## Chapter 4: local pieces need loop consistency, not just pairwise agreement

Section 4.2 is a useful warning against a too-naive open-set implementation.
Thurston glues ideal tetrahedra and then imposes consistency around each edge.
It is not enough for every neighboring pair of pieces to fit. Going all the
way around an edge must return with the correct total angle and no residual
holonomy.

Flower's exact equations will be different, but the architectural point is
directly applicable:

    pairwise-compatible overlaps
        do not imply
    globally-compatible geometry.

For a cycle of patches

    U_0 -> U_1 -> ... -> U_k -> U_0

the composed transition around the loop should be checked. If the intended
material atlas has no defect there, the accumulated transition should return
to the starting material frame. If it does not, the residual should be
represented deliberately as curvature, twist, defect or inconsistency rather
than silently absorbed into the render mesh.

This suggests two separate tests:

1. topological overlap consistency: the incidence data describes the intended
   surface;
2. geometric loop consistency: composing local transition data around a
   contractible material loop gives the allowed residual.

GT3M reference: section 4.2, "Gluing consistency conditions."

## Chapter 6: a discrete realization can be changed without changing the thing represented

In section 6.1 Thurston replaces an arbitrary singular simplex by a canonical
straight simplex with the same vertices. The straightening operation acts on
the chain representation while preserving the homological information needed
for the argument.

Flower should not literally use hyperbolic straightening, but this gives a
useful representation principle:

    authoritative state != one particular discretization.

A render triangulation may be subdivided, collapsed, retriangulated or
canonicalized while preserving the same material/topological state. Such an
operation should have an explicit invariant it promises to preserve.

For Flower, candidates include:

- patch identity and overlap;
- selected material region;
- intrinsic area/growth assigned to each patch;
- boundary marking;
- accumulated fold measure;
- embedding within a declared approximation tolerance.

"Same vertices" is not the important part for us. The important part is that a
discrete carrier can be replaced by a better-behaved carrier through a
controlled map while the represented invariant is held fixed.

GT3M reference: Chapter 6, section 6.1, straightening singular simplices.

## Section 8.9: train tracks as finite carriers for a complicated continuum

Train tracks are especially relevant to Flower.

Thurston starts with many nearly parallel leaves of a lamination inside thin,
branching corridors and collapses each corridor to a branching graph. The
graph carries the possible paths of the leaves. He describes train tracks as
"analogous to decimal approximations of real numbers."

This is not the same object as Flower's open cover:

    open-cover / nerve net
        says what material neighborhoods overlap;

    train-track-like carrier
        says how a directional family of folds, veins or flow lines travels
        through that material.

The two layers can coexist.

For example, a petal can have an authoritative patch cover U_i. A fold field
inside those patches may later be compressed to a much smaller branching
carrier tau. Refining the fold pattern need not refine the whole material
topology.

A useful eventual representation is:

    MATERIAL NET
        patch graph / nerve

    FOLD CARRIER
        branches and switches embedded in material coordinates

    BRANCH DATA
        bend amount
        direction
        scale / wavelength
        optional transverse width or measure.

This is closer to "half computation and half storage" than a dense array of
per-vertex wrinkle values.

GT3M reference: section 8.9, "The structure of geodesic laminations: train
tracks."

## Measured train tracks: conservation at a switch

GT3M goes further. If a lamination carries a transverse measure, a number
mu(b) is assigned to each branch b of a carrying train track. At every switch,
the total entering measure equals the total exiting measure.

Schematically,

    sum entering mu(b) = sum exiting mu(b).

Conversely, suitable nonnegative branch values satisfying these switch
conditions determine a measured lamination carried by the track.

Flower should not impose this conservation law on arbitrary growth. But it is
a very attractive model for quantities which really should be conserved while
branching, for example:

- a vein-like transported flow;
- a conserved "fold flux";
- material transport through a branching procedural brush;
- a resolution-independent directional density.

This gives a way to store a continuum-like family with a finite graph plus
numbers, rather than one object per visible strand.

GT3M reference: section 8.9, around the measured train-track discussion.

## Section 11.1: separate infinitesimal strain from infinitesimal rotation

Chapter 11 contains a decomposition that is conceptually useful even though
its setting is hyperbolic 3-space. For a deformation vector field X, Thurston
splits its covariant derivative into symmetric and antisymmetric parts:

    nabla X = symmetric part + antisymmetric part.

The antisymmetric part describes infinitesimal rotation. The symmetric part
measures infinitesimal strain, i.e. distortion of the metric.

This is exactly the distinction Flower should preserve at the implementation
level:

    rigid/local rotation
        should not count as growth;

    change of intrinsic preferred metric
        is growth;

    elastic strain of the current embedding
        is the mismatch between current geometry and preferred geometry.

A petal can rotate dramatically in 3-space while its intrinsic growth remains
zero. Conversely, intrinsic growth can increase while the visible position
changes only slightly for a time.

That suggests diagnostics which report these separately rather than reducing
everything to vertex displacement:

    intrinsic growth increment
    elastic strain
    bending
    rigid/local rotation
    render displacement.

GT3M reference: Chapter 11, section 11.1, decomposition of the covariant
derivative of a deformation field.

## Section 13.7: combinatorics plus local continuous data can determine geometry

Thurston's circle-pattern construction is another strong model for Flower.
Start with a cell division of a surface and prescribed intersection-angle data.
Associate a radius to each vertex. These finite data determine local triangles
of centers and hence a piecewise metric; the radii can then be adjusted so
that the curvature conditions at vertices are satisfied.

The specific circle-pattern theorem is not our flower model. The useful design
pattern is:

    finite combinatorics
        +
    a small amount of local continuous data
        +
    compatibility equations
        ->
    global metric geometry.

This argues against storing the flower only as millions of independent point
positions. A patch/cell net plus local growth variables may determine far more
of the geometry than a naive mesh representation suggests.

It also gives a concrete implementation idea for later experiments: choose a
cellulation of material space, attach one or a few intrinsic scale variables
to each cell or vertex, derive edge lengths from neighboring variables, and
solve compatibility/curvature conditions. This would be a discrete intrinsic
geometry generator rather than merely a render mesh deformation.

Thurston also remarks that his proof gives a practical iterative algorithm:
changing one radius affects the curvature most strongly at its own vertex, so
local radius adjustments converge reasonably well. The analogous Flower
question is whether local intrinsic-growth variables can be relaxed using
mostly local curvature/strain residuals.

GT3M reference: section 13.7, "Constructing patterns of circles."

## A useful three-net model

GT3M now suggests that Flower may eventually want three related but distinct
combinatorial objects:

    1. MATERIAL COVER / NERVE
       what is topologically near what;
       used for touch selection and hop distance.

    2. GEOMETRY CELLULATION
       finite carrier for intrinsic metric/growth variables and compatibility
       equations.

    3. TRAIN-TRACK-LIKE FOLD CARRIER
       finite carrier for organized directional bending or ruffling.

None of these is the raylib triangle mesh.

They may initially coincide for simplicity. The architecture should not assume
they must coincide forever.

A fourth object is then purely derived:

    4. RENDER TESSELLATION
       whatever density of triangles is currently useful to display the
       embedding.

This is probably a better long-term interpretation of "the underlying net
should be topological" than trying to make one graph do topology, mechanics,
fold organization and rasterization simultaneously.
