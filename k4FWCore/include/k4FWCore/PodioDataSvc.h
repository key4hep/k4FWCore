/*
 * Copyright (c) 2014-2023 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef FWCORE_PODIODATASVC_H
#define FWCORE_PODIODATASVC_H

#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/IConversionSvc.h"
// PODIO
#include <utility>
#include "podio/CollectionBase.h"
#include "podio/CollectionIDTable.h"
#include "podio/Frame.h"
#include "podio/ROOTFrameReader.h"
// Forward declarations
class DataWrapperBase;
class PodioOutput;
template <typename T> class MetaDataHandle;

/** @class PodioEvtSvc EvtDataSvc.h
 *
 *   An EvtDataSvc for PODIO classes
 *
 *  @author B. Hegner
 */
class PodioDataSvc : public DataSvc {
  template <typename T> friend class MetaDataHandle;
  friend class PodioOutput;

public:
  typedef std::vector<std::pair<std::string, podio::CollectionBase*>> CollRegistry;

  virtual StatusCode initialize();
  virtual StatusCode reinitialize();
  virtual StatusCode finalize();
  virtual StatusCode clearStore();
  virtual StatusCode i_setRoot(std::string root_path, IOpaqueAddress* pRootAddr);
  virtual StatusCode i_setRoot(std::string root_path, DataObject* pRootObj);

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

  const podio::Frame& getEventFrame() const { return m_eventframe; }

  /// Resets caches of reader and event store, increases event counter
  void endOfRead();

  /// TODO: Make this private again after conversions have been properly solved
  podio::Frame& getMetaDataFrame() { return m_metadataframe; }

private:
  /// PODIO reader for ROOT files
  podio::ROOTFrameReader m_reader;
  /// PODIO Frame, used to initialise collections
  podio::Frame m_eventframe;
  /// PODIO Frame, used to store metadata
  podio::Frame m_metadataframe;
  /// Counter of the event number
  int m_eventNum{0};
  /// Number of events in the file / to process
  int m_availableEventMax{-1};
  int m_requestedEventMax{-1};
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
  bool     m_bounds_check_needed{true};
};
#endif  // CORE_PODIODATASVC_H
