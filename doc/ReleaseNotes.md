# v01-05

* 2026-01-19 Juan Miguel Carceller ([PR#373](https://github.com/key4hep/k4FWCore/pull/373))
  - Turn `%` into `%%` in the argparse help of `k4run` to avoid a python error in `EfficiencyFilter.cpp`.

* 2026-01-16 Thomas Madlener ([PR#371](https://github.com/key4hep/k4FWCore/pull/371))
  - Ensure we can still configure and build with EDM4hep 1.0

* 2026-01-07 Juan Miguel Carceller ([PR#370](https://github.com/key4hep/k4FWCore/pull/370))
  - Fix crash when running with IOSvc and DataHandles of types that are not collections.

* 2026-01-07 Juan Miguel Carceller ([PR#369](https://github.com/key4hep/k4FWCore/pull/369))
  - Add missing test forgotten in b233ef11c3649ea2213f74be169d59cc80d49999 (https://github.com/key4hep/k4FWCore/pull/345)

* 2026-01-07 Thomas Madlener ([PR#368](https://github.com/key4hep/k4FWCore/pull/368))
  - Move the scheduling of the stopping of the event loop to later in the loop to give precedence of other failures during the event loop that would otherwise be shadowed.

* 2025-12-23 Juan Miguel Carceller ([PR#366](https://github.com/key4hep/k4FWCore/pull/366))
  - Fix the construction of details::ReadHandle from Gaudi following the changes in  https://gitlab.cern.ch/gaudi/Gaudi/-/merge_requests/1856 to fix compilation with Gaudi 40.2 when tests are enabled

* 2025-12-19 Thomas Madlener ([PR#362](https://github.com/key4hep/k4FWCore/pull/362))
  - Introduce the `type` subcommand to the `outputCommands` grammar to keep / drop all collections of a given datatype.

* 2025-12-19 Thomas Madlener ([PR#344](https://github.com/key4hep/k4FWCore/pull/344))
  - Add documentation about histograms

* 2025-11-25 Juan Miguel Carceller ([PR#360](https://github.com/key4hep/k4FWCore/pull/360))
  - Add support for passing EventContext in functional algorithms
  - Add a test consumer, transformer and multitransformer that take an `EventContext` and tests that run them

* 2025-11-24 sss ([PR#341](https://github.com/key4hep/k4FWCore/pull/341))
  - Add macro K4_GAUDI_CHECK (in k4FWCore/k4_check.h) to reduce the boilerplate involved in checking Gaudi return status.  If its argument is false, the macro will emit and error and return StatusCode::FAILURE.

* 2025-11-21 Thomas Madlener ([PR#361](https://github.com/key4hep/k4FWCore/pull/361))
  - Make sure to only set an `OutputLevel` on the `ApplicationMgr` if `--log-level` has actually been used instead of setting it to `INFO` (regardless of the user configuration in the option file) even if no `--log-level` has passed

* 2025-11-20 Thomas Madlener ([PR#363](https://github.com/key4hep/k4FWCore/pull/363))
  - Use cmake fixtures to properly denote dependencies between tests

# v01-04

* 2025-11-07 Juan Miguel Carceller ([PR#357](https://github.com/key4hep/k4FWCore/pull/357))
  - Include `<variant>` where it's used, introduced in b233ef11

* 2025-11-05 sss ([PR#355](https://github.com/key4hep/k4FWCore/pull/355))
  - Make interfaces of ICallPositionsTool const.

* 2025-11-05 sss ([PR#354](https://github.com/key4hep/k4FWCore/pull/354))
  - Make methods of ICalibrateCaloHitsTool const, and add an interface that takes a vector of hit information rather than a map.

* 2025-10-30 Thomas Madlener ([PR#351](https://github.com/key4hep/k4FWCore/pull/351))
  - Check the commands for the `KeepDropSwitch` in the constructor instead of re-parsing them every time `isOn` is called. 
  - Move the `KeepDropSwitch` into the `k4FWCore` namespace and deprecate the unnamespaced usage.
  - Make the `IOSvc` fail in `initialize` in case the `outputCommands` are invalid
  - Add unittests for `KeepDropSwitch`

* 2025-10-27 sss ([PR#353](https://github.com/key4hep/k4FWCore/pull/353))
  - Updated noise tool interfaces to remove non-const methods and add interfaces that operate on vectors rather than maps.

* 2025-10-13 Juan Miguel Carceller ([PR#345](https://github.com/key4hep/k4FWCore/pull/345))
  - Add support for setting single values for inputs and outputs by using `KeyValue` (in Python, a single string). Previously, all the inputs and outputs had to be `KeyValues`, which in Python is a list of strings.
  - Use `KeyValue` in several test algorithms that take as input or output a single collection.
  - Add an algorithm `ExampleFunctionalConsumerKeyValues` that uses `KeyValues` to make sure it keeps working (there are still several tests using `KeyValues`, those that read or have as output a list of collections).
  - Add tests using `inputLocations()` and `outputLocations()` that cover both cases when using `KeyValue` and `KeyValues`.

* 2025-10-10 sss ([PR#346](https://github.com/key4hep/k4FWCore/pull/346))
  - ICaloReadCrosstalkMap migrated to have const methods.

* 2025-10-10 sss ([PR#342](https://github.com/key4hep/k4FWCore/pull/342))
  - k4run: add the switch --interactive, -i.  When given, k4run will enter a Python command loop after the configuration has been read, allowing one to inspect the configuration interactively.  This is useful for debugging.

* 2025-09-23 Juan Miguel Carceller ([PR#343](https://github.com/key4hep/k4FWCore/pull/343))
  - Similar to https://github.com/key4hep/EDM4hep/pull/445, remove support for CPACK that is not being used nor tested.

* 2025-09-17 Thomas Madlener ([PR#310](https://github.com/key4hep/k4FWCore/pull/310))
  - Make sure that `load_file` sets the correct `__file__` and `__spec__` information for files that are loaded with it
  - Restore the output of the filename in case loading doesn't work
  - Deprecate passing a file handle to `load_file` and always pass a path-like object

* 2025-09-11 jmcarcell ([PR#339](https://github.com/key4hep/k4FWCore/pull/339))
  - Clean up of functional utilities and code: reserve when possible and use better names for the variables and use `const` in loops when possible
  - Fix exceptions to be constructed the same way with `fmt::format`.

* 2025-09-10 jmcarcell ([PR#338](https://github.com/key4hep/k4FWCore/pull/338))
  - Refactor the CollectionPusher class into the Reader class

* 2025-09-09 Thomas Madlener ([PR#318](https://github.com/key4hep/k4FWCore/pull/318))
  - Deprecate `PodioDataSvc` (and `k4DataSvc` and `FCCDataSvc`) as well as the `PodioInput`, `PodioOutput`.
  - Add `WARNING` messages at runtime to inform users about the upcoming removal.

* 2025-08-13 Juan Miguel Carceller ([PR#340](https://github.com/key4hep/k4FWCore/pull/340))
  - Use . instead of -> for objects that are not pointers in ExampleFunctionalTransformerMultiple

* 2025-08-04 jmcarcell ([PR#337](https://github.com/key4hep/k4FWCore/pull/337))
  - Update tests following https://github.com/key4hep/k4-project-template/pull/30. In this case, tests were already working without installing

* 2025-07-31 jmcarcell ([PR#335](https://github.com/key4hep/k4FWCore/pull/335))
  - Get the available collections from IOSvc instead of peeking in the file (if any) before running, improving the performance when running single-threaded (unchanged when running multithreaded)

* 2025-07-29 jmcarcell ([PR#336](https://github.com/key4hep/k4FWCore/pull/336))
  - Use . instead of -> for objects that are not pointers

* 2025-07-21 jmcarcell ([PR#334](https://github.com/key4hep/k4FWCore/pull/334))
  - Fix the test `ReadLimitedInputsAllEventsIOSvc` that currently depends on tests (with `FIXTURE_REQUIRED`) with the wrong name
  - Make sure `PODIO_DEFAULT_WRITE_RNTUPLE` is not set when running the test `FunctionalProducerMultiple` since the test `FunctionalMix` reads the file it produces through `PodioInput`, which does not support RNTuples
  - Adapt CheckOutputFiles.py to use `podio.reading.get_reader` to automatically get the correct reader (TTree or RNTuple)

* 2025-07-08 jmcarcell ([PR#322](https://github.com/key4hep/k4FWCore/pull/322))
  - Add an EfficiencyFilter, an algorithm that will take an EDM4hep collection and return a subset collection with a certain efficiency. The `Exact` parameter can be used to control whether the efficiency is exactly that number of simply
    each element is kept with `Efficiency` probability.
  - Add a test that runs the EfficiencyFilter both in the exact version (to check that exactly `Efficiency * length` elements are saved) and in the non-exact version

* 2025-07-03 jmcarcell ([PR#328](https://github.com/key4hep/k4FWCore/pull/328))
  - Remove the outdated script k4-print-joboptions. It doesn't work on current files, it's not using the podio readers (so it doesn't support RNTuples, if it was working) and the same functionality can easily be obtained with `podio-dump`.

* 2025-06-24 jmcarcell ([PR#329](https://github.com/key4hep/k4FWCore/pull/329))
  - Do not write properties twice for ApplicationMgr and MessageSvc. Do not write any properties for the NTupleSvc
  - Construct strings in place instead of using `std::stringstream`
  - Remove an extra `;` at the end of each pair of property and value, they are anyway each different strings

* 2025-06-13 jmcarcell ([PR#314](https://github.com/key4hep/k4FWCore/pull/314))
  - Unset DISPLAY to avoid importing ROOT when not showing plots

* 2025-06-12 jmcarcell ([PR#327](https://github.com/key4hep/k4FWCore/pull/327))
  - Format two lines to fix pre-commit after updating to llvm 20

* 2025-06-12 jmcarcell ([PR#326](https://github.com/key4hep/k4FWCore/pull/326))
  - Create a new target to copy the files after all the other targets to prevent errors

* 2025-06-05 Thomas Madlener ([PR#323](https://github.com/key4hep/k4FWCore/pull/323))
  - Switch from `file(GLOB ...)` to explicitly mention source files for k4FWCore plugin sources

* 2025-06-05 jmcarcell ([PR#316](https://github.com/key4hep/k4FWCore/pull/316))
  - Use the Warnings property only if it exists for EventLoopMgr
  - Do not use the Warnings property in the tests since it is going to be removed (warnings will not be silenced in these tests before v40r0)

# v01-03

* 2025-05-27 Thomas Madlener ([PR#317](https://github.com/key4hep/k4FWCore/pull/317))
  - Move the `DataHandle` and `MetaDataHandle` into the `k4FWCore` namespace to make origins more obvious
    - Keep deprecated `DataHandle` and `MetaDataHandle` aliases in global namespace for compatibility to allow for a smoother migration

* 2025-05-26 Thomas Madlener ([PR#321](https://github.com/key4hep/k4FWCore/pull/321))
  - Remove the usage of `PodioDataSvc` from the `DataHandle` as it's not necessary
  - Harmonize the behavior of the constructors
  - Remove the declaration of the unimplemented default constructor

* 2025-05-21 jmcarcell ([PR#306](https://github.com/key4hep/k4FWCore/pull/306))
  - Add a `--log-level` flag to `k4run` to make it possible to configure the log level at runtime via the command line
    - accpeted values are `verbose`, `debug`, `info`, `warning` and `error`
    - The log level will apply to the python logging facilities and will also be set in the `ApplicationMgr.OutputLevel`
  - Make the python logging a bit more informative by adding the log level as well as some module and function information to the message
  - Deprecate the `--verbose` option in favor of `--log-level=verbose`

* 2025-05-20 Thomas Madlener ([PR#320](https://github.com/key4hep/k4FWCore/pull/320))
  - Remove the declaration of `DataHandle::getCollMetadataCellID` since the implementation has been removed long ago

* 2025-05-20 jmcarcell ([PR#315](https://github.com/key4hep/k4FWCore/pull/315))
  - Bump the required version of podio. 1.3 is needed since https://github.com/key4hep/k4FWCore/pull/305 because of `edm4hep::DataTypes` and `edm4hep::LinkTypes`

* 2025-05-15 jmcarcell ([PR#313](https://github.com/key4hep/k4FWCore/pull/313))
  - Use signal to stop immediately after receiving a SIGPIPE. Before, the output from k4run itself would always print and the pipe command (for example `| head`) would only be applied to the output from Gaudi.
  - Add a test that uses pipes

* 2025-05-13 jmcarcell ([PR#305](https://github.com/key4hep/k4FWCore/pull/305))
  - Remove the hardcoded collections in the CollectionMerger, using https://github.com/AIDASoft/podio/pull/761. This applies to all collections that are available in EDM4hep.
  - Fix link collections that are currently not working since the data type name is not `edm4hep::...LinkCollection` but `podio::LinkCollection...`
  - Require a newer version of podio in the CMakeLists.txt

* 2025-05-05 jmcarcell ([PR#304](https://github.com/key4hep/k4FWCore/pull/304))
  - Fix running algorithms with RNTuples as their input and add tests, not working before because the TTRee reader in podio was being used
  - Add two tests to create an rntuple and read it
  - Bump the required version of podio to 1.3 since the function `get_reader` in podio only has support for lists of files as an input after https://github.com/AIDASoft/podio/pull/729

* 2025-04-30 jmcarcell ([PR#307](https://github.com/key4hep/k4FWCore/pull/307))
  - Clean up several CMakeLists.txt files. For example, remove redundant calls to `find_package`.
  - Move python files that will be installed to a single place

* 2025-04-29 Thomas Madlener ([PR#308](https://github.com/key4hep/k4FWCore/pull/308))
  - Add some documentation about the `AlgTimingAuditor` and the `TimelineSvc` to show how timing information can be obtained from Gaudi at different levels of details.

* 2025-03-27 Giovanni Marchiori ([PR#299](https://github.com/key4hep/k4FWCore/pull/299))
  - Remove ICaloReadCellNoisMap interface since it provides the same functionality as INoiseConstTool

* 2025-03-18 jmcarcell ([PR#297](https://github.com/key4hep/k4FWCore/pull/297))
  - Remove the deprecated `input` and `output` properties for IOSvc

* 2025-03-07 Mateusz Jakub Fila ([PR#288](https://github.com/key4hep/k4FWCore/pull/288))
  - Update`UniqueIDGenSvc`documentation and add example usage with a functional algorithm

* 2025-03-05 Mateusz Jakub Fila ([PR#295](https://github.com/key4hep/k4FWCore/pull/295))
  - Replace `UniqueIDGenSvc` property `ThrowIfDuplicate` with `CheckDuplicates`. The service will cache ids and check duplicates only if `CheckDuplicates` is set to `true`. It's enabled by default for the Debug builds.

* 2025-03-05 Thomas Madlener ([PR#290](https://github.com/key4hep/k4FWCore/pull/290))
  - Make the `PodioInput.collections`, `IOSvc.CollectionNames` and `Reader.InputCollections` properties actually work like expected. They now properly limit the collections that are read and no other collections will be available. **This requires building against podio > v1.2`**, otherwiset the current behavior will be used, where collections that are not requested will still be available.

* 2025-02-26 Mateusz Jakub Fila ([PR#287](https://github.com/key4hep/k4FWCore/pull/287))
  - Use the same event and run type in `UniqueIDGenSvc` as in `edm4hep::EventHeader`. Add generating id directly from `edm4hep::EventHeader`. **This changes the ID values !**

* 2025-02-25 Mateusz Jakub Fila ([PR#293](https://github.com/key4hep/k4FWCore/pull/293))
  - Fix typo in docs about migrating from `k4DataSvc`

* 2025-02-23 Mateusz Jakub Fila ([PR#291](https://github.com/key4hep/k4FWCore/pull/291))
  - Fix fixture used in FunctionalMTFile test

* 2025-02-17 Thomas Madlener ([PR#289](https://github.com/key4hep/k4FWCore/pull/289))
  - Bump the `cvmfs-contrib` github action to lastet version `v5` to fix caching issues.

* 2025-02-13 jmcarcell ([PR#286](https://github.com/key4hep/k4FWCore/pull/286))
  - Add a dev3 workflow using the key4hep-build action

* 2025-02-12 jmcarcell ([PR#284](https://github.com/key4hep/k4FWCore/pull/284))
  - Add LANGUAGES CXX to CMakeLists.txt to disable checks for a C compiler

* 2025-02-11 jmcarcell ([PR#283](https://github.com/key4hep/k4FWCore/pull/283))
  - Add a Link collection to the tests

# v01-02-00

* 2025-02-04 jmcarcell ([PR#282](https://github.com/key4hep/k4FWCore/pull/282))
  - Use typeid instead of `EDM4hep::typeName` since not every EDM4hep type has `::typeName`.

* 2025-02-04 Mateusz Jakub Fila ([PR#279](https://github.com/key4hep/k4FWCore/pull/279))
  - Fix algorithm properties related to `eventNumber` and `runNumber` to deduce their type

* 2025-02-03 jmcarcell ([PR#281](https://github.com/key4hep/k4FWCore/pull/281))
  - Improve the responsiveness of k4run by importing a Reader from podio only when it's needed.

* 2025-02-03 jmcarcell ([PR#280](https://github.com/key4hep/k4FWCore/pull/280))
  - Make functional algorithms read `DataWrapper<podio::CollectionBase>` 
  - Add a test producing a collection in a `DataWrapper<podio::CollectionBase>` and reading it in a functional algorithm

* 2025-02-03 jmcarcell ([PR#278](https://github.com/key4hep/k4FWCore/pull/278))
  - Fix compilation warning about comparing integers with different signs, related to https://github.com/key4hep/EDM4hep/pull/398

* 2025-02-03 jmcarcell ([PR#275](https://github.com/key4hep/k4FWCore/pull/275))
  - Make sure that the internal casts that happen for functional algorithms do not silently cast to wrong types.

* 2025-01-31 jmcarcell ([PR#277](https://github.com/key4hep/k4FWCore/pull/277))
  - Use typeid to allow compiling functional algorithms that use Links

* 2025-01-31 Thomas Madlener ([PR#276](https://github.com/key4hep/k4FWCore/pull/276))
  - Clean up the function that reads inputs in functional algorithms by defining an alias for the EDM4hep type and using that when possible. Remove `const` from this type and add it where needed.
  - Change `std::remove_cvref_t` to `std::remove_cv_t` since there the former is not needed.

* 2025-01-29 Thomas Madlener ([PR#273](https://github.com/key4hep/k4FWCore/pull/273))
  - Make it possible to use `edm4hep::utils::ParticleIDMeta` with the `MetadataSvc`
    - Add template specializations for `get` and `put` that defer to the corresponding utility calls in EDM4hep
    - Add tests to ensure that metadata is indeed usable this way with the utilities in EDM4hep

* 2025-01-24 jmcarcell ([PR#269](https://github.com/key4hep/k4FWCore/pull/269))
  - Improve the documentation about extending the parser for k4run

* 2025-01-16 tmadlener ([PR#230](https://github.com/key4hep/k4FWCore/pull/230))
  - Configure the `k4FWCoreConfig.cmake` to put a bit more information in there
    - Make sure all dependencies are also found for dependent packages
    - Make sure dependency versions are discovered again consistently
    - Make sure to export the current k4FWCore version for downstream consumers

* 2025-01-09 jmcarcell ([PR#270](https://github.com/key4hep/k4FWCore/pull/270))
  - Use CMake fixtures for tests that require other tests
    - Tests will now run only if all the setup tests pass, otherwise they will be skipped

* 2024-12-20 Thomas Madlener ([PR#271](https://github.com/key4hep/k4FWCore/pull/271))
  - Remove an unnecessary r-value reference qualifier for sinking the metadata Frame into the `MetadataSvc`.

* 2024-12-20 Mateusz Jakub Fila ([PR#267](https://github.com/key4hep/k4FWCore/pull/267))
  - Added documentation for reading and writing EDM4hep files with the `IOSvc`
  - Moved documentation on `k4DataSvc` to legacy page

* 2024-12-18 jmcarcell ([PR#264](https://github.com/key4hep/k4FWCore/pull/264))
  - Writer: Ignore objects that are not collections in the store and write an output Frame
    - Add `const` where possible
    - Add `[[maybe_unused]]` to avoid a warning about not using the result of a `.release()` (the warning is correct in most cases, in this case it's the Gaudi store who owns it).
    - Remove some code in the Writer that is not necessary.

* 2024-12-10 jmcarcell ([PR#265](https://github.com/key4hep/k4FWCore/pull/265))
  - Remove the check for TrackerHit3D from edm4hep

* 2024-12-09 jmcarcell ([PR#263](https://github.com/key4hep/k4FWCore/pull/263))
  - Mark the DataHandle and DataWrapper destructors with override

* 2024-12-09 jmcarcell ([PR#262](https://github.com/key4hep/k4FWCore/pull/262))
  - Write the configuration_metadata at initialize() in the Writer, to avoid having properties that are deleted because other algorithms are wrapped in a Sequencer and this Sequencer is deleted before `finalize()` is called for Writer.
  - Add tests for different combinations of old/functional algorithms and using PodioOutput or IOSvc and Writer.

* 2024-12-04 jmcarcell ([PR#261](https://github.com/key4hep/k4FWCore/pull/261))
  - Add missing test dependencies

* 2024-11-28 Wouter Deconinck ([PR#257](https://github.com/key4hep/k4FWCore/pull/257))
  - Require podio 1.0.1 in CMake

* 2024-11-01 jmcarcell ([PR#256](https://github.com/key4hep/k4FWCore/pull/256))
  - Update service retrieval after deprecations in Gaudi v39.1, see https://gitlab.cern.ch/gaudi/Gaudi/-/merge_requests/1637
  - Throw when it's not possible to retrieve ApplicationMgr - it should not happen

* 2024-10-30 Mateusz Jakub Fila ([PR#252](https://github.com/key4hep/k4FWCore/pull/252))
  - Added thread-safe`EventCounter` algorithm

# v01-01-02

* 2024-10-29 jmcarcell ([PR#253](https://github.com/key4hep/k4FWCore/pull/253))
  - Fix DataHandle::get() for non-collection types and remove dead code. A `reinterpret_cast` was changed to `static_cast` in https://github.com/key4hep/k4FWCore/pull/250 that made it fail at compile time for non-collection types. Now the `static_cast` is properly wrapped around an `if constexpr` and code around it has been deleted since it looked impossible to trigger.

* 2024-10-28 jmcarcell ([PR#254](https://github.com/key4hep/k4FWCore/pull/254))
  - Don't add duplicated options for ToolSvcs

# v01-01-01

* 2024-10-28 jmcarcell ([PR#250](https://github.com/key4hep/k4FWCore/pull/250))
  - Use `std::unique_ptr<podio::CollectionBase>` for the collections in the store instead of `std::shared_ptr<podio::CollectionBase>`. 
  - Fix leak in the `Writer` that otherwise would have been difficult to fix (without the change above)
  - Clean up `FunctionalUtils.h`: remove some unused overloads and change some names.

* 2024-10-17 jmcarcell ([PR#251](https://github.com/key4hep/k4FWCore/pull/251))
  - Use size_t in the arguments for getting the input and output locations to remove warnings, and fix some comments

* 2024-10-15 jmcarcell ([PR#247](https://github.com/key4hep/k4FWCore/pull/247))
  - Overhaul the `UniqueIDGenSvc`:
    - Use bit manipulation with the `UniqueIDGenSvc` which is about 1000 times faster than the string manipulation that was being done
    - Add a check for the case where an ID is repeated - trying to catch the case where the event number and run number is always the same, then the same random number sequence will be generated.

* 2024-10-09 jmcarcell ([PR#244](https://github.com/key4hep/k4FWCore/pull/244))
  - Update README: remove note about GaudiAlg being outdated since it's not there anymore and Use Key4hep instead of Key4HEP

# v01-01-00

* 2024-10-03 jmcarcell ([PR#243](https://github.com/key4hep/k4FWCore/pull/243))
  - Fix overwritting the input file for IOSvc from the command line. The k4FWCore wrapper of the ApplicationMgr checks the number of events (needed for multithreading) and if the file exists but this was happening right after the parsing of the file, without taking into account the input file can be overwritten from the command line.
  - Add a test where the input file is being overwritten from the command line

* 2024-10-01 jmcarcell ([PR#233](https://github.com/key4hep/k4FWCore/pull/233))
  - Use podio::Reader and podio::Writer with IOSvc, so that it is easy to write TTrees or RNTuples by changing the `IOType` parameter given to `IOSvc` in the steering file.

* 2024-09-30 jmcarcell ([PR#242](https://github.com/key4hep/k4FWCore/pull/242))
  - Add a docstring for the function that adds the arguments
  - Remove dead code, obvious comments and global variables
  - Use f-strings
  - Fix arguments when a property is a std::vector. Related to this fix now `k4run` will also display properties for which the default value is an empty list, previously these were not displayed.
  - Improve formatting of the message with all the values of all the properties

* 2024-09-29 Leonhard Reichenbach ([PR#207](https://github.com/key4hep/k4FWCore/pull/207))
  - Ensure ROOT is in batch mode by setting `gROOT.SetBatch(True)` in `k4run`

* 2024-09-26 jmcarcell ([PR#236](https://github.com/key4hep/k4FWCore/pull/236))
  - Add an argument for add_test_with_env not to have to repeat the test names
  - Also add some documentation for the function since it's getting complicated

* 2024-09-25 jmcarcell ([PR#239](https://github.com/key4hep/k4FWCore/pull/239))
  - Fix histograms for Gaudi v39. After https://gitlab.cern.ch/gaudi/Gaudi/-/merge_requests/1586 the "old" histograms are renamed to  `StaticRootHistogram` from `RootHistogram`
  - Add a configurable histogram: `RootHistogram`, whose bins, title and other properties can be set from python as shown in the test file

* 2024-09-20 jmcarcell ([PR#237](https://github.com/key4hep/k4FWCore/pull/237))
  - Assign to a const char* when using std::getenv

* 2024-09-17 Mateusz Jakub Fila ([PR#223](https://github.com/key4hep/k4FWCore/pull/223))
  - Added tests reading metadata from input file
  - Fixed accessing input file metadata in MetadataSvc

* 2024-09-16 jmcarcell ([PR#234](https://github.com/key4hep/k4FWCore/pull/234))
  - Add the possibility of starting from the Nth event instead of the first one with the argument `FirstEventEntry` to `IOSvc`.
  -  Add a test that ignores some events.

* 2024-09-12 Mateusz Jakub Fila ([PR#224](https://github.com/key4hep/k4FWCore/pull/224))
  - Fixed data race in `EventHeaderCreator` and make it ready for GaudiHive

* 2024-09-10 jmcarcell ([PR#235](https://github.com/key4hep/k4FWCore/pull/235))
  - Use the Key4hepConfig flag to set the standard, compiler flags and rpath magic.

* 2024-09-05 jmcarcell ([PR#232](https://github.com/key4hep/k4FWCore/pull/232))
  - Remove a few unused properties that do not exist anymore or belong to GaudiAlg which is not being built

* 2024-09-05 jmcarcell ([PR#213](https://github.com/key4hep/k4FWCore/pull/213))
  - Fail if `IOSvc` or `ApplicationMgr` are not imported with `from k4FWCore import IOSvc` (it can be imported from `Configurables`). The python wrapper in k4FWCore does a few minor things and importing from `Configurables` doesn't fail currently and can lead to hard to solve issues.
  - Add an error message when reading a file without the `events` category. Currently it fails with an error in python that is not very clear.
  - Use `Input` and `Output` for IOSvc, deprecate `input` and `output` since all the other properties are capitalized. The same for `IOType` instead of `ioType`. This will print a warning about a duplicated property that should be ignored for now.
  - Suppress two warnings about using external input when using the default `EventLoopMgr`.
  - Import `ApplicationMgr` always from `k4FWCore`

* 2024-09-04 Mateusz Jakub Fila ([PR#231](https://github.com/key4hep/k4FWCore/pull/231))
  - Fixed missing checks for accessing metadata with `MetaDataHandle` when used with `IOSvc`

* 2024-09-02 Mateusz Jakub Fila ([PR#228](https://github.com/key4hep/k4FWCore/pull/228))
  - fixed virtual specifiers, removed redundant methods, uncluttered code

* 2024-08-26 jmcarcell ([PR#222](https://github.com/key4hep/k4FWCore/pull/222))
  - Fix warnings when warnings are enabled. Warnings will be enabled later.

* 2024-08-23 Mateusz Jakub Fila ([PR#227](https://github.com/key4hep/k4FWCore/pull/227))
  - fixed typos
  - updated the usage example in readme

* 2024-08-09 jmcarcell ([PR#221](https://github.com/key4hep/k4FWCore/pull/221))
  - Fix wrong error message when using std::map as input or output with transformers

* 2024-08-09 jmcarcell ([PR#220](https://github.com/key4hep/k4FWCore/pull/220))
  - Use Links instead of Associations

* 2024-08-08 jmcarcell ([PR#215](https://github.com/key4hep/k4FWCore/pull/215))
  - Add a metadata service that is automatically added when importing `IOSvc` from `k4FWCore`, if no metadata is added then the service won't do anything. If metadata is added, the metadata service will hold a `podio::Frame` with the metadata that the `Writer` will write. The metadata service only works when there is an output file.
  - Add a test showing an example of its usage
  - Make changes in the `MetadaHandle` to make it possible to save and read metadata from DataHandle-based algorithms, so that when switching to `IOSvc` metadata for existing DataHandle algorithms will keep working. Add a test using IOSvc with DataHandle algorithms that write and read metadata.

* 2024-08-07 jmcarcell ([PR#218](https://github.com/key4hep/k4FWCore/pull/218))
  - Remove a few unnecessary includes

* 2024-07-30 tmadlener ([PR#217](https://github.com/key4hep/k4FWCore/pull/217))
  - Add the legacy `AssociationCollection` headers to keep things building after https://github.com/key4hep/EDM4hep/pull/341 has been merged

* 2024-07-26 jmcarcell ([PR#206](https://github.com/key4hep/k4FWCore/pull/206))
  - Delete the version checks before Podio 1.0

* 2024-07-25 Giovanni Marchiori ([PR#214](https://github.com/key4hep/k4FWCore/pull/214))
  - renamed NoiseConstant by NoiseRMS in the noise interfaces (ICaloReadCellNoiseMap, INoiseCaloCellsTool.h and INoiseConstTool.h) to make its physical meaning clearer

* 2024-07-25 jmcarcell ([PR#210](https://github.com/key4hep/k4FWCore/pull/210))
  - Add a filter algorithm, using the `PredicateFilter` algorithm from Gaudi.
  - Add the minimum handle needed to be able to use the filter from Gaudi and make it work with the rest of the functional algorithms
  - Add a test using this filter

* 2024-07-19 jmcarcell ([PR#209](https://github.com/key4hep/k4FWCore/pull/209))
  - Add an algorithm for merging collections that can merge any number of collections into a new one of the same type. The algorithm supports either creating a subset collection (default) or cloning every object in the collection.
  - Add a test using this algorithm to merge a few collections

* 2024-07-15 jmcarcell ([PR#208](https://github.com/key4hep/k4FWCore/pull/208))
  - Add a new test producing histograms, making use of the corresponding service in Gaudi
  - Improve comments and descriptions in tests

* 2024-07-15 jmcarcell ([PR#201](https://github.com/key4hep/k4FWCore/pull/201))
  - FunctionalAlgorithms: Use std::vector instead of std::map for input or output an arbitrary number of collections.
  - Add an `inputLocations` and `outputLocations` methods to be able to retrieve the locations set in the steering file.

* 2024-06-27 tmadlener ([PR#205](https://github.com/key4hep/k4FWCore/pull/205))
  - Remove some of the drift chamber datatypes again as they are going to disappear in EDM4hep with [EDM4hep#333](https://github.com/key4hep/EDM4hep/pull/333)

* 2024-06-27 tmadlener ([PR#204](https://github.com/key4hep/k4FWCore/pull/204))
  - Remove the `MCRecoTrackerHitPlaneAssociation` since it has been / will be removed from EDM4hep in [EDM4hep#331](https://github.com/key4hep/EDM4hep/pull/331)

* 2024-06-25 jmcarcell ([PR#203](https://github.com/key4hep/k4FWCore/pull/203))
  - Try to find podio 1.0 if the version found is not compatible

* 2024-06-24 jmcarcell ([PR#202](https://github.com/key4hep/k4FWCore/pull/202))
  - Don't use `radiusOfInnermostHit` from EDM4hep tracks

* 2024-06-18 jmcarcell ([PR#200](https://github.com/key4hep/k4FWCore/pull/200))
  - Use `edm4hep::labels`

* 2024-06-05 tmadlener ([PR#199](https://github.com/key4hep/k4FWCore/pull/199))
  - Remove the `rootUtils.h` header that has been copied from podio since it has become obsolete with #171

* 2024-06-05 tmadlener ([PR#195](https://github.com/key4hep/k4FWCore/pull/195))
  - Make `MetaDataHandle::get` throw an exception in case the value for the handle is not (yet) available.
  - Add `MetaDataHandle::get(T defaultValue)` overload to get a default value and no exception in case the value is not (yet) available.
  - Add `MetaDataHandle::get_optional()` to retrieve an optional that is engaged and holds the value if the handle already has a value available.
    - The optional is returned directly from the underlying Frame in case that is available (see  [AIDASoft/podio#580](https://github.com/AIDASoft/podio/pull/580)) or is constructed in place in case it is not yet available.

* 2024-05-29 jmcarcell ([PR#197](https://github.com/key4hep/k4FWCore/pull/197))
  - Remove unnecessary messages (IOSvc saying at which entry we are when reading a file and an error message when a cast doesn't work, which is foreseen and will always happen when a functional algorithm reads something generated by the "old" algorithms), one of them an error message that is not an error
  - Fix tests by making a test throw instead of simply outputting an error message

* 2024-05-28 jmcarcell ([PR#198](https://github.com/key4hep/k4FWCore/pull/198))
  - Remove the dQ/dx information from the tracks. Needed after https://github.com/key4hep/EDM4hep/pull/311

* 2024-05-23 zwu0922 ([PR#193](https://github.com/key4hep/k4FWCore/pull/193))
  - Added ICaloReadCrosstalkMap interface definition to support the service tool that reads the crosstalk map of the ALLEGRO ECAL barrel

* 2024-05-21 jmcarcell ([PR#173](https://github.com/key4hep/k4FWCore/pull/173))
  - Add a new way of using functionals in k4FWCore such that it's possible to run using the Gaudi HiveWhiteBoard (multithreaded) that includes:
  - A new service `IOSvc` to take of the input and output, together with a `Reader` and `Writer` algorithm, to be used instead of `PodioInput` and `PodioOutput`
  - Support for reading and writing an arbitrary (specified at runtime) number of collections, using `std::map<std::string, const Coll&>` for reading and  `std::map<std::string, Coll>`, i.e. for reading we get const references to the collections and for writing we put the collections using `std::move` typically in the map
  - Custom implementations of the Consumer, Producer and Transformer to make all of this possible and the interface simple (for example, if we have an algorithm that outputs a `std::map<std::string, edm4hep::MCParticleCollection>` we only have to set the names in python and all the collections will be in the store available for the next algorithms
  - Some utilities on the python side: automatic insertion of the Reader and Writer algorithms when an input or output files are specified, automatic creation of output folders (see https://github.com/key4hep/k4FWCore/pull/170)
  - Tests for several different situations: functionals reading and outputting to files, using only memory, running multithreaded, functionals with an arbitrary number of collections as input and/or output

* 2024-05-17 Leonhard Reichenbach ([PR#194](https://github.com/key4hep/k4FWCore/pull/194))
  - Make the Lcio2EDM4hepTool a friend of PodioDataSvc to enable event parameter conversion

* 2024-04-25 jmcarcell ([PR#192](https://github.com/key4hep/k4FWCore/pull/192))
  - Set only one handler for logging in k4run, preventing duplication of logging messages

* 2024-04-23 jmcarcell ([PR#191](https://github.com/key4hep/k4FWCore/pull/191))
  - Remove the cpu option in k4run

* 2024-04-18 Juraj Smiesko ([PR#190](https://github.com/key4hep/k4FWCore/pull/190))
  - Delete the object wrapped by a DataWrapper when the DataWrapper is  destructed

* 2024-04-12 tmadlener ([PR#178](https://github.com/key4hep/k4FWCore/pull/178))
  - Add `load_file` to `k4FWCore.utils` module to allow loading more options files in user configuration.
    - Fixes #147 in passing

* 2024-04-02 Juraj Smiesko ([PR#186](https://github.com/key4hep/k4FWCore/pull/186))
  - Handle SIGPIPE signal

* 2024-03-25 Dmitry Kalinkin ([PR#188](https://github.com/key4hep/k4FWCore/pull/188))
  - Fix a bug that was preventing successful build on macOS by not accessing EventDataSvc in the constructor of the `PodioInput`.

* 2024-03-25 Juraj Smiesko ([PR#170](https://github.com/key4hep/k4FWCore/pull/170))
  - Output directories are created if they don't exist.

* 2024-03-18 jmcarcell ([PR#185](https://github.com/key4hep/k4FWCore/pull/185))
  - Use setMomentum for particles, fixing an issue when trying to change a const reference

* 2024-03-15 jmcarcell ([PR#184](https://github.com/key4hep/k4FWCore/pull/184))
  - Fix a few compiler warnings about unused status code

* 2024-03-15 jmcarcell ([PR#172](https://github.com/key4hep/k4FWCore/pull/172))
  - Don't use GaudiAlg since it's not built by default in Gaudi since https://gitlab.cern.ch/gaudi/Gaudi/-/merge_requests/1531

* 2024-02-28 tmadlener ([PR#177](https://github.com/key4hep/k4FWCore/pull/177))
  - Format all python sources using black and add it to pre-commit

* 2024-02-26 jmcarcell ([PR#183](https://github.com/key4hep/k4FWCore/pull/183))
  - Fix mistyped header guard

* 2024-02-26 tmadlener ([PR#181](https://github.com/key4hep/k4FWCore/pull/181))
  - Fix deprecation warnings for getting mutable references, introduce in AIDASoft/podio#553

* 2024-02-23 jmcarcell ([PR#180](https://github.com/key4hep/k4FWCore/pull/180))
  - Fix warnings related to the ROOTFrameReader and ROOTFrameWriter

* 2024-02-22 tmadlener ([PR#174](https://github.com/key4hep/k4FWCore/pull/174))
  - Fix usages of `edm4hep::TrackerHit` to `edm4hep::TrackerHit3D` in order to keep things working after key4hep/EDM4hep#252
    - Check for the existance of the `TrackerHit3D` and tpyedef the `TrackerHit` to `TrackerHit3D` if it does not exist

* 2024-10-03 jmcarcell ([PR#243](https://github.com/key4hep/k4FWCore/pull/243))
  - Fix overwritting the input file for IOSvc from the command line. The k4FWCore wrapper of the ApplicationMgr checks the number of events (needed for multithreading) and if the file exists but this was happening right after the parsing of the file, without taking into account the input file can be overwritten from the command line.
  - Add a test where the input file is being overwritten from the command line

* 2024-10-01 jmcarcell ([PR#233](https://github.com/key4hep/k4FWCore/pull/233))
  - Use podio::Reader and podio::Writer with IOSvc, so that it is easy to write TTrees or RNTuples by changing the `IOType` parameter given to `IOSvc` in the steering file.

* 2024-09-30 jmcarcell ([PR#242](https://github.com/key4hep/k4FWCore/pull/242))
  - Add a docstring for the function that adds the arguments
  - Remove dead code, obvious comments and global variables
  - Use f-strings
  - Fix arguments when a property is a std::vector. Related to this fix now `k4run` will also display properties for which the default value is an empty list, previously these were not displayed.
  - Improve formatting of the message with all the values of all the properties

* 2024-09-29 Leonhard Reichenbach ([PR#207](https://github.com/key4hep/k4FWCore/pull/207))
  - Ensure ROOT is in batch mode by setting `gROOT.SetBatch(True)` in `k4run`

* 2024-09-26 jmcarcell ([PR#236](https://github.com/key4hep/k4FWCore/pull/236))
  - Add an argument for add_test_with_env not to have to repeat the test names
  - Also add some documentation for the function since it's getting complicated

* 2024-09-25 jmcarcell ([PR#239](https://github.com/key4hep/k4FWCore/pull/239))
  - Fix histograms for Gaudi v39. After https://gitlab.cern.ch/gaudi/Gaudi/-/merge_requests/1586 the "old" histograms are renamed to  `StaticRootHistogram` from `RootHistogram`
  - Add a configurable histogram: `RootHistogram`, whose bins, title and other properties can be set from python as shown in the test file

* 2024-09-20 jmcarcell ([PR#237](https://github.com/key4hep/k4FWCore/pull/237))
  - Assign to a const char* when using std::getenv

* 2024-09-17 Mateusz Jakub Fila ([PR#223](https://github.com/key4hep/k4FWCore/pull/223))
  - Added tests reading metadata from input file
  - Fixed accessing input file metadata in MetadataSvc

* 2024-09-16 jmcarcell ([PR#234](https://github.com/key4hep/k4FWCore/pull/234))
  - Add the possibility of starting from the Nth event instead of the first one with the argument `FirstEventEntry` to `IOSvc`.
  -  Add a test that ignores some events.

* 2024-09-12 Mateusz Jakub Fila ([PR#224](https://github.com/key4hep/k4FWCore/pull/224))
  - Fixed data race in `EventHeaderCreator` and make it ready for GaudiHive

* 2024-09-10 jmcarcell ([PR#235](https://github.com/key4hep/k4FWCore/pull/235))
  - Use the Key4hepConfig flag to set the standard, compiler flags and rpath magic.

* 2024-09-05 jmcarcell ([PR#232](https://github.com/key4hep/k4FWCore/pull/232))
  - Remove a few unused properties that do not exist anymore or belong to GaudiAlg which is not being built

* 2024-09-05 jmcarcell ([PR#213](https://github.com/key4hep/k4FWCore/pull/213))
  - Fail if `IOSvc` or `ApplicationMgr` are not imported with `from k4FWCore import IOSvc` (it can be imported from `Configurables`). The python wrapper in k4FWCore does a few minor things and importing from `Configurables` doesn't fail currently and can lead to hard to solve issues.
  - Add an error message when reading a file without the `events` category. Currently it fails with an error in python that is not very clear.
  - Use `Input` and `Output` for IOSvc, deprecate `input` and `output` since all the other properties are capitalized. The same for `IOType` instead of `ioType`. This will print a warning about a duplicated property that should be ignored for now.
  - Suppress two warnings about using external input when using the default `EventLoopMgr`.
  - Import `ApplicationMgr` always from `k4FWCore`

* 2024-09-04 Mateusz Jakub Fila ([PR#231](https://github.com/key4hep/k4FWCore/pull/231))
  - Fixed missing checks for accessing metadata with `MetaDataHandle` when used with `IOSvc`

* 2024-09-02 Mateusz Jakub Fila ([PR#228](https://github.com/key4hep/k4FWCore/pull/228))
  - fixed virtual specifiers, removed redundant methods, uncluttered code

* 2024-08-26 jmcarcell ([PR#222](https://github.com/key4hep/k4FWCore/pull/222))
  - Fix warnings when warnings are enabled. Warnings will be enabled later.

* 2024-08-23 Mateusz Jakub Fila ([PR#227](https://github.com/key4hep/k4FWCore/pull/227))
  - fixed typos
  - updated the usage example in readme

* 2024-08-09 jmcarcell ([PR#221](https://github.com/key4hep/k4FWCore/pull/221))
  - Fix wrong error message when using std::map as input or output with transformers

* 2024-08-09 jmcarcell ([PR#220](https://github.com/key4hep/k4FWCore/pull/220))
  - Use Links instead of Associations

* 2024-08-08 jmcarcell ([PR#215](https://github.com/key4hep/k4FWCore/pull/215))
  - Add a metadata service that is automatically added when importing `IOSvc` from `k4FWCore`, if no metadata is added then the service won't do anything. If metadata is added, the metadata service will hold a `podio::Frame` with the metadata that the `Writer` will write. The metadata service only works when there is an output file.
  - Add a test showing an example of its usage
  - Make changes in the `MetadaHandle` to make it possible to save and read metadata from DataHandle-based algorithms, so that when switching to `IOSvc` metadata for existing DataHandle algorithms will keep working. Add a test using IOSvc with DataHandle algorithms that write and read metadata.

* 2024-08-07 jmcarcell ([PR#218](https://github.com/key4hep/k4FWCore/pull/218))
  - Remove a few unnecessary includes

* 2024-07-30 tmadlener ([PR#217](https://github.com/key4hep/k4FWCore/pull/217))
  - Add the legacy `AssociationCollection` headers to keep things building after https://github.com/key4hep/EDM4hep/pull/341 has been merged

* 2024-07-26 jmcarcell ([PR#206](https://github.com/key4hep/k4FWCore/pull/206))
  - Delete the version checks before Podio 1.0

* 2024-07-25 Giovanni Marchiori ([PR#214](https://github.com/key4hep/k4FWCore/pull/214))
  - renamed NoiseConstant by NoiseRMS in the noise interfaces (ICaloReadCellNoiseMap, INoiseCaloCellsTool.h and INoiseConstTool.h) to make its physical meaning clearer

* 2024-07-25 jmcarcell ([PR#210](https://github.com/key4hep/k4FWCore/pull/210))
  - Add a filter algorithm, using the `PredicateFilter` algorithm from Gaudi.
  - Add the minimum handle needed to be able to use the filter from Gaudi and make it work with the rest of the functional algorithms
  - Add a test using this filter

* 2024-07-19 jmcarcell ([PR#209](https://github.com/key4hep/k4FWCore/pull/209))
  - Add an algorithm for merging collections that can merge any number of collections into a new one of the same type. The algorithm supports either creating a subset collection (default) or cloning every object in the collection.
  - Add a test using this algorithm to merge a few collections

* 2024-07-15 jmcarcell ([PR#208](https://github.com/key4hep/k4FWCore/pull/208))
  - Add a new test producing histograms, making use of the corresponding service in Gaudi
  - Improve comments and descriptions in tests

* 2024-07-15 jmcarcell ([PR#201](https://github.com/key4hep/k4FWCore/pull/201))
  - FunctionalAlgorithms: Use std::vector instead of std::map for input or output an arbitrary number of collections.
  - Add an `inputLocations` and `outputLocations` methods to be able to retrieve the locations set in the steering file.

* 2024-06-27 tmadlener ([PR#205](https://github.com/key4hep/k4FWCore/pull/205))
  - Remove some of the drift chamber datatypes again as they are going to disappear in EDM4hep with [EDM4hep#333](https://github.com/key4hep/EDM4hep/pull/333)

* 2024-06-27 tmadlener ([PR#204](https://github.com/key4hep/k4FWCore/pull/204))
  - Remove the `MCRecoTrackerHitPlaneAssociation` since it has been / will be removed from EDM4hep in [EDM4hep#331](https://github.com/key4hep/EDM4hep/pull/331)

* 2024-06-25 jmcarcell ([PR#203](https://github.com/key4hep/k4FWCore/pull/203))
  - Try to find podio 1.0 if the version found is not compatible

* 2024-06-24 jmcarcell ([PR#202](https://github.com/key4hep/k4FWCore/pull/202))
  - Don't use `radiusOfInnermostHit` from EDM4hep tracks

* 2024-06-18 jmcarcell ([PR#200](https://github.com/key4hep/k4FWCore/pull/200))
  - Use `edm4hep::labels`

* 2024-06-05 tmadlener ([PR#199](https://github.com/key4hep/k4FWCore/pull/199))
  - Remove the `rootUtils.h` header that has been copied from podio since it has become obsolete with #171

* 2024-06-05 tmadlener ([PR#195](https://github.com/key4hep/k4FWCore/pull/195))
  - Make `MetaDataHandle::get` throw an exception in case the value for the handle is not (yet) available.
  - Add `MetaDataHandle::get(T defaultValue)` overload to get a default value and no exception in case the value is not (yet) available.
  - Add `MetaDataHandle::get_optional()` to retrieve an optional that is engaged and holds the value if the handle already has a value available.
    - The optional is returned directly from the underlying Frame in case that is available (see  [AIDASoft/podio#580](https://github.com/AIDASoft/podio/pull/580)) or is constructed in place in case it is not yet available.

* 2024-05-29 jmcarcell ([PR#197](https://github.com/key4hep/k4FWCore/pull/197))
  - Remove unnecessary messages (IOSvc saying at which entry we are when reading a file and an error message when a cast doesn't work, which is foreseen and will always happen when a functional algorithm reads something generated by the "old" algorithms), one of them an error message that is not an error
  - Fix tests by making a test throw instead of simply outputting an error message

* 2024-05-28 jmcarcell ([PR#198](https://github.com/key4hep/k4FWCore/pull/198))
  - Remove the dQ/dx information from the tracks. Needed after https://github.com/key4hep/EDM4hep/pull/311

* 2024-05-23 zwu0922 ([PR#193](https://github.com/key4hep/k4FWCore/pull/193))
  - Added ICaloReadCrosstalkMap interface definition to support the service tool that reads the crosstalk map of the ALLEGRO ECAL barrel

* 2024-05-21 jmcarcell ([PR#173](https://github.com/key4hep/k4FWCore/pull/173))
  - Add a new way of using functionals in k4FWCore such that it's possible to run using the Gaudi HiveWhiteBoard (multithreaded) that includes:
  - A new service `IOSvc` to take of the input and output, together with a `Reader` and `Writer` algorithm, to be used instead of `PodioInput` and `PodioOutput`
  - Support for reading and writing an arbitrary (specified at runtime) number of collections, using `std::map<std::string, const Coll&>` for reading and  `std::map<std::string, Coll>`, i.e. for reading we get const references to the collections and for writing we put the collections using `std::move` typically in the map
  - Custom implementations of the Consumer, Producer and Transformer to make all of this possible and the interface simple (for example, if we have an algorithm that outputs a `std::map<std::string, edm4hep::MCParticleCollection>` we only have to set the names in python and all the collections will be in the store available for the next algorithms
  - Some utilities on the python side: automatic insertion of the Reader and Writer algorithms when an input or output files are specified, automatic creation of output folders (see https://github.com/key4hep/k4FWCore/pull/170)
  - Tests for several different situations: functionals reading and outputting to files, using only memory, running multithreaded, functionals with an arbitrary number of collections as input and/or output

* 2024-05-17 Leonhard Reichenbach ([PR#194](https://github.com/key4hep/k4FWCore/pull/194))
  - Make the Lcio2EDM4hepTool a friend of PodioDataSvc to enable event parameter conversion

* 2024-04-25 jmcarcell ([PR#192](https://github.com/key4hep/k4FWCore/pull/192))
  - Set only one handler for logging in k4run, preventing duplication of logging messages

* 2024-04-23 jmcarcell ([PR#191](https://github.com/key4hep/k4FWCore/pull/191))
  - Remove the cpu option in k4run

* 2024-04-18 Juraj Smiesko ([PR#190](https://github.com/key4hep/k4FWCore/pull/190))
  - Delete the object wrapped by a DataWrapper when the DataWrapper is  destructed

* 2024-04-12 tmadlener ([PR#178](https://github.com/key4hep/k4FWCore/pull/178))
  - Add `load_file` to `k4FWCore.utils` module to allow loading more options files in user configuration.
    - Fixes #147 in passing

* 2024-04-02 Juraj Smiesko ([PR#186](https://github.com/key4hep/k4FWCore/pull/186))
  - Handle SIGPIPE signal

* 2024-03-25 Dmitry Kalinkin ([PR#188](https://github.com/key4hep/k4FWCore/pull/188))
  - Fix a bug that was preventing successful build on macOS by not accessing EventDataSvc in the constructor of the `PodioInput`.

* 2024-03-25 Juraj Smiesko ([PR#170](https://github.com/key4hep/k4FWCore/pull/170))
  - Output directories are created if they don't exist.

* 2024-03-18 jmcarcell ([PR#185](https://github.com/key4hep/k4FWCore/pull/185))
  - Use setMomentum for particles, fixing an issue when trying to change a const reference

* 2024-03-15 jmcarcell ([PR#184](https://github.com/key4hep/k4FWCore/pull/184))
  - Fix a few compiler warnings about unused status code

* 2024-03-15 jmcarcell ([PR#172](https://github.com/key4hep/k4FWCore/pull/172))
  - Don't use GaudiAlg since it's not built by default in Gaudi since https://gitlab.cern.ch/gaudi/Gaudi/-/merge_requests/1531

* 2024-02-28 tmadlener ([PR#177](https://github.com/key4hep/k4FWCore/pull/177))
  - Format all python sources using black and add it to pre-commit

* 2024-02-26 jmcarcell ([PR#183](https://github.com/key4hep/k4FWCore/pull/183))
  - Fix mistyped header guard

* 2024-02-26 tmadlener ([PR#181](https://github.com/key4hep/k4FWCore/pull/181))
  - Fix deprecation warnings for getting mutable references, introduce in AIDASoft/podio#553

* 2024-02-23 jmcarcell ([PR#180](https://github.com/key4hep/k4FWCore/pull/180))
  - Fix warnings related to the ROOTFrameReader and ROOTFrameWriter

* 2024-02-22 tmadlener ([PR#174](https://github.com/key4hep/k4FWCore/pull/174))
  - Fix usages of `edm4hep::TrackerHit` to `edm4hep::TrackerHit3D` in order to keep things working after key4hep/EDM4hep#252
    - Check for the existance of the `TrackerHit3D` and tpyedef the `TrackerHit` to `TrackerHit3D` if it does not exist

