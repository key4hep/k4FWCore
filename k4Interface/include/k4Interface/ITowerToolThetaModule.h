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
#ifndef RECINTERFACE_ITOWERTOOLTHETAMODULE_H
#define RECINTERFACE_ITOWERTOOLTHETAMODULE_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// datamodel
#include "edm4hep/CalorimeterHit.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/Cluster.h"

/** @class ITowerToolThetaModule RecInterface/RecInterface/ITowerToolThetaModule.h ITowerToolThetaModule.h
 *
 *  Abstract interface to tower building tool.
 *
 *  @author Anna Zaborowska
 *  @Modified by Tong Li (t.li at cern.ch), for sliding-window clustering at FCCee where the CaloTower is created from Theta-Phi
 */

class ITowerToolThetaModule : virtual public IAlgTool {
public:
  DeclareInterfaceID(ITowerToolThetaModule, 1, 0);

  /**  Find number of calorimeter towers.
   *   @param[out] nTheta number of towers in theta.
   *   @param[out] nPhi number of towers in phi.
   */
  virtual void towersNumber(int& nTheta, int& nPhi) = 0;
  /**  Build calorimeter towers.
   *   @param[out] aTowers Calorimeter towers.
   *   @param[in] fillTowersCells Whether to fill maps of cells into towers, for later use in attachCells
   *   @return Size of the cell collection.
   */
  virtual uint buildTowers(std::vector<std::vector<float>>& aTowers, bool fillTowersCells = true) = 0;
  /**  Get the map of cells contained within a tower.
   *   @return Map of cells in a tower
   */
  virtual std::map<std::pair<uint, uint>, std::vector<edm4hep::CalorimeterHit>> cellsInTowers() const = 0;
  /**  Get the tower IDs in theta.
   *   @param[in] aTheta Position of the calorimeter cell in theta
   *   @return ID (theta) of a tower
   */
  virtual uint idTheta(float aTheta) const = 0;
  /**  Get the tower IDs in phi.
   *   @param[in] aPhi Position of the calorimeter cell in phi
   *   @return ID (phi) of a tower
   */
  virtual uint idPhi(float aPhi) const = 0;
  /**  Get the theta position of the centre of the tower.
   *   @param[in] aIdTheta ID (theta) of a tower
   *   @return Position of the centre of the tower
   */
  virtual float theta(int aIdTheta) const = 0;
  /**  Get the phi position of the centre of the tower.
   *   @param[in] aIdPhi ID (phi) of a tower
   *   @return Position of the centre of the tower
   */
  virtual float phi(int aIdPhi) const = 0;
  /**  Find cells belonging to a cluster.
   *   @param[in] aTheta Position of the middle tower of a cluster in theta
   *   @param[in] aPhi Position of the middle tower of a cluster in phi
   *   @param[in] aHalfThetaFinal Half size of cluster in theta (in units of tower size). Cluster size is 2*aHalfThetaFinal+1
   *   @param[in] aHalfPhiFinal Half size of cluster in phi (in units of tower size). Cluster size is 2*aHalfPhiFinal+1
   *   @param[out] aEdmCluster Cluster of interest
   *   @param[out] aEdmClusterCells Cluster cells which belong to the cluster of interest
   */
  virtual void attachCells(float aTheta, float aPhi, uint aHalfThetaFinal, uint aHalfPhiFinal,
                           edm4hep::MutableCluster& aEdmCluster, edm4hep::CalorimeterHitCollection* aEdmClusterCells,
                           bool aEllipse) = 0;
};

#endif /* RECINTERFACE_ITOWERTOOLTHETAMODULE_H */
