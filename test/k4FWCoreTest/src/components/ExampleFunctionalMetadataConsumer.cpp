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

#include "k4FWCore/Consumer.h"
#include "k4FWCore/MetadataUtils.h"

#include <string>
#include <vector>

struct ExampleFunctionalMetadataConsumer final : k4FWCore::Consumer<void(const edm4hep::MCParticleCollection& input)> {
  ExampleFunctionalMetadataConsumer(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, KeyValues("InputCollection", {"MCParticles"})) {}

  StatusCode initialize() override {
    m_particleNum = k4FWCore::getParameter<int>("NumberOfParticles", this).value_or(0);
    if (m_particleNum != 3) {
      error() << "NumberOfParticles expected to be 3 but is " << m_particleNum << endmsg;
      return StatusCode::FAILURE;
    }

    m_particleTime = k4FWCore::getParameter<float>("ParticleTime", this).value_or(0);
    if (m_particleTime != 1.5) {
      error() << "ParticleTime expected to be 1.5 but is " << m_particleTime << endmsg;
      return StatusCode::FAILURE;
    }
    m_PDGValues = k4FWCore::getParameter<std::vector<int>>("PDGValues", this).value_or(std::vector<int>{});
    if (m_PDGValues != std::vector<int>{1, 2, 3, 4}) {
      error() << "PDGValues expected to be {1, 2, 3, 4} but is {";
      for (const auto& pdg : m_PDGValues) {
        error() << pdg << ", ";
      }
      error() << "}" << endmsg;
      return StatusCode::FAILURE;
    }
    m_metadataString = k4FWCore::getParameter<std::string>("MetadataString", this).value_or("");
    if (m_metadataString != "hello") {
      error() << "MetadataString expected to be 'hello' but is '" << m_metadataString << "'" << endmsg;
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

  void operator()(const edm4hep::MCParticleCollection& input) const override {
    // Check that it's possible to get metadata parameters from the main loop
    auto particleNum = k4FWCore::getParameter<int>("NumberOfParticles", this).value_or(-1);
    if (input.size() != particleNum) {
      error() << "Input MCParticleCollection size is not " << particleNum << endmsg;
      return;
    }
    int i = 0;
    for (const auto& particle : input) {
      if (particle.getTime() != m_particleTime) {
        error() << "ParticleTime expected to be " << m_particleTime << " but is " << particle.getTime() << endmsg;
        return;
      }
      if (particle.getPDG() != m_PDGValues[i]) {
        error() << "PDGValues expected to be " << m_PDGValues[i] << " but is " << particle.getPDG() << endmsg;
        return;
      }
      ++i;
    }
  }

  StatusCode finalize() override {
    auto particleNum = k4FWCore::getParameter<int>("NumberOfParticles", this).value_or(-1);
    if (particleNum != 3) {
      error() << "NumberOfParticles expected to be 3 but is " << particleNum << endmsg;
      return StatusCode::FAILURE;
    }

    // Putting parameters in the main loop fails
    // auto eventMetadataInt = k4FWCore::getParameter<int>("EventMetadataInt", this).value_or(-1);
    // if (eventMetadataInt != 5) {
    //   error() << "EventMetadataInt is not 5" << endmsg;
    //   return StatusCode::FAILURE;
    // }

    auto finalizeMetadataInt = k4FWCore::getParameter<int>("FinalizeMetadataInt", this).value_or(-1);
    if (finalizeMetadataInt != 10) {
      error() << "FinalizeMetadataInt expected to be 10 but is " << finalizeMetadataInt << endmsg;
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

private:
  int              m_particleNum;
  float            m_particleTime;
  std::string      m_metadataString;
  std::vector<int> m_PDGValues;
};

DECLARE_COMPONENT(ExampleFunctionalMetadataConsumer)
