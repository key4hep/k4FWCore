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

#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/Service.h"

#include "podio/Frame.h"

#include "k4FWCore/IMetadataSvc.h"

#include <memory>

class MetadataSvc : public extends<Service, IMetadataSvc> {
  using extends::extends;

public:
  StatusCode initialize() override;
  StatusCode finalize() override;

protected:
  SmartIF<IDataProviderSvc> m_dataSvc;

  std::unique_ptr<podio::Frame> m_frame;

  const podio::Frame* getFrame() const override;
  podio::Frame* getFrame() override;
  void setFrame(podio::Frame frame) override;
};

#endif
