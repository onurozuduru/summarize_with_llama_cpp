///////////////////////////////////////////////////////////////////////////////
// File: main.cpp
//
// License: MIT
//
// Copyright (C) 2025 Onur Ozuduru
//
// Follow Me!
//   github: github.com/onurozuduru
///////////////////////////////////////////////////////////////////////////////

#include "argument_parser.h"
#include "model.h"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
  try {
    // Parse command line arguments
    ArgumentParser parser{"Document Summarizer\nIt reads from stdin and "
                          "summarizes the input text with the given model."};
    parser.add_option<float>("temperature", "t", "The temperature", false, 0.5f)
        .add_option<std::string>("model", "m", "The path to the model file",
                                 false, std::string{DEFAULT_MODEL_PATH})
        .parse(argc, argv);

    const auto temperature = parser.get_option<float>("temperature");
    const auto model_path = parser.get_option<std::string>("model");

    const std::int32_t number_of_gpu_layers{99};
    const std::size_t prediction_length{512U};

    // Read prompt_context from stdin
    const std::string prompt_context{std::istreambuf_iterator<char>(std::cin),
                                     std::istreambuf_iterator<char>()};

    // Check if anything was provided
    if (prompt_context.empty()) {
      std::cout << "Nothing to summarize!" << std::endl;
      return 0;
    }

    std::cout << "Model path: " << model_path << std::endl;

    // Set the system and user prompts
    const std::string system_prompt{
        "You are a document summarizer. User will provide a technical text and "
        "you will summarize it. Be brief and direct. Include only essential "
        "information. Keep your summary short with few sentences. Only focus "
        "on human readable text. Write ONLY 3-5 sentences, then "
        "stop.\n\nTEXT:\n"};
    const std::string user_prompt_end{"\n\nSHORT SUMMARY (Be brief and "
                                      "precise, stop after 3-5 sentences):\n"};

    std::string user_prompt;
    user_prompt.reserve(prompt_context.size() + user_prompt_end.size());
    user_prompt.append(prompt_context).append(user_prompt_end);

    std::vector<llama_chat_message> messages;
    messages.push_back({"system", system_prompt.c_str()});
    messages.push_back({"user", user_prompt.c_str()});

    auto model = model_wrapper::Model{model_path, temperature,
                                      number_of_gpu_layers, prediction_length};

    model.generate_response(model.get_formatted_prompt(messages), std::cout);

  } catch (const std::exception &e) {
    std::cerr << "Failed: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
