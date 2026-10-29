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

/// @file
/// @brief Restrict Gaudi properties to a fixed set of allowed values.
///
/// This header hooks into the two customization points that Gaudi::Property already provides, so that the
/// properties stay plain Gaudi::Property objects:
///
///  - **Enum properties** (`Gaudi::Property<Mode>`): an enum that is registered with K4FWCORE_ENUM_PROPERTY can be
///    used as the type of a property directly. The user sets it with the name of an enumerator (`"Fast"`) from the
///    steering file or the command line, and the code works with the enumerator (`Mode::Fast`). Any other name is
///    rejected when the property is set. This uses Gaudi's `StringConverter` and `toStream` customization points.
///  - **Restricting values** (`Gaudi::Property<int, k4FWCore::OneOf<0, 1>>`): the k4FWCore::OneOf and
///    k4FWCore::OneOfStrings verifiers only accept a fixed set of integers, enumerators or strings. This uses the
///    `VERIFIER` template argument of Gaudi::Property.
///
/// In both cases an invalid value is rejected as soon as it is set, i.e. before initialize() is called, and the job
/// fails with an error naming the property, the given value and the allowed ones, e.g.
/// `Cannot convert ''TheBlue'' for property 'Gandalf' [...]: std::invalid_argument, 'TheBlue' is not one of 'TheGrey',
/// 'TheWhite'`. An invalid default does not even let the component be constructed.
///
/// @code
/// enum class Mode { Fast, Precise, Experimental };
/// // Register the enumerators that can be selected by name. This has to be done in the global namespace, right after
/// // the declaration of the enum, before any Gaudi::Property<Mode> is used
/// K4FWCORE_ENUM_PROPERTY(Mode, Fast, Precise, Experimental);
///
/// struct MyAlg : k4FWCore::Transformer<...> {
///   // The user writes Mode="Precise", the code gets Mode::Precise
///   Gaudi::Property<Mode> m_mode{this, "Mode", Mode::Fast, "One of " + k4FWCore::enumNameList<Mode>()};
///   // Only a subset of the enumerators is allowed for this property
///   Gaudi::Property<Mode, k4FWCore::OneOf<Mode::Fast, Mode::Precise>> m_stableMode{
///       this, "StableMode", Mode::Fast, "One of " + k4FWCore::OneOf<Mode::Fast, Mode::Precise>::list()};
///   // A few allowed integers or strings
///   Gaudi::Property<int, k4FWCore::OneOf<4, 8>> m_numLayers{this, "NumLayers", 4};
///   Gaudi::Property<std::string, k4FWCore::OneOfStrings<"DCH", "STT">> m_tracker{this, "Tracker", "DCH"};
/// };
/// @endcode
///
/// A complete, compiled and tested example is given in
/// test/k4FWCoreTest/src/components/ExampleFunctionalPropertyChoices.cpp, with the job options in
/// test/k4FWCoreTest/options/ExampleFunctionalPropertyChoices.py.

#include <Gaudi/Parsers/CommonParsers.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/ToStream.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <ostream>
#include <ranges>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace k4FWCore {

/// @brief The enumerators of an enum that can be selected by name, e.g. in a Gaudi::Property<E>.
///
/// It is specialized by K4FWCORE_ENUM_PROPERTY and K4FWCORE_ENUM_PROPERTY_NAMES, which should be used to register an
/// enum. A specialization has a static `values` member, a range of `std::pair<E, std::string_view>` that maps each
/// enumerator to its name. The order of the entries is the order in which the names are listed by enumNameList().
///
/// @tparam E The enum type
template <typename E>
struct EnumNames;

/// An enum that is registered with EnumNames, and that can therefore be the type of a Gaudi::Property
template <typename E>
concept NamedEnum = std::is_enum_v<E> && requires {
  { std::ranges::begin(EnumNames<E>::values)->first } -> std::convertible_to<E>;
  { std::ranges::begin(EnumNames<E>::values)->second } -> std::convertible_to<std::string_view>;
};

namespace detail {
  /// Helper function to determine the *unqualified* name of an enum value (at
  /// compile time). This is a similar trick that magic_enum also plays
  template <auto V>
  consteval std::string_view enumValueName() {
    const std::string_view funcName = std::source_location::current().function_name();

    std::size_t start, end;
    // function_name is not standardized, so compilers have obviously chosen
    // to do different things
    if (auto p = funcName.find("V = "); p != funcName.npos) {
      start = p + 4;
      end = funcName.find_first_of(";]", start);
    } else {
      constexpr std::string_view prefix = "enumValueName<";
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

/// @brief Register enumerators with EnumNames under the names they have in the code.
///
/// @tparam Vs The enumerators that can be selected. They have to be named enumerators of the same enum
template <auto... Vs>
  requires(sizeof...(Vs) > 0 && std::is_enum_v<std::common_type_t<decltype(Vs)...>> &&
           (std::same_as<decltype(Vs), std::common_type_t<decltype(Vs)...>> && ...))
struct Enumerators {
  using EnumType = std::common_type_t<decltype(Vs)...>;
  static constexpr std::array<std::pair<EnumType, std::string_view>, sizeof...(Vs)> values{
      {{Vs, detail::enumValueName<Vs>()}...}};
};

/// Look up the enumerator with the given name
///
/// @param name The name of the enumerator, as registered with EnumNames
/// @tparam E   The enum type
/// @return The enumerator, or std::nullopt if no enumerator has this name
template <NamedEnum E>
constexpr std::optional<E> enumFromName(std::string_view name) {
  for (const auto& [value, valueName] : EnumNames<E>::values) {
    if (valueName == name) {
      return value;
    }
  }
  return std::nullopt;
}

/// Look up the name of an enumerator
///
/// @param value The enumerator
/// @tparam E    The enum type
/// @return The name of the enumerator, as registered with EnumNames, or std::nullopt if it is not registered
template <NamedEnum E>
constexpr std::optional<std::string_view> enumToName(E value) {
  for (const auto& [v, name] : EnumNames<E>::values) {
    if (v == value) {
      return name;
    }
  }
  return std::nullopt;
}

namespace detail {
  /// Render a value for property documentation and error messages: strings and registered enumerators are quoted
  /// (`'A'`), other enumerators are given by their underlying value and everything else is formatted as is (`1`)
  template <typename T>
  std::string formatChoice(const T& value) {
    if constexpr (NamedEnum<T>) {
      if (const auto name = enumToName(value)) {
        return fmt::format("'{}'", *name);
      }
      return fmt::format("{}", std::to_underlying(value));
    } else if constexpr (std::is_enum_v<T>) {
      return fmt::format("{}", std::to_underlying(value));
    } else if constexpr (std::is_convertible_v<const T&, std::string_view>) {
      return fmt::format("'{}'", std::string_view{value});
    } else {
      return fmt::format("{}", value);
    }
  }

  /// Render values as a comma separated list with formatChoice
  template <typename... Ts>
  std::string formatChoices(const Ts&... values) {
    return fmt::format("{}", fmt::join(std::array<std::string, sizeof...(Ts)>{formatChoice(values)...}, ", "));
  }

  /// A string literal that can be used as a template argument. A std::string or a plain
  /// string literal cannot be, but a class holding the characters in an array can.
  template <std::size_t N>
  struct FixedString {
    char data[N]{};
    constexpr FixedString(const char (&str)[N]) { std::copy_n(str, N, data); }
    constexpr std::string_view view() const { return {data, N - 1}; }
  };
} // namespace detail

/// The names of all enumerators registered with EnumNames rendered as `'A', 'B', 'C'`, e.g. for property
/// documentation
template <NamedEnum E>
std::string enumNameList() {
  return fmt::format("'{}'", fmt::join(EnumNames<E>::values | std::views::values, "', '"));
}

/// @brief A verifier for a Gaudi::Property that only accepts a fixed set of integers or enumerators.
///
/// Gaudi calls the verifier of a property whenever a value is set: when the property is constructed with its
/// default, and when it is assigned, which includes setting it from the steering file or the command line. An
/// invalid value is rejected with a std::invalid_argument that names the value and the allowed ones, e.g.
/// `2 is not one of 0, 1`, and the property keeps its previous value.
/// @code
/// Gaudi::Property<int, k4FWCore::OneOf<0, 1>> m_numBreakfasts{this, "NumBreakfasts", 1};
/// Gaudi::Property<Mode, k4FWCore::OneOf<Mode::Fast, Mode::Precise>> m_mode{this, "Mode", Mode::Fast};
/// @endcode
///
/// Floating point numbers are not supported, because they cannot be compared reliably with `==`.
///
/// @tparam Allowed The allowed values, in the order in which they are listed by list()
template <auto... Allowed>
  requires(sizeof...(Allowed) > 0 &&
           ((std::is_integral_v<decltype(Allowed)> || std::is_enum_v<decltype(Allowed)>) && ...))
struct OneOf {
  /// Throw std::invalid_argument if @p value is not one of the allowed values
  template <typename T>
    requires(std::equality_comparable_with<const T&, decltype(Allowed)> && ...)
  void operator()(const T& value) const {
    if (!((value == Allowed) || ...)) {
      throw std::invalid_argument(detail::formatChoice(value) + " is not one of " + list());
    }
  }

  /// The allowed values rendered as `1, 2, 3` or `'A', 'B'` for registered enumerators, e.g. for property
  /// documentation
  static std::string list() { return detail::formatChoices(Allowed...); }
};

/// @brief A verifier for a Gaudi::Property<std::string> that only accepts a fixed set of strings.
///
/// The string version of OneOf, see there for when it is called.
/// @code
/// Gaudi::Property<std::string, k4FWCore::OneOfStrings<"DCH", "STT">> m_tracker{this, "Tracker", "DCH"};
/// @endcode
/// If the string selects what the code does, rather than being used as is (e.g. as a name), prefer a
/// Gaudi::Property of an enum registered with EnumNames.
///
/// @tparam Allowed The allowed strings, in the order in which they are listed by list()
template <detail::FixedString... Allowed>
  requires(sizeof...(Allowed) > 0)
struct OneOfStrings {
  /// Throw std::invalid_argument if @p value is not one of the allowed strings
  void operator()(std::string_view value) const {
    if (!((value == Allowed.view()) || ...)) {
      throw std::invalid_argument(detail::formatChoice(value) + " is not one of " + list());
    }
  }

  /// The allowed strings rendered as `'A', 'B', 'C'`, e.g. for property documentation
  static std::string list() { return detail::formatChoices(Allowed.view()...); }
};

} // namespace k4FWCore

/// @brief Convert a Gaudi::Property of an enum registered with k4FWCore::EnumNames from and to the names of its
/// enumerators.
///
/// This is the customization point that Gaudi::Property uses to set a property from the steering file or the
/// command line, and to print it. A name that is not registered is rejected with a std::invalid_argument naming the
/// allowed ones. An enumerator without a registered name (which can only be assigned from C++) is printed as its
/// underlying value.
template <k4FWCore::NamedEnum E>
struct Gaudi::Details::Property::StringConverter<E> {
  std::string toString(const E& value) const {
    if (const auto name = k4FWCore::enumToName(value)) {
      return std::string{*name};
    }
    return std::to_string(std::to_underlying(value));
  }

  E fromString(const E& /* refValue */, const std::string& input) const {
    // Use the Gaudi string parser to remove the quotes that strings from Python have
    std::string name;
    if (!Gaudi::Parsers::parse(name, input).isSuccess()) {
      throw std::invalid_argument("cannot parse '" + input + "' as a string");
    }
    if (const auto value = k4FWCore::enumFromName<E>(name)) {
      return *value;
    }
    throw std::invalid_argument(k4FWCore::detail::formatChoice(name) + " is not one of " + k4FWCore::enumNameList<E>());
  }
};

/// Implementation of K4FWCORE_ENUM_PROPERTY and K4FWCORE_ENUM_PROPERTY_NAMES. Gaudi::Property prints its value with
/// Gaudi::Utils::toStream, whose generic version uses `operator<<`, which does not exist for an enum. Print the name
/// of the enumerator instead, quoted like a std::string, as that is what it is on the Python side
#define K4FWCORE_DETAIL_ENUM_TOSTREAM(E)                                                                               \
  template <>                                                                                                          \
  inline std::ostream& Gaudi::Utils::toStream<E>(const E& value, std::ostream& os) {                                   \
    if (const auto name = k4FWCore::enumToName(value)) {                                                               \
      return Gaudi::Utils::toStream(std::string{*name}, os);                                                           \
    }                                                                                                                  \
    return os << std::to_underlying(value);                                                                            \
  }                                                                                                                    \
  static_assert(true, "require a semicolon after the macro")

/// @brief Register an enum, so that it can be the type of a Gaudi::Property, selected by the names of the
/// enumerators.
///
/// @code
/// enum class Mode { Fast, Precise };
/// K4FWCORE_ENUM_PROPERTY(Mode, Fast, Precise);
/// // The user writes "Fast" or "Precise"
/// Gaudi::Property<Mode> m_mode{this, "Mode", Mode::Fast, "One of " + k4FWCore::enumNameList<Mode>()};
/// @endcode
///
/// It has to be used in the global namespace, after the declaration of the enum and before any Gaudi::Property<E>
/// is used. Enumerators that are not listed cannot be selected. The enumerators are in scope (via `using enum`), so
/// they can be given without the name of the enum, but `Mode::Fast` works as well.
///
/// @param E   The enum type
/// @param ... The enumerators that can be selected, in the order in which they are listed by enumNameList()
#define K4FWCORE_ENUM_PROPERTY(E, ...)                                                                                 \
  template <>                                                                                                          \
  struct k4FWCore::EnumNames<E> {                                                                                      \
    using enum E;                                                                                                      \
    static constexpr auto values = k4FWCore::Enumerators<__VA_ARGS__>::values;                                         \
  };                                                                                                                   \
  K4FWCORE_DETAIL_ENUM_TOSTREAM(E)

/// @brief Register an enum, so that it can be the type of a Gaudi::Property, selected by custom names.
///
/// @code
/// enum class Mode { Fast, Precise };
/// K4FWCORE_ENUM_PROPERTY_NAMES(Mode, {Fast, "fast"}, {Precise, "precise"});
/// // The user writes "fast" or "precise"
/// Gaudi::Property<Mode> m_mode{this, "Mode", Mode::Fast, "One of " + k4FWCore::enumNameList<Mode>()};
/// @endcode
///
/// See K4FWCORE_ENUM_PROPERTY for where it has to be used. As there, the enumerators can be given with or without
/// the name of the enum.
///
/// @param E   The enum type
/// @param ... `{enumerator, name}` pairs, in the order in which they are listed by enumNameList()
#define K4FWCORE_ENUM_PROPERTY_NAMES(E, ...)                                                                           \
  template <>                                                                                                          \
  struct k4FWCore::EnumNames<E> {                                                                                      \
    using enum E;                                                                                                      \
    static constexpr std::pair<E, std::string_view> values[] = {__VA_ARGS__};                                          \
  };                                                                                                                   \
  K4FWCORE_DETAIL_ENUM_TOSTREAM(E)

#endif // FWCORE_PROPERTYCHOICES_H
