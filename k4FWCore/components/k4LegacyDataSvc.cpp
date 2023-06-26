
#include "k4LegacyDataSvc.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT(k4LegacyDataSvc)

/// Standard Constructor
k4LegacyDataSvc::k4LegacyDataSvc(const std::string& name, ISvcLocator* svc) : PodioLegacyDataSvc(name, svc) {
  declareProperty("inputs", m_filenames = {}, "Names of the files to read");
  declareProperty("input", m_filename = "", "Name of the file to read");
  declareProperty("FirstEventEntry", m_1stEvtEntry = 0, "First event to read");
}

/// Standard Destructor
k4LegacyDataSvc::~k4LegacyDataSvc() {}
