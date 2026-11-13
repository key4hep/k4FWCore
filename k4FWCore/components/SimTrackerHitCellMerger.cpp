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

// Gaudi
#include "Gaudi/Property.h"

// edm4hep
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"

// k4FWCore
#include "k4FWCore/GaudiChecks.h"
#include "k4FWCore/PropertyChoices.h"
#include "k4FWCore/Transformer.h"

// C++
#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

/** @class SimTrackerHitCellMerger
 *
 *  Gaudi transformer that accumulates the Geant4 step lengths and energy deposits of all
 *  simulated hits within a cell (sharing the same cellID), and writes the result as a new, "merged"
 *  edm4hep::SimTrackerHitCollection with one entry per cell (or per cell and track, see below).
 *
 *  The purpose of the algorithms is to provide the dx part for dN/dx (or dE/dx) calculations, where dN is
 *  provided by the digitiser. In a full processing chain, the dx would need to come from tracking, but especially
 *  in the case of the straw tube tracker, getting the actual path length inside the sensitive volumes from the
 *  track is not trivial. Therefore, this algorithm serves as an intermediate temporary solution using truth
 *  information to provide dx.
 *
 *  A single Geant4 track can leave several SimTrackerHits in one cell (one per step), and several
 *  tracks (the primary plus its delta rays, conversions, other particles, ...) can cross the very same cell.
 *  How to handle cases with multiple tracks in a cell is configurable via the MultiTrackCellHandling property,
 *  which can take one of the following values:
 *
 *   - "SumAll"              : one output hit per cellID. pathLength and eDep are summed over *every*
 *                             contributing hit, irrespective of which track produced it. The MCParticle
 *                             relation of the output hit points to the most primary contributor (see below),
 *                             i.e. to the most primary track crossing the cell.
 *   - "MostPrimaryInCell"   : one output hit per cellID, but only the hits of the most primary contributor
 *                             *to that cell* are summed. Contributions of later (secondary) tracks are
 *                             dropped. Note that the ranking is relative to the cell, so a secondary that
 *                             crosses a cell no primary ever touched is the most primary one there and is
 *                             kept. Selecting genuine primaries is left to the caller, e.g. by filtering
 *                             the input collection beforehand.
 *   - "PerTrack" (default)  : one output hit per (cellID, track) pair, each summing only the hits of that
 *                             track. For a given cell more than one entry may appear, once per contributing
 *                             track, each entry carrying its own MCParticle relation. Entries of a given cell
 *                             are written in order of increasing Geant4 track number, so the first entry for
 *                             a cell is the most primary one.
 *   - "SkipMultiTrackCells" : one output hit per cellID, but only for cells that were crossed by exactly
 *                             one track. Cells with an ambiguous composition are dropped altogether,
 *                             which gives a clean but biased sample of unambiguous single-track cells.
 *
 *  Identifying the most primary track: edm4hep does not persist the Geant4 trackID, and by the time this
 *  algorithm runs there is no Geant4 left to ask - G4Step and G4Track only exist inside the simulation
 *  process.
 *  What this algorithm therefore uses, is:
 *
 *   - MCParticle::isCreatedInSimulation(), the BITCreatedInSimulation bit of the simulator status. This is
 *     genuine Geant4 truth written by DDG4: it separates the particles that came from the generator (the
 *     true primaries) from those that Geant4 created during tracking.
 *   - the index of the MCParticle within its collection, which can be used as a proxy for the particle ID.
 *     DDG4 stores particles in a dd4hep::sim::Geant4ParticleMap, i.e. a std::map<int, Geant4Particle*>,
 *     and writes them to the MCParticle collection in ascending map-key order, so the collection index
 *     increases monotonically with the particle ID.
 *
 *  Contributors are ranked by those two in that order, so the "most primary" track of a cell is the
 *  generator particle with the lowest Geant4 track number, and indices are only compared if both particles
 *  are have the same CreatedInSimulation value.
 *
 *  Note that hits flagged isProducedBySecondary() do not point to the secondary that actually created them
 *  (it was not kept in the MCParticle collection) but to its surviving ancestor, so their step length is
 *  booked on that ancestor. Set ExcludeSecondaryHits to true to skip such hits altogether.
 *
 *  Overlay hits can be ignored via the ExcludeOverlayHits property.
 *
 *  Of the remaining fields of a merged SimHit, time, position and momentum need to be configured, since they
 *  are not additive. This can be done via the RepresentativeKinematics property:
 *
 *   - "EarliestHit" : they are copied from the earliest (smallest time) of the summed hits.
 *   - "Average"     : they are the unweighted arithmetic mean over the summed hits, i.e. the centroid of
 *                     the track segment(s) that were merged.
 *
 *  The quality bit field is always copied from the earliest summed hit, since bit flags cannot be averaged.
 *
 *  Inputs:
 *      - @param InputSimTrackerHits Name of the input edm4hep::SimTrackerHitCollection, default
 *  "SimTrackerHits"
 *
 *  Properties:
 *      - @param MultiTrackCellHandling How to treat several Geant4 tracks crossing the same cell, one of
 *  "SumAll", "MostPrimaryInCell", "PerTrack" (default) or "SkipMultiTrackCells" (see above)
 *      - @param RepresentativeKinematics Where the time, position and momentum of a merged hit come from,
 *  either "EarliestHit" or "Average" (see above)
 *      - @param ExcludeSecondaryHits Skip SimTrackerHits flagged isProducedBySecondary() instead of
 *  booking their step length on the ancestor MCParticle they point to
 *      - @param ExcludeOverlayHits Skip SimTrackerHits flagged isOverlay() instead of booking their step
 *      - @param MinPathLength_mm Do not write merged hits whose accumulated path length, in mm, is below
 *  this value
 *
 *  Outputs:
 *      - @param OutputSimTrackerHits Name of the output edm4hep::SimTrackerHitCollection of merged hits,
 *  default "MergedSimTrackerHits"
 *
 *  @author Andreas Loeschcke Centeno, Claude Code
 */

struct SimTrackerHitCellMerger final : k4FWCore::MultiTransformer<std::tuple<edm4hep::SimTrackerHitCollection>(
                                           const edm4hep::SimTrackerHitCollection&)> {

  SimTrackerHitCellMerger(const std::string& name, ISvcLocator* svcLoc)
      : MultiTransformer(name, svcLoc, {KeyValues("InputSimTrackerHits", {"SimTrackerHits"})},
                         {KeyValues("OutputSimTrackerHits", {"MergedSimTrackerHits"})}) {}

  /// How to treat several Geant4 tracks crossing the same cell
  enum class TrackHandling { SumAll, MostPrimaryInCell, PerTrack, SkipMultiTrackCells };

  /// Where the time, position and momentum of a merged hit come from
  enum class RepresentativeKinematics { EarliestHit, Average };

  static inline const k4FWCore::PropertyChoices<TrackHandling> s_trackHandlingChoices{
      {"SumAll", TrackHandling::SumAll},
      {"MostPrimaryInCell", TrackHandling::MostPrimaryInCell},
      {"PerTrack", TrackHandling::PerTrack},
      {"SkipMultiTrackCells", TrackHandling::SkipMultiTrackCells}};

  static inline const k4FWCore::PropertyChoices<RepresentativeKinematics> s_representativeChoices{
      {"EarliestHit", RepresentativeKinematics::EarliestHit}, {"Average", RepresentativeKinematics::Average}};

  StatusCode initialize() override {
    K4_GAUDI_CHECK(MultiTransformer::initialize());
    K4_GAUDI_CHECK(s_trackHandlingChoices.parse(m_multiTrackCellHandling, m_trackHandling, this));
    K4_GAUDI_CHECK(s_representativeChoices.parse(m_representativeKinematics, m_representative, this));

    info() << "Aggregating SimTrackerHits per cellID with MultiTrackCellHandling = '"
           << m_multiTrackCellHandling.value() << "' and RepresentativeKinematics = '"
           << m_representativeKinematics.value() << "'" << endmsg;
    return StatusCode::SUCCESS;
  }

  std::tuple<edm4hep::SimTrackerHitCollection>
  operator()(const edm4hep::SimTrackerHitCollection& simTrackerHits) const override {

    auto output = edm4hep::SimTrackerHitCollection();

    // Accumulate per cell and, within a cell, per contributing track. The inner map is ordered so that
    // iterating it yields the contributors from the most to the least primary one, i.e. begin() is the
    // most primary track crossing the cell.
    std::unordered_map<std::uint64_t, std::map<TrackKey, Contribution>> cellMap;

    for (const auto& hit : simTrackerHits) {

      if (m_excludeSecondaryHits && hit.isProducedBySecondary()) {
        continue;
      }

      if (m_excludeOverlayHits && hit.isOverlay()) {
        continue;
      }

      const auto particle = hit.getParticle();
      if (!particle.isAvailable()) {
        warning() << "SimTrackerHit in cell " << hit.getCellID()
                  << " has no MCParticle relation, its path length cannot be attributed to a track. Skipping it."
                  << endmsg;
        continue;
      }

      cellMap[hit.getCellID()][TrackKey::of(particle)].add(hit);
    }

    // Loop over the cellMap, accumulate the path lengths and add to the output collection
    for (const auto& [cellID, contributions] : cellMap) {

      switch (m_trackHandling) {

      case TrackHandling::PerTrack:
        // One output hit per (cell, track), from the most to the least primary track
        for (const auto& contribution : contributions) {
          addMergedHit(output, cellID, contribution.second);
        }
        break;

      case TrackHandling::MostPrimaryInCell:
        // Only the most primary track's steps are summed, everything else in the cell is dropped
        addMergedHit(output, cellID, contributions.begin()->second);
        break;

      case TrackHandling::SkipMultiTrackCells:
        // Keep only cells whose composition is unambiguous
        if (contributions.size() > 1) {
          debug() << "Skipping cell " << cellID << " crossed by " << contributions.size() << " tracks" << endmsg;
          break;
        }
        addMergedHit(output, cellID, contributions.begin()->second);
        break;

      case TrackHandling::SumAll: {
        // Sum over all tracks, but attribute the merged hit to the most primary one
        Contribution merged;
        for (const auto& contribution : contributions) {
          merged.merge(contribution.second);
        }
        merged.particle = contributions.begin()->second.particle;
        addMergedHit(output, cellID, merged);
        break;
      }
      }
    }

    return std::make_tuple(std::move(output));
  }

private:
  /// Ranking key of a track contributing to a cell, ordered so that the most primary track comes first:
  /// generator particles before Geant4-created ones, then by increasing MCParticle index (the proxy for
  /// the Geant4 track number), with the collectionID as a final tie breaker for the (pathological) case
  /// of hits pointing into more than one MCParticle collection.
  struct TrackKey {
    bool createdInSimulation = false;
    int index = 0;
    std::uint32_t collectionID = 0;

    auto operator<=>(const TrackKey&) const = default;

    static TrackKey of(const edm4hep::MCParticle& particle) {
      const auto objectID = particle.getObjectID();
      return TrackKey{particle.isCreatedInSimulation(), objectID.index, objectID.collectionID};
    }
  };

  /// Helper struct to hold the temporary accumulation of the SimHits, one per TrackKey in the cell
  struct Contribution {
    /// Accumulated step lengths and energy deposits of one track in one cell, or of a whole cell once the
    /// contributions of its individual tracks have been merged
    double pathLength_mm = 0.;
    double eDep_GeV = 0.;
    int nHits = 0;
    edm4hep::MCParticle particle{};
    /// Earliest of the summed hits, source of the quality bits and, for RepresentativeKinematics
    /// "EarliestHit", of the time, position and momentum. Note that a default constructed edm4hep
    /// handle owns a fresh zero filled object rather than being empty, so isAvailable() is true for
    /// it and cannot be used to tell whether a hit has been booked yet: use nHits for that.
    edm4hep::SimTrackerHit earliestHit{};
    /// Running sums used by RepresentativeKinematics "Average"
    double timeSum_ns = 0.;
    std::array<double, 3> positionSum_mm{};
    std::array<double, 3> momentumSum_GeV{};

    /// Book one simulated hit
    void add(const edm4hep::SimTrackerHit& hit) {
      if (nHits == 0 || hit.getTime() < earliestHit.getTime()) {
        earliestHit = hit;
      }
      pathLength_mm += hit.getPathLength();
      eDep_GeV += hit.getEDep();
      nHits++;
      particle = hit.getParticle();
      timeSum_ns += hit.getTime();
      const auto& position_mm = hit.getPosition();
      positionSum_mm[0] += position_mm.x;
      positionSum_mm[1] += position_mm.y;
      positionSum_mm[2] += position_mm.z;
      const auto& momentum_GeV = hit.getMomentum();
      momentumSum_GeV[0] += momentum_GeV.x;
      momentumSum_GeV[1] += momentum_GeV.y;
      momentumSum_GeV[2] += momentum_GeV.z;
    }

    /// Fold the contribution of another track of the same cell in. The MCParticle relation is left
    /// untouched, since a merged cell has to be attributed to one chosen track by the caller.
    void merge(const Contribution& other) {
      if (other.nHits == 0) {
        return;
      }
      if (nHits == 0 || other.earliestHit.getTime() < earliestHit.getTime()) {
        earliestHit = other.earliestHit;
      }
      pathLength_mm += other.pathLength_mm;
      eDep_GeV += other.eDep_GeV;
      nHits += other.nHits;
      timeSum_ns += other.timeSum_ns;
      for (std::size_t i = 0; i < 3; i++) {
        positionSum_mm[i] += other.positionSum_mm[i];
        momentumSum_GeV[i] += other.momentumSum_GeV[i];
      }
    }
  };

  /// Create one merged hit out of an accumulated contribution
  void addMergedHit(edm4hep::SimTrackerHitCollection& output, std::uint64_t cellID,
                    const Contribution& contribution) const {
    if (contribution.pathLength_mm < m_minPathLength_mm) {
      debug() << "Dropping merged hit in cell " << cellID << " with path length " << contribution.pathLength_mm
              << " mm below MinPathLength_mm" << endmsg;
      return;
    }

    auto mergedHit = output.create();
    mergedHit.setCellID(cellID);
    mergedHit.setPathLength(contribution.pathLength_mm);
    mergedHit.setEDep(contribution.eDep_GeV);
    mergedHit.setParticle(contribution.particle);
    // Bit flags cannot be averaged, so they always come from the earliest summed hit
    mergedHit.setQuality(contribution.earliestHit.getQuality());

    if (m_representative == RepresentativeKinematics::Average) {
      const double norm = 1. / contribution.nHits;
      mergedHit.setTime(contribution.timeSum_ns * norm);
      mergedHit.setPosition({contribution.positionSum_mm[0] * norm, contribution.positionSum_mm[1] * norm,
                             contribution.positionSum_mm[2] * norm});
      mergedHit.setMomentum({static_cast<float>(contribution.momentumSum_GeV[0] * norm),
                             static_cast<float>(contribution.momentumSum_GeV[1] * norm),
                             static_cast<float>(contribution.momentumSum_GeV[2] * norm)});
    } else {
      mergedHit.setTime(contribution.earliestHit.getTime());
      mergedHit.setPosition(contribution.earliestHit.getPosition());
      mergedHit.setMomentum(contribution.earliestHit.getMomentum());
    }
  }

  /// Configurable property steering what to do when several Geant4 tracks cross the same cell
  Gaudi::Property<std::string> m_multiTrackCellHandling{
      this, "MultiTrackCellHandling", "PerTrack",
      "How to treat several Geant4 tracks in the same cell, one of " + s_trackHandlingChoices.list() +
          ": 'SumAll' sums the step lengths of every track, 'MostPrimaryInCell' sums only those of the most "
          "primary track of that cell (determined via generator status and index in the MCParticle collection), "
          "'PerTrack' writes one output hit per cell and track (default value), "
          "'SkipMultiTrackCells' drops cells that were crossed by more than one track"};

  /// Configurable property steering where the time, position and momentum of a merged hit come from
  Gaudi::Property<std::string> m_representativeKinematics{
      this, "RepresentativeKinematics", "Average",
      "Where the time, position and momentum of a merged hit come from, one of " + s_representativeChoices.list() +
          ": 'EarliestHit' copies them from the earliest of the summed hits, 'Average' takes their unweighted "
          "arithmetic mean over the summed hits (default value)"};

  /// Configurable property to skip hits that were created by a secondary which is not kept in the
  /// MCParticle collection, and whose step length would otherwise be booked on its surviving ancestor
  Gaudi::Property<bool> m_excludeSecondaryHits{
      this, "ExcludeSecondaryHits", false,
      "Skip SimTrackerHits flagged isProducedBySecondary() instead of attributing them to the ancestor "
      "MCParticle they point to (default false)"};

  /// Configurable property to skip overlay hits
  Gaudi::Property<bool> m_excludeOverlayHits{
      this, "ExcludeOverlayHits", false,
      "Skip SimTrackerHits flagged isOverlay() instead of booking their step length (default false)"};

  /// Configurable property to suppress merged hits with a negligible accumulated path length
  Gaudi::Property<float> m_minPathLength_mm{
      this, "MinPathLength_mm", 0.,
      "Do not write merged hits whose accumulated path length, in mm, is below this value"};

  /// Parsed version of m_multiTrackCellHandling, set in initialize()
  TrackHandling m_trackHandling{TrackHandling::PerTrack};

  /// Parsed version of m_representativeKinematics, set in initialize()
  RepresentativeKinematics m_representative{RepresentativeKinematics::Average};
};

DECLARE_COMPONENT(SimTrackerHitCellMerger)
