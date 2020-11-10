
#ifndef K4FWCORE_K4DATASVC_H
#define K4FWCORE_K4DATASVC_H

#include "k4FWCore/PodioDataSvc.h"

class k4DataSvc : public PodioDataSvc {

public:
  /// Standard Constructor
  k4DataSvc(const std::string& name, ISvcLocator* svc);

  /// Standard Destructor
  virtual ~k4DataSvc();
};
#endif
