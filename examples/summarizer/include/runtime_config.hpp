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
 * @file runtime_config.hpp
 * @brief Configuration Parser for llama.cpp Applications
 * 
 * Parses config.txt to extract runtime paths and ports.
 * 
 * Config Format (config.txt):
 *   Line 1: /path/to/llama.cpp/build/bin
 *   Line 2: /path/to/model.gguf
 *   Line 3: port_x (llama-server port)
 *   Line 4: port_y (app-server port)
 * 
 * @author Pooria Yousefi
 * @date October 6, 2025
 */

#ifndef POORIAYOUSEFI_LLAMA_APP_RUNTIME_CONFIG_HPP
#define POORIAYOUSEFI_LLAMA_APP_RUNTIME_CONFIG_HPP

#include <string>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <wordexp.h>

namespace pooriayousefi
{
namespace llama
{
namespace app
{

/**
 * @struct RuntimeConfig
 * @brief Runtime configuration for llama.cpp-based applications
 * 
 * Holds paths, ports, and provides path expansion utilities.
 */
struct RuntimeConfig
{
    std::string llama_bin_path;      ///< Path to llama.cpp/build-cpu/bin
    std::string model_path;          ///< Path to .gguf model file
    int llama_server_port;           ///< port_x: llama-server port
    int app_server_port;             ///< port_y: application server port
    
    /**
     * @brief Expand shell patterns like ~ and environment variables
     * 
     * @param path Path string potentially containing shell patterns
     * @return Expanded absolute path
     */
    static std::string expand_path(const std::string& path)
    {
        wordexp_t exp_result;
        wordexp(path.c_str(), &exp_result, 0);
        std::string expanded = exp_result.we_wordc > 0 ? exp_result.we_wordv[0] : path;
        wordfree(&exp_result);
        return expanded;
    }
    
    /**
     * @brief Trim whitespace from string
     * 
     * @param str Input string
     * @return Trimmed string
     */
    static std::string trim(const std::string& str)
    {
        size_t start = str.find_first_not_of(" \t\r\n");
        size_t end = str.find_last_not_of(" \t\r\n");
        
        if (start == std::string::npos)
        {
            return "";
        }
        
        return str.substr(start, end - start + 1);
    }
    
    /**
     * @brief Parse configuration from file
     * 
     * @param config_path Path to config.txt
     * @return RuntimeConfig instance
     * 
     * @throws std::runtime_error if parsing fails
     */
    static RuntimeConfig from_file(const std::string& config_path)
    {
        std::ifstream file(config_path);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open config file: " + config_path);
        }
        
        RuntimeConfig config;
        std::string line;
        int line_num = 0;
        
        // Line 1: llama bin path
        if (std::getline(file, line))
        {
            line_num++;
            line = trim(line);
            if (line.empty() || line[0] == '#')
            {
                throw std::runtime_error("Line 1: llama bin path is required");
            }
            config.llama_bin_path = expand_path(line);
        }
        else
        {
            throw std::runtime_error("Config file is empty");
        }
        
        // Line 2: model path
        if (std::getline(file, line))
        {
            line_num++;
            line = trim(line);
            if (line.empty() || line[0] == '#')
            {
                throw std::runtime_error("Line 2: model path is required");
            }
            config.model_path = expand_path(line);
        }
        else
        {
            throw std::runtime_error("Missing model path in config");
        }
        
        // Line 3: llama-server port (port_x)
        if (std::getline(file, line))
        {
            line_num++;
            line = trim(line);
            if (line.empty() || line[0] == '#')
            {
                throw std::runtime_error("Line 3: llama-server port is required");
            }
            
            std::stringstream ss(line);
            ss >> config.llama_server_port;
            if (ss.fail())
            {
                throw std::runtime_error("Line 3: Invalid port number");
            }
        }
        else
        {
            throw std::runtime_error("Missing llama-server port in config");
        }
        
        // Line 4: app-server port (port_y)
        if (std::getline(file, line))
        {
            line_num++;
            line = trim(line);
            if (line.empty() || line[0] == '#')
            {
                throw std::runtime_error("Line 4: app-server port is required");
            }
            
            std::stringstream ss(line);
            ss >> config.app_server_port;
            if (ss.fail())
            {
                throw std::runtime_error("Line 4: Invalid port number");
            }
        }
        else
        {
            throw std::runtime_error("Missing app-server port in config");
        }
        
        file.close();
        return config;
    }
    
    /**
     * @brief Get llama-server executable path
     * 
     * @return Full path to llama-server binary
     */
    std::string get_llama_server_path() const
    {
        return llama_bin_path + "/llama-server";
    }
    
    /**
     * @brief Get llama-server URL
     * 
     * @return URL for HTTP requests (e.g., "http://localhost:8080")
     */
    std::string get_llama_server_url() const
    {
        return "http://localhost:" + std::to_string(llama_server_port);
    }
    
    /**
     * @brief Validate configuration
     * 
     * Checks that:
     *  - llama-server executable exists and is executable
     *  - Model file exists and is readable
     *  - Ports are in valid range (1024-65535)
     *  - Ports are different from each other
     * 
     * @throws std::runtime_error if validation fails
     */
    void validate() const
    {
        // Check if llama-server exists
        std::string server_path = get_llama_server_path();
        if (access(server_path.c_str(), X_OK) != 0)
        {
            throw std::runtime_error("llama-server not found or not executable: " + server_path);
        }
        
        // Check if model exists
        if (access(model_path.c_str(), R_OK) != 0)
        {
            throw std::runtime_error("Model file not found or not readable: " + model_path);
        }
        
        // Validate ports
        if (llama_server_port < 1024 || llama_server_port > 65535)
        {
            throw std::runtime_error("Invalid llama-server port: " + std::to_string(llama_server_port));
        }
        
        if (app_server_port < 1024 || app_server_port > 65535)
        {
            throw std::runtime_error("Invalid app-server port: " + std::to_string(app_server_port));
        }
        
        if (llama_server_port == app_server_port)
        {
            throw std::runtime_error("llama-server and app-server ports must be different");
        }
    }
    
    /**
     * @brief Print configuration to stdout
     * 
     * Useful for debugging and verification.
     */
    void print() const
    {
        std::cout << "Runtime Configuration:" << std::endl;
        std::cout << "  Llama Bin Path:    " << llama_bin_path << std::endl;
        std::cout << "  Model Path:        " << model_path << std::endl;
        std::cout << "  Llama Server Port: " << llama_server_port << " (port_x)" << std::endl;
        std::cout << "  App Server Port:   " << app_server_port << " (port_y)" << std::endl;
        std::cout << "  Llama Server URL:  " << get_llama_server_url() << std::endl;
    }
};

} // namespace app
} // namespace llama
} // namespace pooriayousefi

#endif // POORIAYOUSEFI_LLAMA_APP_RUNTIME_CONFIG_HPP
