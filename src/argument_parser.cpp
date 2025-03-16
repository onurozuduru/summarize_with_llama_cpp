///////////////////////////////////////////////////////////////////////////////
// File: argument_parser.cpp
//
// License: MIT
//
// Copyright (C) 2025 Onur Ozuduru
//
// Follow Me!
//   github: github.com/onurozuduru
///////////////////////////////////////////////////////////////////////////////

#include "argument_parser.h"
#include <any>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

ArgumentParser::ArgumentParser(std::string description)
    : program_description(std::move(description)) {
  add_flag("help", "h", "Show this help message", false);
}

void ArgumentParser::process_option(const std::string &name, int &current_index,
                                    int argc, char **argv) {
  if (!options.contains(name)) {
    throw std::runtime_error("Unknown option: --" + name);
  }

  Option &option = options[name];
  option.is_provided = true;

  if (option.is_flag) {
    option.value = true;
    return;
  }

  if (current_index + 1 >= argc) {
    throw std::runtime_error("Option --" + name + " requires a value");
  }

  std::string value = argv[++current_index];
  try {
    option.value = option.converter(value);
  } catch (const std::exception &e) {
    throw std::runtime_error("Failed to parse value for --" + name + ": " +
                             e.what());
  }
}

std::string ArgumentParser::generate_help() const {
  std::stringstream sstream;
  sstream << "Usage: " << program_name << " [OPTIONS]";

  sstream << "\n\n";

  if (!program_description.empty()) {
    sstream << program_description << "\n\n";
  }

  sstream << "Options:\n";

  // Use fixed column width instead of calculating exact padding
  constexpr int OPTION_COLUMN_WIDTH = 25;

  for (const auto &[name, opt] : options) {
    std::string option_text = "  ";

    if (!opt.short_name.empty()) {
      option_text += "-" + opt.short_name + ", ";
    }

    option_text += "--" + name;

    if (option_text.length() < OPTION_COLUMN_WIDTH) {
      sstream << option_text
              << std::string(OPTION_COLUMN_WIDTH - option_text.length(), ' ');
    } else {
      sstream << option_text << "\n" << std::string(OPTION_COLUMN_WIDTH, ' ');
    }

    sstream << opt.description;

    if (opt.is_mandatory) {
      sstream << " [required]";
    }

    sstream << "\n";
  }

  return sstream.str();
}

void ArgumentParser::validate_option_name(const std::string &name,
                                          bool is_short) const {
  if (name.empty()) {
    throw std::invalid_argument("Option name cannot be empty");
  }

  if (is_short && name.length() != 1) {
    throw std::invalid_argument("Short option name must be a single character");
  }

  if (options.contains(name)) {
    throw std::invalid_argument("Option name '" + name + "' already exists");
  }

  if (is_short && short_to_long.contains(name)) {
    throw std::invalid_argument("Short option name '" + name +
                                "' already exists");
  }
}

void ArgumentParser::process_arguments(int argc, char **argv) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    // Handle separator: everything after -- is a positional argument
    if (arg == "--") {
      collect_remaining_positional(i + 1, argc, argv);
      break;
    }
    // Handle long options (--option)
    else if (arg.starts_with("--")) {
      std::string name = arg.substr(2);
      process_option(name, i, argc, argv);
    }
    // Handle short options (-o)
    else if (arg.starts_with("-") && arg.length() > 1) {
      std::string short_name = arg.substr(1);
      if (short_to_long.contains(short_name)) {
        process_option(short_to_long[short_name], i, argc, argv);
      } else {
        throw std::runtime_error("Unknown option: " + arg);
      }
    }
    // Everything else is a positional argument
    else {
      positional.push_back(arg);
    }
  }
}

void ArgumentParser::collect_remaining_positional(std::size_t start_index,
                                                  int argc, char **argv) {
  for (auto i = start_index; i < argc; ++i) {
    positional.push_back(argv[i]);
  }
}

bool ArgumentParser::is_help_requested() const {
  return options.contains("help") &&
         std::any_cast<bool>(options.at("help").value);
}

void ArgumentParser::validate_required_options() const {
  std::vector<std::string> missing;

  for (const auto &[name, opt] : options) {
    if (opt.is_mandatory && !opt.is_provided) {
      missing.push_back(name);
    }
  }

  if (!missing.empty()) {
    std::stringstream sstream;
    sstream << "Missing required arguments: ";
    for (size_t i = 0; i < missing.size(); ++i) {
      sstream << "--" << missing[i];
      if (i < missing.size() - 1) {
        sstream << ", ";
      }
    }
    throw std::runtime_error(sstream.str());
  }
}

ArgumentParser &ArgumentParser::add_flag(const std::string &name,
                                         const std::string &short_name,
                                         const std::string &description,
                                         bool is_mandatory) {
  return register_option<bool>(name, short_name, description, true,
                               is_mandatory, false);
}

void ArgumentParser::parse(int argc, char **argv) {
  if (argc < 1) {
    throw std::runtime_error("Invalid argument count");
  }

  program_name = argv[0];
  positional.clear();

  // Process all arguments
  process_arguments(argc, argv);

  // Handle help flag first
  if (is_help_requested()) {
    print_help();
    exit(0);
  }

  // Validate that all required options were provided
  validate_required_options();
}

bool ArgumentParser::is_provided(const std::string &name) const {
  if (!options.contains(name)) {
    throw std::runtime_error("Option not registered: " + name);
  }

  return options.at(name).is_provided;
}

const std::vector<std::string> &ArgumentParser::get_positional() const {
  return positional;
}

void ArgumentParser::print_help() const {
  std::cout << generate_help() << std::endl;
}
