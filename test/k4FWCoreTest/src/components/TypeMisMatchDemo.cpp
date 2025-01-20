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

struct TypeMisMatchDemoMultiple final
    : k4FWCore::Transformer<edm4hep::MCParticleCollection(const std::vector<const edm4hep::TrackCollection*>&)> {
  TypeMisMatchDemoMultiple(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValues("InputCollections", {"MCParticles1", "MCParticles2"})},
                    {KeyValues("OutputCollection", {"OutputMCParticles"})}) {}

  edm4hep::MCParticleCollection operator()(const std::vector<const edm4hep::TrackCollection*>& input) const final {
    for (const auto& trackColl : input) {
      debug() << "collection size: " << trackColl->size() << " type = " << trackColl->getTypeName() << endmsg;
      auto track = (*trackColl)[0];
      // The next line goes boom
      debug() << track.getTrackerHits().size() << endmsg;
    }

    return edm4hep::MCParticleCollection{};
  }
};

DECLARE_COMPONENT(TypeMisMatchDemoMultiple)
