###############################################################################
#File: ModelConfig.cmake
#
#License: MIT
#
#Copyright (C) 2025 Onur Ozuduru
#
#Follow Me!
#  github: github.com/onurozuduru
###############################################################################

# Model configuration options
set(MODEL_URL
    "https://huggingface.co/HuggingFaceTB/SmolLM2-1.7B-Instruct-GGUF/resolve/main/smollm2-1.7b-instruct-q4_k_m.gguf"
    CACHE STRING
    "URL to download GGUF model"
)

set(MODEL_NAME "smollm2.gguf" CACHE STRING "Name of the model file")

set(MODEL_PATH
    "${CMAKE_BINARY_DIR}/models"
    CACHE PATH
    "Directory to store models"
)

# Create model directory if it doesn't exist
file(MAKE_DIRECTORY ${MODEL_PATH})

# Logic to check for model and download if needed
if(NOT EXISTS "${MODEL_PATH}/${MODEL_NAME}" AND MODEL_URL)
    message(STATUS "Downloading model ${MODEL_NAME} from ${MODEL_URL}")

    file(
        DOWNLOAD ${MODEL_URL} "${MODEL_PATH}/${MODEL_NAME}"
        SHOW_PROGRESS
        STATUS STATUS_CODE
        TLS_VERIFY ON
    )

    if(NOT STATUS_CODE EQUAL 0)
        message(WARNING "Failed to download model!")
    else()
        message(
            STATUS
            "Model downloaded successfully to ${MODEL_PATH}/${MODEL_NAME}"
        )
    endif()
elseif(EXISTS "${MODEL_PATH}/${MODEL_NAME}")
    message(STATUS "Using existing model at ${MODEL_PATH}/${MODEL_NAME}")
else()
    message(
        WARNING
        "No model specified or found. Please provide MODEL_URL or place model at ${MODEL_PATH}/${MODEL_NAME}"
    )
endif()

# Configure model path as a define for dynamic loading
add_compile_definitions(DEFAULT_MODEL_PATH="${MODEL_PATH}/${MODEL_NAME}")
