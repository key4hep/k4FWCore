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

#include <iostream>
#include <sstream>
#include <stdexcept>

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

bool KeepDropSwitch::getFlag(const std::string& astring) const {
  bool flag = true;
  for (const auto& cmdline : m_commandlines) {
    std::vector<std::string> words = split(cmdline, ' ');
    if (words.size() != 2) {
      std::ostringstream msg;
      msg << "malformed command string : " << cmdline;
      throw std::invalid_argument(msg.str());
    }
    std::string cmd = words[0];
    std::string pattern = words[1];
    Cmd theCmd = UNKNOWN;
    if (cmd == "keep")
      theCmd = KEEP;
    else if (cmd == "drop")
      theCmd = DROP;
    else {
      std::ostringstream msg;
      msg << "malformed command in line: " << std::endl;
      msg << cmdline << std::endl;
      msg << "should be keep or drop, lower case" << std::endl;
      throw std::invalid_argument(msg.str());
    }
    bool match = wildcmp(pattern.c_str(), astring.c_str());
    if (not match)
      continue;
    else if (theCmd == KEEP)
      flag = true;
    else
      flag = false;
  }
  return flag;
}

KeepDropSwitch::Cmd KeepDropSwitch::extractCommand(const std::string& cmdline) const {
  auto words = split(cmdline, ' ');
  for (auto& word : words)
    std::cout << "'" << word << "' ";
  std::cout << std::endl;
  return UNKNOWN;
}
