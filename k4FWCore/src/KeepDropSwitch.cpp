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
#include "k4FWCore/KeepDropSwitch.h"

#include <fmt/format.h>

#include <sstream>
#include <stdexcept>

namespace {
int wildcmp(const char* wild, const char* string) {
  // Written by Jack Handy - <A href="mailto:jakkhandy@hotmail.com">jakkhandy@hotmail.com</A>
  const char *cp = nullptr, *mp = nullptr;
  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }
  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string + 1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }
  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}

std::vector<std::string> split(const std::string& s, char delim) {
  std::vector<std::string> elems;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    if (!item.empty())
      elems.push_back(item);
  }
  return elems;
}
} // namespace

namespace k4FWCore {
KeepDropSwitch::KeepDropSwitch(const InputCommands& cmds) {
  m_commandlines.reserve(cmds.size());
  for (const auto& cmdLine : cmds) {
    auto [cmd, arg] = extractCommand(cmdLine);
    if (cmd == Cmd::INVALID) {
      throw std::invalid_argument(fmt::format("'{}' is not a valid command for the KeepDropSwitch", cmdLine));
    }
    m_commandlines.emplace_back(cmd, std::move(arg));
  }
}

bool KeepDropSwitch::isOn(const std::string& astring) const {
  auto im = m_cache.find(astring);
  if (im != m_cache.end())
    return im->second;
  else {
    bool val = getFlag(astring);
    m_cache.insert(std::pair<std::string, bool>(astring, val));
    return val;
  }
}

bool KeepDropSwitch::getFlag(const std::string& astring) const noexcept {
  bool flag = true;
  for (const auto& [cmd, pattern] : m_commandlines) {
    bool match = wildcmp(pattern.c_str(), astring.c_str());
    if (not match)
      continue;
    flag = (cmd == Cmd::KEEP);
  }
  return flag;
}

KeepDropSwitch::OutputCommand KeepDropSwitch::extractCommand(const std::string& cmdline) const {
  auto words = split(cmdline, ' ');
  if (words.size() != 2) {
    return {Cmd::INVALID, ""};
  }
  if (words[0] == "keep") {
    return {Cmd::KEEP, words[1]};
  }
  if (words[0] == "drop") {
    return {Cmd::DROP, words[1]};
  }

  return {Cmd::INVALID, ""};
}

} // namespace k4FWCore
