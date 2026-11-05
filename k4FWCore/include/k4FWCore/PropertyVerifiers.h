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
#ifndef FWCORE_PROPERTYVERIFIERS_H
#define FWCORE_PROPERTYVERIFIERS_H

/// @file
/// @brief Verifiers that restrict a Gaudi::Property to a fixed set of allowed values.
///
/// They are used as the `VERIFIER` template argument of Gaudi::Property, so the properties stay plain
/// Gaudi::Property objects. An invalid value is rejected as soon as it is set, i.e. before initialize() is called,
/// and the job fails with an error naming the property, the given value and the allowed ones. An invalid default
/// does not even let the component be constructed.
///
/// @code
/// Gaudi::Property<int, k4FWCore::OneOf<4, 8>> m_numLayers{this, "NumLayers", 4,
///                                                         "One of " + k4FWCore::OneOf<4, 8>::list()};
/// Gaudi::Property<std::string, k4FWCore::OneOf<"DCH", "STT">> m_tracker{this, "Tracker", "DCH"};
/// @endcode
///
/// A complete, compiled and tested example is given in
/// test/k4FWCoreTest/src/components/ExampleFunctionalPropertyVerifiers.cpp, with the job options in
/// test/k4FWCoreTest/options/ExampleFunctionalPropertyVerifiers.py.

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace k4FWCore {

namespace detail {
  /// A string literal that can be used as a template argument. A std::string or a plain
  /// string literal cannot be, but a class holding the characters in an array can.
  template <std::size_t N>
  struct FixedString {
    char data[N]{};
    constexpr FixedString(const char (&str)[N]) { std::copy_n(str, N, data); }
    constexpr std::string_view view() const { return {data, N - 1}; }
  };

  /// An allowed value of OneOf. Integers and string literals cannot both be taken by an `auto` template
  /// parameter (a string literal would decay to a pointer, which is not allowed), so they are wrapped in this class,
  /// whose deduction guides pick how to store them.
  template <typename T>
  struct Choice {
    T value;

    template <typename U>
    constexpr Choice(const U& v) : value(v) {}

    /// The allowed value, as the property value is compared to it
    constexpr auto get() const {
      if constexpr (requires { value.view(); }) {
        return value.view();
      } else {
        return value;
      }
    }
  };

  template <std::integral T>
  Choice(T) -> Choice<T>;
  template <std::size_t N>
  Choice(const char (&)[N]) -> Choice<FixedString<N>>;

  /// Render a value for property documentation and error messages: strings are quoted (`'A'`), integers are
  /// given as is (`1`)
  template <typename T>
  std::string formatChoice(const T& value) {
    if constexpr (std::is_convertible_v<const T&, std::string_view>) {
      return "'" + std::string{std::string_view{value}} + "'";
    } else {
      return std::to_string(value);
    }
  }

  /// Render values as a comma separated list with formatChoice
  template <typename... Ts>
  std::string formatChoices(const Ts&... values) {
    std::string list;
    ((list += (list.empty() ? "" : ", ") + formatChoice(values)), ...);
    return list;
  }
} // namespace detail

/// @brief A verifier for a Gaudi::Property that only accepts a fixed set of integers or strings.
///
/// Gaudi calls the verifier of a property whenever a value is set: when the property is constructed with its
/// default, and when it is assigned, which includes setting it from the steering file or the command line. An
/// invalid value is rejected with a std::invalid_argument that names the value and the allowed ones, e.g.
/// `2 is not one of 0, 1` or `'TPC' is not one of 'DCH', 'STT'`, and the property keeps its previous value.
/// @code
/// Gaudi::Property<int, k4FWCore::OneOf<0, 1>> m_numBreakfasts{this, "NumBreakfasts", 1};
/// Gaudi::Property<std::string, k4FWCore::OneOf<"DCH", "STT">> m_tracker{this, "Tracker", "DCH"};
/// @endcode
///
/// The allowed values have to be either all integers or all strings. Floating point numbers are not supported,
/// because they cannot be compared reliably with `==`.
///
/// @tparam Allowed The allowed values, in the order in which they are listed by list()
template <detail::Choice... Allowed>
  requires(sizeof...(Allowed) > 0 && ((std::integral<decltype(Allowed.get())> && ...) ||
                                      (std::same_as<decltype(Allowed.get()), std::string_view> && ...)))
struct OneOf {
  /// Throw std::invalid_argument if @p value is not one of the allowed values
  template <typename T>
    requires(std::equality_comparable_with<const T&, decltype(Allowed.get())> && ...)
  void operator()(const T& value) const {
    if (!((value == Allowed.get()) || ...)) {
      throw std::invalid_argument(detail::formatChoice(value) + " is not one of " + list());
    }
  }

  /// The allowed values rendered as `1, 2, 3` or `'A', 'B', 'C'`, e.g. for property documentation
  static std::string list() { return detail::formatChoices(Allowed.get()...); }
};

} // namespace k4FWCore

#endif // FWCORE_PROPERTYVERIFIERS_H
