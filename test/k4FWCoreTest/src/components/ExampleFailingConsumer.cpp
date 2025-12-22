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

#include <stdexcept>
#include <string>

/// This is an example algorithm that will throw an exception in its operator
/// call. It still has an input to make it properly schedule
struct ExampleFailingConsumer final : k4FWCore::Consumer<void(const edm4hep::MCParticleCollection&)> {

  ExampleFailingConsumer(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, KeyValue("InputCollection", "MCParticles")) {}

  void operator()(const edm4hep::MCParticleCollection&) const override {
    throw std::runtime_error("An inevitable error. Really nothing we can do here");
  }

private:
};

DECLARE_COMPONENT(ExampleFailingConsumer);
