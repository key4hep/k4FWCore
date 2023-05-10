#ifndef FWCORE_PODIODATASVC_H
#define FWCORE_PODIODATASVC_H

#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/IConversionSvc.h"
// PODIO
#include "podio/CollectionBase.h"
#include "podio/CollectionIDTable.h"
#include "podio/ROOTFrameReader.h"
#include "podio/Frame.h"
#include <utility>
// Forward declarations
class DataWrapperBase;

/** @class PodioEvtSvc EvtDataSvc.h
 *
 *   An EvtDataSvc for PODIO classes
 *
 *  @author B. Hegner
 */
class PodioDataSvc : public DataSvc {
public:
  typedef std::vector<std::pair<std::string, podio::CollectionBase*>> CollRegistry;

  virtual StatusCode initialize();
  virtual StatusCode reinitialize();
  virtual StatusCode finalize();
  virtual StatusCode clearStore();
  virtual StatusCode i_setRoot( std::string root_path, IOpaqueAddress* pRootAddr );
  virtual StatusCode i_setRoot( std::string root_path, DataObject* pRootObj );

  /// Standard Constructor
  PodioDataSvc(const std::string& name, ISvcLocator* svc);

  /// Standard Destructor
  virtual ~PodioDataSvc();

  // Use DataSvc functionality except where we override
  using DataSvc::registerObject;
  /// Overriding standard behaviour of evt service
  /// Register object with the data store.
  virtual StatusCode registerObject(std::string_view parentPath, std::string_view fullPath,
                                    DataObject* pObject) override final;

  StatusCode readCollection(const std::string& collectionName);

  const podio::Frame& getFrame() const { return m_frame; }

  /// Resets caches of reader and event store, increases event counter
  void endOfRead();

private:
  /// PODIO reader for ROOT files
  podio::ROOTFrameReader m_reader;
  /// PODIO Frame, used to initialise collections
  podio::Frame m_frame;
  /// Counter of the event number
  int m_eventNum{0};
  /// Number of events in the file / to process
  int m_eventMax{-1};
  /// Whether reading from file at all
  bool m_reading_from_file{false};

  SmartIF<IConversionSvc> m_cnvSvc;

  // Registry of data wrappers; needed for memory management
  std::vector<DataWrapperBase*> m_podio_datawrappers;

protected:
  /// ROOT file name the input is read from. Set by option filename
  std::vector<std::string> m_filenames;
  std::string              m_filename;
  /// Jump to nth events at the beginning. Set by option FirstEventEntry
  /// This option is helpful when we want to debug an event in the middle of a file
  unsigned m_1stEvtEntry{0};
};
#endif  // CORE_PODIODATASVC_H
