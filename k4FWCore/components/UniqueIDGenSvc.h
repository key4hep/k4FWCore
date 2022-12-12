#ifndef FWCORE_UNIQUEIDGENSVC_H
#define FWCORE_UNIQUEIDGENSVC_H

#include <cstdint>
#include <string>

#include <GaudiKernel/Service.h>
#include "k4Interface/IUniqueIDGenSvc.h"

/** @class UniqueIDGenSvc
 *  Generate unique, reproducible numbers using
 *  event number, run number, algorithm name, and global seed.
 *  To be used as reproducible per event seeds for random number generators used inside algorithms.
 *
 */
class UniqueIDGenSvc : public extends<Service, IUniqueIDGenSvc> {
public:
  UniqueIDGenSvc(const std::string& name, ISvcLocator* svcLoc);
  StatusCode initialize() override;
  size_t     getUniqueID(uint32_t evt_num, uint32_t run_num, const std::string& name) const override;

private:
  Gaudi::Property<int64_t> m_seed{this, "Seed", {123456789}};
};

#endif
