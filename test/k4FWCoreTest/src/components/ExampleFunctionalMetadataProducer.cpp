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

#include "k4FWCore/MetadataUtils.h"
#include "k4FWCore/Producer.h"

#include <string>
#include <vector>

struct ExampleFunctionalMetadataProducer final : k4FWCore::Producer<edm4hep::MCParticleCollection()> {
  ExampleFunctionalMetadataProducer(const std::string& name, ISvcLocator* svcLoc)
      : Producer(name, svcLoc, {}, KeyValues("OutputCollection", {"MCParticles"})) {}

  StatusCode initialize() override {
    k4FWCore::putParameter("NumberOfParticles", m_particleNum.value(), this);
    k4FWCore::putParameter("ParticleTime", m_particleTime.value(), this);
    k4FWCore::putParameter("PDGValues", m_PDGValues.value(), this);
    k4FWCore::putParameter("MetadataString", m_metadataString.value(), this);
    return StatusCode::SUCCESS;
  }

  edm4hep::MCParticleCollection operator()() const override {
    auto coll = edm4hep::MCParticleCollection();
    for (int i = 0; i < m_particleNum.value(); ++i) {
      auto particle = coll.create();
      particle.setPDG(m_PDGValues.value()[i]);
      particle.setTime(m_particleTime.value());
    }
    return coll;
  }

private:
  Gaudi::Property<int>         m_particleNum{this, "NumberOfParticles", 3, "How many particles will be produced"};
  Gaudi::Property<float>       m_particleTime{this, "ParticleTime", 1.5, "Which time will be set for the particles"};
  Gaudi::Property<std::string> m_metadataString{this, "MetadataString", "hello", "Example of a string"};
  Gaudi::Property<std::vector<int>> m_PDGValues{
      this, "PDGValues", {1, 2, 3, 4}, "Values of the PDG used for the particles"};
};

DECLARE_COMPONENT(ExampleFunctionalMetadataProducer)
