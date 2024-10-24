cmake_minimum_required (VERSION 3.8)
project(AllianceSagePlayer-deps)

set(SDL2_VERSION "2.30.3")
set(SDL2_IMAGE_VERSION "2.8.2")
set(SDL2_TTF_VERSION "2.22.0")

set(DIR_SDL2 ${CMAKE_CURRENT_LIST_DIR}/SDL2-${SDL2_VERSION})
set(DIR_SDL2_IMAGE ${CMAKE_CURRENT_LIST_DIR}/SDL2_image-${SDL2_IMAGE_VERSION})
set(DIR_SDL2_TTF ${CMAKE_CURRENT_LIST_DIR}/SDL2_ttf-${SDL2_TTF_VERSION})
set(DIR_SPINE_CPP ${CMAKE_CURRENT_LIST_DIR}/spine-cpp-3.8)

set(DIR_TEMP ${CMAKE_CURRENT_BINARY_DIR}/temp)

function(download_and_extract HOST_URL ARCHIVE_NAME)
	if (NOT EXISTS "${DIR_TEMP}/${ARCHIVE_NAME}.zip")
		file(DOWNLOAD "${HOST_URL}" "${DIR_TEMP}/${ARCHIVE_NAME}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${DIR_TEMP}/${ARCHIVE_NAME}")
		file(ARCHIVE_EXTRACT INPUT "${DIR_TEMP}/${ARCHIVE_NAME}.zip" DESTINATION "${DIR_TEMP}")
	endif()
endfunction()

function(copy_files SRC_GLOB DIR_DST MATCH_CONDITION)
	file(GLOB SRC_FILES "${SRC_GLOB}")
	foreach(FILE IN LISTS SRC_FILES)
		if (NOT MATCH_CONDITION OR NOT FILE MATCHES "${MATCH_CONDITION}")
			file(COPY ${FILE} DESTINATION "${DIR_DST}")
		endif()
	endforeach()
endfunction()

function(replace_names_in_file FILE_PATH OLD_NAME NEW_NAME)
	file(READ ${FILE_PATH} FILE)
	string(REPLACE ${OLD_NAME} ${NEW_NAME} MODIFIED_FILE "${FILE}")
	file(WRITE ${FILE_PATH} "${MODIFIED_FILE}")
endfunction()

# Fetch SDL2
if (NOT EXISTS "${DIR_SDL2}")
	set(SDL_VC_BUILT "SDL2-devel-${SDL2_VERSION}-VC")
	download_and_extract("https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/${SDL_VC_BUILT}.zip" ${SDL_VC_BUILT})

	copy_files("${DIR_TEMP}/SDL2-${SDL2_VERSION}/include/*.h" "${DIR_SDL2}/include/SDL2" "test")
	copy_files("${DIR_TEMP}/SDL2-${SDL2_VERSION}/lib/x64/*.lib" "${DIR_SDL2}/lib/x64" "test")
	copy_files("${DIR_TEMP}/SDL2-${SDL2_VERSION}/lib/x64/*.dll" "${CMAKE_CURRENT_LIST_DIR}/../../x64/Debug" "")
	copy_files("${DIR_TEMP}/SDL2-${SDL2_VERSION}/lib/x64/*.dll" "${CMAKE_CURRENT_LIST_DIR}/../../x64/Release" "")
endif()

set(
	HEADERS_TO_RENAME
	"\"SDL.h\":<SDL2/SDL.h>"
	"\"SDL_version.h\":<SDL2/SDL_version.h>"
	"\"begin_code.h\":<SDL2/begin_code.h>"
	"\"close_code.h\":<SDL2/close_code.h>"
)

# Fetch SDL2_image
if (NOT EXISTS "${DIR_SDL2_IMAGE}")
	set(SDL_IMAGE_VC_BUILT "SDL2_image-devel-${SDL2_IMAGE_VERSION}-VC")
	download_and_extract("https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/${SDL_IMAGE_VC_BUILT}.zip" ${SDL_IMAGE_VC_BUILT})

	copy_files("${DIR_TEMP}/SDL2_image-${SDL2_IMAGE_VERSION}/include/*.h" "${DIR_SDL2_IMAGE}/include/SDL2_image" "")
	copy_files("${DIR_TEMP}/SDL2_image-${SDL2_IMAGE_VERSION}/lib/x64/*.lib" "${DIR_SDL2_IMAGE}/lib/x64" "")
	copy_files("${DIR_TEMP}/SDL2_image-${SDL2_IMAGE_VERSION}/lib/x64/*.dll" "${CMAKE_CURRENT_LIST_DIR}/../../x64/Debug" "")
	copy_files("${DIR_TEMP}/SDL2_image-${SDL2_IMAGE_VERSION}/lib/x64/*.dll" "${CMAKE_CURRENT_LIST_DIR}/../../x64/Release" "")

	# Rename include header
	foreach(HEADERS IN LISTS HEADERS_TO_RENAME)
		string(REPLACE ":" ";" HEADER ${HEADERS})
		list(GET HEADER 0 OLD_HEADER)
		list(GET HEADER 1 NEW_HEADER)
		replace_names_in_file("${DIR_SDL2_IMAGE}/include/SDL2_image/SDL_image.h" ${OLD_HEADER} ${NEW_HEADER})
	endforeach()
endif()

# Fetch SDL2_TTF
if (NOT EXISTS "${DIR_SDL2_TTF}")
	set(SDL_TTF_VC_BUILT "SDL2_ttf-devel-${SDL2_TTF_VERSION}-VC")
	download_and_extract("https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/${SDL_TTF_VC_BUILT}.zip" ${SDL_TTF_VC_BUILT})

	copy_files("${DIR_TEMP}/SDL2_ttf-${SDL2_TTF_VERSION}/include/*.h" "${DIR_SDL2_TTF}/include/SDL2_ttf" "")
	copy_files("${DIR_TEMP}/SDL2_ttf-${SDL2_TTF_VERSION}/lib/x64/*.lib" "${DIR_SDL2_TTF}/lib/x64" "")
	copy_files("${DIR_TEMP}/SDL2_ttf-${SDL2_TTF_VERSION}/lib/x64/*.dll" "${CMAKE_CURRENT_LIST_DIR}/../../x64/Debug" "")
	copy_files("${DIR_TEMP}/SDL2_ttf-${SDL2_TTF_VERSION}/lib/x64/*.dll" "${CMAKE_CURRENT_LIST_DIR}/../../x64/Release" "")

	# Rename include header
	foreach(HEADERS IN LISTS HEADERS_TO_RENAME)
		string(REPLACE ":" ";" HEADER ${HEADERS})
		list(GET HEADER 0 OLD_HEADER)
		list(GET HEADER 1 NEW_HEADER)
		replace_names_in_file("${DIR_SDL2_TTF}/include/SDL2_ttf/SDL_ttf.h" ${OLD_HEADER} ${NEW_HEADER})
	endforeach()
endif()

#Fetch spine-cpp 3.8
if (NOT EXISTS "${DIR_SPINE_CPP}")
	set (SPINE_RUNTIME_URL "https://github.com/EsotericSoftware/spine-runtimes/archive/refs/heads/3.8.zip")
	download_and_extract("${SPINE_RUNTIME_URL}" "spine-runtimes-3.8")

	file(COPY "${DIR_TEMP}/spine-runtimes-3.8/spine-cpp/spine-cpp/src" DESTINATION "${DIR_SPINE_CPP}")
	file(COPY "${DIR_TEMP}/spine-runtimes-3.8/spine-cpp/spine-cpp/include" DESTINATION "${DIR_SPINE_CPP}")
endif()
