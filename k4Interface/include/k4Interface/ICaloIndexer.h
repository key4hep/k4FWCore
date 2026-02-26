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
/**
 * @file k4Interface/ICaloIndexer.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2026
 * @brief Interface for cell indexing.
 */

#ifndef K4INTERFACE_ICALOINDEXER_H
#define K4INTERFACE_ICALOINDEXER_H

#include <cstdint>
#include <memory>
#include <span>

/**
 * @brief Interface for cell indexing.
 *
 * Given a list of cell IDs, this defines a fast ID->index mapping.
 * In particular, for a valid identifier @c id,
 * <code>cellIDs()[index(id)] == id</code>.
 */
class ICaloIndexer {
public:
  /// Index type.
  using index_t = uint32_t;
  static constexpr index_t INVALID = static_cast<index_t>(-1);

  virtual ~ICaloIndexer() = default;

  /**
   * @brief Return the index of an identifier.
   * @param id The identifier to look for.
   *
   * Returns the index of @c id in @c cellIDs(), or @c INVALID.
   */
  virtual index_t index(uint64_t id) const = 0;

  /**
   * @brief Return the set of all identifiers that we index.
   */
  virtual std::span<const uint64_t> cellIDs() const = 0;

  /**
   * @brief Return the IDs of the detector(s) that we index.
   */
  virtual std::span<const int> detIDs() const = 0;

  /**
   * @brief Number of bits in the cell ID used for the detector ID.
   */
  virtual size_t detIDBits() const = 0;
};

#endif // not K4INTERFACE_ICALOINDEXER_H
