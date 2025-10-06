/*
 * Copyright 2025 Pooria Yousefi
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file llama_client.hpp
 * @brief HTTP Client Wrapper for llama.cpp Server
 * 
 * Internal HTTP client for communicating with llama-server.
 * Uses cpp-httplib for header-only HTTP support.
 * 
 * @author Pooria Yousefi
 * @date October 6, 2025
 */

#ifndef POORIAYOUSEFI_LLAMA_APP_LLAMA_CLIENT_HPP
#define POORIAYOUSEFI_LLAMA_APP_LLAMA_CLIENT_HPP

#include <string>
#include <stdexcept>
#include <chrono>
#include "httplib.h"
#include "json.hpp"

namespace pooriayousefi
{
namespace llama
{
namespace app
{

using json = nlohmann::json;

/**
 * @class LlamaClient
 * @brief HTTP client for llama.cpp server communication
 * 
 * Provides a simple interface to communicate with a running llama-server instance.
 * All requests use JSON over HTTP protocol.
 */
class LlamaClient
{
private:
    std::string host_;
    int port_;
    int timeout_seconds_;
    
    /**
     * @brief Parse URL into host and port
     */
    void parse_url(const std::string& url)
    {
        // Remove http:// or https:// prefix
        std::string clean_url = url;
        if (clean_url.find("http://") == 0)
        {
            clean_url = clean_url.substr(7);
        }
        else if (clean_url.find("https://") == 0)
        {
            clean_url = clean_url.substr(8);
        }
        
        // Find port separator
        size_t colon_pos = clean_url.find(':');
        if (colon_pos != std::string::npos)
        {
            host_ = clean_url.substr(0, colon_pos);
            port_ = std::stoi(clean_url.substr(colon_pos + 1));
        }
        else
        {
            host_ = clean_url;
            port_ = 8080; // Default port
        }
    }

public:
    /**
     * @brief Construct HTTP client for llama-server
     * 
     * @param url Server URL (e.g., "http://localhost:8080")
     * @param timeout Request timeout in seconds (default: 300)
     */
    LlamaClient(const std::string& url, int timeout = 300)
        : timeout_seconds_(timeout)
    {
        parse_url(url);
    }
    
    /**
     * @brief Generate text completion from prompt
     * 
     * @param prompt Input text prompt
     * @param max_tokens Maximum tokens to generate (default: 512)
     * @param temperature Sampling temperature (default: 0.7)
     * @return JSON response from llama-server
     * 
     * @throws std::runtime_error if request fails
     */
    json complete(const std::string& prompt, int max_tokens = 512, float temperature = 0.7)
    {
        json request = {
            {"prompt", prompt},
            {"n_predict", max_tokens},
            {"temperature", temperature},
            {"stop", json::array({"User:", "\n\n"})}
        };
        
        httplib::Client client(host_, port_);
        client.set_read_timeout(std::chrono::seconds(timeout_seconds_));
        client.set_write_timeout(std::chrono::seconds(timeout_seconds_));
        
        std::string request_body = request.dump();
        
        auto res = client.Post("/completion", request_body, "application/json");
        
        if (!res)
        {
            throw std::runtime_error("HTTP request failed: connection error");
        }
        
        if (res->status != 200)
        {
            throw std::runtime_error("HTTP request failed with status: " + std::to_string(res->status));
        }
        
        return json::parse(res->body);
    }
    
    /**
     * @brief Chat completion (for instruction-tuned models)
     * 
     * @param messages Array of message objects with "role" and "content"
     * @param max_tokens Maximum tokens to generate (default: 512)
     * @param temperature Sampling temperature (default: 0.7)
     * @return JSON response from llama-server
     * 
     * @throws std::runtime_error if request fails
     */
    json chat(const json& messages, int max_tokens = 512, float temperature = 0.7)
    {
        json request = {
            {"messages", messages},
            {"max_tokens", max_tokens},
            {"temperature", temperature}
        };
        
        httplib::Client client(host_, port_);
        client.set_read_timeout(std::chrono::seconds(timeout_seconds_));
        client.set_write_timeout(std::chrono::seconds(timeout_seconds_));
        
        std::string request_body = request.dump();
        
        auto res = client.Post("/v1/chat/completions", request_body, "application/json");
        
        if (!res)
        {
            throw std::runtime_error("HTTP request failed: connection error");
        }
        
        if (res->status != 200)
        {
            throw std::runtime_error("HTTP request failed with status: " + std::to_string(res->status));
        }
        
        return json::parse(res->body);
    }
    
    /**
     * @brief Check if server is alive and responding
     * 
     * @return true if server responds to health check
     */
    bool is_alive()
    {
        try
        {
            httplib::Client client(host_, port_);
            client.set_read_timeout(std::chrono::seconds(5));
            client.set_write_timeout(std::chrono::seconds(5));
            
            auto res = client.Get("/health");
            
            return (res && res->status == 200);
        }
        catch (...)
        {
            return false;
        }
    }
    
    /**
     * @brief Get available models from server
     * 
     * @return JSON response containing model information
     * 
     * @throws std::runtime_error if request fails
     */
    json get_models()
    {
        httplib::Client client(host_, port_);
        client.set_read_timeout(std::chrono::seconds(timeout_seconds_));
        client.set_write_timeout(std::chrono::seconds(timeout_seconds_));
        
        auto res = client.Get("/v1/models");
        
        if (!res)
        {
            throw std::runtime_error("HTTP request failed: connection error");
        }
        
        if (res->status != 200)
        {
            throw std::runtime_error("Failed to get models, status: " + std::to_string(res->status));
        }
        
        return json::parse(res->body);
    }
    
    /**
     * @brief Get server URL
     * 
     * @return Server URL as string
     */
    std::string get_server_url() const
    {
        return "http://" + host_ + ":" + std::to_string(port_);
    }
};

} // namespace app
} // namespace llama
} // namespace pooriayousefi

#endif // POORIAYOUSEFI_LLAMA_APP_LLAMA_CLIENT_HPP
