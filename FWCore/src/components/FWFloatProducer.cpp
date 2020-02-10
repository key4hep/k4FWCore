#include "FWFloatProducer.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "FWCore/PodioDataSvc.h"

DECLARE_COMPONENT(FWFloatProducer)

FWFloatProducer::FWFloatProducer(const std::string& name, ISvcLocator* svcLoc)
    : GaudiAlgorithm(name, svcLoc) {
      
      declareProperty("FloatValueHandle", m_datahandle_float, "Float Value handle (output)");
    }

StatusCode FWFloatProducer::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}


StatusCode FWFloatProducer::execute() {
  float* _float = new float();
  *_float = m_floatValue.value();
  m_datahandle_float.put(&m_floatValue.value());
  return StatusCode::SUCCESS;
}

StatusCode FWFloatProducer::finalize() {
  if (GaudiAlgorithm::finalize().isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}
