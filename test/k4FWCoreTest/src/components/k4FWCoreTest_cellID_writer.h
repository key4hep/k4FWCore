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
#ifndef K4FWCORE_K4FWCORETEST_CELLID_WRITER
#define K4FWCORE_K4FWCORETEST_CELLID_WRITER

// GAUDI
#include "Gaudi/Algorithm.h"

// key4hep
#include "k4FWCore/DataHandle.h"
#include "k4FWCore/MetaDataHandle.h"

#include "edm4hep/Constants.h"

// datamodel
#include "edm4hep/SimTrackerHitCollection.h"

const std::string cellIDtest = "M:3,S-1:3,I:9,J:9,K-1:6";

/** @class k4FWCoreTest_cellID
 *  Lightweight producer for edm data to test cellID
 */
class k4FWCoreTest_cellID_writer : public Gaudi::Algorithm {
public:
  explicit k4FWCoreTest_cellID_writer(const std::string&, ISvcLocator*);
  /**  Initialize.
   *   @return status code
   */
  StatusCode initialize() final;
  /**  Execute.
   *   @return status code
   */
  StatusCode execute(const EventContext&) const final;

private:
  /// Handle for the SimTrackerHits to be written
  mutable DataHandle<edm4hep::SimTrackerHitCollection> m_simTrackerHitWriterHandle{"SimTrackerHits",
                                                                                   Gaudi::DataHandle::Writer, this};
  MetaDataHandle<std::string> m_cellIDHandle{m_simTrackerHitWriterHandle, edm4hep::labels::CellIDEncoding,
                                             Gaudi::DataHandle::Writer};

  // Some properties for the configuration metadata
  Gaudi::Property<int>                m_intProp{this, "intProp", 42, "An integer property"};
  Gaudi::Property<int>                m_intProp2{this, "intProp2", 42, "An integer property"};
  Gaudi::Property<float>              m_floatProp{this, "floatProp", 3.14, "A float property"};
  Gaudi::Property<float>              m_floatProp2{this, "floatProp2", 3.14, "A float property"};
  Gaudi::Property<double>             m_doubleProp{this, "doubleProp", 3.14, "A double property"};
  Gaudi::Property<double>             m_doubleProp2{this, "doubleProp2", 3.14, "A double property"};
  Gaudi::Property<std::string>        m_stringProp{this, "stringProp", "Hello", "A string property"};
  Gaudi::Property<std::string>        m_stringProp2{this, "stringProp2", "Hello", "A string property"};
  Gaudi::Property<std::vector<int>>   m_vectorIntProp{this, "vectorIntProp", {1, 2, 3}, "A vector of integers"};
  Gaudi::Property<std::vector<int>>   m_vectorIntProp2{this, "vectorIntProp2", {1, 2, 3}, "A vector of integers"};
  Gaudi::Property<std::vector<float>> m_vectorFloatProp{this, "vectorFloatProp", {1.1, 2.2, 3.3}, "A vector of floats"};
  Gaudi::Property<std::vector<float>> m_vectorFloatProp2{
      this, "vectorFloatProp2", {1.1, 2.2, 3.3}, "A vector of floats"};
  Gaudi::Property<std::vector<double>> m_vectorDoubleProp{
      this, "vectorDoubleProp", {1.1, 2.2, 3.3}, "A vector of doubles"};
  Gaudi::Property<std::vector<double>> m_vectorDoubleProp2{
      this, "vectorDoubleProp2", {1.1, 2.2, 3.3}, "A vector of doubles"};
  Gaudi::Property<std::vector<std::string>> m_vectorStringProp{
      this, "vectorStringProp", {"one", "two", "three"}, "A vector of strings"};
  Gaudi::Property<std::vector<std::string>> m_vectorStringProp2{
      this,
      "vectorStringProp2",
      {"one", "two", "three"},
  };
};
#endif /* K4FWCORE_K4FWCORETEST_CELLID_WRITER */
