/*****************************************************************************\
* (c) Copyright 2000-2019 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

#include "GaudiKernel/IInterface.h"

#include "podio/CollectionBase.h"
#include "podio/ROOTFrameWriter.h"

#include <exception>
#include <memory>
#include <vector>

/**
 * The interface implemented by any class making IO and reading RawEvent Data
 */
class IIOSvc : virtual public IInterface {

public:
  struct EndOfInput : std::logic_error {
    EndOfInput() : logic_error( "Reached end of input while more data were expected" ){};
  };

public:
  /// InterfaceID
  DeclareInterfaceID( IIOSvc, 1, 0 );

  virtual std::tuple<std::vector<std::shared_ptr<podio::CollectionBase>>, std::vector<std::string>, podio::Frame> next( ) = 0;
  virtual std::shared_ptr<std::vector<std::string>> getCollectionNames() const = 0;

  virtual std::shared_ptr<podio::ROOTFrameWriter> getWriter() = 0;
  virtual void deleteWriter() = 0;
  virtual void deleteReader() = 0;
};
