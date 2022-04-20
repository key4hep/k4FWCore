#ifndef FWCORE_IUNIQUEIDGENSVC_H
#define FWCORE_IUNIQUEIDGENSVC_H

#include <string>
#include <cstdint>
#include <functional>
#include <bitset>
#include <limits>

#include <GaudiKernel/IInterface.h>

struct GAUDI_API IUniqueIDGenSvc : extend_interfaces<IInterface> {
  DeclareInterfaceID(IUniqueIDGenSvc, 2, 0);
  virtual size_t getUniqueID(uint32_t evt_num, uint32_t run_num, std::string name) const = 0;
};

#endif
