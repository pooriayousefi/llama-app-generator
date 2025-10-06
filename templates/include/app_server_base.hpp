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
 * @file app_server_base.hpp
 * @brief CRTP Template Base Class for Application Servers
 * 
 * This is the core templatized server that integrates:
 * 1. LlamaClient (internal wrapper to llama-server on port_x)
 * 2. HTTP server (exposes YOUR API on port_y)
 * 3. Business logic interface (you implement process_request())
 * 
 * Architecture:
 *   GUI/CLI Client --[port_y]--> AppServer --[port_x]--> llama-server
 *                                    |
 *                                    +--> YOUR business logic
 * 
 * Uses CRTP (Curiously Recurring Template Pattern) for:
 * - Zero runtime overhead (compile-time polymorphism)
 * - No vtable lookups
 * - Better inlining and optimization
 * 
 * @example Usage
 * @code
 *   class MyApp : public AppServerBase<MyApp>
 *   {
 *   public:
 *       using AppServerBase<MyApp>::AppServerBase;
 *       
 *       json process_request(const json& request)
 *       {
 *           // Your business logic here
 *           return {...};
 *       }
 *   };
 * @endcode
 * 
 * @author Pooria Yousefi
 * @date October 6, 2025
 */

#ifndef POORIAYOUSEFI_LLAMA_APP_APP_SERVER_BASE_HPP
#define POORIAYOUSEFI_LLAMA_APP_APP_SERVER_BASE_HPP

#include <string>
#include <iostream>
#include <memory>
#include "json.hpp"
#include "httplib.h"
#include "llama_client.hpp"
#include "runtime_config.hpp"

namespace pooriayousefi
{
namespace llama
{
namespace app
{

using json = nlohmann::json;

namespace color
{
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
}

/**
 * @class AppServerBase
 * @brief CRTP base class for llama.cpp-based HTTP servers
 * 
 * Provides template method pattern using CRTP for compile-time polymorphism.
 * Derived classes must implement process_request() to handle business logic.
 * 
 * @tparam Derived The derived class type (CRTP pattern)
 */
template<typename Derived>
class AppServerBase
{
protected:
    RuntimeConfig config_;              ///< Runtime configuration
    std::unique_ptr<LlamaClient> llama_client_;  ///< HTTP client for llama-server
    httplib::Server server_;            ///< HTTP server for client API
    
    /**
     * @brief Construct application server
     * 
     * @param cfg Runtime configuration
     */
    AppServerBase(const RuntimeConfig& cfg) 
        : config_(cfg)
        , llama_client_(std::make_unique<LlamaClient>(config_.get_llama_server_url()))
    {
        setup_routes();
    }
    
    /**
     * @brief Get derived instance (CRTP idiom)
     * 
     * @return Reference to derived class instance
     */
    Derived& derived()
    {
        return static_cast<Derived&>(*this);
    }
    
    /**
     * @brief Get derived instance (const version)
     * 
     * @return Const reference to derived class instance
     */
    const Derived& derived() const
    {
        return static_cast<const Derived&>(*this);
    }
    
    /**
     * @brief Setup HTTP routes
     * 
     * Configures /health and /api endpoints. The /api endpoint
     * delegates to derived class's process_request() method using CRTP.
     */
    void setup_routes()
    {
        // Health check endpoint
        server_.Get("/health", [](const httplib::Request&, httplib::Response& res)
        {
            json response = {
                {"status", "ok"},
                {"service", "app-server"}
            };
            res.set_content(response.dump(), "application/json");
        });
        
        // Main API endpoint - delegates to derived class's process_request()
        // CRTP magic: calls Derived::process_request() at compile-time!
        server_.Post("/api", [this](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                json request = json::parse(req.body);
                
                // Call derived class's business logic (CRTP compile-time polymorphism!)
                json response = derived().process_request(request);
                
                res.set_content(response.dump(), "application/json");
            }
            catch (const std::exception& e)
            {
                json error = {
                    {"error", e.what()},
                    {"status", "failed"}
                };
                res.status = 500;
                res.set_content(error.dump(), "application/json");
            }
        });
    }

public:
    /**
     * @brief Start the HTTP server
     * 
     * Binds to configured port and begins listening for requests.
     * This is a blocking call.
     * 
     * @throws std::runtime_error if server fails to start
     */
    void start()
    {
        std::string host = "0.0.0.0";
        int port = config_.app_server_port;
        
        std::cout << color::GREEN << "Server listening on " << host << ":" << port << color::RESET << std::endl;
        std::cout << color::BLUE << "Endpoints:" << color::RESET << std::endl;
        std::cout << "  GET  /health - Health check" << std::endl;
        std::cout << "  POST /api    - Main API endpoint" << std::endl;
        std::cout << std::endl;
        
        if (!server_.listen(host, port))
        {
            throw std::runtime_error("Failed to start server on port " + std::to_string(port));
        }
    }
    
    /**
     * @brief Stop the HTTP server
     * 
     * Gracefully shuts down the server.
     */
    void stop()
    {
        server_.stop();
    }
    
    /**
     * @brief Get LlamaClient for derived classes to use
     * 
     * Provides access to llama-server client for LLM operations.
     * 
     * @return Pointer to LlamaClient instance
     */
    LlamaClient* get_llama_client()
    {
        return llama_client_.get();
    }
    
    /**
     * @brief Get runtime configuration
     * 
     * @return Reference to RuntimeConfig
     */
    const RuntimeConfig& get_config() const
    {
        return config_;
    }
};

} // namespace app
} // namespace llama
} // namespace pooriayousefi

#endif // POORIAYOUSEFI_LLAMA_APP_APP_SERVER_BASE_HPP
