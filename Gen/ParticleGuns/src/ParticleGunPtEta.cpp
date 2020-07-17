
#include <cmath>

#include "ParticleGunPtEta.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "HepPDT/ParticleID.hh"
#include "Pythia8/ParticleData.h"

DECLARE_COMPONENT(ParticleGunPtEta)

ParticleGunPtEta::ParticleGunPtEta(const std::string& name, ISvcLocator* svcLoc) : GaudiAlgorithm(name, svcLoc) {
  declareProperty("MCParticles", m_MCParticlesHandle, "MCParticle data handle (output)");

}

ParticleGunPtEta::~ParticleGunPtEta() {}

StatusCode ParticleGunPtEta::initialize() {
  // check momentum and angles
  if ((m_minEta > m_maxEta) || (m_minPhi > m_maxPhi)) return Error("Incorrect values for eta or phi!");
  m_deltaPhi = m_maxPhi - m_minPhi;
  m_deltaEta = m_maxEta - m_minEta;
  // setup particle information
  m_masses.clear();
  auto pd = Pythia8::ParticleData();
  info() << "Particle type chosen randomly from :";
  for (auto icode: m_pdgCodes) {
    info() << " " << icode;
    m_masses.push_back(pd.m0(icode));
  }
  info() << endmsg;
  info() << "Eta range: " << m_minEta << "  <-> " << m_maxEta << endmsg;
  info() << "Phi range: " << m_minPhi / Gaudi::Units::rad << " rad <-> " << m_maxPhi / Gaudi::Units::rad << " rad"
         << endmsg;
  // write additional branches
  if (m_writeParticleGunBranches) {
   m_datahandle_particlegun_pt = std::make_unique<DataHandle<float>>("ParticleGun_Pt", Gaudi::DataHandle::Writer, this);
   m_datahandle_particlegun_eta = std::make_unique<DataHandle<float>>("ParticleGun_Eta", Gaudi::DataHandle::Writer, this);
   m_datahandle_particlegun_costheta = std::make_unique<DataHandle<float>>("ParticleGun_costheta", Gaudi::DataHandle::Writer, this);
   m_datahandle_particlegun_phi = std::make_unique<DataHandle<float>>("ParticleGun_Phi", Gaudi::DataHandle::Writer, this);
  }

  // initialize random number generator
     std::random_device rd;  //Will be used to obtain a seed for the random number engine
    m_gen = std::mt19937(rd()); //Standard mersenne_twister_engine seeded with rd()
    m_rndmflat = std::uniform_real_distribution<>(1.0, 2.0); 

  return StatusCode::SUCCESS;
}


StatusCode ParticleGunPtEta::execute() {
  edm4hep::MCParticleCollection* particles = new edm4hep::MCParticleCollection();
  auto particle = particles->create();
  // todo:  vertexsmeartool
  particle.setVertex({0., 0., 0.});
  particle.setTime(0.);
  float px(0.), py(0.), pz(0.);
  // Generate values for eta  and phi
  double phi = m_minPhi + m_rndmflat(m_gen) * (m_deltaPhi);
  double eta = m_minEta + m_rndmflat(m_gen) * (m_deltaEta);
  double pt = m_minPt + m_rndmflat(m_gen) * (m_maxPt - m_minPt);
  if (m_logSpacedPt) {
    pt = pow(10, std::log10(m_minPt) + (std::log10(m_maxPt) - std::log10(m_minPt)) * m_rndmflat(m_gen));
  }
  /// if user has provided the list options, use the list
  if (!m_ptList.empty()) {
    pt = m_ptList[m_rndmflat(m_gen) * m_ptList.size()];
  }
  if (!m_etaList.empty()) {
    eta = m_etaList[m_rndmflat(m_gen) * m_etaList.size()];
  }
  // Transform to x,y,z coordinates
  px = pt * cos(phi);
  py = pt * sin(phi);
  pz = pt * sinh(eta);
  // randomly choose a particle type
  unsigned int currentType = (unsigned int)(m_pdgCodes.size() * m_rndmflat(m_gen));
  // protect against funnies
  if (currentType >= m_pdgCodes.size()) currentType = 0;
  // set momenta
  particle.setMomentum({px, py, pz});
  double momentumP2 = px*px + py*py + pz*pz;
  particle.setMass(m_masses[currentType]);
  particle.setPDG(m_pdgCodes[currentType]);
  debug() << " -> " << m_pdgCodes[currentType] << endmsg << "   P   = " << sqrt(momentumP2) << endmsg;
  /// additional branches in rootfile
  if (m_writeParticleGunBranches) { 
    float* _particlegun_phi = new float(phi);
    float* _particlegun_eta =  new float(eta);
    float* _particlegun_costheta = new float (cos(2*atan(exp(eta))));
    float* _particlegun_pt = new float(pt);
    m_datahandle_particlegun_pt->put(_particlegun_pt);
    m_datahandle_particlegun_eta->put(_particlegun_eta);
    m_datahandle_particlegun_costheta->put(_particlegun_costheta);
    m_datahandle_particlegun_phi->put(_particlegun_phi);
  }
  m_MCParticlesHandle.put(particles);
  return StatusCode::SUCCESS;
}
