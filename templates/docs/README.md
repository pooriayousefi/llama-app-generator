# {{PROJECT_NAME}}

A llama.cpp-powered application generated using **llama-app-generator**.

Generated on {{DATE}}

## Overview

This project uses the **pooriayousefi::llama::app** namespace and provides a three-tier architecture:

```
llama-server (port_x) ←→ AppServer (port_y) ←→ CLI/GUI Client
```

- **llama-server**: The llama.cpp HTTP server providing LLM inference
- **AppServer**: Your custom application server with business logic (CRTP pattern)
- **Client**: CLI or GUI client for end users

## Architecture

### Three-Tier Design

1. **llama-server** (port_x)
   - Pre-built llama.cpp binary
   - Serves LLM inference via HTTP
   - Configured in `config.txt`

2. **AppServer** (port_y)
   - Your application logic in `src/server.cpp`
   - Uses CRTP (Curiously Recurring Template Pattern)
   - Internal `LlamaClient` communicates with llama-server
   - Exposes REST API for clients

3. **CLI/GUI Client**
   - Example CLI in `src/client.cpp`
   - Communicates with AppServer via HTTP
   - Can be replaced with GUI (web, desktop, mobile)

### CRTP Pattern

The server uses compile-time polymorphism:

```cpp
class MyApp : public AppServerBase<MyApp>
{
public:
    using AppServerBase<MyApp>::AppServerBase;
    
    json process_request(const json& request)
    {
        // Your business logic here
        return {...};
    }
};
```

Benefits:
- Zero runtime overhead (no virtual functions)
- Better compiler optimization
- Type-safe at compile time

## Build Requirements

- **C++17** compiler (g++, clang++, MSVC)
- **Python 3** (for build script)
- **llama.cpp** pre-built binaries
- **Model file** (.gguf format)

## Setup

### 1. Configure `config.txt`

Edit `config.txt` with 4 lines:

```
/path/to/llama.cpp/build-cpu/bin
/path/to/model.gguf
8080
8081
```

- Line 1: Path to llama.cpp binaries directory
- Line 2: Path to your .gguf model file
- Line 3: port_x (llama-server port)
- Line 4: port_y (app-server port)

### 2. Build the Project

```bash
python3 build.py
```

This creates:
- `bin/server` - Application server
- `bin/client` - CLI client

### 3. Start llama-server

In a separate terminal:

```bash
# Read config
source <(sed -n '1p;2p;3p' config.txt | \
  awk 'NR==1{bin=$0} NR==2{model=$0} NR==3{port=$0} \
  END{print "LLAMA_BIN="bin; print "MODEL="model; print "PORT="port}')

# Start llama-server
$LLAMA_BIN/llama-server \
  --model $MODEL \
  --port $PORT \
  --host 0.0.0.0 \
  --ctx-size 2048
```

Or use a shell script:

```bash
#!/bin/bash
# start_llama_server.sh

LLAMA_BIN=$(sed -n '1p' config.txt)
MODEL=$(sed -n '2p' config.txt)
PORT=$(sed -n '3p' config.txt)

$LLAMA_BIN/llama-server \
  --model $MODEL \
  --port $PORT \
  --host 0.0.0.0 \
  --ctx-size 2048
```

### 4. Start App Server

```bash
./bin/server
```

Expected output:
```
==================================
  {{PROJECT_NAME}} Server
  namespace: pooriayousefi::llama::app
==================================
LLM Server: http://localhost:8080
API Port:   8081

Server listening on 0.0.0.0:8081
Endpoints:
  GET  /health  - Health check
  POST /api     - Main API

Press Ctrl+C to stop
```

### 5. Test with Client

```bash
# Echo (no LLM)
./bin/client echo "Hello World"

# Text completion
./bin/client complete "Explain quantum computing in simple terms"

# Chat
./bin/client chat "Tell me a joke"

# Summarize
./bin/client summarize "Long text to summarize here..."
```

## API Reference

### Health Check

```bash
curl http://localhost:8081/health
```

Response:
```json
{
  "status": "ok",
  "service": "app-server"
}
```

### Main API Endpoint

**Endpoint**: `POST /api`

**Request Format**:
```json
{
  "action": "complete|chat|summarize|echo",
  "prompt": "...",    // for complete
  "message": "...",   // for chat or echo
  "text": "..."       // for summarize
}
```

**Examples**:

1. **Complete** (text completion):
```json
{
  "action": "complete",
  "prompt": "What is the meaning of life?"
}
```

2. **Chat** (conversation):
```json
{
  "action": "chat",
  "message": "Tell me a joke"
}
```

3. **Summarize** (text summarization):
```json
{
  "action": "summarize",
  "text": "Long text to summarize..."
}
```

4. **Echo** (testing, no LLM):
```json
{
  "action": "echo",
  "message": "Hello"
}
```

## Customization

### Editing Business Logic

Edit `src/server.cpp` and implement `process_request()`:

```cpp
class MyApp : public AppServerBase<MyApp>
{
public:
    using AppServerBase<MyApp>::AppServerBase;
    
    json process_request(const json& request)
    {
        std::string action = request.value("action", "unknown");
        
        if (action == "my_custom_action")
        {
            return handle_my_action(request);
        }
        // ... other actions
        
        return {
            {"status", "error"},
            {"message", "Unknown action: " + action}
        };
    }

private:
    json handle_my_action(const json& request)
    {
        // Use get_llama_client() to call LLM
        auto llm_client = get_llama_client();
        
        std::string prompt = "...";
        json llm_response = llm_client->complete(prompt, 512, 0.7);
        
        return {
            {"status", "success"},
            {"result", llm_response["content"]}
        };
    }
};
```

### Adding New Endpoints

You can add custom HTTP routes by modifying `setup_routes()` in a derived class.

### Building a GUI Client

Replace `src/client.cpp` with:
- **Web UI**: React/Vue/Angular → calls `POST /api`
- **Desktop**: Electron/Qt → HTTP client
- **Mobile**: React Native/Flutter → HTTP requests

## License

Copyright 2025 Pooria Yousefi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

### Namespace Protection

This project uses the **pooriayousefi::llama::app** namespace.

Per the Apache License 2.0 and the NOTICE file, this namespace MUST be preserved in all derivative works. See the NOTICE file for details.

## Third-Party Libraries

- **nlohmann/json** - JSON for Modern C++ (MIT License)
- **cpp-httplib** - C++ HTTP/HTTPS library (MIT License)
- **llama.cpp** - LLM inference (MIT License)

## Troubleshooting

### Server Won't Start

1. Check if port is already in use:
   ```bash
   lsof -i :8081
   ```

2. Verify config.txt paths exist and are correct

3. Ensure llama-server is running on port_x

### Client Connection Failed

1. Verify server is running:
   ```bash
   curl http://localhost:8081/health
   ```

2. Check firewall settings

3. Verify port_y in config.txt matches server port

### LLM Errors

1. Check llama-server logs
2. Verify model file is valid .gguf format
3. Ensure llama-server has enough memory
4. Try reducing `ctx-size` parameter

## Support

For issues with llama-app-generator:
- GitHub: <https://github.com/pooriayousefi/llama-app-generator>
- Email: <pooriayousefi@aol.com>

For issues with llama.cpp:
- GitHub: https://github.com/ggerganov/llama.cpp
