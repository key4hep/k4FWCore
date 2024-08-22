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
#ifndef RECINTERFACE_ITOWERTOOL_H
#define RECINTERFACE_ITOWERTOOL_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// datamodel
#include "edm4hep/CalorimeterHit.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/Cluster.h"

/** @class ITowerTool RecInterface/RecInterface/ITowerTool.h ITowerTool.h
 *
 *  Abstract interface to tower building tool.
 *
 *  @author Anna Zaborowska
 */

class ITowerTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(ITowerTool, 1, 0);

  /**  Find number of calorimeter towers.
   *   @param[out] nEta number of towers in eta.
   *   @param[out] nPhi number of towers in phi.
   */
  virtual void towersNumber(int& nEta, int& nPhi) = 0;

  /**  Build calorimeter towers.
   *   @param[out] aTowers Calorimeter towers.
   *   @param[in] fillTowersCells Whether to fill maps of cells into towers, for later use in attachCells
   *   @return Size of the cell collection.
   */
  virtual uint buildTowers(std::vector<std::vector<float>>& aTowers, bool fillTowersCells = true) = 0;
  /**  Get the radius for the position calculation.
   *   @return Radius
   */
  virtual float radiusForPosition() const = 0;
  /**  Get the tower IDs in eta.
   *   @param[in] aEta Position of the calorimeter cell in eta
   *   @return ID (eta) of a tower
   */
  virtual uint idEta(float aEta) const = 0;
  /**  Get the tower IDs in phi.
   *   @param[in] aPhi Position of the calorimeter cell in phi
   *   @return ID (phi) of a tower
   */
  virtual uint idPhi(float aPhi) const = 0;
  /**  Get the eta position of the centre of the tower.
   *   @param[in] aIdEta ID (eta) of a tower
   *   @return Position of the centre of the tower
   */
  virtual float eta(int aIdEta) const = 0;
  /**  Get the phi position of the centre of the tower.
   *   @param[in] aIdPhi ID (phi) of a tower
   *   @return Position of the centre of the tower
   */
  virtual float phi(int aIdPhi) const = 0;
  /**  Find cells belonging to a cluster.
   *   @param[in] aEta Position of the middle tower of a cluster in eta
   *   @param[in] aPhi Position of the middle tower of a cluster in phi
   *   @param[in] aHalfEtaFinal Half size of cluster in eta (in units of tower size). Cluster size is 2*aHalfEtaFinal+1
   *   @param[in] aHalfPhiFinal Half size of cluster in phi (in units of tower size). Cluster size is 2*aHalfPhiFinal+1
   *   @param[out] aEdmCluster Cluster of interest
   *   @param[out] aEdmClusterCells Cluster cells which belong to the cluster of interest
   */
  virtual void attachCells(float aEta, float aPhi, uint aHalfEtaFinal, uint aHalfPhiFinal,
                           edm4hep::MutableCluster& aEdmCluster, edm4hep::CalorimeterHitCollection* aEdmClusterCells,
                           bool aEllipse) = 0;
};

#endif /* RECINTERFACE_ITOWERTOOL_H */
