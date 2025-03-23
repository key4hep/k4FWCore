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

#include "k4FWCore/MetadataUtils.h"
#include "k4FWCore/Transformer.h"

#include "edm4hep/ParticleIDCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/utils/ParticleIDUtils.h"

#include "Gaudi/Property.h"

#include <string>

struct ExampleParticleIDProducer final
    : k4FWCore::Transformer<edm4hep::ParticleIDCollection(const edm4hep::ReconstructedParticleCollection&)> {
  ExampleParticleIDProducer(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValues("InputCollection", {"RecoParticles"})},
                    KeyValues("ParticleIDCollection", {"reco_PIDs"})) {}

  StatusCode initialize() final {
    m_pidMeta = {m_pidAlgoName, m_pidParamNames};
    std::string collname = outputLocations("ParticleIDCollection")[0];
    k4FWCore::putParameter(collname, m_pidMeta);
    return StatusCode::SUCCESS;
  }

  edm4hep::ParticleIDCollection operator()(const edm4hep::ReconstructedParticleCollection& recos) const override {
    auto pidColl = edm4hep::ParticleIDCollection{};
    for (const auto& r : recos) {
      auto pid = pidColl.create();
      pid.setAlgorithmType(m_pidMeta.algoType());
      pid.setPDG(r.getPDG() - 10);
      pid.setParticle(r);
      for (size_t i = 0; i < m_pidMeta.paramNames.size(); ++i) {
        pid.addToParameters(i * 0.5f);
      }
    }

    return pidColl;
  }

private:
  Gaudi::Property<std::string> m_pidAlgoName{
      this, "PIDAlgoName", "fancyPID", "The name of the ParticleID algorithm that should be used for the metadata"};
  Gaudi::Property<std::vector<std::string>> m_pidParamNames{
      this,
      "PIDParamNames",
      {"p1", "p2", "p3"},
      "The names of the parameters of the PID algorithm that will be stored into metadata"};

  edm4hep::utils::ParticleIDMeta m_pidMeta{};
};

DECLARE_COMPONENT(ExampleParticleIDProducer);
