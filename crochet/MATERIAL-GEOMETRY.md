# Explicit material geometry and a finite realization benchmark

Canonical work records are [Flexible Pipes issue #123, SUN FLOWER-METRIC-01](https://github.com/isomorphisms/flexible-pipes/issues/123),
[issue #135, SUN FLOWER-REALIZATION-01](https://github.com/isomorphisms/flexible-pipes/issues/135),
and [issue #136, SUN FLOWER-HANDMADE-01](https://github.com/isomorphisms/flexible-pipes/issues/136).
This module consumes the counted construction from issue #121. It does not
change the retained Flower application's input, hold/release state or Lua gate.

The implemented slice supplies checked reference metrics, reference edge
lengths, explicit R3 placements, singular-value stretch measurements, an
angle-defect estimator with boundary turning, mesh refinement comparisons and
material-ID exports. The increasing patch uses an explicitly **synthetic**
gauge. No yarn specimen, calibrated material law, physical equilibrium solver,
GPU execution or phone session is implied by these calculations.

## Four separate metric fixtures

All use the diagonal local metric `du² + R(u)² dtheta²`, with theta periodic.
Curvature magnitude `kappa` is positive and Gaussian curvature is `-kappa`.

| Family | Radius function | Meaning |
| --- | --- | --- |
| Flat polar | `R(u)=u` | Flat disk if the first row is zero; otherwise flat annulus |
| Hyperbolic polar | `R(u)=sinh(sqrt(kappa)*u)/sqrt(kappa)` | Intrinsic distance circles about a center |
| Exponential annulus | `R(u)=R0*exp(sqrt(kappa)*u)` | Horocyclic band in a parabolic hyperbolic quotient |
| Cylinder | `R(u)=R0` | Flat intrinsic annulus with a cylindrical R3 realization |

The sinh fixture is required by the existing metric job. The exponential
annulus is an additional construction benchmark, and does not replace it.
The `kappa=4` fixture catches missing square roots and radius scale factors.
The positive `radius_scale` field is used only by the exponential/cylinder
families; polar radius is fixed by the definition shown above.

`flower_reference_edge` returns a local developed geodesic length. For polar
H2 it uses the stable half-distance sinh/asinh identity; for the exponential
band it develops to `x=sqrt(kappa)*R0*theta`, `z=exp(-sqrt(kappa)*u)` in scaled
upper-half-space H2. The caller supplies unwrapped adjacent angles. This is
neither graph-hop distance nor a global shortest-path solver constrained to
the finite band. In particular a local developed geodesic between boundary
points may leave the finite band. This distinction is retained in exports.

## The extra information supplied by the synthetic gauge

The accepted input pattern carries the counts `8,12,18,27` and exact parent,
child, cyclic-neighbor and seam IDs. A separate supplied circumference width
`a=pi/16` and row height `h=log(3/2)` choose

`R0=8a/(2pi)`, `k=log(3/2)/h`, `u_r=rh`, `R(u)=R0 exp(ku)`.

Thus `2pi R(u_r)=N_r a`. This is our exponential interpolation choice between
four measured-or-chosen rows, not a consequence of the combinatorial ledger.
The current API accepts synthetic calibration only. It cannot set a flag
that promotes the chosen dimensions to measured yarn data.

Scaling both gauge lengths by two scales the realization by two and divides
curvature magnitude by four. It leaves every material ID, count and attachment
unchanged. The no-increase pattern uses the same gauge and a cylinder. A
constant stitch count alone does not prove the real sample is intrinsically
flat; flatness belongs to this chosen control metric.

## Finite R3 realization and its precise limits

For the exponential band set `R=R0 exp(ku)` and

`X(u,theta)=(R cos(theta), R sin(theta), [F(kR)-F(kR_first)]/k)`,

where `F(t)=sqrt(1-t²)+log(t/(1+sqrt(1-t²)))`.
Then `R'=kR` and `Z'=sqrt(1-(kR)²)`, giving the supplied metric exactly in the
smooth calculation. The fixture has `k=1` and radii `1/4,3/8,9/16,27/32`.
It is finite, has two boundaries and is an annulus. It is not a geodesic disk
or a complete hyperbolic plane. The next idealized row would exceed `kR=1`;
the API refuses that rotational ansatz instead of producing an imaginary Z.

Positive R and strictly increasing Z prove that this **specific** smooth
revolution has no self-intersection. The exported equal-angle polygonal rings
also have strictly separated Z intervals, with a convex polygon frustum in
each interval, so this specific chord mesh has no self-intersection. There is
no general triangle collision detector, finite-thickness contact law, arbitrary
fold acceptance, yarn topology model or contact certificate for edited meshes.

The numerical model has zero thickness, no tension, no external loading, and
no stretching/bending energy minimization. Its shape is supplied by the chosen
analytic ansatz. It is a geometric benchmark for realization work, not completion
of the requested mechanical realization job. A mechanical model still needs
an explicit constitutive choice, reference curvature, thickness/contact policy,
boundary conditions, equilibrium residual and independent failure tests. Those
are separate from reconstructing one exact isometry of the supplied metric.

For the hyperbolic **polar** fixtures, the code deliberately places points at
`(u cos(theta),u sin(theta),0)`. That flat placement is distorted. Its nonunit
stretch and reference/realized edge discrepancy must remain visible. There is
no false attempt to use the revolution formula with `R'=cosh(ku)>1`.

All resulting objects are viewed with ordinary Euclidean viewing rays. The
metric belongs to the material surface, not to the space containing it.

## Triangle and curvature measurements

Each display triangle gets three independently computed reference metric
edge lengths, which define a Euclidean reference triangle. This is a
piecewise-flat approximation; it is not the exact smooth metric inside a face.
The realized straight triangle and reference triangle define a 3-by-2 linear
map. Its two singular values are the local stretches. The implementation
adapts the checked calculation in
[Mostow, icky/mostow_sheet.c at 9c4bfd742f774c9b309df3acce049fc1aa930bf3](https://github.com/isomorphismes/mostow/blob/9c4bfd742f774c9b309df3acce049fc1aa930bf3/icky/mostow_sheet.c).
It reports minimum/maximum stretch, two-sided factor `max(sigma_max,1/sigma_min)`
and anisotropy `sigma_max/sigma_min`; these last two quantities differ.
Degenerate reference and realized triangles fail explicitly.

The curvature estimator is the **piecewise-flat vertex angle defect**:
`2pi-sum(face angles)` at interior vertices. At a boundary vertex the turning
term is `pi-sum(face angles)`. Signed boundary contributions are essential:
the annulus has chi zero, while the disk has chi one. The code reports interior
sum, boundary sum and their residual from `2pi*chi` separately. Topology is
not deduced from a curvature sign; an independent triangulation oracle can
check the actual exported incidence.

Gauss–Bonnet equality for a PL triangulation is an algebraic consistency check,
not proof that its local curvature matches the smooth target. The report also
gives the known smooth area and integrated target curvature. Refining the
**reference** triangulation compares interior defects and PL area with those
known integrals. Boundary collar error makes coarse interior defect differ from
the full target integral. Tests require improvement under refinement rather
than claiming exact equality for a coarse triangulation.

The exponential smooth map has stretches exactly one by the analytic pullback
identity. Its straight chord triangles generally do not. Their reported
distortion converges under reference/display refinement; it must not be replaced
by the analytic value one. The cylinder control independently checks the chord
factor `sin(pi/N)/(pi/N)` and unit meridional stretch.

## Authoritative identities, files and program interfaces

The ledger remains authoritative. At each density, 65 material markers retain
the original IDs and typed foundation/stitch parents. Mesh vertices are samples
of a chosen material chart, not new crochet stitches. The density-one mesh has
3 radial intervals and 216 periodic sectors; density two has 6 and 432.
Because 216 is divisible by 8,12,18 and 27, all stitch markers coincide with
sampled positions at these densities without duplicating the periodic seam.
There are 864 vertices/1296 triangles and 3024 vertices/5184 triangles respectively.

The material-edge file reads the ledger's 65 cyclic edges and 57 counted-stitch
parent attachments. For each it reports reference developed geodesic length,
R3 straight chord length and their ratio. It does not claim either is yarn
strand length. Circumferential stitch width is a separate smooth circle-arc
calibration. Foundation chain edges have no invented R3 positions.

Compile `stitch_ledger.c`, `material_geometry.c` and `test_material_geometry.c`
through the pinned owned ICK compiler with C17, optimization, strict warnings
and the math library. The actual ICK `←` and `÷` syntax is retained; do not
substitute a stock compiler. The CLI is:

* No arguments: run the host assertions.
* `--emit OUTPUT.tsv`: run assertions and write deterministic scalar evidence.
* `--mesh OUTPUT.obj`: write density-one finite R3 chord mesh.
* `--mesh-fine OUTPUT.obj`: write density-two chord mesh.
* `--markers OUTPUT.tsv`: write immutable IDs, parent role, material coordinates and XYZ.
* `--material-edges OUTPUT.tsv`: write intended/realized edge comparison.

These writers check open/write/close results. Output files are not transactional
on I/O failure. Mathematical operations leave their output unchanged on refusal.
The bounded host implementation allows up to 64 radial intervals and 864 angular
sectors. It uses binary64 without fast-math, checked finite parameters and explicit
degeneracy tolerances, not interval arithmetic or a proof of every rounded result.

Host evidence comes from the exact-head execution receipt. Independent AI-CI
must read the scalar and mesh outputs, reconstruct references, and reject wrong
radius factors, wrong curvature sign, omitted boundary terms, malformed input and
preferred/realized conflation. A successful compile alone establishes none of this.
The preserved application’s two display densities do not count as two reference
metric discretizations. Current isolated fixtures do not claim integration of
these readouts into its hold-growth UI or acceptance of the parked prototype.

MIRO A1 and C67 are separate consumers. This first benchmark computes on a host;
neither phone is asked to compile. No GPU double support is assumed. A later
native Euclidean viewer can load the bounded geometry while preserving marker
IDs, but armv7/arm64 packaging, GPU execution, picking and hands-on phone behavior
still need their own evidence.

## Primary physical/mathematical sources

[Henderson and Taimina, Crocheting the Hyperbolic Plane](https://pi.math.cornell.edu/~dtaimina/crochet/hplane.htm)
connects repeated stitch increases, refinement and manipulable physical models.
Our finite joined-round recipe is explicitly an adaptation, not a claim to have
recreated one of Taimina's specimens. The
[Institute For Figuring interview](https://www.theiff.org/publications/cab16-crochet.html)
and [Crochet Coral Reef](https://www.theiff.org/exhibits/reef.html) motivate material
experiments; resemblance to a reef form does not certify constant curvature.
[Efrati, Sharon and Kupferman, Elastic theory of unconstrained non-Euclidean plates](https://arxiv.org/abs/0810.2411)
separates reference geometry, stretching and bending in a continuum plate model.
That separation guides the future mechanics boundary. It does not calibrate
crocheted yarn or make the present analytic map a mechanical solution.
