#ifndef TEST_UNIQUEIDGENSVC_H
#define TEST_UNIQUEIDGENSVC_H

// GAUDI
#include <GaudiAlg/GaudiAlgorithm.h>

#include "k4Interface/IUniqueIDGenSvc.h"

class TestUniqueIDGenSvc : public GaudiAlgorithm {
public:
  explicit TestUniqueIDGenSvc(const std::string&, ISvcLocator*);
  virtual ~TestUniqueIDGenSvc();
  /**  Initialize.
   *   @return status code
   */
  virtual StatusCode initialize() final;
  /**  Execute.
   *   @return status code
   */
  virtual StatusCode execute() final;
  /**  Finalize.
   *   @return status code
   */
  virtual StatusCode finalize() final;

private:
  SmartIF<IUniqueIDGenSvc> m_service;
};

#endif // TEST_UNIQUEIDGENSVC_H
