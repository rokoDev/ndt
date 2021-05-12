include(FetchContent)

# Setup directory to which dependencies will be downloaded
get_filename_component(deps_content "../deps_content"
                       REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(FETCHCONTENT_BASE_DIR ${deps_content})

# If dependencies are not available then download it and make available
FetchContent_Declare(
	fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG        9cb347b4b2e80fc9fbf57b8621746663c3f870f6
)

FetchContent_Declare(
	VersionHelper
    GIT_REPOSITORY https://github.com/rokoDev/VersionHelper.git
    GIT_TAG        cea7d1349d5f5167c045b90d12e91d2fefdc99b6
)

FetchContent_Declare(
	boost_pfr
	GIT_REPOSITORY https://github.com/boostorg/pfr.git
	GIT_TAG        da12b52759ea52c3e2341690f208fea898643bbb
)

# If necessary it's possible to use VersionHelper from different location instead of downloaded from GitHub
# string(TOUPPER VersionHelper UP_VersionHelper)
# set(FETCHCONTENT_SOURCE_DIR_${UP_VersionHelper} ${CMAKE_CURRENT_LIST_DIR}/../../VersionHelper)

FetchContent_MakeAvailable(fmt VersionHelper boost_pfr)

# If use IDE add fmt target into deps source group
set_target_properties(fmt PROPERTIES FOLDER deps)

# Generate sources with information about ndt version
list(APPEND CMAKE_MODULE_PATH "${versionhelper_SOURCE_DIR}")
include(VersionInfoUtils)
m_generate_version_info(PROJECT_NAME ${ProjectName}
	                    CPP_NAMESPACE "ndt"
	                    BUILD_TYPES Debug Release
	                    IDE_SRC_GROUP "generated"
	                    TARGET_NAME "ndt")