#ifndef DELPHESUTILS_H
#define DELPHESUTILS_H


edm4hep::Track convertTrack(Candidate const* cand, const double magFieldBz)
{
  edm4hep::Track track;
  // Delphes does not really provide any information that would go into the
  // track itself. But some information can be used to at least partially
  // populate a TrackState
  edm4hep::TrackState trackState{};
  trackState.D0 = cand->D0;
  trackState.Z0 = cand->DZ;

  // Delphes calculates this from the momentum 4-vector at the track
  // perigee so this should be what we want. Note that this value
  // only undergoes smearing in the TrackSmearing module but e.g.
  // not in the MomentumSmearing module
  trackState.phi = cand->Phi;
  // Same thing under different name in Delphes
  trackState.tanLambda = cand->CtgTheta;
  // Only do omega when there is actually a magnetic field.
  double varOmega = 0;
  if (magFieldBz) {
    // conversion to have omega in [1/mm]
    constexpr double c_light = 299792458;
    constexpr double a = c_light * 1e3 * 1e-15;

    trackState.omega = a * magFieldBz / cand->PT * std::copysign(1.0, cand->Charge);
    // calculate variation using simple error propagation, assuming
    // constant B-field -> relative error on pT is relative error on omega
    varOmega = cand->ErrorPT * cand->ErrorPT / cand->PT / cand->PT * trackState.omega * trackState.omega;
  }

  // fill the covariance matrix. Indices on the diagonal are 0, 5,
  // 9, 12, and 14, corresponding to D0, phi, omega, Z0 and
  // tan(lambda) respectively. Currently Delphes doesn't provide
  // correlations
  auto& covMatrix = trackState.covMatrix;
  covMatrix[0] = cand->ErrorD0 * cand->ErrorD0;
  covMatrix[5] = cand->ErrorPhi * cand->ErrorPhi;
  covMatrix[9] = varOmega;
  covMatrix[12] = cand->ErrorDZ * cand->ErrorDZ;
  covMatrix[14] = cand->ErrorCtgTheta * cand->ErrorCtgTheta;

  track.addToTrackStates(trackState);

  return track;
}


#endif
