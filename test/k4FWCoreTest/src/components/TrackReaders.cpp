/*
 * Copyright (c) 2014-2024 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/TrackCollection.h"

#include "k4FWCore/Transformer.h"

#include <Gaudi/Property.h>
#include <GaudiKernel/ISvcLocator.h>

#include <string>

struct TrackReader final : k4FWCore::Transformer<edm4hep::MCParticleCollection(const edm4hep::TrackCollection&)> {
  TrackReader(const std::string& name, ISvcLocator* svcLoc)
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

DECLARE_COMPONENT(TrackReader)

struct TrackReaderMultiple final
    : k4FWCore::Transformer<edm4hep::MCParticleCollection(const std::vector<const edm4hep::TrackCollection*>&)> {
  TrackReaderMultiple(const std::string& name, ISvcLocator* svcLoc)
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

DECLARE_COMPONENT(TrackReaderMultiple)
