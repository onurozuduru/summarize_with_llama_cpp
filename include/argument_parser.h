///////////////////////////////////////////////////////////////////////////////
// File: argument_parser.h
//
// License: MIT
//
// Copyright (C) 2025 Onur Ozuduru
//
// Follow Me!
//   github: github.com/onurozuduru
///////////////////////////////////////////////////////////////////////////////

#include <any>
#include <functional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

/**
 * \brief A simple command line argument parser
 */
class ArgumentParser {
private:
  /**
   * \brief Represents a command line option with its properties and value
   */
  struct Option {
    /**
     * \brief Long name of the option without leading dashes
     */
    std::string name;

    /** \brief Short name of the option (single character) without leading dash
     */
    std::string short_name;

    /**
     * \brief Description text shown in help message
     */
    std::string description;

    /**
     * \brief True if option is a flag (doesn't take a value)
     */
    bool is_flag;

    /**
     * \brief True if option is required/mandatory
     */
    bool is_mandatory;

    /**
     * \brief The option's value (type-erased)
     */
    std::any value;

    /**
     * \brief Converter function to parse string argument into typed value
     */
    std::function<std::any(const std::string &)> converter;

    /**
     * \brief True if option was provided on command line
     */
    bool is_provided{false};
  };

  /**
   * \brief Maps long option names to their corresponding Option objects
   */
  std::unordered_map<std::string, Option> options;

  /**
   * \brief Maps short option names to their corresponding long option names
   * for quick lookup
   */
  std::unordered_map<std::string, std::string> short_to_long;

  /**
   * \brief Stores positional command line arguments (not associated with any
   * option)
   */
  std::vector<std::string> positional;

  /**
   * \brief Name of the program (extracted from argv[0])
   */
  std::string program_name;

  /**
   * \brief Description of the program to show in help message
   */
  std::string program_description;

  /**
   * \brief Converts a string argument to a typed value
   *
   * \tparam T The target type to convert to (string, bool, int, float, double)
   * \param val The string value to convert
   * \return std::any The converted value wrapped in std::any
   * \throws std::runtime_error If T is not one of the supported types
   * \throws std::invalid_argument If the string cannot be converted to the
   * requested type
   * \throws std::out_of_range If the converted value would be out of range for
   * the target type
   */
  template <typename T> static std::any convert(const std::string &val) {
    if constexpr (std::is_same_v<T, std::string>) {
      return val;
    } else if constexpr (std::is_same_v<T, bool>) {
      return (val == "true" || val == "1" || val == "yes");
    } else if constexpr (std::is_same_v<T, int>) {
      return std::stoi(val);
    } else if constexpr (std::is_same_v<T, float>) {
      return std::stof(val);
    } else if constexpr (std::is_same_v<T, double>) {
      return std::stod(val);
    } else {
      throw std::runtime_error("Unsupported type conversion");
    }
  }

  /**
   * \brief Process a single option from the command line
   *
   * \param name The long name of the option being processed
   * \param current_index Current index in argv array, may be updated if option
   * consumes a value
   * \param argc Number of command line arguments
   * \param argv Array of command line arguments
   *
   * \throws std::runtime_error If the option is unknown or value parsing fails
   * \throws std::runtime_error If a non-flag option is provided without a value
   *
   * \details Handles both flag options (which don't require a value) and value
   * options (which consume the next argument as their value). For value
   * options, the index cuurent_index is incremented to skip the value in the
   * main parsing loop.
   */
  void process_option(const std::string &name, int &current_index, int argc,
                      char **argv);

  /**
   * \brief Generates help text for command line usage
   * \return Formatted help string
   *
   * \details Creates a help message showing program usage, description,
   *          and available options in a readable format
   */
  std::string generate_help() const;

  /**
   * \brief Validate option names and check for duplicates
   * \param name The option name to validate
   * \param is_short Whether this is a short option name
   * \throws std::invalid_argument if the name is invalid or already exists
   */
  void validate_option_name(const std::string &name,
                            bool is_short = false) const;

  /**
   * \brief Internal helper method to register any type of option
   *
   * \tparam T The type of the option value
   * \param name Long option name
   * \param short_name Short option name
   * \param description Option description
   * \param is_flag Whether this option is a flag (no value)
   * \param is_mandatory Whether this option is required
   * \param default_val Default value
   * \return Reference to this parser for method chaining
   */
  template <typename T>
  ArgumentParser &register_option(const std::string &name,
                                  const std::string &short_name,
                                  const std::string &description, bool is_flag,
                                  bool is_mandatory, T default_val) {
    validate_option_name(name);
    if (!short_name.empty()) {
      validate_option_name(short_name, true);
    }

    Option opt{name,         short_name,  description, is_flag,
               is_mandatory, default_val, convert<T>};
    options[name] = opt;
    if (!short_name.empty()) {
      short_to_long[short_name] = name;
    }
    return *this;
  }

  /**
   * \brief Process all command line arguments
   * \param argc Argument count
   * \param argv Argument array
   */
  void process_arguments(int argc, char **argv);

  /**
   * \brief Add all remaining arguments as positional
   * \param start_index Starting index in argv
   * \param argc Argument count
   * \param argv Argument array
   */
  void collect_remaining_positional(std::size_t start_index, int argc,
                                    char **argv);

  /**
   * \brief Check if help was requested
   * \return true if help option was provided
   */
  bool is_help_requested() const;

  /**
   * \brief Validate that all required options were provided
   * \throws std::runtime_error if any mandatory option is missing
   */
  void validate_required_options() const;

public:
  /**
   * \brief Construct a new Argument Parser
   * \param description Program description text
   */
  explicit ArgumentParser(std::string description = "");

  /**
   * \brief Add an option that takes a value
   * \tparam T Type of the option value
   * \param name Long option name
   * \param short_name Short option name (single character)
   * \param description Option description
   * \param is_mandatory Whether the option is required
   * \param default_val Default value if not provided
   * \return Reference to this parser for method chaining
   */
  template <typename T>
  ArgumentParser &add_option(const std::string &name,
                             const std::string &short_name,
                             const std::string &description, bool is_mandatory,
                             T default_val = T{}) {
    return register_option<T>(name, short_name, description, false,
                              is_mandatory, default_val);
  }

  /**
   * \brief Add a flag option
   * \param name Long option name
   * \param short_name Short option name (single character)
   * \param description Option description
   * \param is_mandatory Whether the option is required
   * \return Reference to this parser for method chaining
   */
  ArgumentParser &add_flag(const std::string &name,
                           const std::string &short_name,
                           const std::string &description, bool is_mandatory);

  /**
   * \brief Parse command line arguments
   * \param argc Argument count
   * \param argv Argument array
   * \throw std::runtime_error for parsing errors
   */
  void parse(int argc, char **argv);

  /**
   * \brief Get the value of an option
   * \tparam T Type to cast the option value to
   * \param name Option name
   * \return The option value
   * \throw std::runtime_error if option doesn't exist
   * \throw std::bad_any_cast if type doesn't match
   */
  template <typename T> T get_option(const std::string &name) const {
    if (!options.contains(name)) {
      throw std::runtime_error("Option not registered: " + name);
    }

    return std::any_cast<T>(options.at(name).value);
  }

  /**
   * \brief Check if an option was provided on the command line
   * \param name Option name
   * \return true if the option was provided
   */
  bool is_provided(const std::string &name) const;

  /**
   * \brief Get positional arguments
   * \return Vector of positional arguments
   */
  const std::vector<std::string> &get_positional() const;

  /**
   * \brief Print help message to stdout
   */
  void print_help() const;
};
