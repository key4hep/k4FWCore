#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/TrackCollection.h"

#include "k4FWCore/Transformer.h"

#include <Gaudi/Property.h>
#include <GaudiKernel/ISvcLocator.h>

#include <string>

struct TypeMisMatchDemo final : k4FWCore::Transformer<edm4hep::MCParticleCollection(const edm4hep::TrackCollection&)> {
  TypeMisMatchDemo(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValues("InputCollection", {"MCParticles"})},
                    {KeyValues("OutputCollection", {"MCParticles2"})}) {}

  edm4hep::MCParticleCollection operator()(const edm4hep::TrackCollection& inputs) const final {
    debug() << inputs.size() << " type = " << inputs.getTypeName() << endmsg;
    auto track = inputs[0];
    // The next line goes boom
    debug() << track.getTrackerHits().size() << endmsg;

    return edm4hep::MCParticleCollection{};
  }
};

DECLARE_COMPONENT(TypeMisMatchDemo)
