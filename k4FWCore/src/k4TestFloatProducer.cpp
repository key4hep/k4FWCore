#include "k4TestFloatProducer.h"

DECLARE_COMPONENT(k4TestFloatProducer)

k4TestFloatProducer::k4TestFloatProducer(const std::string& name, ISvcLocator* svcLoc)
    : GaudiAlgorithm(name, svcLoc) {
      declareProperty("Float", m_datahandle_float, "Float Value handle (output)");
    }

// setup before event processing
// (standard boilerplate code)
StatusCode k4TestFloatProducer::initialize() {
  if (GaudiAlgorithm::initialize().isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

// process events
// create a single float as output
// value is taken from options.
StatusCode k4TestFloatProducer::execute() {
  // the datahandle takes ownership of the memory,
  // need to pass a raw pointer.
  float* _float = new float();
  *_float = m_floatValue.value();
  m_datahandle_float.put(_float);
  return StatusCode::SUCCESS;
}

// after all events have been processed:
// (standard boilerplate code)
StatusCode k4TestFloatProducer::finalize() {
  if (GaudiAlgorithm::finalize().isFailure()) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}
