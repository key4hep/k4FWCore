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
#include "TRandom3.h"
#include "edm4hep/EventHeaderCollection.h"
#include "k4FWCore/Transformer.h"
#include "k4Interface/IUniqueIDGenSvc.h"
#include "podio/UserDataCollection.h"

// Example functional algorithms using getUniqueID to seed a PRNG
// Selected parts of the algorithm are shown as an example in the docs
// doc/uniqueIDGen.md

class ExampleRNGSeedingAlg final
    : public k4FWCore::Transformer<podio::UserDataCollection<double>(const edm4hep::EventHeaderCollection&)> {
public:
  ExampleRNGSeedingAlg(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc, {KeyValues("EventHeaderCollection", {"EventHeader"})},
                    {KeyValues("OutputCollection", {"RandomNumbers"})}) {}

  // locate the  UniqueIDGenSvc service during initialization
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

    return StatusCode::SUCCESS;
  }

  podio::UserDataCollection<double> operator()(const edm4hep::EventHeaderCollection& evtHeader) const final {
    // obtain unique value from the first header in a collection
    auto uid = m_uniqueIDSvc->getUniqueID(evtHeader, name());
    // or from a given EventHeader object
    // auto uid = m_uniqueIDSvc->getUniqueID(evtHeader[0], name());

    // seed TRandom3 or some other PRNG of your choice
    auto prng = TRandom3(uid);

    auto coll = podio::UserDataCollection<double>();
    coll.push_back(prng.Rndm());
    return coll;
  }

private:
  SmartIF<IUniqueIDGenSvc> m_uniqueIDSvc{nullptr};
};

DECLARE_COMPONENT(ExampleRNGSeedingAlg);
