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
#ifndef K4FWCORE_K4FWCORETEST_HELLOWORLDALG
#define K4FWCORE_K4FWCORETEST_HELLOWORLDALG

// GAUDI
#include "Gaudi/Algorithm.h"
#include "Gaudi/Property.h"

class k4FWCoreTest_HelloWorldAlg : public Gaudi::Algorithm {
public:
  explicit k4FWCoreTest_HelloWorldAlg(const std::string&, ISvcLocator*);
  virtual ~k4FWCoreTest_HelloWorldAlg();
  /**  Initialize.
   *   @return status code
   */
  virtual StatusCode initialize() final;
  /**  Execute.
   *   @return status code
   */
  virtual StatusCode execute(const EventContext&) const final;
  /**  Finalize.
   *   @return status code
   */
  virtual StatusCode finalize() final;

private:
  // member variable
  Gaudi::Property<std::string> theMessage{this, "PerEventPrintMessage", "Hello ",
                                          "The message to printed for each Event"};
};

#endif /* K4FWCORE_K4FWCORETEST_HELLOWORLDALG */
