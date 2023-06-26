
#ifndef K4FWCORE_K4LEGACYDATASVC_H
#define K4FWCORE_K4LEGACYDATASVC_H

#include "k4FWCore/PodioLegacyDataSvc.h"

class k4LegacyDataSvc : public PodioLegacyDataSvc {
public:
  /// Standard Constructor
  k4LegacyDataSvc(const std::string& name, ISvcLocator* svc);

  /// Standard Destructor
  virtual ~k4LegacyDataSvc();
};
#endif
