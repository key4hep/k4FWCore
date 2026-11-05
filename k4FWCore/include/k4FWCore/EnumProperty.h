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
#ifndef K4FWCORE_ENUMPROPERTY_H
#define K4FWCORE_ENUMPROPERTY_H

#include <Gaudi/Parsers/CommonParsers.h>
#include <Gaudi/Parsers/InputData.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/StatusCode.h>
#include <GaudiKernel/System.h>
#include <GaudiKernel/ToStream.h>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <ostream>
#include <ranges>
#include <set>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

/** @file k4FWCore/EnumProperty.h
 *
 *  Machinery to make it possible to use enums as the type of a Gaudi::Property
 *  such that setting it via the enum name is possible from python.
 *
 *  The mechanism implemented requires to opt-in by specializing
 *  k4FWCore::EnumNames, deriving from k4FWCore::Enumerators with the
 *  enumerators that can be selected. They are selected by their names in the
 *  code. Enumerators that are not listed cannot be selected, i.e. it is
 *  possible to restrict the values to a subset of the enumerators. In addition
 *  the enum has to be registered with K4FWCORE_REGISTER_ENUM_FOR_PROPERTY,
 *  which provides the printout of the enum that Gaudi::Property needs:
 *
 *  @code
 *  enum class Mode { Fast, Precise, Experimental };
 *
 *  // Opt in; names are taken from the enumerators. Experimental is not selectable.
 *  template <>
 *  struct k4FWCore::EnumNames<Mode> : k4FWCore::Enumerators<Mode::Fast, Mode::Precise> {};
 *  K4FWCORE_REGISTER_ENUM_FOR_PROPERTY(Mode);
 *
 *  struct MyAlg : k4FWCore::Consumer<void()> {
 *    Gaudi::Property<Mode>              m_mode{this, "Mode", Mode::Fast, "One of " + k4FWCore::enumNameList<Mode>()};
 *    Gaudi::Property<std::vector<Mode>> m_modes{this, "Modes", {Mode::Fast}};
 *  };
 *  @endcode
 *
 *  Alternatively, the enumerators can be registered with a function
 *  `k4FWCoreEnumNames` defined next to the enum, which is found by
 *  argument-dependent lookup (ADL). Only its return type is used, it is never
 *  called. This form is the only one that works for an enum declared inside the
 *  class that uses it as the type of a property. In this case the enum is
 *  registered with K4FWCORE_REGISTER_ALG_ENUM_FOR_PROPERTY, inside the class:
 *
 *  @code
 *  struct MyAlg : k4FWCore::Consumer<void()> {
 *    enum class Mode { Fast, Precise, Experimental };
 *    friend constexpr k4FWCore::Enumerators<Mode::Fast, Mode::Precise> k4FWCoreEnumNames(Mode) { return {}; }
 *    K4FWCORE_REGISTER_ALG_ENUM_FOR_PROPERTY(Mode);
 *
 *    Gaudi::Property<Mode> m_mode{this, "Mode", Mode::Fast, "One of " + k4FWCore::enumNameList<Mode>()};
 *  };
 *  @endcode
 *
 *  @note The return type of `k4FWCoreEnumNames` has to be given explicitly,
 *  `auto` does not work.
 *
 *  @note (Especially for Clang) with internal linkage it is possible that there
 *  is a warning about the function not being needed. This warning can be
 *  silenced using the `[[maybe_unused]]` attribute (after the function name).
 *
 *  If the enumerators are registered in both ways, the specialization of
 *  k4FWCore::EnumNames is used.
 *
 *  From Python the property is then set by name, e.g. `MyAlg(Mode="Precise",
 *  Modes=["Fast", "Precise"])`. Any other name is rejected when the property
 *  is set, with an error listing the allowed names.
 *
 *  @attention The specialization of k4FWCore::EnumNames, or the declaration of
 *  `k4FWCoreEnumNames`, and the registration with one of the macros must come
 *  after the enum and before the first use of `Gaudi::Property<E>`,
 *  `Gaudi::Property<std::vector<E>>`, `Gaudi::Property<std::set<E>>`,
 *  `Gaudi::Utils::toStream` or anything else that checks k4FWCore::NamedEnum
 *  for this enum. K4FWCORE_REGISTER_ENUM_FOR_PROPERTY must be used at global
 *  namespace scope, K4FWCORE_REGISTER_ALG_ENUM_FOR_PROPERTY in the class that
 *  declares the enum. A late or missing registration usually makes the
 *  compilation fail, either with an error about an explicit specialization
 *  after instantiation (of k4FWCore::EnumNames or `Gaudi::Utils::toStream`),
 *  or with a likely obscure error because k4FWCore::NamedEnum was evaluated as
 *  false or the printout is missing, e.g. about `no match for operator<<` in
 *  `toStream`. This is not guaranteed though, and translation units that
 *  disagree on the registration (e.g. because it is not visible everywhere the
 *  enum is used) are not diagnosed either, so register the enum right after its
 *  declaration, in the same header.
 *
 *  The name of a registered enumerator is converted to a string unquoted by
 *  `toString` (e.g. `Precise`) and quoted a'la Python by `toStream` (e.g.
 *  `'Precise'`). An enumerator that is not registered (which can only be
 *  assigned from C++) is converted to its underlying integer by both.
 */
namespace k4FWCore {

namespace details {
  /// An enum for which a function `k4FWCoreEnumNames(E)` is declared that can be found by argument-dependent lookup
  template <typename E>
  concept HasEnumNamesFunction = std::is_enum_v<E> && requires(E e) { decltype(k4FWCoreEnumNames(e))::values; };

  /// Boolean template variable to determine whether something can be used as
  /// an enum value -> name table
  template <typename E, typename T>
  inline constexpr bool isEnumNameTable = false;
  template <typename E, std::size_t N>
  inline constexpr bool isEnumNameTable<E, std::array<std::pair<E, std::string_view>, N>> = (N > 0);

  /** The *unqualified* name of the enumerator @p V.
   *
   *  For a value that is not a named enumerator, e.g. `static_cast<E>(7)`, the result is not an identifier.
   *
   * @note this uses source_location internally, so the implementation quite
   * literally depends on the function name. Currently supports GCC and Clang
   */
  template <auto V>
  consteval std::string_view enumValueName() {
    const std::string_view funcName = std::source_location::current().function_name();

    // function_name() is not standardized, so we have to try and parse the
    // following versions
    //  - GCC:   "... k4FWCore::details::enumValueName() [with auto V = Mode::Fast; std::string_view = ...]"
    //  - Clang: "... k4FWCore::details::enumValueName() [V = Mode::Fast]"
    //           or "... k4FWCore::details::enumValueName<Mode::Fast>()"
    std::size_t start, end;
    if (const auto p = funcName.find("V = "); p != funcName.npos) {
      start = p + 4;
      end = funcName.find_first_of(";]", start);
    } else {
      constexpr std::string_view prefix = "enumValueName<";
      start = funcName.find(prefix) + prefix.size();
      end = funcName.find('>', start);
    }
    auto name = funcName.substr(start, end - start);

    if (const auto colon = name.rfind("::"); colon != name.npos) {
      name.remove_prefix(colon + 2);
    }
    return name;
  }

  /// Whether @p name is a non-empty C++ identifier (ASCII only)
  consteval bool isIdentifier(std::string_view name) {
    if (name.empty()) {
      return false;
    }
    const auto isAlpha = [](const char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; };
    const auto isDigit = [](const char c) { return c >= '0' && c <= '9'; };
    if (!isAlpha(name.front())) {
      return false;
    }
    return std::ranges::all_of(name, [&](const char c) { return isAlpha(c) || isDigit(c); });
  }

  /// Whether all names in a `values` table of k4FWCore::EnumNames are different
  template <typename R>
  consteval bool hasUniqueNames(const R& values) {
    for (auto i = std::ranges::begin(values); i != std::ranges::end(values); ++i) {
      for (auto j = std::ranges::next(i); j != std::ranges::end(values); ++j) {
        if (i->second == j->second) {
          return false;
        }
      }
    }
    return true;
  }
} // namespace details

/** The enumerators of an enum that can be selected by name, e.g. in a Gaudi::Property<E>.
 *
 *  A specialization provides a `static constexpr` member `values` of type
 *  `std::array<std::pair<E, std::string_view>, N>` that maps each selectable enumerator to its name, in the order
 *  in which the names are listed by k4FWCore::enumNameList. Usually a specialization derives from
 *  k4FWCore::Enumerators. The enum then also has to be registered with K4FWCORE_REGISTER_ENUM_FOR_PROPERTY.
 *
 *  If it is not specialized, `values` is taken from the return type of `k4FWCoreEnumNames(E)`, if such a function is
 *  found by argument-dependent lookup, and is missing otherwise.
 *
 *  @tparam E the enum type
 */
template <typename E>
struct EnumNames {};

template <details::HasEnumNamesFunction E>
struct EnumNames<E> : decltype(k4FWCoreEnumNames(std::declval<E>())){};

/// An enum registered with k4FWCore::EnumNames (by a specialization or via
/// `k4FWCoreEnumNames`), which can be the type of a Gaudi::Property once it is
/// also registered with K4FWCORE_REGISTER_ENUM_FOR_PROPERTY or
/// K4FWCORE_REGISTER_ALG_ENUM_FOR_PROPERTY
template <typename E>
concept NamedEnum = std::is_enum_v<E> && requires { EnumNames<E>::values; } &&
                    details::isEnumNameTable<E, std::remove_cv_t<decltype(EnumNames<E>::values)>>;

/** Register enumerators with k4FWCore::EnumNames under the names they have in
 *  the code, either by deriving a specialization of k4FWCore::EnumNames from it,
 *  or as the return type of `k4FWCoreEnumNames`.
 *
 *  @tparam V0, Vs the enumerators that can be selected, in the order in which they are listed by
 *                 k4FWCore::enumNameList. They have to be named enumerators of the same enum, with distinct values.
 */
template <auto V0, auto... Vs>
  requires(std::is_enum_v<decltype(V0)> && (std::same_as<decltype(V0), decltype(Vs)> && ...))
struct Enumerators {
  using EnumType = decltype(V0);

  static constexpr std::array<std::pair<EnumType, std::string_view>, 1 + sizeof...(Vs)> values{
      {{V0, details::enumValueName<V0>()}, {Vs, details::enumValueName<Vs>()}...}};

  static_assert(details::isIdentifier(details::enumValueName<V0>()) &&
                    (details::isIdentifier(details::enumValueName<Vs>()) && ...),
                "k4FWCore::Enumerators: all values must be named enumerators (a value like static_cast<E>(7) "
                "has no name)");
  static_assert(details::hasUniqueNames(values),
                "k4FWCore::Enumerators: all values must be different (enumerators with the same value have the same "
                "name)");
};

/** Look up the enumerator with the given name (exact, case-sensitive match)
 *
 *  @param name the name of the enumerator, as registered with k4FWCore::EnumNames
 *  @return the enumerator, or std::nullopt if no enumerator has this name
 */
template <NamedEnum E>
constexpr std::optional<E> enumFromName(std::string_view name) {
  for (const auto& [value, valueName] : EnumNames<E>::values) {
    if (valueName == name) {
      return value;
    }
  }
  return std::nullopt;
}

/** Look up the name of an enumerator
 *
 *  @param value the enum value, as registered with k4FWCore::EnumNames
 *  @return the name of the enumerator as registered with k4FWCore::EnumNames
 *          (the first one, if it is registered more than once), or
 *          std::nullopt if it is not registered
 */
template <NamedEnum E>
constexpr std::optional<std::string_view> enumToName(E value) {
  for (const auto& [v, name] : EnumNames<E>::values) {
    if (v == value) {
      return name;
    }
  }
  return std::nullopt;
}

/// The names registered with k4FWCore::EnumNames rendered as `'A', 'B', 'C'`,
/// e.g. for property documentation
template <NamedEnum E>
std::string enumNameList() {
  std::string result;
  for (const auto& [value, name] : EnumNames<E>::values) {
    if (!result.empty()) {
      result += ", ";
    }
    result += '\'';
    result += name;
    result += '\'';
  }
  return result;
}

namespace details {
  /// The printout of an enum registered with k4FWCore::EnumNames: the name of the enumerator, quoted a'la Python (an
  /// unregistered value is printed as its underlying integer). The implementation of the `toStream` that
  /// K4FWCORE_REGISTER_ENUM_FOR_PROPERTY and K4FWCORE_REGISTER_ALG_ENUM_FOR_PROPERTY define
  template <NamedEnum E>
  std::ostream& enumToStream(const E& obj, std::ostream& s) {
    if (const auto name = k4FWCore::enumToName(obj)) {
      return Gaudi::Utils::toStream(std::string{*name}, s);
    }
    return s << +static_cast<std::underlying_type_t<E>>(obj);
  }
} // namespace details

} // namespace k4FWCore

namespace Gaudi::Parsers {
/** parse an enum registered with k4FWCore::EnumNames from the name of an enumerator
 *  (quoted or unquoted)
 *  @see k4FWCore/EnumProperty.h
 *  @param result (output) the enumerator, unchanged in case of failure
 *  @param input  (input) the string to be parsed
 *  @return status code
 */
template <k4FWCore::NamedEnum E>
StatusCode parse(E& result, std::string_view input) {
  std::string name;
  if (parse(name, input).isFailure()) {
    return StatusCode::FAILURE;
  }
  if (const auto value = k4FWCore::enumFromName<E>(name)) {
    result = *value;
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

/** helper for the parsers of containers of enums registered with k4FWCore::EnumNames
 *
 *  The input is parsed as @p Names, a container of strings, with the existing parsers. Then each name is parsed
 *  into an element of @p Container. @p result is only changed if all elements could be parsed.
 *
 *  @tparam Names     the container of strings, e.g. std::vector<std::string>
 *  @tparam Container the container of enums, e.g. std::vector<E>
 */
template <typename Names, typename Container>
StatusCode parseNamedEnums_(Container& result, std::string_view input) {
  Names names;
  if (parse(names, input).isFailure()) {
    return StatusCode::FAILURE;
  }
  Container values;
  for (const auto& name : names) {
    typename Container::value_type value{};
    if (parse(value, name).isFailure()) {
      return StatusCode::FAILURE;
    }
    values.insert(values.end(), std::move(value));
  }
  result = std::move(values);
  return StatusCode::SUCCESS;
}

/// parse a std::vector of enums registered with k4FWCore::EnumNames from a list of names
template <k4FWCore::NamedEnum E>
StatusCode parse(std::vector<E>& result, std::string_view input) {
  return parseNamedEnums_<std::vector<std::string>>(result, input);
}

/// parse a std::set of enums registered with k4FWCore::EnumNames from a list of names
template <k4FWCore::NamedEnum E>
StatusCode parse(std::set<E>& result, std::string_view input) {
  return parseNamedEnums_<std::set<std::string>>(result, input);
}
} // namespace Gaudi::Parsers

/** The conversion of a Gaudi::Property of an enum registered with k4FWCore::EnumNames from and to a string.
 *
 *  toString gives the name of the enumerator, unquoted (an unregistered value
 *  is converted to its underlying integer). fromString accepts the name of a
 *  registered enumerator (quoted or unquoted), otherwise it throws a
 *  std::invalid_argument that lists the allowed names.
 *  @see k4FWCore/EnumProperty.h
 */
template <k4FWCore::NamedEnum E>
struct Gaudi::Details::Property::StringConverter<E> {
  std::string toString(const E& obj) const {
    if (const auto name = k4FWCore::enumToName(obj)) {
      return std::string{*name};
    }
    return std::to_string(+static_cast<std::underlying_type_t<E>>(obj));
  }

  E fromString(const E& /* refValue */, const std::string& s) const {
    using Gaudi::Parsers::InputData;
    E buffer{};
    if (!parse(buffer, InputData{s}).isSuccess()) {
      throw std::invalid_argument("cannot parse '" + s + "' to " + System::typeinfoName(typeid(E)) +
                                  ", expected one of " + k4FWCore::enumNameList<E>());
    }
    return buffer;
  }
};

/** Register an enum whose names are registered with k4FWCore::EnumNames, so
 *  that it can be the type of a Gaudi::Property
 *
 *  @code
 *  enum class Mode { Fast, Precise };
 *  template <>
 *  struct k4FWCore::EnumNames<Mode> : k4FWCore::Enumerators<Mode::Fast, Mode::Precise> {};
 *  K4FWCORE_REGISTER_ENUM_FOR_PROPERTY(Mode);
 *  // The user writes "Fast" or "Precise"
 *  Gaudi::Property<Mode> m_mode{this, "Mode", Mode::Fast, "One of " + k4FWCore::enumNameList<Mode>()};
 *  @endcode
 *
 *  It has to be used at global namespace scope, after the specialization of
 *  k4FWCore::EnumNames and before the first use of the enum as the type of a
 *  Gaudi::Property (see the documentation at the top of
 *  k4FWCore/EnumProperty.h). For an enum declared inside a class use
 *  K4FWCORE_REGISTER_ALG_ENUM_FOR_PROPERTY.
 *
 *  Gaudi::Property prints its value with Gaudi::Utils::toStream, whose generic
 *  version uses `operator<<`. A generic overload for all named enums would not
 *  be found from inside Gaudi, so this macro defines an explicit specialization
 *  of Gaudi::Utils::toStream for @p E, see k4FWCore::details::enumToStream.
 *  Parsing needs no registration.
 *
 *  @param E the enum type
 */
#define K4FWCORE_REGISTER_ENUM_FOR_PROPERTY(E)                                                                         \
  static_assert(k4FWCore::NamedEnum<E>, "K4FWCORE_REGISTER_ENUM_FOR_PROPERTY: specialize k4FWCore::EnumNames<" #E      \
                                        "> before registering the enum");                                              \
  template <>                                                                                                          \
  inline std::ostream& Gaudi::Utils::toStream<E>(const E& obj, std::ostream& s) {                                      \
    return k4FWCore::details::enumToStream(obj, s);                                                                    \
  }                                                                                                                    \
  static_assert(true, "require a semicolon after the macro")

/** Register an enum declared inside a class, whose names are registered via
 *  `k4FWCoreEnumNames`, so that it can be the type of a Gaudi::Property
 *
 *  @code
 *  struct MyAlg : k4FWCore::Consumer<void()> {
 *    enum class Mode { Fast, Precise };
 *    friend constexpr k4FWCore::Enumerators<Mode::Fast, Mode::Precise> k4FWCoreEnumNames(Mode) { return {}; }
 *    K4FWCORE_REGISTER_ALG_ENUM_FOR_PROPERTY(Mode);
 *    // The user writes "Fast" or "Precise"
 *    Gaudi::Property<Mode> m_mode{this, "Mode", Mode::Fast, "One of " + k4FWCore::enumNameList<Mode>()};
 *  };
 *  @endcode
 *
 *  It has to be used inside the class that declares the enum, after
 *  `k4FWCoreEnumNames` and before the first use of the enum as the type of a
 *  Gaudi::Property (see the documentation at the top of
 *  k4FWCore/EnumProperty.h). This also works if the enum is the type of a
 *  property of the same class. It defines the friend function
 *  `toStream(const E&, std::ostream&)`, which is found by argument-dependent
 *  lookup and prints the enum, see k4FWCore::details::enumToStream.
 *
 *  @param E the enum type
 */
#define K4FWCORE_REGISTER_ALG_ENUM_FOR_PROPERTY(E)                                                                     \
  static_assert(k4FWCore::NamedEnum<E>, "K4FWCORE_REGISTER_ALG_ENUM_FOR_PROPERTY: declare k4FWCoreEnumNames(" #E       \
                                        ") before registering the enum");                                              \
  friend std::ostream& toStream(const E& obj, std::ostream& s) { return k4FWCore::details::enumToStream(obj, s); }     \
  static_assert(true, "require a semicolon after the macro")

#endif // K4FWCORE_ENUMPROPERTY_H
