#ifndef RECINTERFACE_ITOPOCLUSTERINPUTTOOL_H
#define RECINTERFACE_ITOPOCLUSTERINPUTTOOL_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

#include <unordered_map>

namespace edm4hep {
  class CalorimeterHit;
}
/** @class ITopoClusterInputTool RecInterface/RecInterface/ITopoClusterInput.h ITopoClusterInputTool.h
 *
 *  Abstract interface to topo cluster input tool.
 *
 *  @author Coralie Neubueser
 */

class ITopoClusterInputTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(ITopoClusterInputTool, 1, 0);


  virtual StatusCode cellIDMap(std::unordered_map<uint64_t, double>& aCells) = 0;
 };

#endif /* RECINTERFACE_ITOPOCLUSTERINPUTTOOL_H */
