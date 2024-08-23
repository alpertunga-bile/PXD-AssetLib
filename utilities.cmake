function(string_starts_with str search)
  string(FIND "${str}" "${search}" out)
  if("${out}" EQUAL 0)
    return(true)
  endif()
  return(false)
endfunction()

function(copy_dll_files project_target binary_dir dll_files)

  foreach(dll_file IN LISTS dll_files)
    get_filename_component(DLL_FILENAME ${dll_file} NAME_WE)
    add_custom_target(${DLL_FILENAME}_DLL_COPY_TARGET 
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dll_file} ${binary_dir}
    )

    add_dependencies(${project_target} ${DLL_FILENAME}_DLL_COPY_TARGET)
  endforeach()
  
endfunction()

