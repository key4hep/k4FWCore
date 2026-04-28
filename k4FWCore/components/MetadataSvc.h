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
#ifndef FWCORE_METADATASVC_H
#define FWCORE_METADATASVC_H

#include "Gaudi/Property.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/Service.h"

#include "podio/Frame.h"

#include "k4FWCore/IMetadataSvc.h"

#include <map>
#include <memory>
#include <string>

class MetadataSvc : public extends<Service, IMetadataSvc> {
  using extends::extends;

public:
  StatusCode initialize() override;
  StatusCode finalize() override;

  bool throwIfDuplicate() const override { return m_throwIfDuplicate; }
  bool skipIfSameValue() const override { return m_skipIfSameValue; }

protected:
  SmartIF<IDataProviderSvc> m_dataSvc;

  std::unique_ptr<podio::Frame> m_frame;

  const podio::Frame* getFrame() const override;
  podio::Frame* getFrame() override;
  void setFrame(podio::Frame frame) override;
  void throwIfRunning() const override;

private:
  void applyPropertyParameters();

  bool m_paramsApplied{false};

  Gaudi::Property<bool> m_throwIfDuplicate{this, "ThrowIfDuplicate", true,
                                           "Throw an exception if a metadata parameter is already set"};
  Gaudi::Property<bool> m_skipIfSameValue{
      this, "SkipIfSameValue", false,
      "Silently skip putting a metadata parameter if the existing value is the same as the new value"};
  Gaudi::Property<bool> m_setAtFinalize{
      this, "SetAtFinalize", false,
      "When true, apply the Python-configured parameters at finalize time instead of at initialize time"};
  Gaudi::Property<std::map<std::string, int>> m_intParameters{
      this, "IntParameters", {}, "Metadata int parameters to be set directly from Python"};
  Gaudi::Property<std::map<std::string, float>> m_floatParameters{
      this, "FloatParameters", {}, "Metadata float parameters to be set directly from Python"};
  Gaudi::Property<std::map<std::string, double>> m_doubleParameters{
      this, "DoubleParameters", {}, "Metadata double parameters to be set directly from Python"};
  Gaudi::Property<std::map<std::string, std::string>> m_stringParameters{
      this, "StringParameters", {}, "Metadata string parameters to be set directly from Python"};
};

#endif
