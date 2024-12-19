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
#ifndef FWCORE_IMETADATASERVICE_H
#define FWCORE_IMETADATASERVICE_H

#include "GaudiKernel/IInterface.h"

#include "podio/Frame.h"

class IMetadataSvc : virtual public IInterface {
  friend class Writer;

public:
  DeclareInterfaceID(IMetadataSvc, 1, 0);

  virtual void setFrame(podio::Frame frame) = 0;

  template <typename T> void put(const std::string& name, const T& obj) {
    if (!getFrame()) {
      setFrame(podio::Frame{});
    }
    getFrame()->putParameter(name, obj);
  }

  template <typename T> std::optional<T> get(const std::string& name) const {
    const auto* frame = getFrame();
    if (!frame) {
      return std::nullopt;
    }
    return frame->getParameter<T>(name);
  }

protected:
  virtual podio::Frame*       getFrame()       = 0;
  virtual const podio::Frame* getFrame() const = 0;
};

#endif
