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
#ifndef TESTDD4HEP_ITESTDD4HEPSVC_H
#define TESTDD4HEP_ITESTDD4HEPSVC_H

// Gaudi
#include "GaudiKernel/IService.h"

/** @class ITestDD4hepSvc Test/TestDD4hep/TestDD4hep/ITestDD4hepSvc.h ITestDD4hepSvc.h
 *
 *  Interface to the test service.
 *
 *  @author Anna Zaborowska
 */

class ITestDD4hepSvc : virtual public IService {
public:
  DeclareInterfaceID(ITestDD4hepSvc, 1, 0);
};
#endif /* TESTDD4HEP_ITESTDD4HEPSVC_H */
