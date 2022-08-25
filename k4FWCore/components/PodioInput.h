#ifndef FWCORE_PODIOINPUT_H
#define FWCORE_PODIOINPUT_H
// Gaaudi
#include "GaudiAlg/GaudiAlgorithm.h"

// STL
#include <string>
#include <vector>

// forward declarations
// from k4FWCore:
class PodioDataSvc;

/** @class PodioInput
 *
 *  Class that allows to read ROOT files written with PodioOutput
 *
 *  @author J. Lingemann
 */

class PodioInput : public GaudiAlgorithm {
public:
  /// Constructor.
  PodioInput(const std::string& name, ISvcLocator* svcLoc);
  /// Initialization of PodioInput. Acquires the data service, opens root file and creates trees.
  virtual StatusCode initialize();
  /// Execute. Re-creates collections that are specified to be read and sets references.
  virtual StatusCode execute();
  /// Finalize. Closes ROOT file.
  virtual StatusCode finalize();

private:
  /// Name of collections to read. Set by option collections (this is temporary)
  Gaudi::Property<std::vector<std::string>> m_collectionNames{this, "collections", {}, "Places of collections to read"};
  /// Collection IDs (retrieved with CollectionIDTable from ROOT file, using collection names)
  std::vector<int> m_collectionIDs;
  /// Data service: needed to register objects and get collection IDs. Just an observing pointer.
  PodioDataSvc* m_podioDataSvc;
};

#endif
