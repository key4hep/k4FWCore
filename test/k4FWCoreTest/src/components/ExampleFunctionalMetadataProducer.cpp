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
    // Putting metadata in the main loop doesn't work
    // k4FWCore::putParameter("EventMetadataInt", 5, this);
    auto coll = edm4hep::MCParticleCollection();
    for (int i = 0; i < m_particleNum.value(); ++i) {
      auto particle = coll.create();
      particle.setPDG(m_PDGValues.value()[i]);
      particle.setTime(m_particleTime.value());
    }
    return coll;
  }

  StatusCode finalize() override {
    k4FWCore::putParameter("FinalizeMetadataInt", 10, this);
    return StatusCode::SUCCESS;
  }

private:
  Gaudi::Property<int> m_particleNum{this, "NumberOfParticles", 3, "How many particles will be produced"};
  Gaudi::Property<float> m_particleTime{this, "ParticleTime", 1.5, "Which time will be set for the particles"};
  Gaudi::Property<std::string> m_metadataString{this, "MetadataString", "hello", "Example of a string"};
  Gaudi::Property<std::vector<int>> m_PDGValues{
      this, "PDGValues", {1, 2, 3, 4}, "Values of the PDG used for the particles"};

  // Some properties for the configuration metadata
  Gaudi::Property<int> m_intProp{this, "intProp", 42, "An integer property"};
  Gaudi::Property<int> m_intProp2{this, "intProp2", 42, "An integer property"};
  Gaudi::Property<float> m_floatProp{this, "floatProp", 3.14, "A float property"};
  Gaudi::Property<float> m_floatProp2{this, "floatProp2", 3.14, "A float property"};
  Gaudi::Property<double> m_doubleProp{this, "doubleProp", 3.14, "A double property"};
  Gaudi::Property<double> m_doubleProp2{this, "doubleProp2", 3.14, "A double property"};
  Gaudi::Property<std::string> m_stringProp{this, "stringProp", "Hello", "A string property"};
  Gaudi::Property<std::string> m_stringProp2{this, "stringProp2", "Hello", "A string property"};
  Gaudi::Property<std::vector<int>> m_vectorIntProp{this, "vectorIntProp", {1, 2, 3}, "A vector of integers"};
  Gaudi::Property<std::vector<int>> m_vectorIntProp2{this, "vectorIntProp2", {1, 2, 3}, "A vector of integers"};
  Gaudi::Property<std::vector<float>> m_vectorFloatProp{this, "vectorFloatProp", {1.1, 2.2, 3.3}, "A vector of floats"};
  Gaudi::Property<std::vector<float>> m_vectorFloatProp2{
      this, "vectorFloatProp2", {1.1, 2.2, 3.3}, "A vector of floats"};
  Gaudi::Property<std::vector<double>> m_vectorDoubleProp{
      this, "vectorDoubleProp", {1.1, 2.2, 3.3}, "A vector of doubles"};
  Gaudi::Property<std::vector<double>> m_vectorDoubleProp2{
      this, "vectorDoubleProp2", {1.1, 2.2, 3.3}, "A vector of doubles"};
  Gaudi::Property<std::vector<std::string>> m_vectorStringProp{
      this, "vectorStringProp", {"one", "two", "three"}, "A vector of strings"};
  Gaudi::Property<std::vector<std::string>> m_vectorStringProp2{
      this,
      "vectorStringProp2",
      {"one", "two", "three"},
  };
};

DECLARE_COMPONENT(ExampleFunctionalMetadataProducer)
