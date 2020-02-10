#ifndef FWCORE_FWFLOATPRODUCER_H
#define FWCORE_FWFLOATPRODUCER_H

#include "FWCore/DataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"


// forward declarations
class PodioDataSvc;

class FWFloatProducer : public GaudiAlgorithm {

public:
  FWFloatProducer(const std::string& name, ISvcLocator* svcLoc);

  virtual StatusCode initialize();
  virtual StatusCode execute();
  virtual StatusCode finalize();

private:
  Gaudi::Property<float> m_floatValue{this, "FloatValue", 0.0, "Value of Float to produce"};
  DataHandle<float> m_datahandle_float{"FloatHandle", Gaudi::DataHandle::Writer, this};
};

#endif
