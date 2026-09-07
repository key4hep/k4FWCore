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
#include "edm4hep/SimCalorimeterHitCollection.h"

#include <string>
#include <tuple>

// Produces calorimeter hits whose contributions have no MCParticle set.

using retType = std::tuple<edm4hep::SimCalorimeterHitCollection, edm4hep::CaloHitContributionCollection>;

struct ExampleCaloHitsWithoutParticles final : k4FWCore::Producer<retType()> {
  ExampleCaloHitsWithoutParticles(const std::string& name, ISvcLocator* svcLoc)
      : Producer(name, svcLoc, {},
                 {KeyValues("OutputCollectionSimCalorimeterHits", {"SimCalorimeterHits"}),
                  KeyValues("OutputCollectionCaloHitContributions", {"CaloHitContributions"})}) {}

  retType operator()() const override {
    auto simCaloHits = edm4hep::SimCalorimeterHitCollection();
    auto contributions = edm4hep::CaloHitContributionCollection();

    for (int i = 0; i < 3; ++i) {
      // No setParticle call: the relation stays unset
      auto contribution = contributions.create();
      contribution.setEnergy(1.f);
      contribution.setTime(1.f);

      auto caloHit = simCaloHits.create();
      caloHit.setCellID(i + 1);
      caloHit.setEnergy(1.f);
      caloHit.setPosition({0, 0, 0});
      caloHit.addToContributions(contribution);
    }

    return std::make_tuple(std::move(simCaloHits), std::move(contributions));
  }
};

DECLARE_COMPONENT(ExampleCaloHitsWithoutParticles)
