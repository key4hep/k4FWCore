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
#ifndef SIMG4INTERFACE_ISIMG4SVC_H
#define SIMG4INTERFACE_ISIMG4SVC_H

// Gaudi
#include "GaudiKernel/IService.h"

// Geant
class G4Event;

/** @class ISimG4Svc SimG4Interface/SimG4Interface/ISimG4Svc.h ISimG4Svc.h
 *
 *  Interface to the main Geant simulation service.
 *
 *  @author Anna Zaborowska
 */

class ISimG4Svc : virtual public IService {
public:
  DeclareInterfaceID(ISimG4Svc, 1, 0);
  /**  Simulate the event with Geant.
   *   @param[in] aEvent An event to be processed.
   *   @return status code
   */
  virtual StatusCode processEvent(G4Event& aEvent) = 0;
  /**  Retrieve the processed event.
   *   @param[out] aEvent The processed event.
   *   @return status code
   */
  virtual StatusCode retrieveEvent(G4Event*& aEvent) = 0;
  /**  Terminate the event simulation.
   *   @return status code
   */
  virtual StatusCode terminateEvent() = 0;
};
#endif /* SIMG4INTERFACE_ISIMG4SVC_H */
