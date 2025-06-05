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
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/edm4hep.h>

#include "k4FWCore/Transformer.h"
#include "k4Interface/IUniqueIDGenSvc.h"

#include <numeric>
#include <random>
#include <string>

struct EfficiencyFilter final : k4FWCore::Transformer<podio::CollectionBase*(const podio::CollectionBase&,
                                                                             const edm4hep::EventHeaderCollection&)> {
  EfficiencyFilter(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValues("InputCollection", {""}), KeyValues("EventHeader", {"EventHeader"})},
                    {KeyValues("OutputCollection", {""})}) {}

  StatusCode initialize() final {
    StatusCode sc = Transformer::initialize();
    if (sc.isFailure()) {
      error() << "Unable to initialize base class Service." << endmsg;
      return sc;
    }

    m_uniqueIDSvc = service("UniqueIDGenSvc");
    if (!m_uniqueIDSvc) {
      error() << "Unable to locate the UniqueIDGenSvc" << endmsg;
      return StatusCode::FAILURE;
    }

    if (m_efficiency < 0.0 || m_efficiency > 1.0) {
      error() << "Efficiency must be between 0 and 1." << endmsg;
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  podio::CollectionBase* operator()(const podio::CollectionBase& coll,
                                    const edm4hep::EventHeaderCollection& evtHeader) const override {

    debug() << "EfficiencyFilter called for collection of type " << coll.getTypeName() << endmsg;

    const auto uid = m_uniqueIDSvc->getUniqueID(evtHeader, name());

    auto ptr = dispatchByType(
        coll, uid,
        edm4hep::edm4hepDataTypes{}); // Dispatch to the correct createSubsetCollection template function based on type
    if (!ptr) {
      ptr = dispatchByType(coll, uid, edm4hep::edm4hepLinkTypes{}); // Try link types if not found in data types
    }
    if (!ptr) {
      throw std::runtime_error(fmt::format(
          "EfficiencyFilter: No createSubsetCollection function found for collection type '{}'", coll.getTypeName()));
    }
    return ptr;
  }

private:
  SmartIF<IUniqueIDGenSvc> m_uniqueIDSvc{nullptr};

  Gaudi::Property<double> m_efficiency{this, "Efficiency", 0.5,
                                       "The efficiency of the filter, between 0 and 1. If 0.5, each item has a 50% "
                                       "chance to be kept in the output collection."};
  Gaudi::Property<bool> m_exact{
      this, "Exact", true,
      "If true, the output collection will have exactly the number of elements given by the efficiency. For example, "
      "if the efficiency is 0.5 and the input collection has 10 elements, the output collection will have exactly 5 "
      "elements. The result is floored."
      "If false, the output collection will have a random number of elements, with the average number of elements "
      "given by the efficiency."};

  template <typename... T>
  podio::CollectionBase* dispatchByType(const podio::CollectionBase& coll, size_t uid,
                                        podio::utils::TypeList<T...>&&) const {
    std::string_view typeName = coll.getTypeName();
    podio::CollectionBase* result = nullptr;
    (..., (typeName == T::collection_type::typeName
               ? (result = createSubsetCollection<typename T::collection_type>(&coll, uid))
               : nullptr));
    return result;
  }

  template <typename T>
  podio::CollectionBase* createSubsetCollection(const podio::CollectionBase* source, const size_t uid) const {

    std::mt19937 randomGen(uid);
    std::uniform_real_distribution<double> uniformDist(0.0, 1.0);

    auto ret = new T();
    ret->setSubsetCollection();
    const auto ptr = static_cast<T*>(ret);
    const auto sourceColl = static_cast<const T*>(source);

    if (m_exact) {
      // If exact, we will keep exactly the number of elements given by the efficiency
      const size_t nToKeep = static_cast<size_t>(static_cast<double>(sourceColl->size()) * m_efficiency);

      std::vector<size_t> indexes(sourceColl->size());
      std::iota(indexes.begin(), indexes.end(), 0);
      std::ranges::shuffle(indexes, randomGen);
      // Preserve the original ordering
      indexes.resize(nToKeep);
      std::ranges::sort(indexes);
      for (const auto index : indexes) {
        ptr->push_back((*sourceColl)[index]);
      }
    } else {
      for (const auto& item : *sourceColl) {
        if (uniformDist(randomGen) < m_efficiency) {
          ptr->push_back(item);
        }
      }
    }
    return ret;
  }
};

DECLARE_COMPONENT(EfficiencyFilter)
