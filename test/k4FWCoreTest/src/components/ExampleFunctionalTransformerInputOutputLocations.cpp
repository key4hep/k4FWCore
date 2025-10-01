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

#include "Gaudi/Property.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/RecoMCParticleLinkCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHit3DCollection.h"
#include "podio/UserDataCollection.h"

#include "k4FWCore/Transformer.h"

#include <stdexcept>
#include <string>

// Which type of collection we are reading
using FloatColl = std::vector<const podio::UserDataCollection<float>*>;
using ParticleColl = std::vector<const edm4hep::MCParticleCollection*>;
using SimTrackerHitColl = std::vector<const edm4hep::SimTrackerHitCollection*>;
using TrackerHitColl = std::vector<const edm4hep::TrackerHit3DCollection*>;
using TrackColl = std::vector<const edm4hep::TrackCollection*>;
using RecoColl = std::vector<const edm4hep::ReconstructedParticleCollection*>;
using LinkColl = std::vector<const edm4hep::RecoMCParticleLinkCollection*>;

using retType = std::tuple<std::vector<podio::UserDataCollection<float>>, std::vector<edm4hep::MCParticleCollection>,
                           std::vector<edm4hep::MCParticleCollection>, std::vector<edm4hep::SimTrackerHitCollection>,
                           std::vector<edm4hep::TrackerHit3DCollection>, std::vector<edm4hep::TrackCollection>,
                           std::vector<edm4hep::ReconstructedParticleCollection>,
                           std::vector<edm4hep::RecoMCParticleLinkCollection>>;

struct ExampleFunctionalTransformerRuntimeCollectionsMultiple final
    : k4FWCore::MultiTransformer<retType(const FloatColl&, const ParticleColl&, const SimTrackerHitColl&,
                                         const TrackerHitColl&, const TrackColl&, const RecoColl&, const LinkColl&)> {
  // The pairs in KeyValue can be changed from python and they correspond
  // to the names of the input collections
  ExampleFunctionalTransformerRuntimeCollectionsMultiple(const std::string& name, ISvcLocator* svcLoc)
      : MultiTransformer(name, svcLoc,
                         {
                             KeyValues("InputCollectionFloat", {"VectorFloat"}),
                             KeyValues("InputCollectionParticles", {"MCParticles1"}),
                         },
                         {
                             KeyValues("OutputCollectionFloat", {"VectorFloat"}),
                             KeyValues("OutputCollectionParticles1", {"MCParticles1"}),
                         }) {}

  // This is the function that will be called to transform the data
  // Note that the function has to be const, as well as the collections
  // we get from the input
  retType operator()(const FloatColl& floatVec, const ParticleColl& particlesVec,
                     const SimTrackerHitColl& simTrackerHitVec, const TrackerHitColl& trackerHitVec,
                     const TrackColl& trackVec, const RecoColl& recoVec, const LinkColl& linkVec) const override {

    return {};
  }

private:
  Gaudi::Property<int> m_offset{this, "Offset", 10, "Integer to add to the dummy values written to the edm"};
};

DECLARE_COMPONENT(ExampleFunctionalTransformerRuntimeCollectionsMultiple)
