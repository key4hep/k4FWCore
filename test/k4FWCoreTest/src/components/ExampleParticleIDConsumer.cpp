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

#include <edm4hep/ReconstructedParticleCollection.h>
#include "k4FWCore/Consumer.h"
#include "k4FWCore/MetadataUtils.h"

#include "edm4hep/ParticleIDCollection.h"
#include "edm4hep/utils/ParticleIDUtils.h"

#include <Gaudi/Property.h>

#include "fmt/format.h"
#include "fmt/ranges.h"

#include <algorithm>
#include <stdexcept>
#include <string>

struct ExampleParticleIDConsumer final
    : k4FWCore::Consumer<void(const edm4hep::ParticleIDCollection&, const edm4hep::ParticleIDCollection&,
                              const edm4hep::ReconstructedParticleCollection&)> {
  ExampleParticleIDConsumer(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc,
                 {KeyValues("ParticleIDCollection1", {"PIDs1"}), KeyValues("ParticleIDCollection2", {"PIDs2"}),
                  KeyValues("RecoParticleCollection", {"recos"})}) {}

  bool checkAlgoMetadata(const edm4hep::utils::ParticleIDMeta& pidMeta, const Gaudi::Property<std::string>& algoName,
                         const Gaudi::Property<std::vector<std::string>>& paramNames) const {
    if (pidMeta.algoName != algoName) {
      fatal() << fmt::format(
                     "The PID algorithm name from metadata does not match the expected one from the properties: "
                     "(expected {}, actual {})",
                     algoName.value(), pidMeta.algoName)
              << endmsg;
      return false;
    }

    if (!std::ranges::equal(pidMeta.paramNames, paramNames)) {
      fatal() << fmt::format(
                     "The PID parameter names retrieved from metadata does not match the expected ones from the "
                     "properties: (expected {}, actual {})",
                     paramNames.value(), pidMeta.paramNames)
              << endmsg;
      return false;
    }

    return true;
  }

  void checkPIDForAlgo(const edm4hep::utils::PIDHandler& pidHandler, const edm4hep::ReconstructedParticle& reco,
                       const edm4hep::utils::ParticleIDMeta& pidMeta, const int paramIndex) const {
    auto maybePID = pidHandler.getPID(reco, pidMeta.algoType());
    if (!maybePID) {
      throw std::runtime_error(
          fmt::format("Could net retrieve the {} PID object for reco particle {}", pidMeta.algoName, reco.id().index));
    }
    auto pid      = maybePID.value();
    auto paramVal = pid.getParameters()[paramIndex];

    // As set in the producer
    if (paramVal != paramIndex * 0.5f) {
      throw std::runtime_error(
          fmt::format("Could not retrieve the correct parameter value for param {} (expected {}, actual {})",
                      pidMeta.paramNames[paramIndex], paramIndex * 0.5f, paramVal));
    }
  }

  StatusCode initialize() final {
    m_pidMeta1 =
        k4FWCore::getParameter<edm4hep::utils::ParticleIDMeta>(inputLocations("ParticleIDCollection1")[0]).value();

    m_pidMeta2 =
        k4FWCore::getParameter<edm4hep::utils::ParticleIDMeta>(inputLocations("ParticleIDCollection2")[0]).value();

    if (!checkAlgoMetadata(m_pidMeta1, m_pidAlgoName1, m_pidParamNames1) ||
        !checkAlgoMetadata(m_pidMeta2, m_pidAlgoName2, m_pidParamNames2)) {
      return StatusCode::FAILURE;
    }

    m_paramIndex1 = edm4hep::utils::getParamIndex(m_pidMeta1, m_paramOfInterest1.value()).value_or(-1);
    m_paramIndex2 = edm4hep::utils::getParamIndex(m_pidMeta2, m_paramOfInterest2.value()).value_or(-1);
    if (m_paramIndex1 < 0 || m_paramIndex2 < 0) {
      error() << fmt::format("Could not get a parameter index for {} (got {}) or {} (got {})",
                             m_paramOfInterest1.value(), m_paramIndex1, m_paramOfInterest2.value(), m_paramIndex2)
              << endmsg;
    }

    return StatusCode::SUCCESS;
  }

  void operator()(const edm4hep::ParticleIDCollection& pidColl1, const edm4hep::ParticleIDCollection& pidColl2,
                  const edm4hep::ReconstructedParticleCollection& recos) const {
    auto pidHandler = edm4hep::utils::PIDHandler::from(pidColl1, pidColl2);
    pidHandler.addMetaInfos(m_pidMeta1, m_pidMeta2);

    for (const auto r : recos) {
      auto pids = pidHandler.getPIDs(r);
      if (pids.size() != 2) {
        throw std::runtime_error(
            fmt::format("Could not get 2 ParticleID objects related to reco particle {}", r.id().index));
      }

      checkPIDForAlgo(pidHandler, r, m_pidMeta1, m_paramIndex1);
      checkPIDForAlgo(pidHandler, r, m_pidMeta2, m_paramIndex2);
    }
  }

private:
  edm4hep::utils::ParticleIDMeta m_pidMeta1{};
  edm4hep::utils::ParticleIDMeta m_pidMeta2{};

  int m_paramIndex1{};
  int m_paramIndex2{};

  Gaudi::Property<std::string> m_pidAlgoName1{
      this, "PIDAlgoName1", "fancyPID",
      "The name of the first ParticleID algorithm that should be used for the metadata"};
  Gaudi::Property<std::vector<std::string>> m_pidParamNames1{
      this,
      "PIDParamNames1",
      {"p1", "p2", "p3"},
      "The names of the parameters of the first PID algorithm that will be stored into metadata"};
  Gaudi::Property<std::string> m_paramOfInterest1{this, "ParamName1", "p1",
                                                  "The name of the parameter that should be checked"};
  Gaudi::Property<std::string> m_pidAlgoName2{
      this, "PIDAlgoName2", "fancyPID",
      "The name of the second ParticleID algorithm that should be used for the metadata"};
  Gaudi::Property<std::vector<std::string>> m_pidParamNames2{
      this,
      "PIDParamNames2",
      {"p1", "p2", "p3"},
      "The names of the parameters of the second PID algorithm that will be stored into metadata"};
  Gaudi::Property<std::string> m_paramOfInterest2{this, "ParamName2", "p2",
                                                  "The name of the parameter that should be checked"};
};

DECLARE_COMPONENT(ExampleParticleIDConsumer);
