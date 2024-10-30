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

#include "Gaudi/Accumulators.h"
#include "Gaudi/Functional/Consumer.h"

class EventCounter final : public Gaudi::Functional::Consumer<void(const EventContext&)> {
public:
  using Consumer::Consumer;

  void operator()(const EventContext& ctx) const override {
    ++m_count;
    if (m_frequency && (ctx.evt() % m_frequency == 0)) {
      info() << "Processing event " << ctx.evt() << endmsg;
    }
  }

  StatusCode finalize() override {
    info() << "Processed " << m_count.value() << " events" << endmsg;
    return Consumer::finalize();
  }

private:
  mutable Gaudi::Accumulators::Counter<> m_count{this, "count"};
  Gaudi::Property<unsigned int>          m_frequency{this, "Frequency", 1, "How often to print the event number"};
};

DECLARE_COMPONENT(EventCounter)
