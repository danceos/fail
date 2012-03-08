
option( EXP_MHTEST    "MH Testcampaign" on) 
option( EXP_FAULTCOV   "Fault coverage experiment" OFF) 
option( EXP_HSCSIMPLE    "HSC simple experiment" OFF) 
option( EXP_COOLCHECKSUM "Test campaign for chb's cool checksum" OFF) 
option( EXP_CHECKSUM_OOSTUBS "OOStuBS Checksum" OFF) 


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/experiments.hpp.in
		${CMAKE_CURRENT_BINARY_DIR}/experiments.hpp @ONLY}
)


