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
#ifndef RECINTERFACE_ICALORIMETERTOOL_H
#define RECINTERFACE_ICALORIMETERTOOL_H

#include "k4Interface/ICaloIndexer.h"

// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include <unordered_map>
#include <vector>

namespace dd4hep {
namespace DDSegmentation {
  class Segmentation;
}
} // namespace dd4hep

#include <memory>

/** @class ICalorimeterTool RecInterface/RecInterface/ICalorimeterTool.h ICalorimeterTool.h
 *
 *  Abstract interface to calorimeter geometry tool
 *
 *  @author Anna Zaborowska
 */

class ICalorimeterTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(ICalorimeterTool, 1, 0);

  /** Fill vector with all existing cells for this geometry.
   */
  virtual const std::vector<uint64_t>& cellIDs() const = 0;

  /** Prepare a map of all existing cells in current geometry.
   *   @param[out] aCells map of existing cells (and deposited energy, set to 0)
   *   return Status code.
   */
  virtual StatusCode prepareEmptyCells(std::unordered_map<uint64_t, double>& aCells) const = 0;

  /** Return the segmentation associated with this geometry.
   */
  virtual const dd4hep::DDSegmentation::Segmentation* segmentation() const = 0;

  /** Return the name specified for the readout.
   */
  virtual const std::string& readoutName() const = 0;

  /** Return the subdetector ID.
   */
  virtual int id() const = 0;

  /** Return a new indexer object for this subdetector.
   *
   * Returns a null pointer if indexing is not implemented.
   */
  virtual std::unique_ptr<ICaloIndexer> indexer() const { return nullptr; }
};

#endif /* RECINTERFACE_ICALORIMETERTOOL_H */
