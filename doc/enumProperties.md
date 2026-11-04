<!--
Copyright (c) 2014-2024 Key4hep-Project.

This file is part of Key4hep.
See https://key4hep.github.io/key4hep-doc/ for further info.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->

# Enum properties

Properties that select a behaviour of an algorithm, e.g. a mode of operation or
a method, often have only a few valid values. Using an `enum` as the type of the
property is preferable over using a `std::string`:

- Only the allowed values can be set. Any other value is rejected when the
  property is set, i.e. before the algorithm runs, with an error that lists the
  allowed values. There is no need to validate the value in `initialize`.
- The code branches on the enumerators, with the usual compiler support (e.g.
  warnings about unhandled cases in a `switch`), instead of comparing strings.
- In the steering file the values are still set by name.

This is implemented in `k4FWCore/EnumProperty.h`. All names below are in the
`k4FWCore` namespace.

## Registering an enum

An enum can only be the type of a `Gaudi::Property` after it has been
registered. The names that can be used in the steering file are the names of the
enumerators. There are two ways to register an enum.

The first one is the `K4FWCORE_ENUM_PROPERTY` macro, which takes the enum and
the enumerators that can be selected. It has to be used at global scope, so it
works for an enum declared at namespace scope:

```cpp
#include "k4FWCore/EnumProperty.h"

enum class Gandalf { TheGrey, TheWhite };

K4FWCORE_ENUM_PROPERTY(Gandalf, TheGrey, TheWhite);
```

The second one is the `K4FWCORE_ENUM_PROPERTY_FRIEND` macro, which is used
inside a class, next to the enum. It defines two friend functions, which are
found by argument-dependent lookup. Here the enumerators have to be given with
the name of the enum. This is the only way to register an enum that is declared
inside the algorithm that uses it:

```cpp
struct ExampleFunctionalEnumProperties final : k4FWCore::Consumer<void()> {
  enum class Resurrection { NoResurrection, WithResurrection };
  K4FWCORE_ENUM_PROPERTY_FRIEND(Resurrection, Resurrection::NoResurrection, Resurrection::WithResurrection);
  // ...
};
```

The registration has to come after the enum and before its first use as the type
of a property. Otherwise the compilation usually fails, e.g. with an error about
a specialization after instantiation, or with an obscure error about a missing
`operator<<` in `toStream`.

## Using enum properties

Once registered, an enum can be used as the type of a property, alone or in a
`std::vector` (or a `std::set`). `k4FWCore::enumNameList<E>()` gives the list of
allowed names (`'TheGrey', 'TheWhite'`), which is useful for the documentation
string of the property:

```cpp
Gaudi::Property<Gandalf> m_gandalf{this, "Gandalf", Gandalf::TheGrey, "One of " + k4FWCore::enumNameList<Gandalf>()};
Gaudi::Property<Resurrection> m_resurrection{this, "SurvivalRate", Resurrection::WithResurrection,
                                             "One of " + k4FWCore::enumNameList<Resurrection>()};
Gaudi::Property<std::vector<Member>> m_fellowship{this, "Fellowship", {Member::Frodo, Member::Sam},
                                                  "Any of " + k4FWCore::enumNameList<Member>()};
```

In the code the property holds the enumerator:

```cpp
const auto* wizard = m_gandalf == Gandalf::TheWhite ? "the White" : "the Grey";
```

`toString()` of the property gives the name of the enumerator (e.g. `TheWhite`),
for a list it gives `[ 'Frodo' , 'Sam' ]`. If the name of a single value is
needed, `k4FWCore::enumToName(value)` can be used, and `k4FWCore::enumFromName<E>(name)`
for the other direction.

## Setting enum properties

In the steering file the properties are set by name:

::::{tab-set}
:::{tab-item} Python
```python
alg = ExampleFunctionalEnumProperties(
    "ExampleFunctionalEnumProperties",
    Gandalf="TheWhite",
    SurvivalRate="WithResurrection",
    Fellowship=["Frodo", "Sam", "Gandalf"],
)
```
:::
:::{tab-item} CLI
```sh
k4run <steering-file> --ExampleFunctionalEnumProperties.Gandalf TheWhite \
  --ExampleFunctionalEnumProperties.Fellowship Frodo Sam Gandalf
```
:::
::::

Setting a name that is not registered results in an error, and the job stops:

```
ERROR Property::fromString 	 Cannot convert ''TheBlue'' for property 'Gandalf' in class 'ExampleFunctionalEnumProperties': std::invalid_argument, cannot parse ''TheBlue'' to (anonymous namespace)::Gandalf, expected one of 'TheGrey', 'TheWhite'
```

For a list the error contains the whole value, but not the allowed names.

A complete example can be found
[here](https://github.com/key4hep/k4FWCore/blob/main/test/k4FWCoreTest/src/components/ExampleFunctionalEnumProperties.cpp),
with the corresponding [steering
file](https://github.com/key4hep/k4FWCore/blob/main/test/k4FWCoreTest/options/ExampleFunctionalEnumProperties.py).
For more details, e.g. restricting the selectable values to a subset of the
enumerators, see the documentation in `k4FWCore/EnumProperty.h`.
