#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/utils/ParticleIDUtils.h>

#include <podio/Reader.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>

bool checkPIDForAlgo(const edm4hep::utils::PIDHandler& pidHandler, const edm4hep::ReconstructedParticle& reco,
                     const edm4hep::utils::ParticleIDMeta& pidMeta, const int paramIndex) {
  auto maybePID = pidHandler.getPID(reco, pidMeta.algoType());
  if (!maybePID) {
    fmt::print("Could not retrieve the {} PID object for reco particle {}", pidMeta.algoName, reco.id().index);
    return false;
  }
  auto pid      = maybePID.value();
  auto paramVal = pid.getParameters()[paramIndex];

  // As set in the producer
  if (paramVal != paramIndex * 0.5f) {
    fmt::print("Could not retrieve the correct parameter value for param {} (expected {}, actual {})",
               pidMeta.paramNames[paramIndex], paramIndex * 0.5f, paramVal);
    return false;
  }

  return true;
}

bool checkAlgoMetadata(const edm4hep::utils::ParticleIDMeta& pidMeta, const std::string& algoName,
                       const std::vector<std::string>& paramNames) {
  if (pidMeta.algoName != algoName) {
    fmt::print(
        "The PID algorithm name from metadata does not match the expected one from the properties: (expected {}, "
        "actual {})\n",
        algoName, pidMeta.algoName);
    return false;
  }

  if (!std::ranges::equal(pidMeta.paramNames, paramNames)) {
    fmt::print(
        "The PID parameter names retrieved from metadata does not match the expected ones from the properties: "
        "(expected {}, actual {})\n",
        paramNames, pidMeta.paramNames);
    return false;
  }

  return true;
}

// Test configuration (this needs to match the settings in
// options/ExampleParticleIDMetadata.py)
constexpr auto                 pidCollectionName1 = "RecoParticlesPIDs_1";
constexpr auto                 pidCollectionName2 = "RecoParticlesPIDs_2";
constexpr auto                 pidAlgo1           = "PIDAlgo1";
constexpr auto                 pidAlgo2           = "PIDAlgo2";
constexpr auto                 pidParam1          = "single_param";
constexpr auto                 pidParam2          = "param_2";
const std::vector<std::string> paramNames1        = {"single_param"};
const std::vector<std::string> paramNames2        = {"param_1", "param_2", "param_3"};

int main(int, char* argv[]) {
  auto       reader   = podio::makeReader(argv[1]);
  const auto metadata = reader.readFrame(podio::Category::Metadata, 0);

  const auto pidMeta1 = edm4hep::utils::PIDHandler::getAlgoInfo(metadata, pidCollectionName1).value();
  const auto pidMeta2 = edm4hep::utils::PIDHandler::getAlgoInfo(metadata, pidCollectionName2).value();

  if (!checkAlgoMetadata(pidMeta1, pidAlgo1, paramNames1) || !checkAlgoMetadata(pidMeta2, pidAlgo2, paramNames2)) {
    return 1;
  }

  const auto paramIndex1 = edm4hep::utils::getParamIndex(pidMeta1, pidParam1).value_or(-1);
  const auto paramIndex2 = edm4hep::utils::getParamIndex(pidMeta2, pidParam2).value_or(-1);
  if (paramIndex1 == -1 || paramIndex2 == -1) {
    fmt::print("Could not get a parameter index for '{}' (got {}) or '{}' (got {})\n", pidParam1, paramIndex1,
               pidParam2, paramIndex2);
  }

  const auto event      = reader.readEvent(0);
  const auto pidHandler = edm4hep::utils::PIDHandler::from(event, metadata);

  const auto& recos = event.get<edm4hep::ReconstructedParticleCollection>("RecoParticles");
  for (const auto r : recos) {
    auto pids = pidHandler.getPIDs(r);
    if (pids.size() != 2) {
      fmt::print("Failed to retrieve the two expected ParticlID objects related to reco particle {}", r.id().index);
      return 1;
    }

    if (!checkPIDForAlgo(pidHandler, r, pidMeta1, paramIndex1) ||
        !checkPIDForAlgo(pidHandler, r, pidMeta2, paramIndex2)) {
      return 1;
    }
  }

  return 0;
}
