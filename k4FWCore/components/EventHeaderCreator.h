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
#ifndef K4FWCORE_EVENTHEADERCREATOR
#define K4FWCORE_EVENTHEADERCREATOR

#include "Gaudi/Algorithm.h"
#include "k4FWCore/DataHandle.h"
#include <edm4hep/Constants.h>
#include <edm4hep/EventHeaderCollection.h>
/***
 * Algorithm that creates an EventHeader collection and fills it with eventNumber and runNumber
 */

namespace edm4hep {
class EventHeaderCollection;
}

class EventHeaderCreator : public Gaudi::Algorithm {
public:
  EventHeaderCreator(const std::string& name, ISvcLocator* svcLoc);

  StatusCode execute(const EventContext&) const override;

private:
  // Since the type of the runNumber and eventNumber changed recently in EDM4hep, deduce their type
  // instead of hardcoding it to avoid warnings
  // Run number value (fixed for the entire job, to be set by the job submitter)
  Gaudi::Property<decltype(std::declval<edm4hep::EventHeader>().getRunNumber())> m_runNumber{this, "runNumber", 1,
                                                                                             "Run number value"};
  // Event number offset, use it if you want two separated jobs with the same run number
  Gaudi::Property<decltype(std::declval<edm4hep::EventHeader>().getEventNumber())> m_eventNumberOffset{
      this, "eventNumberOffset", 0,
      "Event number offset, eventNumber will be filled with 'event_index + eventNumberOffset'"};

  // datahandle for the EventHeader
  mutable k4FWCore::DataHandle<edm4hep::EventHeaderCollection> m_headerCol{edm4hep::labels::EventHeader,
                                                                           Gaudi::DataHandle::Writer, this};
};

#endif
