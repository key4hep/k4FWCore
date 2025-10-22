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
#ifndef EXAMPLES_KEEPDROPSWITCH_H
#define EXAMPLES_KEEPDROPSWITCH_H

#include <map>
#include <string>
#include <vector>

namespace k4FWCore {

class KeepDropSwitch {
public:
  enum Cmd { KEEP, DROP, UNKNOWN };
  typedef std::vector<std::string> CommandLines;
  KeepDropSwitch() = default;
  explicit KeepDropSwitch(const CommandLines& cmds) { m_commandlines = cmds; }
  bool isOn(const std::string& astring) const;

private:
  bool getFlag(const std::string& astring) const;
  Cmd extractCommand(const std::string& cmdLine) const;
  CommandLines m_commandlines;
  mutable std::map<std::string, bool> m_cache;
};

} // namespace k4FWCore

using KeepDropSwitch [[deprecated("Use k4FWCore::KeepDropSwitch instead")]] = k4FWCore::KeepDropSwitch;

#endif
