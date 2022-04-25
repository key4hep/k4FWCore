#ifndef FWCORE_UNIQUEIDGENSVC_H
#define FWCORE_UNIQUEIDGENSVC_H

#include <string>
#include <cstdint>

#include <GaudiKernel/Service.h>
#include "k4Interface/IUniqueIDGenSvc.h"


// Generate a random, unique*, reproducible number using
// event number, event run, and algorithm name.
// To be used as ID across events and algorithms.
class UniqueIDGenSvc : public extends<Service, IUniqueIDGenSvc> {
public:
  UniqueIDGenSvc(const std::string& name, ISvcLocator* svcLoc);
  StatusCode initialize() override;
  size_t getUniqueID(uint32_t evt_num, uint32_t run_num, std::string name) const override;
};

#endif
