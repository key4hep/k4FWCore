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

