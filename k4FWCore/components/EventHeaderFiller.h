#ifndef K4FWCORE_EVENTHEADERFILLER
#define K4FWCORE_EVENTHEADERFILLER

#include "k4FWCore/DataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"

/***
 * Algortihm that fills the EventHeader with eventNumber and runNumber
 */

namespace edm4hep {
  class EventHeaderCollection;
}

class EventHeaderFiller : public GaudiAlgorithm {

public:
  EventHeaderFiller(const std::string& name, ISvcLocator* svcLoc);

  virtual StatusCode initialize();
  virtual StatusCode execute();
  virtual StatusCode finalize();

  private :
    // Run number value (fixed for the entire job, to be set by the job submitter)
    Gaudi::Property<int> m_runNumber{this, "runNumber", 1, "Run number value"};
    // Event number offset, use it if you want two separated jobs with the same run number
    Gaudi::Property<int> m_eventNumberOffset{this, "eventNumberOffset", 0, "Event number offset, eventNumber will be filled with 'event_index + eventNumberOffset'"}; 
    // datahandle for the EventHeader
    DataHandle<edm4hep::EventHeaderCollection> m_headerCol{"EventHeader", Gaudi::DataHandle::Writer, this};
};

#endif
