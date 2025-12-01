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
#ifndef FWCORE_TUPLEWRITER_H
#define FWCORE_TUPLEWRITER_H

#include "Gaudi/Functional/details.h"
#include "Gaudi/Functional/utilities.h"

// #include "GaudiKernel/CommonMessaging.h"

#include "k4FWCore/FunctionalUtils.h"

#include <algorithm>
#include <stdexcept>
#include <strings.h>
#include <type_traits>
#include <utility>
#include <variant>

#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <TFile.h>
#include <TTree.h>

namespace k4FWCore {

namespace details {

  template <typename Signature, typename Traits_>
  struct TupleWriter;

  template <typename... Out, typename... In, typename Traits_>
  struct TupleWriter<std::tuple<Out...>(const In&...), Traits_>
      : Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_> {
    using Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>::DataHandleMixin;

    static_assert(((std::is_base_of_v<podio::CollectionBase, In> || isVectorLike_v<In> ||
                    std::is_same_v<In, EventContext>) &&
                   ...),
                  "TupleWriter input types must be EDM4hep collections or vectors of collection pointers");

    static constexpr size_t N_in = filter_evtcontext<In...>::size;

    using base_class = Gaudi::Functional::details::DataHandleMixin<std::tuple<>, std::tuple<>, Traits_>;

    using KeyValue = base_class::KeyValue;
    using KeyValues = base_class::KeyValues;

    using NTupleTypes = std::variant<Out...>;

    template <typename T>
    using InputHandle_t = Gaudi::Functional::details::InputHandle_t<Traits_, T>;

    tuple_of_handle_vec<InputHandle_t, filter_evtcontext_t<In...>>::type m_inputs;

    std::array<Gaudi::Property<DataObjID>, N_in> m_inputLocationsSingle;
    std::array<Gaudi::Property<std::vector<DataObjID>>, N_in> m_inputLocationsVector;

    Gaudi::Property<std::string> m_outputFile{this, "OutputFile", "tree.root", "The name of the output ROOT file"};
    Gaudi::Property<std::vector<std::string>> m_Names{
        this, "Names", {"tree"}, "The name of the output ROOT tree or RNTuple"};
    Gaudi::Property<std::vector<std::string>> m_treeDescription{
        this, "Descriptions", {"TTree created from TupleWriter.h"}, "The name of the output ROOT tree"};
    Gaudi::Property<bool> m_RNTuple{this, "RNTuple", false,
                                    "If true, the output will be an RNTuple instead of a TTree"};

    template <typename IArgs, std::size_t... I>
    TupleWriter(std::string name, ISvcLocator* locator, const IArgs& inputs, std::index_sequence<I...>)
        : base_class(std::move(name), locator),
          // The input locations are filled by creating a property with a callback function
          // that creates the handles because that is when the input locations become available
          // (from a steering file, for example) and the handles have to be created for
          // Gaudi to know the data flow
          m_inputLocationsSingle{makeInputPropSingle<InputHandle_t<EventStoreType_t>, KeyValue>(
              std::get<I>(inputs), std::get<I>(m_inputs), this)...},
          m_inputLocationsVector{makeInputPropVector<InputHandle_t<EventStoreType_t>, KeyValues>(
              std::get<I>(inputs), std::get<I>(m_inputs), this)...} {}

    TupleWriter(std::string name, ISvcLocator* locator,
                const Gaudi::Functional::details::RepeatValues_<std::variant<KeyValue, KeyValues>, N_in>& inputs)
        : TupleWriter(std::move(name), locator, inputs, std::make_index_sequence<N_in>{}) {}

    // Prevent assigning with copy
    struct NonCopiableMap {
      NonCopiableMap() = default;
      NonCopiableMap(const NonCopiableMap&) = delete;
      NonCopiableMap& operator=(const NonCopiableMap&) = delete;
      NonCopiableMap(NonCopiableMap&&) = default;
      NonCopiableMap& operator=(NonCopiableMap&&) = default;
      NTupleTypes& operator[](const std::string& key) { return m_map[key]; }
      void fill() {
        if (m_parent)
          m_parent->fill(m_index);
      }
      std::map<std::string, NTupleTypes> m_map;
      int m_index{0};
      const TupleWriter* m_parent{nullptr};
    };

    struct Slot {
      NonCopiableMap map;
      bool initialized{false};
      bool filled{false};
      std::unique_ptr<TTree> tree;                 // TTree backend
      std::unique_ptr<ROOT::RNTupleWriter> writer; // RNTuple backend
      std::unique_ptr<ROOT::REntry> rentry;        // RNTuple entry
    };

    mutable std::unique_ptr<TFile> m_file{}; // Only for TTree backend
    mutable std::vector<Slot> m_slots;       // One per requested output
    mutable size_t m_currentMapIndex{0};

    void initializeSlots() const {
      if (m_slots.empty()) {
        if (m_Names.size() != m_treeDescription.size()) {
          throw std::runtime_error("TupleWriter: Name and TreeDescription properties must have the same size");
        }
        m_slots.resize(m_Names.size());
        for (size_t i = 0; i < m_slots.size(); ++i) {
          m_slots[i].map.m_index = static_cast<int>(i);
          m_slots[i].map.m_parent = this;
        }
        if (!m_RNTuple) {
          m_file = std::make_unique<TFile>(m_outputFile.value().c_str(), "RECREATE");
        }
      }
    }

    void initializeBackend(size_t index) const {
      initializeSlots();
      auto& slot = m_slots[index];
      if (slot.initialized)
        return;

      if (!m_RNTuple) {
        slot.tree = std::make_unique<TTree>(m_Names.value()[index].c_str(), m_treeDescription.value()[index].c_str());
        for (const auto& [key, var] : slot.map.m_map) {
          std::visit(
              [&slot, &key](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                slot.tree->Branch(key.c_str(), &std::get<T>(slot.map.m_map[key]));
              },
              var);
        }
      } else {
        auto model = ROOT::RNTupleModel::Create();
        for (const auto& [key, var] : slot.map.m_map) {
          std::visit(
              [&key, &model](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                model->MakeField<T>(key);
              },
              var);
        }
        model->Freeze();
        slot.rentry = model->CreateBareEntry();
        for (const auto& [key, var] : slot.map.m_map) {
          std::visit(
              [&slot, &key](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                slot.rentry->BindRawPtr(key, &std::get<T>(slot.map.m_map[key]));
              },
              var);
        }
        slot.writer = ROOT::RNTupleWriter::Recreate(std::move(model), m_Names[index], m_outputFile);
      }
      slot.initialized = true;
    }

    void fill(size_t index) const {
      initializeSlots();
      auto& slot = m_slots[index];
      if (!slot.initialized) {
        initializeBackend(index);
      }
      if (m_RNTuple) {
        slot.writer->Fill(*slot.rentry);
      } else {
        slot.tree->Fill();
      }
      slot.filled = true;
    }

    StatusCode execute(const EventContext& ctx) const final {
      m_currentMapIndex = 0;
      try {
        filter_evtcontext<In...>::apply(*this, ctx, m_inputs);
      } catch (GaudiException& e) {
        (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
      initializeSlots();
      for (size_t i = 0; i < m_slots.size(); ++i) {
        if (!m_slots[i].filled) {
          fill(i);
        }
        m_slots[i].filled = false; // reset for next event
      }
      return StatusCode::SUCCESS;
    }

    StatusCode finalize() final {
      if (!m_RNTuple) {
        if (!m_file)
          return StatusCode::SUCCESS; // no events processed
        m_file->cd();
        for (auto& slot : m_slots) {
          if (slot.tree)
            slot.tree->Write();
        }
        m_file->Close();
        for (auto& slot : m_slots) {
          if (slot.tree) [[maybe_unused]]
            auto leaked = slot.tree.release();
        }
      } else {
        // RNTuple writers automatically flush on destruction
        m_slots.clear();
      }
      return StatusCode::SUCCESS;
    }

    virtual void operator()(const In&...) const = 0;

    auto inputLocations(size_t i) const {
      if (i >= N_in) {
        throw std::out_of_range("Called inputLocations with an index out of range, index: " + std::to_string(i) +
                                ", number of inputs: " + std::to_string(N_in));
      }
      std::vector<std::string> names;
      if (!m_inputLocationsSingle[i].name().empty()) {
        names.push_back(m_inputLocationsSingle[i].value().key());
      } else {
        for (const auto& id : m_inputLocationsVector[i].value()) {
          names.push_back(id.key());
        }
      }
      return names;
    }

    auto inputLocations(std::string_view name) const {
      std::vector<std::string> names;
      const auto it =
          std::ranges::find_if(m_inputLocationsVector, [&name](const auto& prop) { return prop.name() == name; });
      if (it != m_inputLocationsVector.end()) {
        for (const auto& id : it->value()) {
          names.push_back(id.key());
        }
      } else {
        const auto it2 =
            std::ranges::find_if(m_inputLocationsSingle, [&name](const auto& prop) { return prop.name() == name; });
        if (it2 == m_inputLocationsSingle.end()) {
          throw std::out_of_range("Called inputLocations with a name that does not exist: " + std::string(name));
        } else {
          names.push_back(it2->value().key());
        }
      }
      return names;
    }

    bool isReEntrant() const final { return false; }

    NonCopiableMap& getNextTupleMap() const {
      initializeSlots();
      if (m_currentMapIndex >= m_slots.size()) {
        throw std::out_of_range("Requested more NTuple maps than available: " + std::to_string(m_currentMapIndex) +
                                " requested, " + std::to_string(m_slots.size()) + " available");
      }
      return m_slots[m_currentMapIndex++].map;
    }
  };

} // namespace details

template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
using TupleWriter = details::TupleWriter<Signature, Traits_>;

} // namespace k4FWCore

#endif // FWCORE_TUPLEWRITER_H
