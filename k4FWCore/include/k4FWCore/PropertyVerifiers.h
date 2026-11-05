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
/// Gaudi::Property<std::string, k4FWCore::OneOf<{"DCH", "STT"}>> m_tracker{this, "Tracker", "DCH"};
/// @endcode
///
/// A complete, compiled and tested example is given in
/// test/k4FWCoreTest/src/components/ExampleFunctionalPropertyVerifiers.cpp, with the job options in
/// test/k4FWCoreTest/options/ExampleFunctionalPropertyVerifiers.py.

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace k4FWCore {

namespace detail {
  /// A string literal that can be used as a template argument. A std::string or a plain
  /// string literal cannot be, but a class holding the characters in an array can. It can hold any null terminated
  /// character array that fits into its N characters (including the terminating null character).
  template <std::size_t N>
  struct FixedString {
    char data[N]{};
    std::size_t size{};

    constexpr FixedString() = default;
    template <std::size_t M>
      requires(M <= N)
    constexpr FixedString(const char (&str)[M]) : size(M - 1) {
      // Without this check the last character of an array that is not null terminated would be silently dropped. As
      // FixedString is created at compile time, throwing here makes the compilation fail
      if (str[M - 1] != '\0') {
        throw std::invalid_argument("FixedString: the character array has to be null terminated");
      }
      std::copy_n(str, M, data);
    }
    constexpr std::string_view view() const { return {data, size}; }
  };

  /// @brief Allowed values of OneOf.
  ///
  /// Integers and string literals cannot both be taken by an `auto` template parameter (a string literal would decay
  /// to a pointer, which is not allowed), and neither can a list of values, so they are stored in this class. Its
  /// deduction guides create it from a single value (`4`, `"DCH"`), a braced list (`{4, 8}`, `{"DCH", "STT"}`) or a
  /// `std::array` of integers.
  ///
  /// @tparam T The type in which the values are stored, an integer type or a FixedString
  /// @tparam N The number of values
  template <typename T, std::size_t N>
    requires(N > 0)
  struct Choices {
    std::array<T, N> values{};

    template <typename... Us>
      requires(sizeof...(Us) == N && (std::constructible_from<T, const Us&> && ...))
    constexpr Choices(const Us&... vs) : values{T(vs)...} {}

    template <typename U>
    constexpr Choices(const std::array<U, N>& vs) {
      std::copy(vs.begin(), vs.end(), values.begin());
    }

    /// The i-th allowed value, as the property value is compared to it
    constexpr auto get(std::size_t i) const {
      if constexpr (requires { values[i].view(); }) {
        return values[i].view();
      } else {
        return values[i];
      }
    }

    /// Whether @p value is one of the allowed values
    template <typename V>
    constexpr bool contains(const V& value) const {
      for (std::size_t i = 0; i < N; ++i) {
        if (value == get(i)) {
          return true;
        }
      }
      return false;
    }
  };

  template <std::integral U, std::integral... Us>
  Choices(U, Us...) -> Choices<std::common_type_t<U, Us...>, 1 + sizeof...(Us)>;
  // The strings are stored in FixedStrings that fit the longest one
  template <std::size_t... Ms>
  Choices(const char (&... strs)[Ms]) -> Choices<FixedString<std::max({Ms...})>, sizeof...(Ms)>;
  template <std::integral U, std::size_t N>
  Choices(std::array<U, N>) -> Choices<U, N>;

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

  /// The integer types that OneOf accepts: all integral types except bool and the character types, whose values
  /// would be listed as numbers. `signed char` and `unsigned char` are accepted, as they are std::int8_t and
  /// std::uint8_t
  template <typename T>
  concept IntegerChoice =
      std::integral<T> && !std::same_as<T, bool> && !std::same_as<T, char> && !std::same_as<T, wchar_t> &&
      !std::same_as<T, char8_t> && !std::same_as<T, char16_t> && !std::same_as<T, char32_t>;
} // namespace detail

/// @brief A verifier for a Gaudi::Property that only accepts a fixed set of integers or strings.
///
/// Gaudi calls the verifier of a property whenever a value is set: when the property is constructed with its
/// default, and when it is assigned, which includes setting it from the steering file or the command line. An
/// invalid value is rejected with a std::invalid_argument that names the value and the allowed ones, e.g.
/// `2 is not one of 0, 1` or `'TPC' is not one of 'DCH', 'STT'`, and the property keeps its previous value.
///
/// The allowed values can be given one by one, as a braced list, or as a `std::array` of integers, and these can
/// be combined:
/// @code
/// Gaudi::Property<int, k4FWCore::OneOf<0, 1>> m_numBreakfasts{this, "NumBreakfasts", 1};
/// Gaudi::Property<int, k4FWCore::OneOf<{0, 1}>> m_numLunches{this, "NumLunches", 1};
/// Gaudi::Property<std::string, k4FWCore::OneOf<{"DCH", "STT"}>> m_tracker{this, "Tracker", "DCH"};
///
/// static constexpr std::array layerCounts{4, 8, 15};
/// Gaudi::Property<int, k4FWCore::OneOf<layerCounts>> m_numLayers{this, "NumLayers", 4};
/// @endcode
///
/// The allowed values have to be either all integers (but not `bool` or characters) or all strings. A `std::array` of
/// strings cannot be used, as the lengths of the strings would have to be known from its type. Floating point numbers
/// are not supported, because they cannot be compared reliably with `==`.
///
/// @tparam Allowed The allowed values, in the order in which they are listed by list()
template <detail::Choices... Allowed>
  requires(sizeof...(Allowed) > 0 && ((detail::IntegerChoice<decltype(Allowed.get(0))> && ...) ||
                                      (std::same_as<decltype(Allowed.get(0)), std::string_view> && ...)))
struct OneOf {
  /// Throw std::invalid_argument if @p value is not one of the allowed values
  template <typename T>
    requires(std::equality_comparable_with<const T&, decltype(Allowed.get(0))> && ...)
  void operator()(const T& value) const {
    if (!(Allowed.contains(value) || ...)) {
      throw std::invalid_argument(detail::formatChoice(value) + " is not one of " + list());
    }
  }

  /// The allowed values rendered as `1, 2, 3` or `'A', 'B', 'C'`, e.g. for property documentation
  static std::string list() {
    std::string list;
    (
        [&list] {
          for (std::size_t i = 0; i < Allowed.values.size(); ++i) {
            list += (list.empty() ? "" : ", ") + detail::formatChoice(Allowed.get(i));
          }
        }(),
        ...);
    return list;
  }
};

} // namespace k4FWCore

#endif // FWCORE_PROPERTYVERIFIERS_H
