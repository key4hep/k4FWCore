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

#include <podio/utilities/TypeHelpers.h>

#include "edm4hep/edm4hep.h"

#include "k4FWCore/Transformer.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

struct CollectionMerger final
    : k4FWCore::Transformer<podio::CollectionBase*(const std::vector<const podio::CollectionBase*>&)> {
  CollectionMerger(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValues("InputCollections", {"MCParticles"})},
                    {KeyValues("OutputCollection", {"NewMCParticles"})}) {
    if (System::cmdLineArgs()[0].find("genconf") != std::string::npos) {
      return;
    }

    addToMapAll(edm4hep::edm4hepDataTypes{});
    addToMapAll(edm4hep::edm4hepLinkTypes{});
  }

  podio::CollectionBase* operator()(const std::vector<const podio::CollectionBase*>& input) const override {
    podio::CollectionBase* ret = nullptr;
    debug() << "Merging " << input.size() << " collections" << endmsg;
    std::string_view type = "";
    for (const auto& coll : input) {
      debug() << "Merging collection of type " << coll->getTypeName() << " with " << coll->size() << " elements"
              << endmsg;
      if (type.empty()) {
        type = coll->getTypeName();
      } else if (type != coll->getTypeName()) {
        throw std::runtime_error("Different collection types are not supported");
      }
      (this->*m_map.at(coll->getTypeName()))(coll, ret);
    }
    return ret;
  }

private:
  using MergeType = void (CollectionMerger::*)(const podio::CollectionBase*, podio::CollectionBase*&) const;
  std::map<std::string_view, MergeType> m_map;
  Gaudi::Property<bool> m_copy{this, "Copy", false,
                               "Copy the elements of the collections instead of creating a subset collection"};

  template <typename T>
  void addToMap() {
    m_map[T::collection_type::typeName] = &CollectionMerger::mergeCollections<typename T::collection_type>;
  }

  template <typename... T>
  void addToMapAll(podio::utils::TypeList<T...>&&) {
    (addToMap<T>(), ...);
  }

  template <typename T>
  void mergeCollections(const podio::CollectionBase* source, podio::CollectionBase*& ret) const {
    if (!ret) {
      ret = new T();
      if (!m_copy) {
        ret->setSubsetCollection();
      }
    }
    const auto ptr = static_cast<T*>(ret);
    const auto sourceColl = static_cast<const T*>(source);
    if (m_copy) {
      std::ranges::transform(*sourceColl, std::back_inserter(*ptr), [](const auto& elem) { return elem.clone(); });
    } else {
      std::ranges::copy(*sourceColl, std::back_inserter(*ptr));
    }
  }
};

DECLARE_COMPONENT(CollectionMerger)
