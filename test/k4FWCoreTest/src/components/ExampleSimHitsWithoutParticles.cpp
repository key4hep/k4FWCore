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

#include "k4FWCore/Producer.h"

#include "edm4hep/CaloHitContributionCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"

#include <string>
#include <tuple>

// Produces sim hits whose MCParticle relations are deliberately left unset. This
// is what the hits of an event that has already been through an overlay look like
// when the background particles were not kept, and it is also simply what a
// SimCalorimeterHit built without a particle looks like. OverlayTiming has to
// leave such relations unset instead of indexing its particle collection with the
// -1 of the unset ObjectID.

using retType = std::tuple<edm4hep::MCParticleCollection, edm4hep::SimTrackerHitCollection,
                           edm4hep::SimCalorimeterHitCollection, edm4hep::CaloHitContributionCollection>;

struct ExampleSimHitsWithoutParticles final : k4FWCore::Producer<retType()> {
  ExampleSimHitsWithoutParticles(const std::string& name, ISvcLocator* svcLoc)
      : Producer(name, svcLoc, {},
                 {KeyValues("OutputCollectionParticles", {"MCParticles"}),
                  KeyValues("OutputCollectionSimTrackerHits", {"SimTrackerHits"}),
                  KeyValues("OutputCollectionSimCalorimeterHits", {"SimCalorimeterHits"}),
                  KeyValues("OutputCollectionCaloHitContributions", {"CaloHitContributions"})}) {}

  retType operator()() const override {
    // A couple of particles, so that the output particle collection is not empty
    // and a wrong index would go unnoticed.
    auto particles = edm4hep::MCParticleCollection();
    const edm4hep::Vector3d v{0, 0, 0};
    particles.create(1, 2, 3, 4.f, 5.f, 6.f, v, v, v);
    particles.create(2, 3, 4, 5.f, 6.f, 7.f);

    auto simTrackerHits = edm4hep::SimTrackerHitCollection();
    auto simCaloHits = edm4hep::SimCalorimeterHitCollection();
    auto contributions = edm4hep::CaloHitContributionCollection();

    for (int i = 0; i < 3; ++i) {
      // No setParticle call: the relation stays unset.
      auto trackerHit = simTrackerHits.create();
      trackerHit.setCellID(i + 1);
      trackerHit.setTime(1.f);
      trackerHit.setPosition({0, 0, 0});

      auto contribution = contributions.create();
      contribution.setEnergy(1.f);
      contribution.setTime(1.f);

      auto caloHit = simCaloHits.create();
      caloHit.setCellID(i + 1);
      caloHit.setEnergy(1.f);
      caloHit.setPosition({0, 0, 0});
      caloHit.addToContributions(contribution);
    }

    return std::make_tuple(std::move(particles), std::move(simTrackerHits), std::move(simCaloHits),
                           std::move(contributions));
  }
};

DECLARE_COMPONENT(ExampleSimHitsWithoutParticles)
