#---------------------------------------------
# Macro declared in "DeclareMRPTLib.cmake":
#---------------------------------------------
define_mrpt_lib(
	# Lib name
	topography
	# Dependencies
	mrpt-obs
	)

if(BUILD_mrpt-topography)
	if(CMAKE_MRPT_HAS_WXWIDGETS)
		target_link_libraries(topography PRIVATE imp_wxwidgets)
	endif()
endif()
