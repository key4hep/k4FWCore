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

// #include "Gaudi/Property.h"

#include "edm4hep/MCParticleCollection.h"

#include "k4FWCore/TupleWriter.h"

#include <stdexcept>
#include <string>

struct ExampleTupleWriter final
    // We specify the types we want to write into the tuple here, using a std::tuple
    : k4FWCore::TupleWriter<std::tuple<int, double, std::string, edm4hep::Vector3d>(
          const edm4hep::MCParticleCollection& input)> {
  // The pair in KeyValues can be changed from python and it corresponds
  // to the name of the input collection
  ExampleTupleWriter(const std::string& name, ISvcLocator* svcLoc)
      : TupleWriter(name, svcLoc, KeyValue("InputCollection", "MCParticles")) {}

  // This is the function that will be called to fill the tuple
  // Note that the function has to be const, as well as the collections
  // we get from the input
  void operator()(const edm4hep::MCParticleCollection& input) const override {
    debug() << "Received MCParticle collection with " << input.size() << " elements" << endmsg;
    if (input.size() != 2) {
      fatal() << "Wrong size of MCParticle collection, expected 2 got " << input.size() << endmsg;
      throw std::runtime_error("Wrong size of MCParticle collection");
    }

    const auto particle = input[0];
    // The assignment of types works like `std::variant<types...> = value;` works
    // If both float and double are in the variant types, both types can be chosen
    // Remember that variant = 3.14 is always a double. 3.14f is a float.
    // If float is not specified but double is, all float values will be assigned to double
    // If double is not specified but float is, assigning doubles will fail at compile time
    // If neither float nor double are specified, assigning any floating point value will fail at compile time
    // For integers a bigger integer (like std::size_t) can not be assigned to a smaller one (like int)
    // sign to unsigned and viceversa works fine.

    auto& NTupleMap = getNTupleMap();

    NTupleMap["PDG"] = particle.getPDG();
    NTupleMap["GeneratorStatus"] = particle.getGeneratorStatus();
    NTupleMap["SimulatorStatus"] = particle.getSimulatorStatus();
    NTupleMap["Charge"] = particle.getCharge();
    NTupleMap["Time"] = particle.getTime();
    NTupleMap["Mass"] = particle.getMass();

    // std::string
    NTupleMap["Text"] = "hello!";

    // More complex types can be used as long as they are supported by ROOT TTrees/RNTuples
    // Here, an edm4hep::Vector3d will be saved, and its values can be accessed with .x, .y, .z
    NTupleMap["Vertex"] = edm4hep::Vector3d(1.0, 2.0, 3.0);

    // The choice of type can be forced using static_cast
    NTupleMap["CastedToInt"] = static_cast<int>(3.3);

    NTupleMap["Counter"] = m_counter++;

    // Optionally fill the tuple here if you want to fill more than once per event (for example, once per particle)
    // In this case, no automatic filling will happen
    // fill();
  }

  mutable std::atomic<int> m_counter = 0;
};

DECLARE_COMPONENT(ExampleTupleWriter)
