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

#include "TFile.h"
#include "TTree.h"

#include "Gaudi/Property.h"

#include "k4FWCore/Producer.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "podio/UserDataCollection.h"

#include <memory>
#include <string>
#include <tuple>

using TFileProducerReturnType =
    std::tuple<podio::UserDataCollection<float>, edm4hep::MCParticleCollection, edm4hep::SimTrackerHitCollection>;

/// Produces framework data while managing a user-declared ROOT file.
struct ExampleFunctionalProducerWithTFile final : k4FWCore::Producer<TFileProducerReturnType()> {
  ExampleFunctionalProducerWithTFile(const std::string& name, ISvcLocator* svcLoc)
      : Producer(name, svcLoc, {},
                 {KeyValue("OutputCollectionFloat", "VectorFloat"),
                  KeyValue("OutputCollectionParticles", "MCParticles"),
                  KeyValue("OutputCollectionSimTrackerHits", "SimTrackerHit")}) {
    setProperty("Cardinality", 1).ignore();
  }

  bool isReEntrant() const override { return false; }

  StatusCode initialize() override {
    if (Producer::initialize().isFailure()) {
      return StatusCode::FAILURE;
    }

    m_file.reset(TFile::Open("output_TestAlgorithmWithTFile_myTFile.root", "RECREATE"));
    if (!m_file || m_file->IsZombie()) {
      error() << "Could not create the user-declared ROOT file" << endmsg;
      return StatusCode::FAILURE;
    }
    m_tree = std::make_unique<TTree>("mytree", "testing");
    m_tree->Branch("value", &m_value, "value/F");
    return StatusCode::SUCCESS;
  }

  TFileProducerReturnType operator()() const override {
    auto floatVector = podio::UserDataCollection<float>();
    floatVector.push_back(125.);
    floatVector.push_back(25.);

    auto particles = edm4hep::MCParticleCollection();
    auto particle = particles.create();
    particle.setMomentum({m_magicNumberOffset + 5.0, m_magicNumberOffset + 6.0, m_magicNumberOffset + 7.0});
    particle.setMass(m_magicNumberOffset + 8);

    auto hits = edm4hep::SimTrackerHitCollection();
    auto hit = hits.create();
    hit.setPosition({3, 4, 5});

    m_value += 1.1;
    m_tree->Fill();

    return std::make_tuple(std::move(floatVector), std::move(particles), std::move(hits));
  }

  StatusCode finalize() override {
    m_file->cd();
    m_tree->Write();
    m_file->Close();
    return Producer::finalize();
  }

private:
  Gaudi::Property<int> m_magicNumberOffset{this, "magicNumberOffset", 0,
                                           "Integer to add to the dummy values written to the EDM"};
  mutable Float_t m_value{0.1};
  std::unique_ptr<TFile> m_file;
  std::unique_ptr<TTree> m_tree;
};

DECLARE_COMPONENT(ExampleFunctionalProducerWithTFile)
