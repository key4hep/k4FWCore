#ifndef SIMDELPHESINTERFACE_IDELPHESAVEOUTPUTTOOL_H
#define SIMDELPHESINTERFACE_IDELPHESAVEOUTPUTTOOL_H

#include "GaudiKernel/IAlgTool.h"

class Delphes;
namespace edm4hep {
class MCParticleCollection;
}

/** @class IDelphesSaveOutputTool SimG4Interface/SimG4Interface/IDelphesSaveOutputTool.h IDelphesSaveOutputTool.h
 */

class IDelphesSaveOutputTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IDelphesSaveOutputTool, 1, 0);

  /**  Save the certain objects from Delphes to EDM.
   *   @param[in] delphes: Delphes module from which we can retrieve the output.
   *   @param[in] MCParticles: EDM collection holding MC particles to establish relations.
   *   @return status code
   */
  virtual StatusCode saveOutput(Delphes& delphes, const edm4hep::MCParticleCollection& mcParticles) = 0;
};
#endif /* SIMDELPHESINTERFACE_IDELPHESAVEOUTPUTTOOL_H */
