///////////////////////////////////////////////////////////////////////////////
// File: model.h
//
// License: MIT
//
// Copyright (C) 2025 Onur Ozuduru
//
// Follow Me!
//   github: github.com/onurozuduru
///////////////////////////////////////////////////////////////////////////////

#include "llama-cpp.h"
#include <ostream>
#include <string>
#include <vector>

namespace model_wrapper {
/**
 * \brief Model wrapper
 */
class Model {
private:
  const float m_temperature;
  const std::size_t m_prediction_length;

  llama_model_ptr m_model{nullptr};
  const llama_vocab *m_vocab;
  llama_sampler_ptr m_sampler{nullptr};

  /**
   * \brief Tokenize the prompt
   * \p
   * \return The tokens
   * \throw std::runtime_error if the prompt cannot be tokenized
   */
  std::vector<llama_token> tokenize_prompt(const std::string &prompt);

  /**
   * \brief Initialize the sampler
   */
  void initialize_sampler();

  /**
   * \brief Create the context.
   * \details The context size is determined as
   * number_of_tokens + m_prediction_length.
   * \param number_of_tokens The number of tokens
   * \return The context pointer
   * \throw std::runtime_error if the context cannot be created
   */
  llama_context_ptr create_context(const std::size_t number_of_tokens);

public:
  /**
   * \brief Construct a new Model object
   * \param model_path The path to the model file in GGUF format
   * \param temperature The temperature
   * \param number_of_gpu_layers The number of GPU layers to use
   * \param prediction_length The maximum number of tokens to predict
   * \throw std::runtime_error if the model cannot be loaded
   */
  Model(const std::string_view model_path, const float temperature,
        const int32_t number_of_gpu_layers,
        const std::size_t prediction_length);

  /**
   * \brief Apply the chat template to the messages
   * \param messages The messages to format
   * \return The formatted prompt
   * \throw std::runtime_error if the chat template cannot be applied
   */
  std::string
  get_formatted_prompt(const std::vector<llama_chat_message> &messages);

  /**
   * \brief Generate a responde from the prompt
   * \details The model generates a response to the prompt by sampling tokens
   * and this function writes to out each token
   * \param prompt The prompt to respond to
   * \param out The output stream to write the response to
   * \throw std::runtime_error if the generation fails
   */
  void generate_response(const std::string &prompt, std::ostream &out);
};
} // namespace model_wrapper
