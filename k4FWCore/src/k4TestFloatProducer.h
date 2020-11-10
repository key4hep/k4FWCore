#ifndef K4FWCORE_TESTFLOATPRODUCER_H
#define K4FWCORE_TESTFLOATPRODUCER_H

#include "k4FWCore/DataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"


// forward declarations
class PodioDataSvc;
/***
 * Simple test algorithm which outputs a single float each event 
 * The value of the float can be set in the options,
 * using the FloatValue property.
 *
 */
class k4TestFloatProducer : public GaudiAlgorithm {

public:
  FWFloatProducer(const std::string& name, ISvcLocator* svcLoc);

  virtual StatusCode initialize();
  virtual StatusCode execute();
  virtual StatusCode finalize();

private:
  /// value which can be set from options
  Gaudi::Property<float> m_floatValue{this, "FloatValue", 0.0, "Value of Float to produce"};
  /// datahandle for the output
  DataHandle<float> m_datahandle_float{"Float", Gaudi::DataHandle::Writer, this};
};

#endif
