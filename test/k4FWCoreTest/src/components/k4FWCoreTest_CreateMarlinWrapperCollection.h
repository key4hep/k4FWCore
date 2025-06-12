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
#ifndef K4FWCORE_K4FWCORETEST_CREATEMARLINWRAPPERCOLLECTION
#define K4FWCORE_K4FWCORETEST_CREATEMARLINWRAPPERCOLLECTION

#include "Gaudi/Algorithm.h"
#include "GaudiKernel/IDataProviderSvc.h"

/** @class k4FWCoreTest_CreateMarlinWrapperCollection
 *  Produce a collection the same way that the Marlin wrapper does.
 */
class k4FWCoreTest_CreateMarlinWrapperCollection : public Gaudi::Algorithm {
public:
  explicit k4FWCoreTest_CreateMarlinWrapperCollection(const std::string&, ISvcLocator*);
  /**  Execute.
   *   @return status code
   */
  StatusCode initialize() override;
  StatusCode execute(const EventContext&) const final;

private:
  SmartIF<IDataProviderSvc> m_eventDataSvc;
  Gaudi::Property<std::string> m_outputCollectionName{this, "OutputCollection", "MCParticles",
                                                      "Name of the output collection"};
};
#endif /* K4FWCORE_K4FWCORETEST_CREATEMARLINWRAPPERCOLLECTION */
