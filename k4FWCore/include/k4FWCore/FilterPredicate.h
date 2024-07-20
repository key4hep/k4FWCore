
#include "Gaudi/Functional/FilterPredicate.h"
#include "k4FWCore/FunctionalUtils.h"

namespace k4FWCore {

  template <typename Signature>
  using FilterPredicate = Gaudi::Functional::FilterPredicate<Signature, details::BaseClass_t>;
}

