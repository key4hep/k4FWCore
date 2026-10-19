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
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace k4FWCore {

/// @brief Restrict a Gaudi property to a fixed set of allowed values.
///
/// Minimal stand-in class for the `choices` argument of Python's
/// argparse.add_argument(). A PropertyChoices is a table that maps every value
/// the user may give to a property (the "key") to the value the code works with
/// (the "value"). It is the one source of truth for
///  - validating the property,
///  - the list of allowed values quoted in the property documentation, and
///  - the list of allowed values quoted in the error message.
///
/// It can be used in the initialize() method of a Gaudi Functional to ensure
/// that a given property is filled with one of the allowed values.
///
/// Both the key and value type are template parameters for PropertyChoices, so technically any copyable type
/// can be used. Floating point *values* are fine, but floating point *keys* are not supported at the moment,
/// because they cannot be compared reliably with `==`. There could be a solution with a tolerance, but this
/// would come with some overhead, so it is not implemented yet.
///
/// Usage always has the same three steps, independent of the types involved:
/// @code
/// // 1. First, create the table of allowed values once (static, so that it can be used
/// //    in the property documentation). There are two ways of creating the table, explained below.
/// //    Here the example uses the helper function makePropertyChoices.
/// static inline const auto s_choices = k4FWCore::makePropertyChoices<...>();
///
/// // 2. Declare the property and document the allowed values with list()
/// Gaudi::Property<...> m_property{this, "Name", default, "One of " + s_choices.list()};
///
/// // 3. In initialize(), validate the property and get the value for it. On an invalid value
/// //    an error naming the property and the allowed values is logged and the job fails
/// K4_GAUDI_CHECK(s_choices.parse(m_property, m_value, this));
/// @endcode
///
/// @par Choosing how to create the table
///
/// How PropertyChoices works is that ultimately the underlying Gaudi::Property becomes the 'key' to the PropertyChoices
/// table, and the 'value' is what the code works with afterwards. Therefore, in the Gaudi steering file, the user input
/// is the table key, and the type of the Gaudi::Property must match the 'key' type in the PropertyChoices. For the
/// algorithm developer, there are two ways to create the key-value pairs from the user input.
///
/// **1. Use the makePropertyChoices helper function**
/// This is a helper function, which automatically creates the key-value pairs from the values given as template
/// arguments. The key is then derived from the value, and its type is determined by the type of the value. Say, you
/// pass a list of integers as allowed values, then the key type will be the same integer. Or when using with an enum,
/// the key type will be a string and the key itself will be the name of the enumerator without the enum class name. So
/// it is basically a helper function to save some hassle of having to write the key-value pairs explicitly, which can
/// be tedious for large lists of allowed values. Examples:
/// - **An enum, by the name of its enumerators.** The property is a `std::string`, the user writes `"Fast"` and
///   the developer can get the corresponding enumerator `Mode::Fast`:
///   `makePropertyChoices<Mode::Fast, Mode::Precise>()`
/// - **One of a few integers.** The property is an `int`, the user writes `15` and the developer can get the
///   corresponding integer `15`:
///   `makePropertyChoices<4, 8, 15>()`
/// - **One of a few strings.** The property is a `std::string`, the user writes `"DCH_info"` and the developer can
///   get the corresponding string `"DCH_info"`:
///   `makePropertyChoices<"DCH_info", "STT_info">()`
///
/// **2. Write out the `{key, value}` pairs explicitly**
///
/// This is the most direct way to create the PropertyChoices.
/// It should be used if the developer wants full control over the keys and value pairs, deciding what type each should
/// be. At the moment, it is also the only way to use floating point numbers as values. Floating point numbers as keys
/// are not supported (see the notes at the end of this documentation). The explicit table works for any copyable value
/// type. If the developer does not pass the second template argument, the key type defaults to `std::string`. Note that
/// at the moment the template parameter order is <ValueType, KeyType>, which may be counter-intuitive. Examples:
/// - **An enum, by custom labels.** The user writes `2` and the developer can get the corresponding enumerator
///   `Mode::Precise`:
///   `PropertyChoices<Mode, int>{{1, Mode::Fast}, {2, Mode::Precise}}`
/// - **A label for a value of any type.** The user writes `"Argon"` and the developer can get the corresponding
///   value e.g. the `double` `0.0023`:
///   `PropertyChoices<double>{{"Argon", 0.0023}, {"ArgonIsobutane", 0.0075}}`
///
/// @par Complete example
///
/// A made up Gaudi functional using all of the above. The same code is compiled
/// and run as a test in test/k4FWCoreTest/src/components/ExampleFunctionalPropertyChoices.cpp.
/// @code
/// enum class Mode { Fast, Precise };
///
/// struct LayerSimulator final : k4FWCore::Producer<edm4hep::MCParticleCollection()> {
///   LayerSimulator(const std::string& name, ISvcLocator* svcLoc)
///       : Producer(name, svcLoc, {}, KeyValue("OutputCollection", "MCParticles")) {}
///
///   // (1) An enum selected by the name of the enumerators: the allowed strings are
///   //     "Fast" and "Precise", the table is built from the enumerator names
///   static inline const auto s_modes = k4FWCore::makePropertyChoices<Mode::Fast, Mode::Precise>();
///
///   // (2) A few allowed integers: the user writes the number, no label is involved
///   static inline const auto s_numLayers = k4FWCore::makePropertyChoices<4, 8, 15, 16, 23, 42>();
///
///   // (3) A few allowed strings, where the string itself is what the code uses,
///   //     e.g. to look up a detector or to name a collection
///   static inline const auto s_detectors = k4FWCore::makePropertyChoices<"DCH_info", "STT_info">();
///
///   // (4) Labels for values of a type that cannot be derived from a name. The user selects
///   //     the gas by name and the code gets the drift velocity (in mm/ns)
///   static inline const k4FWCore::PropertyChoices<double> s_driftVelocities{{"Argon", 0.0023},
///                                                                           {"ArgonIsobutane", 0.0075}};
///
///   // The properties. All of them document their allowed values with list()
///   Gaudi::Property<std::string> m_mode{this, "Mode", "Fast", "One of " + s_modes.list()};
///   Gaudi::Property<int> m_numLayers{this, "NumLayers", 8, "One of " + s_numLayers.list()};
///   Gaudi::Property<std::string> m_detector{this, "Detector", "DCH_info", "One of " + s_detectors.list()};
///   Gaudi::Property<std::string> m_gas{this, "Gas", "Argon", "One of " + s_driftVelocities.list()};
///
///   // What the code works with after validation, filled in initialize()
///   Mode m_modeValue{};
///   int m_numLayersValue{};
///   std::string m_detectorValue;
///   double m_driftVelocity{};
///
///   StatusCode initialize() override {
///     K4_GAUDI_CHECK(Producer::initialize());
///     K4_GAUDI_CHECK(s_modes.parse(m_mode, m_modeValue, this));
///     K4_GAUDI_CHECK(s_numLayers.parse(m_numLayers, m_numLayersValue, this));
///     K4_GAUDI_CHECK(s_detectors.parse(m_detector, m_detectorValue, this));
///     K4_GAUDI_CHECK(s_driftVelocities.parse(m_gas, m_driftVelocity, this));
///     return StatusCode::SUCCESS;
///   }
///
///   edm4hep::MCParticleCollection operator()() const override {
///     // ... use m_modeValue, m_numLayersValue, m_detectorValue and m_driftVelocity
///   }
/// };
/// @endcode
///
/// In the job options the allowed values are given as the user would expect:
/// @code{.py}
/// LayerSimulator("LayerSimulator", Mode="Precise", NumLayers=15, Detector="STT_info", Gas="ArgonIsobutane")
/// @endcode
/// and an invalid one, e.g. `NumLayers=9`, makes initialize() fail with
/// `Invalid NumLayers 9, expected one of 4, 8, 15, 16, 23, 42.`
///
/// @par Further notes
///  - parse(key) returns std::optional, if the validation is to be done without a Gaudi
///    property.
///  - Floating point *values* are fine, e.g. labels that map to a `double` (see (4) above).
///    Floating point *keys* are not supported at the moment, i.e. a `Gaudi::Property<double>` for
///    which the user types the number itself and that may only take a few allowed numbers. Keys
///    are looked up by exact comparison with `==`, which is not reliable for floating point
///    numbers: a number that went through arithmetic, or one that was rounded differently on
///    its way in (e.g. 0.1 + 0.2 and 0.3), differs in the last bits from the allowed one and is
///    rejected, with an error message that lists a value looking identical to what was given.
///    The class therefore refuses to compile for such keys (and makePropertyChoices for
///    floating point arguments), instead of giving results that are right only most of the time.
///    A possible solution is to compare with a tolerance, `|a - b| < eps`. It has issues of its
///    own, which is why it is not done here yet: a suitable `eps` depends on the context (an
///    absolute tolerance is wrong for numbers spanning many orders of magnitude, a relative one
///    for numbers close to zero), so it would have to be passed by the user. A tolerance also
///    makes the allowed values fuzzy, which can be surprising. If the allowed numbers are a
///    small fixed set, selecting them by label, as in (4), avoids the issue.
///  - The order of the entries is the order in which they are listed by list().
///
/// @tparam T   The type the keys are translated to. Any copyable type works, e.g. an
///             enum class, a number or a string
/// @tparam Key The type of the property and with it of the allowed values the user can
///             choose from. Defaults to std::string, which is what is needed whenever the
///             user selects by a name
template <typename T, typename Key = std::string>
class PropertyChoices {
  static_assert(!std::is_floating_point_v<Key>, "Floating point keys cannot be compared reliably");

public:
  using Choice = std::pair<Key, T>;

  /// Create a table from `{key, value}` entries, e.g.
  /// `PropertyChoices<Mode>{{"Fast", Mode::Fast}, {"Precise", Mode::Precise}}`
  PropertyChoices(std::initializer_list<Choice> choices) : m_choices(choices) {}

  /// Translate one of the allowed keys into its value, or return
  /// std::nullopt if the key is not one of the choices
  template <typename K>
    requires requires(const Key& key, const K& other) {
      { key == other } -> std::convertible_to<bool>;
    }
  std::optional<T> parse(const K& value) const {
    const auto it = std::ranges::find_if(m_choices, [&value](const Choice& choice) { return choice.first == value; });
    if (it == m_choices.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  /// Translate the value of a property into @p result. If the value is not one
  /// of the choices an error listing the valid ones is logged and
  /// StatusCode::FAILURE is returned, leaving @p result untouched.
  ///
  /// @param property The property holding the key to translate
  /// @param result   Where to store the translated value
  /// @param comp     The Gaudi component (algorithm, tool) that owns the
  ///                 property, typically "this"
  template <typename GaudiComp>
  StatusCode parse(const Gaudi::Property<Key>& property, T& result, const GaudiComp* comp) const {
    const auto parsed = parse(property.value());
    if (!parsed) {
      comp->error() << "Invalid " << property.name() << " " << format(property.value()) << ", expected one of "
                    << list() << "." << endmsg;
      return StatusCode::FAILURE;
    }
    result = *parsed;
    return StatusCode::SUCCESS;
  }

  /// The allowed keys rendered as `'A', 'B', 'C'` (strings are quoted) or
  /// `1, 2, 3`, for property documentation and error messages
  std::string list() const {
    std::vector<std::string> formatted;
    formatted.reserve(m_choices.size());
    for (const auto& choice : m_choices) {
      formatted.push_back(format(choice.first));
    }
    return fmt::format("{}", fmt::join(formatted, ", "));
  }

private:
  static std::string format(const Key& key) {
    if constexpr (std::is_convertible_v<const Key&, std::string_view>) {
      return fmt::format("'{}'", key);
    } else {
      return fmt::format("{}", key);
    }
  }

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

  /// A string literal that can be used as a template argument. A std::string or a plain
  /// string literal cannot be, but a class holding the characters in an array can.
  template <std::size_t N>
  struct FixedString {
    char data[N]{};
    constexpr FixedString(const char (&str)[N]) { std::copy_n(str, N, data); }
    constexpr std::string_view view() const { return {data, N - 1}; }
  };
} // namespace detail

/// @brief Create a PropertyChoices for a set of enum values or integers. The
/// keys are the values as they would be written in the code, which saves
/// writing every allowed value twice.
///
/// - For **enum values** the keys are the (unqualified) names of the enumerators, and the value
///   is the enumerator. The Gaudi property is a `std::string`.
///   @code
///   enum class Mode { Fast, Precise };
///   static inline const auto s_modes = k4FWCore::makePropertyChoices<Mode::Fast, Mode::Precise>();
///   // equivalent to
///   // k4FWCore::PropertyChoices<Mode> s_modes{{"Fast", Mode::Fast}, {"Precise", Mode::Precise}};
///   @endcode
///   Note that this only works for values that correspond to a named enumerator, and that the
///   strings the user has to type are always the enumerator names. For different labels create
///   the PropertyChoices explicitly.
///
/// - For **integers** the keys are the integers themselves and so are the values. The Gaudi
///   property is of the same integer type.
///   @code
///   static inline const auto s_numLayers = k4FWCore::makePropertyChoices<4, 8, 15, 16, 23, 42>();
///   Gaudi::Property<int> m_numLayers{this, "NumLayers", 8, "One of " + s_numLayers.list()};
///   @endcode
///
/// The order of the template arguments determines the order in which the
/// choices are listed by PropertyChoices::list().
///
/// @tparam Vs The enum values or integers that are allowed. They all need to be of the same
///            kind, and for enums of the same type, which becomes the value type of the
///            returned PropertyChoices
template <auto... Vs>
  requires((std::is_enum_v<decltype(Vs)> || std::is_integral_v<decltype(Vs)>) && ...)
auto makePropertyChoices() {
  using T = std::common_type_t<decltype(Vs)...>;
  if constexpr (std::is_enum_v<T>) {
    return PropertyChoices<T>{{std::string{detail::enum_value_name<Vs>()}, Vs}...};
  } else {
    return PropertyChoices<T, T>{{Vs, Vs}...};
  }
}

/// @brief Create a PropertyChoices for a set of allowed strings, where the string
/// itself is what the code uses afterwards (and not a label for something else).
///
/// @code
/// static inline const auto s_detectors = k4FWCore::makePropertyChoices<"DCH_info", "STT_info">();
/// Gaudi::Property<std::string> m_detector{this, "Detector", "DCH_info", "One of " + s_detectors.list()};
/// @endcode
///
/// Keys and values are both `std::string`. If the strings are labels for something else
/// create the PropertyChoices explicitly.
///
/// @tparam Ss The allowed strings
template <detail::FixedString... Ss>
auto makePropertyChoices() {
  return PropertyChoices<std::string, std::string>{{std::string{Ss.view()}, std::string{Ss.view()}}...};
}

} // namespace k4FWCore

#endif // FWCORE_PROPERTYCHOICES_H
