function(target_disable_warnings tgt)
    if(MSVC)
        target_compile_options(${tgt} PRIVATE /W0)
    else()
        target_compile_options(${tgt} PRIVATE -w)
    endif()
endfunction()

function(target_enable_warnings tgt)
    if(MSVC)
        target_compile_options(${tgt} PRIVATE /W4
            # Disable the following warnings to match clang/gcc
            /wd4018 # 'expression': signed/unsigned mismatch
            /wd4389 # 'operator': signed/unsigned mismatch
            /wd4244 # 'conversion_type': conversion from 'type1' to 'type2', possible loss of data
            /wd4267 # 'variable': conversion from 'size_t' to 'type', possible loss of data
        )
    else()
        target_compile_options(${tgt} PRIVATE -Wall -Wextra -pedantic -Wshadow)
    endif()
endfunction()

function(target_enable_conversion_warnings tgt)
    if(MSVC)
        target_compile_options(${tgt} PRIVATE
            /w44018 # 'expression': signed/unsigned mismatch
            /w44389 # 'operator': signed/unsigned mismatch
            /w44244 # 'conversion_type': conversion from 'type1' to 'type2', possible loss of data
            /w44267 # 'variable': conversion from 'size_t' to 'type', possible loss of data
        )
    else()
        target_compile_options(${tgt} PRIVATE -Wconversion)
    endif()
endfunction()

function(source_enable_conversion_warnings)
    foreach(filename ${ARGV})
        if(MSVC)
            set_property(SOURCE "${filename}" APPEND PROPERTY COMPILE_FLAGS "/w44018 /w44389 /w44244 /w44267")
        else()
            set_property(SOURCE "${filename}" APPEND PROPERTY COMPILE_FLAGS -Wconversion)
        endif()
    endforeach()
endfunction()
