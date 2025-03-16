###############################################################################
#File: StatusMessage.cmake
#
#License: MIT
#
#Copyright (C) 2025 Onur Ozuduru
#
#Follow Me!
#  github: github.com/onurozuduru
###############################################################################

# Define colors for status messages (only on non-Windows platforms)
if(NOT WIN32)
    string(ASCII 27 Esc)
    set(COLOR_RESET "${Esc}[m")
    set(COLOR_CYAN "${Esc}[36m")
    set(COLOR_YELLOW "${Esc}[33m")
else()
    set(COLOR_RESET "")
    set(COLOR_CYAN "")
    set(COLOR_YELLOW "")
endif()

# Function to print a colorized status message with key and value
function(print_status key value)
    # Print formatted key-value pair
    message(STATUS "${COLOR_CYAN}${key}: ${COLOR_YELLOW}${value}${COLOR_RESET}")
endfunction()
