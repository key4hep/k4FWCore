#ifndef FWCORE_IUNIQUEIDGENSVC_H
#define FWCORE_IUNIQUEIDGENSVC_H

#include <bitset>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>

#include <GaudiKernel/IInterface.h>

/** @class IUniqueIDGenSvc
 *
 *  Interface to the UniqueIDGenSvc
 *
 *  @author Placido Fernandez Declara
 */

struct GAUDI_API IUniqueIDGenSvc : extend_interfaces<IInterface> {
  DeclareInterfaceID(IUniqueIDGenSvc, 2, 0);
  virtual size_t getUniqueID(uint32_t evt_num, uint32_t run_num, const std::string& name) const = 0;
};

#endif
