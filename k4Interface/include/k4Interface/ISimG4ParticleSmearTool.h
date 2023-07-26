/*
 * Copyright (c) 2014-2023 Key4hep-Project.
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
#ifndef SIMG4INTERFACE_ISIMG4PARTICLESMEARTOOL_H
#define SIMG4INTERFACE_ISIMG4PARTICLESMEARTOOL_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// CLHEP
namespace CLHEP {
  class Hep3Vector;
}

/** @class ISimG4ParticleSmearTool SimG4Interface/SimG4Interface/ISimG4ParticleSmearTool.h ISimG4ParticleSmearTool.h
 *
 *  Interface to the particle smearing tool.
 *  (smearing its momentum/energy for the purpose of fast simulation)
 *
 *  @author Anna Zaborowska
 */

class ISimG4ParticleSmearTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(ISimG4ParticleSmearTool, 1, 0);

  /**  Smear the momentum of the particle
   *   @param aMom Particle momentum to be smeared.
   *   @param[in] aPdg Particle PDG code.
   *   @return status code
   */
  virtual StatusCode smearMomentum(CLHEP::Hep3Vector& aMom, int aPdg = 0) = 0;

  /**  Check conditions of the smearing model, especially if the given parametrs do not exceed the parameters of the
   * model.
   *   @param[in] aMinMomentum Minimum momentum.
   *   @param[in] aMaxMomentum Maximum momentum.
   *   @param[in] aMaxEta Maximum pseudorapidity.
   *   @return status code
   */
  virtual StatusCode checkConditions(double aMinMomentum, double aMaxMomentum, double aMaxEta) const = 0;
};
#endif /* SIMG4INTERFACE_ISIMG4PARTICLESMEARTOOL_H */
