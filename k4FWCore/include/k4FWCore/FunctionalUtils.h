#ifndef K4FWCORE_FUNCTIONALUTILS_H
#define K4FWCORE_FUNCTIONALUTILS_H

#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/DataObjectHandle.h"
#include "k4FWCore/DataWrapper.h"
#include "edm4hep/MCParticleCollection.h"

#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"
#include "Gaudi/Functional/Consumer.h"
#include <GaudiKernel/FunctionalFilterDecision.h>

// Base class used for the Traits template argument of the
// Gaudi::Functional algorithms
struct BaseClass_t {
  template<typename T>
  using InputHandle = DataObjectReadHandle<DataWrapper<T>>;
  template<typename T>
  using OutputHandle = DataObjectWriteHandle<DataWrapper<T>>;
  
  using BaseClass = Gaudi::Algorithm;
};

#endif
