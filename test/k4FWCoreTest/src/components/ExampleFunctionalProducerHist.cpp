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

#include "k4FWCore/Producer.h"

#include "Gaudi/Accumulators/RootHistogram.h"
#include "GaudiKernel/RndmGenerators.h"

#include <string>

struct ExampleFunctionalProducerHist final : k4FWCore::Producer<edm4hep::MCParticleCollection()> {
  // The pair in KeyValues can be changed from python and it corresponds
  // to the name of the output collection
  ExampleFunctionalProducerHist(const std::string& name, ISvcLocator* svcLoc)
      : Producer(name, svcLoc, {}, KeyValues("OutputCollection", {"MCParticles"})) {}

  // This is the function that will be called to produce the data
  edm4hep::MCParticleCollection operator()() const override {
    // Not thread-safe!
    Rndm::Numbers rndu(randSvc(), Rndm::Flat(0, 1));
    ++m_histograms[rndu()];
    // Return an empty collection since we don't care about the collection for this example
    return {};
  }

private:
  // This is the histogram that will be filled, 1 is the number of dimensions of the histogram (1D)
  mutable Gaudi::Accumulators::RootHistogram<1> m_histograms{this, "Histogram Name", "Histogram Title", {100, 0, 1.}};
};

DECLARE_COMPONENT(ExampleFunctionalProducerHist)
