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

#include "edm4hep/CaloHitContributionCollection.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/ClusterCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/MCRecoCaloAssociationCollection.h"
#include "edm4hep/MCRecoCaloParticleAssociationCollection.h"
#include "edm4hep/MCRecoClusterParticleAssociationCollection.h"
#include "edm4hep/MCRecoParticleAssociationCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/MCRecoTrackerHitPlaneAssociationCollection.h"
#include "edm4hep/ParticleIDCollection.h"
#include "edm4hep/RawCalorimeterHitCollection.h"
#include "edm4hep/RawTimeSeriesCollection.h"
#include "edm4hep/RecDqdxCollection.h"
#include "edm4hep/RecIonizationClusterCollection.h"
#include "edm4hep/RecoParticleVertexAssociationCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/SimPrimaryIonizationClusterCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TimeSeriesCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHit3DCollection.h"
#include "edm4hep/TrackerHitPlaneCollection.h"
#include "edm4hep/TrackerPulseCollection.h"
#include "edm4hep/VertexCollection.h"

#include "k4FWCore/Transformer.h"

#include <map>
#include <memory>
#include <string>
#include <string_view>

template <typename T>
void cloneElements(const std::shared_ptr<podio::CollectionBase>& coll, std::shared_ptr<podio::CollectionBase>& ret) {
  if (!ret) {
    ret.reset(new T());
  }
  const auto ptr        = std::dynamic_pointer_cast<T>(ret);
  const auto sourceColl = std::dynamic_pointer_cast<T>(coll);
  for (const auto& elem : *sourceColl) {
    ptr->push_back(elem.clone());
  }
}

struct CollConcat final : k4FWCore::Transformer<std::shared_ptr<podio::CollectionBase>(
                              const std::map<std::string, const std::shared_ptr<podio::CollectionBase>&>&)> {
  CollConcat(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValues("InputCollections", {"MCParticles"})},
                    {KeyValues("OutputCollection", {"NewMCParticles"})}) {
    if (System::cmdLineArgs()[0].find("genconf") != std::string::npos) {
      return;
    }
    m_map["edm4hep::MCParticleCollection"]                = cloneElements<edm4hep::MCParticleCollection>;
    m_map["edm4hep::SimTrackerHitCollection"]             = cloneElements<edm4hep::SimTrackerHitCollection>;
    m_map["edm4hep::CaloHitContributionCollection"]       = cloneElements<edm4hep::CaloHitContributionCollection>;
    m_map["edm4hep::SimCalorimeterHitCollection"]         = cloneElements<edm4hep::SimCalorimeterHitCollection>;
    m_map["edm4hep::RawCalorimeterHitCollection"]         = cloneElements<edm4hep::RawCalorimeterHitCollection>;
    m_map["edm4hep::CalorimeterHitCollection"]            = cloneElements<edm4hep::CalorimeterHitCollection>;
    m_map["edm4hep::ParticleIDCollection"]                = cloneElements<edm4hep::ParticleIDCollection>;
    m_map["edm4hep::ClusterCollection"]                   = cloneElements<edm4hep::ClusterCollection>;
    m_map["edm4hep::TrackerHit3DCollection"]              = cloneElements<edm4hep::TrackerHit3DCollection>;
    m_map["edm4hep::TrackerHitPlaneCollection"]           = cloneElements<edm4hep::TrackerHitPlaneCollection>;
    m_map["edm4hep::RawTimeSeriesCollection"]             = cloneElements<edm4hep::RawTimeSeriesCollection>;
    m_map["edm4hep::TrackCollection"]                     = cloneElements<edm4hep::TrackCollection>;
    m_map["edm4hep::VertexCollection"]                    = cloneElements<edm4hep::VertexCollection>;
    m_map["edm4hep::ReconstructedParticleCollection"]     = cloneElements<edm4hep::ReconstructedParticleCollection>;
    m_map["edm4hep::MCRecoParticleAssociationCollection"] = cloneElements<edm4hep::MCRecoParticleAssociationCollection>;
    m_map["edm4hep::MCRecoCaloAssociationCollection"]     = cloneElements<edm4hep::MCRecoCaloAssociationCollection>;
    m_map["edm4hep::MCRecoTrackerAssociationCollection"]  = cloneElements<edm4hep::MCRecoTrackerAssociationCollection>;
    m_map["edm4hep::MCRecoTrackerHitPlaneAssociationCollection"] =
        cloneElements<edm4hep::MCRecoTrackerHitPlaneAssociationCollection>;
    m_map["edm4hep::MCRecoCaloParticleAssociationCollection"] =
        cloneElements<edm4hep::MCRecoCaloParticleAssociationCollection>;
    m_map["edm4hep::MCRecoClusterParticleAssociationCollection"] =
        cloneElements<edm4hep::MCRecoClusterParticleAssociationCollection>;
    m_map["edm4hep::MCRecoTrackParticleAssociationCollection"] =
        cloneElements<edm4hep::MCRecoTrackParticleAssociationCollection>;
    m_map["edm4hep::RecoParticleVertexAssociationCollection"] =
        cloneElements<edm4hep::RecoParticleVertexAssociationCollection>;
    m_map["edm4hep::SimPrimaryIonizationClusterCollection"] =
        cloneElements<edm4hep::SimPrimaryIonizationClusterCollection>;
    m_map["edm4hep::TrackerPulseCollection"]         = cloneElements<edm4hep::TrackerPulseCollection>;
    m_map["edm4hep::RecIonizationClusterCollection"] = cloneElements<edm4hep::RecIonizationClusterCollection>;
    m_map["edm4hep::TimeSeriesCollection"]           = cloneElements<edm4hep::TimeSeriesCollection>;
    m_map["edm4hep::RecDqdxCollection"]              = cloneElements<edm4hep::RecDqdxCollection>;
  }

  std::shared_ptr<podio::CollectionBase> operator()(
      const std::map<std::string, const std::shared_ptr<podio::CollectionBase>&>& input) const override {
    std::shared_ptr<podio::CollectionBase> ret;
    debug() << "Input size: " << input.size() << endmsg;
    std::string_view type = "";
    for (const auto& [name, coll] : input) {
      if (type.empty()) {
        type = coll->getTypeName();
      } else if (type != coll->getTypeName()) {
        throw std::runtime_error("Different collection types are not supported");
        return ret;
      }
      m_map.at(coll->getTypeName())(coll, ret);
    }
    return ret;
  }

private:
  std::map<std::string_view,
           std::function<void(const std::shared_ptr<podio::CollectionBase>&, std::shared_ptr<podio::CollectionBase>&)>>
      m_map;
};

DECLARE_COMPONENT(CollConcat)
