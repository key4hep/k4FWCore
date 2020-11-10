
#include "k4DataSvc.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT(k4DataSvc)

/// Standard Constructor
K4DataSvc::k4DataSvc(const std::string& name, ISvcLocator* svc) : PodioDataSvc(name, svc) {
  declareProperty("inputs", m_filenames = {}, "Names of the files to read");
  declareProperty("input", m_filename = "", "Name of the file to read");
}

/// Standard Destructor
k4DataSvc::~k4DataSvc() {}
