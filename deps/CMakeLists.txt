include(FetchContent)

# Setup directory to which dependencies will be downloaded
get_filename_component(deps_content "../deps_content"
                       REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(FETCHCONTENT_BASE_DIR ${deps_content})

# If not available fmt download it and make available
FetchContent_Declare(fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG 9cb347b4b2e80fc9fbf57b8621746663c3f870f6)

FetchContent_MakeAvailable(fmt)

# If use IDE add fmt target into deps/googletest group
set_target_properties(fmt PROPERTIES FOLDER deps)

# If not available VersionHelper download it and make available
FetchContent_Declare(
  VersionHelper
  GIT_REPOSITORY https://github.com/rokoDev/VersionHelper.git
  GIT_TAG        c50c9f6743f8791439313246c56f476a7b1e0b84
)
FetchContent_GetProperties(VersionHelper)
if(NOT versionhelper_POPULATED)
  FetchContent_Populate(VersionHelper)
  list(APPEND CMAKE_MODULE_PATH "${versionhelper_SOURCE_DIR}")
endif()

# Generate sources with information about ndt version
include(VersionInfoUtils)
m_generate_version_info(PROJECT_NAME ${ProjectName}
	                    CPP_NAMESPACE "ndt"
	                    BUILD_TYPES Debug Release
	                    IDE_SRC_GROUP "generated"
	                    TARGET_NAME "ndt")