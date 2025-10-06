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
 * @file generator.cpp
 * @brief llama-app-generator - Template-based llama.cpp Project Generator
 * 
 * Generates templatized llama-powered C++ applications using the
 * pooriayousefi::llama::app namespace and CRTP pattern.
 * 
 * Architecture:
 *   - LlamaClient (internal wrapper) + AppServerBase (CRTP template)
 *   - Config-driven: no repeated builds of llama.cpp
 *   - Three-tier: llama-server | app-server | GUI/CLI client
 * 
 * @author Pooria Yousefi
 * @date October 6, 2025
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

// ANSI colors for terminal output
namespace color
{
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string CYAN = "\033[36m";
}

namespace pooriayousefi
{
namespace llama
{
namespace app
{

/**
 * @class ProjectGenerator
 * @brief Generates llama.cpp-based C++ projects from templates
 */
class ProjectGenerator
{
private:
    std::string project_name_;
    fs::path project_path_;
    fs::path template_dir_;
    fs::path config_path_;
    
    /**
     * @brief Check if directory exists
     */
    bool directory_exists(const fs::path& path) const
    {
        return fs::exists(path) && fs::is_directory(path);
    }
    
    /**
     * @brief Check if file exists
     */
    bool file_exists(const fs::path& path) const
    {
        return fs::exists(path) && fs::is_regular_file(path);
    }
    
    /**
     * @brief Execute shell command
     */
    bool execute_command(const std::string& command) const
    {
        std::cout << color::BLUE << "  $ " << command << color::RESET << std::endl;
        return (system(command.c_str()) == 0);
    }
    
    /**
     * @brief Read entire file into string
     */
    std::string read_file(const fs::path& filepath) const
    {
        std::ifstream file(filepath);
        
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to read file: " + filepath.string());
        }
        
        std::string content;
        std::string line;
        while (std::getline(file, line))
        {
            content += line + "\n";
        }
        file.close();
        
        return content;
    }
    
    /**
     * @brief Write string content to file
     */
    void write_file(const fs::path& filepath, const std::string& content) const
    {
        std::ofstream file(filepath);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to create file: " + filepath.string());
        }
        file << content;
        file.close();
    }
    
    /**
     * @brief Copy file from source to destination
     */
    void copy_file(const fs::path& src, const fs::path& dest) const
    {
        fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
    }
    
    /**
     * @brief Make file executable (Unix-like systems)
     */
    void make_executable(const fs::path& filepath) const
    {
        #ifndef _WIN32
        chmod(filepath.c_str(), 0755);
        #endif
    }
    
    /**
     * @brief Get current date in human-readable format
     */
    std::string get_current_date() const
    {
        time_t now = time(nullptr);
        char buf[80];
        strftime(buf, sizeof(buf), "%B %d, %Y", localtime(&now));
        return std::string(buf);
    }
    
    /**
     * @brief Replace placeholders in template content
     * 
     * Supported placeholders:
     *   {{PROJECT_NAME}} - Name of the generated project
     *   {{DATE}} - Current date
     */
    std::string replace_placeholders(const std::string& content) const
    {
        std::string result = content;
        
        // Replace {{PROJECT_NAME}}
        size_t pos = 0;
        while ((pos = result.find("{{PROJECT_NAME}}", pos)) != std::string::npos)
        {
            result.replace(pos, 16, project_name_);
            pos += project_name_.length();
        }
        
        // Replace {{DATE}}
        std::string date = get_current_date();
        pos = 0;
        while ((pos = result.find("{{DATE}}", pos)) != std::string::npos)
        {
            result.replace(pos, 8, date);
            pos += date.length();
        }
        
        return result;
    }

public:
    /**
     * @brief Construct project generator
     * 
     * @param name Project name
     * @param output_dir Output directory for generated project
     * @param templates Template directory path
     * @param config Configuration file path (config.txt)
     */
    ProjectGenerator(
        const std::string& name,
        const fs::path& output_dir,
        const fs::path& templates,
        const fs::path& config
    )
        : project_name_(name)
        , project_path_(output_dir / name)
        , template_dir_(templates)
        , config_path_(config)
    {
    }
    
    /**
     * @brief Generate complete project structure
     * 
     * Creates directories, copies templates, and sets up build environment.
     * 
     * @throws std::runtime_error if generation fails
     */
    void generate()
    {
        print_header();
        validate_preconditions();
        
        create_project_structure();
        copy_configuration();
        copy_libraries();
        copy_template_headers();
        copy_example_sources();
        create_build_scripts();
        copy_legal_files();
        
        print_success();
    }

private:
    void print_header() const
    {
        std::cout << color::CYAN << "================================================" << color::RESET << std::endl;
        std::cout << color::CYAN << "  llama-app-generator v1.0.0" << color::RESET << std::endl;
        std::cout << color::CYAN << "  namespace: pooriayousefi::llama::app" << color::RESET << std::endl;
        std::cout << color::CYAN << "================================================" << color::RESET << std::endl;
        std::cout << std::endl;
        
        std::cout << color::YELLOW << "Project:     " << project_name_ << color::RESET << std::endl;
        std::cout << color::YELLOW << "Path:        " << project_path_ << color::RESET << std::endl;
        std::cout << color::YELLOW << "Templates:   " << template_dir_ << color::RESET << std::endl;
        std::cout << color::YELLOW << "Config:      " << config_path_ << color::RESET << std::endl;
        std::cout << std::endl;
    }
    
    void validate_preconditions() const
    {
        // Check if project already exists
        if (directory_exists(project_path_))
        {
            std::cout << color::RED << "✗ Error: Project directory already exists!" << color::RESET << std::endl;
            throw std::runtime_error("Project already exists: " + project_path_.string());
        }
        
        // Validate config.txt exists
        if (!file_exists(config_path_))
        {
            std::cout << color::RED << "✗ Error: config.txt not found at: " << config_path_ << color::RESET << std::endl;
            throw std::runtime_error("config.txt not found");
        }
        
        // Validate template directory
        if (!directory_exists(template_dir_))
        {
            std::cout << color::RED << "✗ Error: Template directory not found: " << template_dir_ << color::RESET << std::endl;
            throw std::runtime_error("Template directory not found");
        }
    }
    
    void create_project_structure()
    {
        std::cout << color::YELLOW << "[1/8] Creating project structure..." << color::RESET << std::endl;
        
        fs::create_directories(project_path_ / "src");
        fs::create_directories(project_path_ / "include");
        fs::create_directories(project_path_ / "bin");
        fs::create_directories(project_path_ / "docs");
        
        std::cout << color::GREEN << "  ✓ Created directories" << color::RESET << std::endl;
        std::cout << std::endl;
    }
    
    void copy_configuration()
    {
        std::cout << color::YELLOW << "[2/8] Copying configuration..." << color::RESET << std::endl;
        
        copy_file(config_path_, project_path_ / "config.txt");
        
        std::cout << color::GREEN << "  ✓ Copied config.txt" << color::RESET << std::endl;
        std::cout << std::endl;
    }
    
    void copy_libraries()
    {
        std::cout << color::YELLOW << "[3/8] Copying header-only libraries..." << color::RESET << std::endl;
        
        copy_file(template_dir_ / "include" / "json.hpp", project_path_ / "include" / "json.hpp");
        copy_file(template_dir_ / "include" / "httplib.h", project_path_ / "include" / "httplib.h");
        
        std::cout << color::GREEN << "  ✓ Copied json.hpp (nlohmann/json)" << color::RESET << std::endl;
        std::cout << color::GREEN << "  ✓ Copied httplib.h (cpp-httplib)" << color::RESET << std::endl;
        std::cout << std::endl;
    }
    
    void copy_template_headers()
    {
        std::cout << color::YELLOW << "[4/8] Copying template files..." << color::RESET << std::endl;
        
        copy_file(template_dir_ / "include" / "llama_client.hpp", project_path_ / "include" / "llama_client.hpp");
        copy_file(template_dir_ / "include" / "runtime_config.hpp", project_path_ / "include" / "runtime_config.hpp");
        copy_file(template_dir_ / "include" / "app_server_base.hpp", project_path_ / "include" / "app_server_base.hpp");
        
        std::cout << color::GREEN << "  ✓ Copied llama_client.hpp" << color::RESET << std::endl;
        std::cout << color::GREEN << "  ✓ Copied runtime_config.hpp" << color::RESET << std::endl;
        std::cout << color::GREEN << "  ✓ Copied app_server_base.hpp (CRTP)" << color::RESET << std::endl;
        std::cout << std::endl;
    }
    
    void copy_example_sources()
    {
        std::cout << color::YELLOW << "[5/8] Creating example sources..." << color::RESET << std::endl;
        
        copy_file(template_dir_ / "src" / "example_app.cpp", project_path_ / "src" / "server.cpp");
        copy_file(template_dir_ / "src" / "example_client.cpp", project_path_ / "src" / "client.cpp");
        
        std::cout << color::GREEN << "  ✓ Created src/server.cpp (example application)" << color::RESET << std::endl;
        std::cout << color::GREEN << "  ✓ Created src/client.cpp (CLI client)" << color::RESET << std::endl;
        std::cout << std::endl;
    }
    
    void create_build_scripts()
    {
        std::cout << color::YELLOW << "[6/8] Creating build scripts..." << color::RESET << std::endl;
        
        copy_file(template_dir_ / "scripts" / "build.py", project_path_ / "build.py");
        make_executable(project_path_ / "build.py");
        
        // Process README with placeholders
        if (file_exists(template_dir_ / "docs" / "README.md"))
        {
            std::string readme_content = read_file(template_dir_ / "docs" / "README.md");
            readme_content = replace_placeholders(readme_content);
            write_file(project_path_ / "README.md", readme_content);
            
            std::cout << color::GREEN << "  ✓ Created README.md" << color::RESET << std::endl;
        }
        
        std::cout << color::GREEN << "  ✓ Created build.py (cross-platform)" << color::RESET << std::endl;
        std::cout << std::endl;
    }
    
    void copy_legal_files()
    {
        std::cout << color::YELLOW << "[7/8] Copying legal files..." << color::RESET << std::endl;
        
        if (file_exists(template_dir_ / "legal" / "LICENSE"))
        {
            copy_file(template_dir_ / "legal" / "LICENSE", project_path_ / "LICENSE");
            std::cout << color::GREEN << "  ✓ Copied LICENSE (Apache 2.0)" << color::RESET << std::endl;
        }
        
        if (file_exists(template_dir_ / "legal" / "NOTICE"))
        {
            copy_file(template_dir_ / "legal" / "NOTICE", project_path_ / "NOTICE");
            std::cout << color::GREEN << "  ✓ Copied NOTICE (namespace protection)" << color::RESET << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    void print_success() const
    {
        std::cout << color::GREEN << "================================================" << color::RESET << std::endl;
        std::cout << color::GREEN << "  ✓ Project Created Successfully!" << color::RESET << std::endl;
        std::cout << color::GREEN << "================================================" << color::RESET << std::endl;
        std::cout << std::endl;
        
        std::cout << color::CYAN << "Project Structure:" << color::RESET << std::endl;
        std::cout << "  " << project_path_ << "/" << std::endl;
        std::cout << "  ├── LICENSE                 # Apache License 2.0" << std::endl;
        std::cout << "  ├── NOTICE                  # Namespace protection notice" << std::endl;
        std::cout << "  ├── README.md               # Documentation" << std::endl;
        std::cout << "  ├── config.txt              # Runtime configuration" << std::endl;
        std::cout << "  ├── build.py                # Build script (Python)" << std::endl;
        std::cout << "  ├── include/                # Headers and templates" << std::endl;
        std::cout << "  │   ├── llama_client.hpp    # LLM HTTP client" << std::endl;
        std::cout << "  │   ├── runtime_config.hpp  # Config parser" << std::endl;
        std::cout << "  │   ├── app_server_base.hpp # CRTP base class" << std::endl;
        std::cout << "  │   ├── json.hpp            # nlohmann/json" << std::endl;
        std::cout << "  │   └── httplib.h           # cpp-httplib" << std::endl;
        std::cout << "  ├── src/" << std::endl;
        std::cout << "  │   ├── server.cpp          # Application server (EDIT THIS)" << std::endl;
        std::cout << "  │   └── client.cpp          # CLI client (EDIT THIS)" << std::endl;
        std::cout << "  └── bin/                    # Compiled binaries" << std::endl;
        std::cout << std::endl;
        
        std::cout << color::YELLOW << "Next steps:" << color::RESET << std::endl;
        std::cout << "  cd " << project_path_ << std::endl;
        std::cout << "  cat README.md               # Read the documentation" << std::endl;
        std::cout << "  python3 build.py            # Build the project" << std::endl;
        std::cout << "  ./bin/server                # Start the server" << std::endl;
        std::cout << "  ./bin/client complete \"Hi\" # Test the client" << std::endl;
        std::cout << std::endl;
        
        std::cout << color::BLUE << "Architecture:" << color::RESET << std::endl;
        std::cout << "  llama-server (port_x) ←→ AppServer (LlamaClient + Logic)" << std::endl;
        std::cout << "  AppServer (port_y) ←→ CLI/GUI Client" << std::endl;
        std::cout << std::endl;
        
        std::cout << color::BLUE << "Namespace:" << color::RESET << std::endl;
        std::cout << "  All code uses: pooriayousefi::llama::app" << std::endl;
        std::cout << "  This namespace MUST be preserved (see NOTICE file)" << std::endl;
        std::cout << std::endl;
        
        std::cout << color::BLUE << "To customize:" << color::RESET << std::endl;
        std::cout << "  1. Edit src/server.cpp - implement process_request()" << std::endl;
        std::cout << "  2. Edit src/client.cpp - add new actions or build GUI" << std::endl;
        std::cout << "  3. See README.md for full API reference" << std::endl;
        std::cout << std::endl;
    }
};

} // namespace app
} // namespace llama
} // namespace pooriayousefi

/**
 * @brief Print usage information
 */
void print_usage()
{
    using namespace color;
    
    std::cout << CYAN << "llama-app-generator" << RESET << " - Template-based llama.cpp Project Generator" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: llama-app-generator <project_name> [output_directory]" << std::endl;
    std::cout << std::endl;
    std::cout << YELLOW << "Arguments:" << RESET << std::endl;
    std::cout << "  project_name       Name of the project to generate" << std::endl;
    std::cout << "  output_directory   Directory where project will be created (default: current directory)" << std::endl;
    std::cout << std::endl;
    std::cout << YELLOW << "Description:" << RESET << std::endl;
    std::cout << "  Creates a templatized llama-powered C++ application using:" << std::endl;
    std::cout << "    • CRTP pattern for zero-overhead extensibility" << std::endl;
    std::cout << "    • pooriayousefi::llama::app namespace" << std::endl;
    std::cout << "    • Shared llama.cpp runtime from config.txt" << std::endl;
    std::cout << "    • Apache License 2.0 for namespace protection" << std::endl;
    std::cout << std::endl;
    std::cout << YELLOW << "Architecture:" << RESET << std::endl;
    std::cout << "  Three-tier design:" << std::endl;
    std::cout << "    llama-server (port_x) ←→ AppServer ←→ GUI/CLI client (port_y)" << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "  AppServer contains:" << std::endl;
    std::cout << "    • LlamaClient (internal HTTP wrapper)" << std::endl;
    std::cout << "    • AppServerBase<Derived> (CRTP template)" << std::endl;
    std::cout << std::endl;
    std::cout << YELLOW << "Examples:" << RESET << std::endl;
    std::cout << "  llama-app-generator my-ai-app" << std::endl;
    std::cout << "    Creates: ./my-ai-app/" << std::endl;
    std::cout << std::endl;
    std::cout << "  llama-app-generator chatbot ~/projects" << std::endl;
    std::cout << "    Creates: ~/projects/chatbot/" << std::endl;
    std::cout << std::endl;
    std::cout << YELLOW << "Requirements:" << RESET << std::endl;
    std::cout << "  • config.txt in current directory with:" << std::endl;
    std::cout << "      Line 1: /path/to/llama.cpp/build-cpu/bin" << std::endl;
    std::cout << "      Line 2: /path/to/model.gguf" << std::endl;
    std::cout << "      Line 3: port_x (llama-server port, e.g., 8080)" << std::endl;
    std::cout << "      Line 4: port_y (app-server port, e.g., 8081)" << std::endl;
    std::cout << "  • Pre-built llama.cpp runtime" << std::endl;
    std::cout << "  • .gguf model file" << std::endl;
    std::cout << "  • C++17 compiler (g++, clang++, MSVC)" << std::endl;
    std::cout << "  • Python 3 (for build scripts)" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Main entry point
 */
int main(int argc, char* argv[])
{
    using namespace pooriayousefi::llama::app;
    using namespace color;
    
    if (argc < 2 || argc > 3)
    {
        print_usage();
        return 1;
    }
    
    std::string project_name = argv[1];
    
    // Validate project name
    if (project_name.empty() || project_name[0] == '-')
    {
        std::cerr << RED << "✗ Error: Invalid project name" << RESET << std::endl;
        print_usage();
        return 1;
    }
    
    // Get output directory (default: current directory)
    fs::path output_dir = ".";
    if (argc == 3)
    {
        output_dir = argv[2];
        
        if (!fs::exists(output_dir))
        {
            std::cerr << RED << "✗ Error: Output directory does not exist: " << output_dir << RESET << std::endl;
            return 1;
        }
        
        if (!fs::is_directory(output_dir))
        {
            std::cerr << RED << "✗ Error: Output path is not a directory: " << output_dir << RESET << std::endl;
            return 1;
        }
    }
    
    try
    {
        // Get template directory (relative to executable)
        char exe_path[1024];
        ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
        if (len == -1)
        {
            throw std::runtime_error("Failed to get executable path");
        }
        exe_path[len] = '\0';
        
        fs::path exe_dir = fs::path(exe_path).parent_path();
        fs::path template_dir = exe_dir.parent_path() / "templates";
        fs::path config_path = "config.txt";
        
        // Check if config.txt exists in current directory
        if (!fs::exists(config_path))
        {
            std::cerr << RED << "✗ Error: config.txt not found in current directory" << RESET << std::endl;
            std::cerr << std::endl;
            std::cerr << "Please create config.txt with 4 lines:" << std::endl;
            std::cerr << "  Line 1: /path/to/llama.cpp/build-cpu/bin" << std::endl;
            std::cerr << "  Line 2: /path/to/model.gguf" << std::endl;
            std::cerr << "  Line 3: port_x (llama-server port, e.g., 8080)" << std::endl;
            std::cerr << "  Line 4: port_y (app-server port, e.g., 8081)" << std::endl;
            std::cerr << std::endl;
            return 1;
        }
        
        ProjectGenerator generator(project_name, output_dir, template_dir, config_path);
        generator.generate();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << RED << "✗ Error: " << e.what() << RESET << std::endl;
        return 1;
    }
}
