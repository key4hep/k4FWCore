/*
 * Copyright (c) 2014-2024 Key4hep-Project.
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
#ifndef K4FWCORE_K4FWCORETEST_ALGORITHMWITHTFILE
#define K4FWCORE_K4FWCORETEST_ALGORITHMWITHTFILE

// GAUDI
#include "Gaudi/Algorithm.h"

// edm4hep
#include "TFile.h"
#include "TTree.h"
#include "k4FWCore/DataHandle.h"

#include "podio/UserDataCollection.h"

// datamodel
namespace edm4hep {
  class MCParticleCollection;
  class SimTrackerHitCollection;
  class SimCaloHit;
}  // namespace edm4hep

/** @class k4FWCoreTest_AlgorithmWithTFile
 *  Test producer to check that data can still be written to
 *  a user-declared TFile when using the PodioDataSvc
 *
 */
class k4FWCoreTest_AlgorithmWithTFile : public Gaudi::Algorithm {
public:
  explicit k4FWCoreTest_AlgorithmWithTFile(const std::string&, ISvcLocator*);
  virtual ~k4FWCoreTest_AlgorithmWithTFile();
  /**  Initialize.
   *   @return status code
   */
  virtual StatusCode initialize() final;
  /**  Execute.
   *   @return status code
   */
  virtual StatusCode execute(const EventContext&) const final;
  /**  Finalize.
   *   @return status code
   */
  virtual StatusCode finalize() final;

private:
  /// integer to add to the dummy values written to the edm
  Gaudi::Property<int> m_magicNumberOffset{this, "magicNumberOffset", 0,
                                           "Integer to add to the dummy values written to the edm"};
  /// Handle for the genparticles to be written
  mutable DataHandle<edm4hep::MCParticleCollection> m_mcParticleHandle{"MCParticles", Gaudi::DataHandle::Writer, this};
  /// Handle for the genvertices to be written
  mutable DataHandle<edm4hep::SimTrackerHitCollection> m_simTrackerHitHandle{"SimTrackerHit", Gaudi::DataHandle::Writer, this};

  mutable DataHandle<podio::UserDataCollection<float>> m_vectorFloatHandle{"VectorFloat", Gaudi::DataHandle::Writer, this};

  /// for testing: write a second TFile by user in an algorithm
  mutable Float_t m_value;
  TFile*  m_file;
  TTree*  m_tree;
};
#endif /* K4FWCORE_K4FWCORETEST_ALGORITHMWITHTFILE */
