#ifndef FWCORE_PODIOOUTPUT_H
#define FWCORE_PODIOOUTPUT_H

#include "GaudiAlg/GaudiAlgorithm.h"
#include "k4FWCore/KeepDropSwitch.h"
#include "podio/CollectionBase.h"
#include "podio/ROOTFrameWriter.h"

#include <vector>

// forward declarations
class PodioDataSvc;

class PodioOutput : public GaudiAlgorithm {
public:
  /// Constructor.
  PodioOutput(const std::string& name, ISvcLocator* svcLoc);

  /// Initialization of PodioOutput. Acquires the data service, creates trees and root file.
  virtual StatusCode initialize();
  /// Execute. For the first event creates branches for all collections known to PodioDataSvc and prepares them for
  /// writing. For the following events it reconnects the branches with collections and prepares them for write.
  virtual StatusCode execute();
  /// Finalize. Writes the meta data tree; writes file and cleans up all ROOT-pointers.
  virtual StatusCode finalize();

private:
  /// First event or not
  bool m_firstEvent;
  /// Root file name the output is written to
  Gaudi::Property<std::string> m_filename{this, "filename", "output.root", "Name of the file to create"};
  /// Commands which output is to be kept
  Gaudi::Property<std::vector<std::string>> m_outputCommands{
      this, "outputCommands", {"keep *"}, "A set of commands to declare which collections to keep or drop."};
  Gaudi::Property<std::string> m_filenameRemote{this, "remoteFilename", "",
                                                "An optional file path to copy the outputfile to."};
  /// Switch for keeping or dropping outputs
  KeepDropSwitch m_switch;
  PodioDataSvc*  m_podioDataSvc;
  /// The actual ROOT frame writer
  std::unique_ptr<podio::ROOTFrameWriter> m_framewriter;
  /// The stored collections
  std::vector<podio::CollectionBase*> m_storedCollections;
  /// The collections to write out
  std::vector<std::string> m_collection_names_to_write;
};

#endif
