
#include "K4DataSvc.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT(K4DataSvc)

/// Standard Constructor
K4DataSvc::K4DataSvc(const std::string& name, ISvcLocator* svc) : PodioDataSvc(name, svc) {
  declareProperty("inputs", m_filenames = {}, "Names of the files to read");
  declareProperty("input", m_filename = "", "Name of the file to read");
}

/// Standard Destructor
K4DataSvc::~K4DataSvc() {}
