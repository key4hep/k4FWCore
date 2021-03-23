
#ifndef K4FWCORE_FCCDATASVC_H
#define K4FWCORE_FCCDATASVC_H

#include "k4FWCore/PodioDataSvc.h"

/// an alias to k4DataSvc for backwards compatibility
class FCCDataSvc : public PodioDataSvc {

public:
  /// Standard Constructor
  FCCDataSvc(const std::string& name, ISvcLocator* svc);

  /// Standard Destructor
  virtual ~FCCDataSvc();
};
#endif
