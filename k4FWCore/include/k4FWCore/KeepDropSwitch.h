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
  typedef std::vector<std::string> InputCommands;
  KeepDropSwitch() = default;
  explicit KeepDropSwitch(const InputCommands& cmds);
  bool isOn(const std::string& astring) const;

private:
  enum class Cmd { KEEP, DROP, INVALID };
  static Cmd fromString(const std::string_view cmd) noexcept;

  using OutputCommand = std::tuple<Cmd, std::string>;

  bool getFlag(const std::string& astring) const noexcept;
  OutputCommand extractCommand(const std::string& cmdLine) const;
  mutable std::map<std::string, bool> m_cache;
  std::vector<OutputCommand> m_outputCommands{};
};

} // namespace k4FWCore

#endif
