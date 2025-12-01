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

    void initializeBackend(size_t index) const {

      if (m_Names.size() != m_treeDescription.size()) {
        throw std::runtime_error("TupleWriter: Name and TreeDescription properties must have the same size");
      }

      if (m_filled.empty()) {
        m_filled.resize(m_Names.size());
        if (!m_RNTuple) {
          m_file = std::make_unique<TFile>(m_outputFile.value().c_str(), "RECREATE");
          m_tree.resize(m_Names.size());
        } else {
          m_writer.resize(m_Names.size());
          m_rntupleEntry.resize(m_Names.size());
        }
      }

      if (!m_RNTuple) {
        m_tree[index] =
            std::make_unique<TTree>(m_Names.value()[index].c_str(), m_treeDescription.value()[index].c_str());
        for (const auto& [key, var] : m_NTupleMaps[index].m_map) {
          std::visit(
              [this, &key, index](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                m_tree[index]->Branch(key.c_str(), &std::get<T>(m_NTupleMaps[index].m_map[key]));
              },
              var);
        }
        return;
      }

      auto model = ROOT::RNTupleModel::Create();
      for (const auto& [key, var] : m_NTupleMaps[index].m_map) {
        std::visit(
            [&key, &model](auto&& arg) {
              using T = std::decay_t<decltype(arg)>;
              model->MakeField<T>(key);
            },
            var);
      }
      model->Freeze();
      m_rntupleEntry[index] = model->CreateBareEntry();
      for (const auto& [key, var] : m_NTupleMaps[index].m_map) {
        std::visit(
            [this, &key, index](auto&& arg) {
              using T = std::decay_t<decltype(arg)>;
              m_rntupleEntry[index]->BindRawPtr(key, &std::get<T>(m_NTupleMaps[index].m_map[key]));
            },
            var);
      }
      m_writer[index] = ROOT::RNTupleWriter::Recreate(std::move(model), m_Names[index], m_outputFile);
    }

    void fill(size_t index) const {
      if (index >= m_initFlag.size() || !m_initFlag[index]) {
        initializeBackend(index);
        if (index >= m_initFlag.size()) {
          m_initFlag.resize(m_Names.size());
        }
        m_initFlag[index] = true;
      }
      if (m_RNTuple) {
        m_writer[index]->Fill(*m_rntupleEntry[index]);
      } else {
        m_tree[index]->Fill();
      }
      m_filled[index] = true;
    }

    StatusCode execute(const EventContext& ctx) const final {
      m_currentMapIndex = 0;
      try {
        filter_evtcontext<In...>::apply(*this, ctx, m_inputs);
      } catch (GaudiException& e) {
        (e.code() ? this->warning() : this->error()) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
      for (size_t i = 0; i < m_Names.size(); ++i) {
        if (m_filled.empty() || !m_filled[i]) {
          fill(i);
        }
        m_filled[i] = false;
      }
      return StatusCode::SUCCESS;
    }

    StatusCode finalize() final {
      // This can happen if there were no events processed
      if (!m_file && !m_RNTuple) {
        return StatusCode::SUCCESS;
      }
      if (!m_RNTuple) {
        m_file->cd();
        for (const auto& tree : m_tree) {
          tree->Write();
        }
        m_file->Close();
        // "Leak" as it seems it is not possible to make this not crash
        // when running multithreaded, anyway the process is ending
        for (auto& tree : m_tree) {
          [[maybe_unused]] const auto tmpTree = tree.release();
        }
      } else {
        m_writer.clear();
      }
      return StatusCode::SUCCESS;
    }

    virtual void operator()(const In&...) const = 0;

    /**
     * @brief    Get the input locations for a given input index
     * @param i  The index of the input
     * @return   A range of the input locations
     */
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
    /**
     * @brief       Get the input locations for a given input name
     * @param name  The name of the input
     * @return      A range of the input locations
     */
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

  private:
    mutable std::unique_ptr<TFile> m_file{};
    mutable std::vector<std::unique_ptr<TTree>> m_tree{};

    mutable std::vector<std::unique_ptr<ROOT::RNTupleWriter>> m_writer{};
    mutable std::vector<std::unique_ptr<ROOT::REntry>> m_rntupleEntry{};

    mutable std::vector<bool> m_initFlag{};
    mutable std::vector<bool> m_filled{};

    // A map that can not be copied, to avoid a copy when calling getNTupleMap
    // and not assigning it to a reference
    struct NonCopiableMap {
      friend struct TupleWriter;
      NonCopiableMap() = default;
      NonCopiableMap(const NonCopiableMap&) = delete;            // Assign with auto&
      NonCopiableMap& operator=(const NonCopiableMap&) = delete; // Assign with auto&
      NonCopiableMap(NonCopiableMap&&) = default;
      NonCopiableMap& operator=(NonCopiableMap&&) = default;
      NTupleTypes& operator[](const std::string& key) { return m_map[key]; }
      // auto begin() const { return m_map.begin(); }
      // auto end() const { return m_map.end(); }
      void fill() { m_parent->fill(m_index); }

    private:
      std::map<std::string, NTupleTypes> m_map;
      int m_index{0};
      const TupleWriter* m_parent{nullptr};
    } m_NTupleMap;

    mutable std::vector<NonCopiableMap> m_NTupleMaps;
    mutable size_t m_currentMapIndex{0};

  public:
    NonCopiableMap& getNextTupleMap() const {
      if (m_currentMapIndex >= m_Names.size()) {
        throw std::out_of_range("Requested more NTuple maps than available: " + std::to_string(m_currentMapIndex) +
                                " requested, " + std::to_string(m_NTupleMaps.size()) + " available");
      }
      if (m_NTupleMaps.empty()) {
        m_NTupleMaps.resize(m_Names.size());
        for (size_t i = 0; i < m_Names.size(); ++i) {
          auto& map = m_NTupleMaps[i];
          map.m_index = i;
          map.m_parent = this;
        }
      }
      return m_NTupleMaps[m_currentMapIndex++];
    }
  };

} // namespace details

template <typename Signature, typename Traits_ = Gaudi::Functional::Traits::useDefaults>
using TupleWriter = details::TupleWriter<Signature, Traits_>;

} // namespace k4FWCore

#endif // FWCORE_TUPLEWRITER_H
