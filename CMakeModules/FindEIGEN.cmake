# - Find EIGEN
# Find the EIGEN includes
#
#  EIGEN_INCLUDE_DIR - Where to find EIGEN include sub-directory.
#  EIGEN_FOUND       - True if XIOT found.


IF (EIGEN_INCLUDE_DIR)
  # Already in cache, be silent
  SET(EIGEN_FIND_QUIETLY TRUE)
ENDIF (EIGEN_INCLUDE_DIR)

FIND_PATH(EIGEN_INCLUDE_DIR Eigen/SVD)

# Handle the QUIETLY and REQUIRED arguments and set XIOT_FOUND to
# TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  EIGEN DEFAULT_MSG
  EIGEN_INCLUDE_DIR
)

MARK_AS_ADVANCED( EIGEN_INCLUDE_DIR )
