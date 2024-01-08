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
#ifndef RECINTERFACE_ICALIBRATECALOHITSTOOL_H
#define RECINTERFACE_ICALIBRATECALOHITSTOOL_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

/** @class ICalibrateCaloHitsTool
 *
 *  Abstract interface to calorimeter hits calibration tool
 *
 *  @author Jana Faltova
 *  @date   2016-09
 */

class ICalibrateCaloHitsTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(ICalibrateCaloHitsTool, 1, 0);

  virtual void calibrate(std::unordered_map<uint64_t, double>& aHits) = 0;
};

#endif /* RECINTERFACE_ICALIBRATECALOHITSTOOL_H */
