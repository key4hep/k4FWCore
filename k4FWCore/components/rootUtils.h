/*
 * Copyright (c) 2014-2023 Key4hep-Project.
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
#ifndef PODIO_ROOT_UTILS_H
#define PODIO_ROOT_UTILS_H

#include "podio/CollectionBase.h"
#include "podio/CollectionBranches.h"

#include "TBranch.h"
#include "TClass.h"

#include <string>
#include <vector>

namespace podio::root_utils {
  // Workaround slow branch retrieval for 6.22/06 performance degradation
  // see: https://root-forum.cern.ch/t/serious-degradation-of-i-o-performance-from-6-20-04-to-6-22-06/43584/10
  template <class Tree> TBranch* getBranch(Tree* chain, const char* name) {
    return static_cast<TBranch*>(chain->GetListOfBranches()->FindObject(name));
  }

  inline std::string refBranch(const std::string& name, size_t index) { return name + "#" + std::to_string(index); }

  inline std::string vecBranch(const std::string& name, size_t index) { return name + "_" + std::to_string(index); }

  inline void setCollectionAddresses(podio::CollectionBase* collection, const CollectionBranches& branches) {
    const auto collBuffers = collection->getBuffers();

    if (auto buffer = collBuffers.data) {
      branches.data->SetAddress(buffer);
    }

    if (auto refCollections = collBuffers.references) {
      for (size_t i = 0; i < refCollections->size(); ++i) {
        branches.refs[i]->SetAddress(&(*refCollections)[i]);
      }
    }

    if (auto vecMembers = collBuffers.vectorMembers) {
      for (size_t i = 0; i < vecMembers->size(); ++i) {
        branches.vecs[i]->SetAddress((*vecMembers)[i].second);
      }
    }
  }

}  // namespace podio::root_utils

#endif
