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

#include "Gaudi/Functional/Producer.h"

#include <string>

using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

struct ExampleGaudiFunctionalProducer final : Gaudi::Functional::Producer<int(), BaseClass_t> {
  // The pair in KeyValues can be changed from python and it corresponds
  // to the name of the output collection
  ExampleGaudiFunctionalProducer(const std::string& name, ISvcLocator* svcLoc)
    : Producer(name, svcLoc, KeyValue{"OutputCollectionName", "OutputCollection"}) {}

  // This is the function that will be called to produce the data
  int operator()() const override {
    return 3;
  }
};

DECLARE_COMPONENT(ExampleGaudiFunctionalProducer)
