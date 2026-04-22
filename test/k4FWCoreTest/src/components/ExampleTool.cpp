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

#include "k4FWCore/Consumer.h"
#include "k4FWCore/MetadataUtils.h"

#include <Gaudi/Algorithm.h>
#include <GaudiKernel/AlgTool.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/ToolHandle.h>

struct IExampleTool : virtual IAlgTool {
  DeclareInterfaceID(IExampleTool, 1, 0);
  virtual int getNumber() const = 0;
};

class ExampleToolA : public extends<AlgTool, IExampleTool> {
public:
  using extends::extends;

  StatusCode initialize() override {
    if (auto sc = AlgTool::initialize(); sc.isFailure()) {
      return sc;
    }
    k4FWCore::putParameter("ToolInitParam", getNumber(), this);
    return StatusCode::SUCCESS;
  }

  int getNumber() const override { return m_number; }

  StatusCode finalize() override {
    k4FWCore::putParameter("ToolFinalizeParam", getNumber(), this);
    return AlgTool::finalize();
  }

private:
  Gaudi::Property<int> m_number{this, "Number", 42, "The number returned by getNumber()"};
};

DECLARE_COMPONENT(ExampleToolA)

class ExampleToolB : public extends<AlgTool, IExampleTool> {
public:
  using extends::extends;

  StatusCode initialize() override {
    if (auto sc = AlgTool::initialize(); sc.isFailure()) {
      return sc;
    }
    k4FWCore::putParameter("ToolInitParam", getNumber(), this);
    return StatusCode::SUCCESS;
  }

  int getNumber() const override { return m_number; }

  StatusCode finalize() override {
    k4FWCore::putParameter("ToolFinalizeParam", getNumber(), this);
    if (k4FWCore::getParameter<int>("ToolInitParam", this).value_or(-1) != m_number) {
      error() << "ToolInitParam does not match the expected value of " << m_number << endmsg;
      return StatusCode::FAILURE;
    }
    return AlgTool::finalize();
  }

private:
  Gaudi::Property<int> m_number{this, "Number", 99, "The number returned by getNumber()"};
};

DECLARE_COMPONENT(ExampleToolB)

class ExampleToolC : public extends<AlgTool, IExampleTool> {
public:
  using extends::extends;

  int getNumber() const override { return m_number; }

private:
  Gaudi::Property<int> m_number{this, "Number", 77, "The number returned by getNumber()"};
};

DECLARE_COMPONENT(ExampleToolC)

struct ExampleToolUser final : k4FWCore::Consumer<void()> {

  ExampleToolUser(const std::string& name, ISvcLocator* svcLoc) : Consumer(name, svcLoc) {}

  StatusCode initialize() override {
    if (auto sc = Algorithm::initialize(); sc.isFailure()) {
      return sc;
    }
    if (auto sc = m_toolA.retrieve(); sc.isFailure()) {
      return sc;
    }
    if (auto sc = m_toolB.retrieve(); sc.isFailure()) {
      return sc;
    }
    return m_toolC.retrieve();
  }

  void operator()() const override {
    if (auto n = m_toolA->getNumber(); n != m_expectedToolANumber) {
      error() << "ToolA::getNumber() returned " << n << ", expected " << m_expectedToolANumber << endmsg;
      throw GaudiException("ToolA value mismatch", name(), StatusCode::FAILURE);
    }
    if (auto n = m_toolB->getNumber(); n != m_expectedToolBNumber) {
      error() << "ToolB::getNumber() returned " << n << ", expected " << m_expectedToolBNumber << endmsg;
      throw GaudiException("ToolB value mismatch", name(), StatusCode::FAILURE);
    }
    if (auto n = m_toolC->getNumber(); n != m_expectedToolCNumber) {
      error() << "ToolC::getNumber() returned " << n << ", expected " << m_expectedToolCNumber << endmsg;
      throw GaudiException("ToolC value mismatch", name(), StatusCode::FAILURE);
    }
  }

  StatusCode finalize() override {
    k4FWCore::putParameter("ToolFinalizeParam", m_toolA->getNumber(), this);
    return Algorithm::finalize();
  }

  ToolHandle<IExampleTool> m_toolA{this, "ToolA", ""};
  ToolHandle<IExampleTool> m_toolB{this, "ToolB", ""};
  ToolHandle<IExampleTool> m_toolC{this, "ToolC", "ExampleToolC"};
  Gaudi::Property<int> m_expectedToolANumber{this, "ExpectedToolANumber", 0,
                                             "Expected return value of ToolA->getNumber()"};
  Gaudi::Property<int> m_expectedToolBNumber{this, "ExpectedToolBNumber", 0,
                                             "Expected return value of ToolB->getNumber()"};
  Gaudi::Property<int> m_expectedToolCNumber{this, "ExpectedToolCNumber", 0,
                                             "Expected return value of ToolC->getNumber()"};
};

DECLARE_COMPONENT(ExampleToolUser)
