#ifndef K4FWCORE_K4FWCORETEST_HELLOWORLDALG
#define K4FWCORE_K4FWCORETEST_HELLOWORLDALG

// GAUDI
#include "Gaudi/Property.h"
#include "GaudiAlg/GaudiAlgorithm.h"

class k4FWCoreTest_HelloWorldAlg : public GaudiAlgorithm {
public:
  explicit k4FWCoreTest_HelloWorldAlg(const std::string&, ISvcLocator*);
  virtual ~k4FWCoreTest_HelloWorldAlg();
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
  // member variable
  Gaudi::Property<std::string> theMessage{this, "PerEventPrintMessage", "Hello ",
                                          "The message to printed for each Event"};
};

#endif /* K4FWCORE_K4FWCORETEST_HELLOWORLDALG */
