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
#ifndef FWCORE_PROPERTYCHOICES_H
#define FWCORE_PROPERTYCHOICES_H

#include <Gaudi/Property.h>
#include <GaudiKernel/StatusCode.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <initializer_list>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace k4FWCore {

/// @brief Minimal stand-in for the `choices` argument of Python's
/// argparse.add_argument(), for string-valued Gaudi properties that may only
/// take one of a fixed set of values.
///
/// It replaces a hand-written if/else chain by a single table that is also the
/// one source of truth for the list of valid values quoted in the property
/// documentation and in error messages.
///
/// @code
/// enum class Mode { Fast, Precise };
/// static inline const k4FWCore::PropertyChoices<Mode> s_modes{{"Fast", Mode::Fast}, {"Precise", Mode::Precise}};
///
/// Gaudi::Property<std::string> m_mode{this, "Mode", "Fast", "One of " + s_modes.list()};
/// Mode m_modeValue;
///
/// StatusCode initialize() override {
///   K4_GAUDI_CHECK(s_modes.parse(m_mode, m_modeValue, this));
///   ...
/// }
/// @endcode
///
/// @tparam T The type the strings are translated to. Any copyable type works,
///           e.g. an enum class, as in the example above
template <typename T>
class PropertyChoices {
public:
  using Choice = std::pair<std::string_view, T>;

  PropertyChoices(std::initializer_list<Choice> choices) : m_choices(choices) {}

  /// Translate one of the allowed strings into its value, or return
  /// std::nullopt if the string is not one of the choices
  std::optional<T> parse(std::string_view value) const {
    const auto it = std::ranges::find(m_choices, value, &Choice::first);
    if (it == m_choices.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  /// Translate the value of a string property into @p result. If the value is
  /// not one of the choices an error listing the valid ones is logged and
  /// StatusCode::FAILURE is returned, leaving @p result untouched.
  ///
  /// @param property The property holding the string to translate
  /// @param result   Where to store the translated value
  /// @param comp     The Gaudi component (algorithm, tool) that owns the
  ///                 property, typically "this"
  template <typename GaudiComp>
  StatusCode parse(const Gaudi::Property<std::string>& property, T& result, const GaudiComp* comp) const {
    const auto parsed = parse(property.value());
    if (!parsed) {
      comp->error() << "Invalid " << property.name() << " '" << property.value() << "', expected one of " << list()
                    << "." << endmsg;
      return StatusCode::FAILURE;
    }
    result = *parsed;
    return StatusCode::SUCCESS;
  }

  /// The allowed values rendered as `'A', 'B', 'C'`, for property
  /// documentation and error messages
  std::string list() const {
    std::vector<std::string> quoted;
    quoted.reserve(m_choices.size());
    for (const auto& choice : m_choices) {
      quoted.push_back(fmt::format("'{}'", choice.first));
    }
    return fmt::format("{}", fmt::join(quoted, ", "));
  }

private:
  std::vector<Choice> m_choices;
};

namespace detail {
  /// Helper function to determine the *unqualified* name of an enum value (at
  /// compile time). This is a similar trick that magic_enum also plays
  template <auto V>
  consteval std::string_view enum_value_name() {
    const std::string_view funcName = std::source_location::current().function_name();

    std::size_t start, end;
    // function_name is not standardized, so compilers have obviously chosen
    // to do different things
    if (auto p = funcName.find("V = "); p != funcName.npos) {
      start = p + 4;
      end = funcName.find_first_of(";]", start);
    } else {
      constexpr std::string_view prefix = "enum_value_name<";
      start = funcName.find(prefix) + prefix.size();
      end = funcName.find(">", start);
    }
    auto name = funcName.substr(start, end - start);

    if (auto colon = name.rfind("::"); colon != name.npos) {
      name.remove_prefix(colon + 2);
    }
    return name;
  }
} // namespace detail

/// @brief Create a PropertyChoices for a set of enum values, using the
/// (unqualified) names of the enumerators as the allowed strings.
///
/// This avoids repeating every enumerator once as a string and once as a value
/// when the property values should simply match the enumerator names.
///
/// @code
/// enum class Mode { Fast, Precise };
/// static inline const auto s_modes = k4FWCore::makePropertyChoices<Mode::Fast, Mode::Precise>();
/// // equivalent to
/// // k4FWCore::PropertyChoices<Mode> s_modes{{"Fast", Mode::Fast}, {"Precise", Mode::Precise}};
/// @endcode
///
/// The order of the template arguments determines the order in which the
/// choices are listed by PropertyChoices::list().
///
/// @note This only works for values that correspond to a named enumerator
///
/// @tparam Vs The enum values that are allowed. They all need to share a common
///            type, which becomes the value type of the returned PropertyChoices
template <auto... Vs>
auto makePropertyChoices() {
  using T = std::common_type_t<decltype(Vs)...>;
  return PropertyChoices<T>{{detail::enum_value_name<Vs>(), Vs}...};
}

} // namespace k4FWCore

#endif // FWCORE_PROPERTYCHOICES_H
