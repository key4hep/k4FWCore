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

#include "Gaudi/Property.h"

#include "edm4hep/SimTrackerHitCollection.h"

#include "k4FWCore/Consumer.h"
#include "k4FWCore/MetadataUtils.h"

#include <string>

/// Consumer that looks up the cellID encoding of its input collection in
/// initialize() and fails if it is not there.
///
/// This mirrors what real digitisers do: they need the encoding to build a
/// decoder once, at initialization time, rather than for every event. It exists
/// so that algorithms producing collections at runtime can be tested for
/// publishing their cellID encoding early enough for such consumers to see it.
struct ExampleCellIDEncodingInitConsumer final
    : k4FWCore::Consumer<void(const edm4hep::SimTrackerHitCollection& input)> {
  ExampleCellIDEncodingInitConsumer(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, KeyValue("InputCollection", "SimTrackerHits")) {}

  StatusCode initialize() override {
    const auto collName = inputLocations("InputCollection")[0];
    const auto encoding = k4FWCore::getCellIDEncoding(collName, this);
    if (!encoding.has_value()) {
      error() << "No cellID encoding available for collection " << collName << " during initialize()" << endmsg;
      return StatusCode::FAILURE;
    }
    if (encoding.value() != m_expectedEncoding.value()) {
      error() << "cellID encoding for collection " << collName << " is '" << encoding.value() << "' but '"
              << m_expectedEncoding.value() << "' was expected" << endmsg;
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

  void operator()(const edm4hep::SimTrackerHitCollection& input) const override {
    debug() << "Received SimTrackerHit collection with " << input.size() << " elements" << endmsg;
  }

private:
  Gaudi::Property<std::string> m_expectedEncoding{this, "ExpectedEncoding", "",
                                                  "The cellID encoding the input collection is expected to have"};
};

DECLARE_COMPONENT(ExampleCellIDEncodingInitConsumer)
