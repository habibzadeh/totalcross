CURRENT_DIR=${PWD}
cd ${PWD}/TotalCross/TotalCrossVM/builders/gcc-palm/tcvm
PALM_BASE_DIR=${PWD}
export PALM_BASE_DIR
make_580 -f palmposix.mk
cd ${CURRENT_DIR}/Litebase/LitebaseSDK/builders/gcc-palm
make_580 $1 -j $NUMBER_OF_PROCESSORS
