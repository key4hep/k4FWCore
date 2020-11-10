
#ifndef FWCORE_K4DATASVC_H
#define FWCORE_K4DATASVC_H

#include "k4FWCore/PodioDataSvc.h"

class K4DataSvc : public PodioDataSvc {

public:
  /// Standard Constructor
  K4DataSvc(const std::string& name, ISvcLocator* svc);

  /// Standard Destructor
  virtual ~K4DataSvc();
};
#endif  // FWCORE_K4DATASVC_H
