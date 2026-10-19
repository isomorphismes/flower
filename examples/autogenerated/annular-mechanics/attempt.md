# Annular mechanics: Idriç attempt

The requested result is one finite stretching/bending model for the accepted
four-round synthetic annulus. The metric, construction identity, material model,
numerical minimizer and physical observations remain distinct.

## Type sketch before source

The small pure slice squares a whole-number extension:
squared_extension : Number → Number. Its check is squared_extension 2 ≟ 4.
main writes one confirmation character. This is only an arithmetic acquisition
probe, not a floating solver. The complete model needs typed length, modulus,
thickness, energy, ring positions, an ordered annular meridian, and a bounded
convergence result (converged / exhausted / invalid / outside contact domain).
A successful result must preserve the immutable 65-stitch ledger and must not
upgrade synthetic calibration or host convergence to measured yarn behavior.

The solver requires binary64 trigonometric functions and finite arithmetic,
a six-variable optimization state, checked output, explicit iteration limits,
and a separately implemented energy/stationarity oracle. The native C reference
will keep these boundaries explicit. No generated-C Idriç backend is used.

## Source and actual acquisition probe

Source vocabulary follows current Idriç STYLE.md (blob
d5724bcfbf167833465fda21cc10b18209f3f240) and the repository's earlier
stitch-count example. The real one-step entrypoint files are taken unchanged
from isomorphisms/Idric e8b9173ab2d840ef0cf212698ea2fee97bd9e807.
The probe result is recorded after invoking that entrypoint.

## Failure, fallback and language work

Invoked the unchanged entrypoint with `sh .../idric-entrypoint/edric
--emit-one-step .../annular-mechanics.idric -o .../annular-mechanics.handoff`
on the disposable Linux build host. Exit 2, before type checking:
`Idriç one-step emitter: compiler is not bootstrapped; run ./_/edric bootstrap`.
No Idriç native execution is claimed. The C reference is compiled only through
the declared, source-pinned ICK lane; its success is separate evidence.
The first required capability is acquisition of a bootstrapped compiler,
followed by the checked value 4 and execution through a maintained native
backend. This record does not infer a language defect from missing acquisition.

