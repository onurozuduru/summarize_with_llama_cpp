///////////////////////////////////////////////////////////////////////////////
// File: model.cpp
//
// License: MIT
//
// Copyright (C) 2025 Onur Ozuduru
//
// Follow Me!
//   github: github.com/onurozuduru
///////////////////////////////////////////////////////////////////////////////

#include "model.h"
#include "llama-cpp.h"
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace model_wrapper {

Model::Model(const std::string_view model_path, const float temperature,
             const int32_t number_of_gpu_layers,
             const std::size_t prediction_length)
    : m_temperature(temperature), m_prediction_length(prediction_length) {
  auto model_params = llama_model_default_params();
  model_params.n_gpu_layers = number_of_gpu_layers;

  llama_log_set(
      [](auto log_level, const char *log_message, auto /* user_data */) {
        if (log_level >= GGML_LOG_LEVEL_ERROR) {
          std::cerr << log_message;
        }
      },
      nullptr);

  ggml_backend_load_all();

  m_model = llama_model_ptr{
      llama_model_load_from_file(model_path.data(), model_params)};

  if (!m_model) {
    throw std::runtime_error{"Failed to load model!"};
  }
  m_vocab = llama_model_get_vocab(m_model.get());

  initialize_sampler();
}

std::vector<llama_token> Model::tokenize_prompt(const std::string &prompt) {
  const bool is_adding_special_tokens{true};
  const bool is_parsing_special_tokens{true};

  // llama_tokenize with nullptr returns negative number of tokens if it would
  // have succeeded
  const auto number_of_tokens =
      -llama_tokenize(m_vocab, prompt.data(), prompt.size(), nullptr, 0,
                      is_adding_special_tokens, is_parsing_special_tokens);

  std::vector<llama_token> tokens(number_of_tokens);
  const bool is_tokenized =
      (llama_tokenize(m_vocab, prompt.data(), prompt.size(), tokens.data(),
                      tokens.size(), is_adding_special_tokens,
                      is_parsing_special_tokens) >= 0);

  if (!is_tokenized) {
    throw std::runtime_error{"Failed to tokenize prompt!"};
  }

  return tokens;
}

void Model::initialize_sampler() {
  if (m_sampler) {
    return;
  }

  m_sampler = llama_sampler_ptr{
      llama_sampler_chain_init(llama_sampler_chain_default_params())};
  llama_sampler_chain_add(m_sampler.get(), llama_sampler_init_top_k(35));
  llama_sampler_chain_add(m_sampler.get(), llama_sampler_init_min_p(0.3f, 2));
  llama_sampler_chain_add(m_sampler.get(),
                          llama_sampler_init_temp(m_temperature));
  llama_sampler_chain_add(m_sampler.get(),
                          llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

  // Reduce repetition and overused tokens
  int32_t last_n_tokens_to_penalize = 128;
  float repeat_penalty = 1.5f;
  float frequency_penalty = 0.7f;
  float presence_penalty = 0.7f;

  llama_sampler_chain_add(
      m_sampler.get(),
      llama_sampler_init_penalties(last_n_tokens_to_penalize, repeat_penalty,
                                   frequency_penalty, presence_penalty));
}

llama_context_ptr Model::create_context(const std::size_t number_of_tokens) {
  if (!m_model) {
    throw std::runtime_error{
        "Context cannot be created: Model is not initialized!"};
  }

  const std::size_t context_size = number_of_tokens + m_prediction_length;
  auto context_params = llama_context_default_params();
  context_params.n_ctx = context_size;
  context_params.n_batch = number_of_tokens;

  return llama_context_ptr{
      llama_init_from_model(m_model.get(), context_params)};
}

std::string
Model::get_formatted_prompt(const std::vector<llama_chat_message> &messages) {
  std::string formatted_prompt{};
  const std::string chat_template =
      llama_model_chat_template(m_model.get(), nullptr);

  // Instead of guessing the size of applied template, we can use the function
  // to get the size and then resize the string to that size and apply the
  // template again
  auto formatted_prompt_size = llama_chat_apply_template(
      chat_template.data(), messages.data(), messages.size(), true,
      formatted_prompt.data(), formatted_prompt.size());

  if (formatted_prompt_size > 0) {
    formatted_prompt.resize(formatted_prompt_size);
    formatted_prompt_size = llama_chat_apply_template(
        chat_template.data(), messages.data(), messages.size(), true,
        formatted_prompt.data(), formatted_prompt.size());
  }

  if (formatted_prompt_size < 0) {
    std::runtime_error{"Failed to apply chat template!"};
  }

  return formatted_prompt;
}

void Model::generate_response(const std::string &prompt, std::ostream &out) {
  auto tokens = tokenize_prompt(prompt);
  const auto context = create_context(tokens.size());
  if (!context) {
    throw std::runtime_error{
        "Cannot generate response: Failed to initialize context!"};
  }
  const auto context_size = llama_n_ctx(context.get());

  auto batch = llama_batch_get_one(tokens.data(), tokens.size());
  std::uint32_t number_of_decoded{0U};
  llama_token new_token_id;

  for (int token_position = 0; token_position + batch.n_tokens < context_size;
       ++number_of_decoded) {
    // Evaluate the current
    const bool is_decoded = (llama_decode(context.get(), batch) == 0);
    if (!is_decoded) {
      throw std::runtime_error{"Cannot generate response: Failed to decode!"};
    }

    token_position += batch.n_tokens;

    // Sample the next token
    new_token_id = llama_sampler_sample(m_sampler.get(), context.get(), -1);

    if (llama_vocab_is_eog(m_vocab, new_token_id)) {
      break;
    }

    std::string token_buffer(256, '\0');
    const bool is_render_special_tokens{true};
    const int32_t lstrip{0};
    auto token_string_size = llama_token_to_piece(
        m_vocab, new_token_id, token_buffer.data(), token_buffer.size(), lstrip,
        is_render_special_tokens);
    if (token_string_size < 0) {
      throw std::runtime_error{
          "Cannot generate response: Failed to convert token to string!"};
    }

    std::string_view token_string{token_buffer.data(),
                                  static_cast<size_t>(token_string_size)};
    out << token_string;
    out.flush();

    // Prepare the next batch
    batch = llama_batch_get_one(&new_token_id, 1);
  }

  out << std::endl;
}
} // namespace model_wrapper
