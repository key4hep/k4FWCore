# How to use UniqueIDGenSvc

A Service to generate unique identifiers can be used with the following inputs:
- Event number, Run number and Algorithm name
- Seed number: set in the options python file

To set a seed for the UniqueIDGenSvc, add the following to an options file:

```python
from Configurables import UniqueIDGenSvc
UniqueIDGenSvc().Seed = 987654321
```

Declare the service in the header file:

```cpp
#include <k4FWCore/IUniqueIDGenSvc.h>

SmartIF<IUniqueIDGenSvc> m_service;
```

Initialize the service:

```cpp
StatusCode SomeGaudiAlgorithm::initialize() {
  m_service = serviceLocator()->service("UniqueIDGenSvc");
```

Then use the service during execution:

```cpp
StatusCode MarlinProcessorWrapper::execute() {
  m_service->getUniqueID(1, 2, name());
}
```
