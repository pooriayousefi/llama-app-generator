# llama-app-generator

A template-based generator for creating llama.cpp-powered C++ applications using the CRTP pattern and the **pooriayousefi::llama::app** namespace.

## Overview

**llama-app-generator** generates complete C++17 projects that integrate with llama.cpp for LLM functionality. All generated projects use compile-time polymorphism (CRTP pattern) for zero-overhead extensibility.

### Key Features

- ✅ **CRTP Pattern** - Compile-time polymorphism, zero virtual function overhead
- ✅ **Three-Tier Architecture** - llama-server | app-server | GUI/CLI client
- ✅ **Config-Driven** - No repeated llama.cpp builds
- ✅ **Header-Only** - Uses nlohmann/json and cpp-httplib (no external dependencies)
- ✅ **Cross-Platform** - Python build scripts work on Linux/macOS/Windows
- ✅ **Namespace Protection** - Apache 2.0 license protects pooriayousefi::llama::app
- ✅ **Template Method Pattern** - Easy customization through inheritance

## Quick Start

### 1. Build the Generator

```bash
cd llama-app-generator
python3 build.py
```

This creates `bin/llama-app-generator`.

### 2. Create config.txt

Create a `config.txt` file with 4 lines:

```text
/path/to/llama.cpp/build-cpu/bin
/path/to/model.gguf
8080
8081
```

- **Line 1**: Path to llama.cpp binaries
- **Line 2**: Path to your .gguf model file
- **Line 3**: port_x (llama-server port)
- **Line 4**: port_y (app-server port)

### 3. Generate a Project

```bash
./bin/llama-app-generator my-ai-app
```

Or specify custom output directory:

```bash
./bin/llama-app-generator chatbot ~/projects
```

### 4. Build and Run

```bash
cd my-ai-app
python3 build.py
./bin/server
```

In another terminal:

```bash
./bin/client complete "Hello, how are you?"
```

## Architecture

```
┌─────────────┐         ┌──────────────┐         ┌────────────┐
│ llama-server│◄────────┤  AppServer   ├────────►│ CLI/GUI    │
│  (port_x)   │  HTTP   │  (port_y)    │  HTTP   │  Client    │
└─────────────┘         └──────────────┘         └────────────┘
                              │
                              │ CRTP
                              ▼
                        ┌──────────────┐
                        │ LlamaClient  │
                        │ (internal)   │
                        └──────────────┘
```

### Three-Tier Design

1. **llama-server (port_x)**
   - Pre-built llama.cpp binary
   - Provides LLM inference via HTTP
   - Stateless, reusable across projects

2. **AppServer (port_y)**
   - Your custom business logic
   - Uses CRTP: `class MyApp : public AppServerBase<MyApp>`
   - Internal LlamaClient communicates with llama-server
   - Exposes REST API for clients

3. **CLI/GUI Client**
   - Example CLI included
   - Can be replaced with web UI, desktop app, mobile app
   - Communicates with AppServer via HTTP

## Generated Project Structure

```
my-ai-app/
├── LICENSE                 # Apache License 2.0
├── NOTICE                  # Namespace protection notice
├── README.md               # Project documentation
├── config.txt              # Runtime configuration
├── build.py                # Python build script
├── include/
│   ├── llama_client.hpp    # HTTP client for llama-server
│   ├── runtime_config.hpp  # Config parser
│   ├── app_server_base.hpp # CRTP base class
│   ├── json.hpp            # nlohmann/json (899KB)
│   └── httplib.h           # cpp-httplib (396KB)
├── src/
│   ├── server.cpp          # Application server (EDIT THIS)
│   └── client.cpp          # CLI client (EDIT THIS)
└── bin/
    ├── server              # Compiled server
    └── client              # Compiled client
```

## CRTP Pattern Example

Generated servers use the Curiously Recurring Template Pattern:

```cpp
#include "app_server_base.hpp"

using namespace pooriayousefi::llama::app;

class MyApp : public AppServerBase<MyApp>
{
public:
    using AppServerBase<MyApp>::AppServerBase;
    
    // Implement business logic (called at compile-time via CRTP)
    json process_request(const json& request)
    {
        std::string action = request.value("action", "unknown");
        
        if (action == "chat")
        {
            std::string message = request.value("message", "");
            
            // Use LlamaClient to call llama-server
            json llm_response = get_llama_client()->complete(message, 512, 0.7);
            
            return {
                {"status", "success"},
                {"response", llm_response["content"]}
            };
        }
        
        return {{"status", "error"}, {"message", "Unknown action"}};
    }
};

int main()
{
    RuntimeConfig config = RuntimeConfig::from_file("config.txt");
    MyApp app(config);
    app.start();  // Starts HTTP server
    return 0;
}
```

### Why CRTP?

- **Zero overhead**: No virtual functions, no vtable lookups
- **Inline-friendly**: Compiler can optimize across class boundaries
- **Type-safe**: Errors caught at compile time
- **Performance**: Same as hand-written code

## Customization

### Adding New Actions

Edit `src/server.cpp`:

```cpp
json process_request(const json& request)
{
    std::string action = request.value("action", "unknown");
    
    if (action == "my_custom_action")
    {
        return handle_custom(request);
    }
    // ... other actions
}

json handle_custom(const json& request)
{
    auto llm_client = get_llama_client();
    json result = llm_client->complete("...", 512, 0.7);
    return {{"status", "success"}, {"result", result}};
}
```

### Building a Web UI

Replace CLI client with a web frontend:

```javascript
// React/Vue/Angular example
fetch('http://localhost:8081/api', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({
    action: 'chat',
    message: 'Hello!'
  })
})
.then(res => res.json())
.then(data => console.log(data));
```

## Requirements

- **C++17** compiler (g++, clang++, MSVC)
- **Python 3** (for build scripts)
- **llama.cpp** pre-built binaries
- **.gguf** model file

## Namespace Protection

All generated code uses the **pooriayousefi::llama::app** namespace.

Per Apache License 2.0, this namespace MUST be preserved in derivative works. See the NOTICE file for details.

### Why Namespace Protection?

- **Attribution**: Ensures proper credit to the original author
- **Legal**: Required by Apache 2.0 Section 4(c) and 4(d)
- **Technical**: Prevents name conflicts
- **Trademark**: Protects the signature of the project

## License

Copyright 2025 Pooria Yousefi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this project except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

## Third-Party Libraries

### Included in Generated Projects

- **nlohmann/json** - JSON for Modern C++ (MIT License)  
  https://github.com/nlohmann/json

- **cpp-httplib** - C++ HTTP/HTTPS library (MIT License)  
  https://github.com/yhirose/cpp-httplib

### Runtime Dependencies

- **llama.cpp** - LLM inference engine (MIT License)  
  https://github.com/ggerganov/llama.cpp

## Examples

### Generate and Build

```bash
# Generate project
./bin/llama-app-generator my-chatbot

# Build it
cd my-chatbot
python3 build.py

# Start llama-server (in separate terminal)
# (See generated README.md for details)

# Start app server
./bin/server

# Test with client
./bin/client chat "Tell me a joke"
```

### Custom Output Directory

```bash
./bin/llama-app-generator my-app ~/projects
# Creates: ~/projects/my-app/
```

## Troubleshooting

### Generator Won't Build

**Problem**: Compiler not found

**Solution**: Install g++ or clang++, or set CXX environment variable:
```bash
export CXX=g++
python3 build.py
```

### Generated Project Won't Build

**Problem**: Missing headers

**Solution**: Ensure template files exist in `templates/include/`:
- json.hpp
- httplib.h
- llama_client.hpp
- runtime_config.hpp
- app_server_base.hpp

### config.txt Not Found

**Problem**: Generator can't find config.txt

**Solution**: Create config.txt in the directory where you run the generator.

## Support

- **GitHub**: <https://github.com/pooriayousefi/llama-app-generator>
- **Email**: <pooriayousefi@aol.com>
- **Issues**: <https://github.com/pooriayousefi/llama-app-generator/issues>

## Roadmap

- [ ] JSON-RPC 2.0 support (postponed for now, using simple JSON/HTTP)
- [ ] CMake build option
- [ ] VS Code extension integration
- [ ] Docker container templates
- [ ] Web UI templates (React, Vue)
- [ ] Mobile templates (React Native, Flutter)
- [ ] Multi-model support
- [ ] Streaming responses
- [ ] Authentication/authorization templates

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

**Important**: All contributions must preserve the `pooriayousefi::llama::app` namespace per the Apache License 2.0.

## Acknowledgments

See [ACKNOWLEDGMENTS.md](ACKNOWLEDGMENTS.md) for full acknowledgments including:
- AI development assistance (Anthropic's Claude Sonnet 4.5 Preview)
- Third-party library authors
- Open-source community contributions

## Citation

If you use this project in your research or product, please cite:

```
@software{yousefi2025llamaappgenerator,
  author = {Yousefi, Pooria},
  title = {llama-app-generator: Template-based Generator for llama.cpp Applications},
  year = {2025},
  url = {https://github.com/pooriayousefi/llama-app-generator}
}
```
