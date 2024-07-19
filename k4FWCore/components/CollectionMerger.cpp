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

/** @class CollectionMerger
 *
 *  Merges collections of the same type into a single collection.
 *  The output collection is created as a subset collection if the Copy property is set to false (default).
 *  The collections are merged by copying the elements of the input collections into the output collection.
 *
 *  In both cases, the relations in the new objects point to the original objects. This means that collections
 *  having relations to objects in different collections will need these collections to be present to fully
 *  work. If the collections are not present (for example, after dropping them with the output commands)
 *  and this relations are used, that will almost certainly crash. An example of an usage of this algorithm
 *  is to merge several collections to give to an algorithm so that it can use all of them at the same time.
 *
 *  The collections to be merged are specified in the InputCollections property, which is a list of collection names.
 *  The output collection is specified in the OutputCollection property.
 */

#include "edm4hep/edm4hep.h"

#include "k4FWCore/Transformer.h"

#include <map>
#include <memory>
#include <string>
#include <string_view>

struct CollectionMerger final : k4FWCore::Transformer<std::shared_ptr<podio::CollectionBase>(
                                    const std::vector<const std::shared_ptr<podio::CollectionBase>*>&)> {
  CollectionMerger(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValues("InputCollections", {"MCParticles"})},
                    {KeyValues("OutputCollection", {"NewMCParticles"})}) {
    if (System::cmdLineArgs()[0].find("genconf") != std::string::npos) {
      return;
    }
    m_map["edm4hep::MCParticleCollection"]    = &CollectionMerger::mergeCollections<edm4hep::MCParticleCollection>;
    m_map["edm4hep::SimTrackerHitCollection"] = &CollectionMerger::mergeCollections<edm4hep::SimTrackerHitCollection>;
    m_map["edm4hep::CaloHitContributionCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::CaloHitContributionCollection>;
    m_map["edm4hep::SimCalorimeterHitCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::SimCalorimeterHitCollection>;
    m_map["edm4hep::RawCalorimeterHitCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::RawCalorimeterHitCollection>;
    m_map["edm4hep::CalorimeterHitCollection"] = &CollectionMerger::mergeCollections<edm4hep::CalorimeterHitCollection>;
    m_map["edm4hep::ParticleIDCollection"]     = &CollectionMerger::mergeCollections<edm4hep::ParticleIDCollection>;
    m_map["edm4hep::ClusterCollection"]        = &CollectionMerger::mergeCollections<edm4hep::ClusterCollection>;
    m_map["edm4hep::TrackerHit3DCollection"]   = &CollectionMerger::mergeCollections<edm4hep::TrackerHit3DCollection>;
    m_map["edm4hep::TrackerHitPlaneCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::TrackerHitPlaneCollection>;
    m_map["edm4hep::RawTimeSeriesCollection"] = &CollectionMerger::mergeCollections<edm4hep::RawTimeSeriesCollection>;
    m_map["edm4hep::TrackCollection"]         = &CollectionMerger::mergeCollections<edm4hep::TrackCollection>;
    m_map["edm4hep::VertexCollection"]        = &CollectionMerger::mergeCollections<edm4hep::VertexCollection>;
    m_map["edm4hep::ReconstructedParticleCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::ReconstructedParticleCollection>;
    m_map["edm4hep::MCRecoParticleAssociationCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::MCRecoParticleAssociationCollection>;
    m_map["edm4hep::MCRecoCaloAssociationCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::MCRecoCaloAssociationCollection>;
    m_map["edm4hep::MCRecoTrackerAssociationCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::MCRecoTrackerAssociationCollection>;
    m_map["edm4hep::MCRecoCaloParticleAssociationCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::MCRecoCaloParticleAssociationCollection>;
    m_map["edm4hep::MCRecoClusterParticleAssociationCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::MCRecoClusterParticleAssociationCollection>;
    m_map["edm4hep::MCRecoTrackParticleAssociationCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::MCRecoTrackParticleAssociationCollection>;
    m_map["edm4hep::RecoParticleVertexAssociationCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::RecoParticleVertexAssociationCollection>;
    m_map["edm4hep::TimeSeriesCollection"] = &CollectionMerger::mergeCollections<edm4hep::TimeSeriesCollection>;
    m_map["edm4hep::RecDqdxCollection"]    = &CollectionMerger::mergeCollections<edm4hep::RecDqdxCollection>;
    m_map["edm4hep::GeneratorEventParametersCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::GeneratorEventParametersCollection>;
    m_map["edm4hep::GeneratorPdfInfoCollection"] =
        &CollectionMerger::mergeCollections<edm4hep::GeneratorPdfInfoCollection>;
  }

  std::shared_ptr<podio::CollectionBase> operator()(
      const std::vector<const std::shared_ptr<podio::CollectionBase>*>& input) const override {
    std::shared_ptr<podio::CollectionBase> ret;
    debug() << "Merging " << input.size() << " collections" << endmsg;
    std::string_view type = "";
    for (const auto& coll : input) {
      debug() << "Merging collection of type " << (*coll)->getTypeName() << " with " << (*coll)->size() << " elements"
              << endmsg;
      if (type.empty()) {
        type = (*coll)->getTypeName();
      } else if (type != (*coll)->getTypeName()) {
        throw std::runtime_error("Different collection types are not supported");
        return ret;
      }
      (this->*m_map.at((*coll)->getTypeName()))(*coll, ret);
    }
    return ret;
  }

private:
  using MergeType = void (CollectionMerger::*)(const std::shared_ptr<podio::CollectionBase>&,
                                               std::shared_ptr<podio::CollectionBase>&) const;
  std::map<std::string_view, MergeType> m_map;
  Gaudi::Property<bool>                 m_copy{this, "Copy", false,
                               "Copy the elements of the collections instead of creating a subset collection"};

  template <typename T>
  void mergeCollections(const std::shared_ptr<podio::CollectionBase>& source,
                        std::shared_ptr<podio::CollectionBase>&       ret) const {
    if (!ret) {
      ret.reset(new T());
      if (!m_copy) {
        ret->setSubsetCollection();
      }
    }
    const auto ptr        = std::static_pointer_cast<T>(ret);
    const auto sourceColl = std::static_pointer_cast<T>(source);
    if (m_copy) {
      for (const auto& elem : *sourceColl) {
        ptr->push_back(elem.clone());
      }
    } else {
      for (const auto& elem : *sourceColl) {
        ptr->push_back(elem);
      }
    }
  }
};

DECLARE_COMPONENT(CollectionMerger)
