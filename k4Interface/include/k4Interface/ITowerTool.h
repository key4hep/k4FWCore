#ifndef RECINTERFACE_ITOWERTOOL_H
#define RECINTERFACE_ITOWERTOOL_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// datamodel
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/Cluster.h"

struct tower {
  int theta;
  int phi;
};

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
   *   @return Struct containing number of towers in theta and phi.
   */
  virtual tower towersNumber() = 0;
  /**  Build calorimeter towers.
   *   @param[out] aTowers Calorimeter towers.
   *   @param[in] fillTowersCells Whether to fill maps of cells into towers, for later use in attachCells
   *   @return Size of the cell collection.
   */
  virtual uint buildTowers(std::vector<std::vector<float>>& aTowers, bool fillTowersCells=true) = 0;
  /**  Get the radius for the position calculation.
   *   @return Radius
   */
  virtual float radiusForPosition() const = 0;
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

#endif /* RECINTERFACE_ITOWERTOOL_H */
