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

// This example shows how to restrict Gaudi properties to a fixed set of allowed
// values with k4FWCore::PropertyChoices, for the different kinds of properties.

#include "Gaudi/Property.h"

#include "edm4hep/MCParticleCollection.h"

#include "k4FWCore/GaudiChecks.h"
#include "k4FWCore/Producer.h"
#include "k4FWCore/PropertyChoices.h"

#include <string>

namespace {
enum class Mode { Fast, Precise };
}

struct ExampleFunctionalPropertyChoices final : k4FWCore::Producer<edm4hep::MCParticleCollection()> {
  ExampleFunctionalPropertyChoices(const std::string& name, ISvcLocator* svcLoc)
      : Producer(name, svcLoc, {}, KeyValue("OutputCollection", "MCParticles")) {}

  // (1) An enum selected by the name of the enumerators: the allowed strings are
  //     "Fast" and "Precise", the table is built from the enumerator names
  static inline const auto s_modes = k4FWCore::makePropertyChoices<Mode::Fast, Mode::Precise>();

  // (2) A few allowed integers: the user writes the number, no label is involved
  static inline const auto s_numLayers = k4FWCore::makePropertyChoices<4, 8, 15, 16, 23, 42>();

  // (3) A few allowed strings, where the string itself is what the code uses,
  //     e.g. to look up a detector or to name a collection
  static inline const auto s_detectors = k4FWCore::makePropertyChoices<"DCH_info", "STT_info">();

  // (4) Labels for values of a type that cannot be derived from a name. The user selects
  //     the gas by name and the code gets the drift velocity (in mm/ns). The values are
  //     made up
  static inline const k4FWCore::PropertyChoices<double> s_driftVelocities{{"Argon", 0.0023},
                                                                          {"ArgonIsobutane", 0.0075}};

  // The properties. All of them document their allowed values with list()
  Gaudi::Property<std::string> m_mode{this, "Mode", "Fast", "One of " + s_modes.list()};
  Gaudi::Property<int> m_numLayers{this, "NumLayers", 8, "One of " + s_numLayers.list()};
  Gaudi::Property<std::string> m_detector{this, "Detector", "DCH_info", "One of " + s_detectors.list()};
  Gaudi::Property<std::string> m_gas{this, "Gas", "Argon", "One of " + s_driftVelocities.list()};

  // What the code works with after the validation, filled in initialize()
  Mode m_modeValue{};
  int m_numLayersValue{};
  std::string m_detectorValue;
  double m_driftVelocity{};

  StatusCode initialize() override {
    K4_GAUDI_CHECK(Producer::initialize());
    // Validate the property and translate it. On an invalid value an error naming the
    // property and the allowed values is logged and the job fails
    K4_GAUDI_CHECK(s_modes.parse(m_mode, m_modeValue, this));
    K4_GAUDI_CHECK(s_numLayers.parse(m_numLayers, m_numLayersValue, this));
    K4_GAUDI_CHECK(s_detectors.parse(m_detector, m_detectorValue, this));
    K4_GAUDI_CHECK(s_driftVelocities.parse(m_gas, m_driftVelocity, this));

    info() << "Mode = " << m_mode.value() << ", NumLayers = " << m_numLayersValue << ", Detector = " << m_detectorValue
           << ", drift velocity = " << m_driftVelocity << " mm/ns for Gas = " << m_gas.value() << endmsg;
    return StatusCode::SUCCESS;
  }

  edm4hep::MCParticleCollection operator()() const override {
    auto coll = edm4hep::MCParticleCollection();
    // One particle per layer, or two per layer in the precise mode. The time at which the
    // signal arrives depends on the drift velocity
    const int particlesPerLayer = m_modeValue == Mode::Precise ? 2 : 1;
    for (int layer = 0; layer < m_numLayersValue; ++layer) {
      for (int i = 0; i < particlesPerLayer; ++i) {
        auto particle = coll.create();
        particle.setPDG(layer);
        particle.setTime(static_cast<float>(layer / m_driftVelocity));
      }
    }
    debug() << "Created " << coll.size() << " particles for " << m_detectorValue << endmsg;
    return coll;
  }
};

DECLARE_COMPONENT(ExampleFunctionalPropertyChoices)
