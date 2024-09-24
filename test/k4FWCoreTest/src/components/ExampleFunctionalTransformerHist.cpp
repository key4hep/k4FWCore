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

#include "k4FWCore/Transformer.h"

#include "Gaudi/Accumulators/RootHistogram.h"

#include <string>

#include "GAUDI_VERSION.h"

#if GAUDI_MAJOR_VERSION < 39
namespace Gaudi::Accumulators {
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using StaticRootHistogram =
      Gaudi::Accumulators::RootHistogramingCounterBase<ND, Atomicity, Arithmetic, naming::histogramString>;
}
#endif

struct ExampleFunctionalTransformerHist final
    : k4FWCore::Transformer<edm4hep::MCParticleCollection(const edm4hep::MCParticleCollection& input)> {
  StatusCode initialize() override {
#if GAUDI_MAJOR_VERSION >= 39
    m_customHistogram.createHistogram(*this);
#endif
    return StatusCode::SUCCESS;
  }
  // The pairs in KeyValues can be changed from python and they correspond
  // to the name of the input and output collections respectively
  ExampleFunctionalTransformerHist(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, KeyValues("InputCollection", {"MCParticles"}),
                    KeyValues("OutputCollection", {"MCParticles"})) {}

  // This is the function that will be called to produce the data
  edm4hep::MCParticleCollection operator()(const edm4hep::MCParticleCollection& input) const override {
    // Fill the histogram with the energy of one particle
    ++m_histogram[input[0 + !m_firstParticle.value()].getEnergy()];
#if GAUDI_MAJOR_VERSION >= 39
    ++m_customHistogram[input[0 + !m_firstParticle.value()].getEnergy()];
#endif
    // Return an empty collection since we don't care about the collection
    return {};
  }

private:
  // This is the histogram that will be filled, 1 is the number of dimensions of the histogram (1D)
  mutable Gaudi::Accumulators::StaticRootHistogram<1> m_histogram{
      this, "Histogram Name", "Histogram Title", {100, 0, 10.}};

public:
#if GAUDI_MAJOR_VERSION >= 39
  // This is a histogram with title, name and bins that can be set from python
  void registerCallBack(Gaudi::StateMachine::Transition, std::function<void()>) {}
  mutable Gaudi::Accumulators::RootHistogram<1> m_customHistogram{this, "CustomHistogram"};
#endif

  Gaudi::Property<bool> m_firstParticle{this, "FirstParticle", true};
};

DECLARE_COMPONENT(ExampleFunctionalTransformerHist)
