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
#ifndef TEST_UNIQUEIDGENSVC_H
#define TEST_UNIQUEIDGENSVC_H

// GAUDI
#include <Gaudi/Algorithm.h>

#include "k4Interface/IUniqueIDGenSvc.h"

class TestUniqueIDGenSvc : public Gaudi::Algorithm {
public:
  explicit TestUniqueIDGenSvc(const std::string&, ISvcLocator*);
  virtual ~TestUniqueIDGenSvc();
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
  SmartIF<IUniqueIDGenSvc> m_service;
};

#endif  // TEST_UNIQUEIDGENSVC_H
