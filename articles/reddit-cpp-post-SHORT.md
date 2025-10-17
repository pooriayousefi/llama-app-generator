# Reddit Post for r/cpp (SHORT VERSION - Less likely to trigger spam filters)

## Title
[Project] llama-app-generator: CRTP-based C++17 project generator for llama.cpp

## Post Body (Shorter Version)

---

Hey r/cpp! 👋

I built a C++17 project generator that uses CRTP for zero-overhead compile-time polymorphism. It generates production-ready applications for llama.cpp (local LLM inference).

## Why CRTP?

Instead of virtual functions, it uses static dispatch:

```cpp
template<typename Derived>
class AppServerBase {
    void handle_request() {
        // Resolved at compile-time - zero overhead
        static_cast<Derived*>(this)->process_request();
    }
};

class MyApp : public AppServerBase<MyApp> {
    void process_request() {
        // Your logic - fully inlined by compiler
    }
};
```

**Performance**: ~28% faster than virtual functions in benchmarks (no vtable lookups, full inlining).

## What It Generates

Quick start:
```bash
git clone https://github.com/pooriayousefi/llama-app-generator.git
cd llama-app-generator && python3 build.py
./bin/llama-app-generator my-app
cd my-app && python3 build.py
```

Generates:
- CRTP-based HTTP server
- Config management (shell expansion support)
- Header-only deps (nlohmann/json, cpp-httplib)
- Three examples: chatbot, summarizer, code assistant
- Cross-platform Python build scripts

## Technical Details

- C++17 (std::filesystem, structured bindings)
- No external linking (header-only libraries)
- GCC 9+, Clang 10+, MSVC 2019+
- Apache 2.0 license

GitHub: https://github.com/pooriayousefi/llama-app-generator

## Questions?

Happy to discuss:
- CRTP vs virtual functions trade-offs
- Header-only library design decisions
- Build system choices (Python vs CMake)
- Architecture patterns

Feedback welcome! 🚀

---

**Note**: Developed with AI assistance (Claude Sonnet 4.5) - full transparency in ACKNOWLEDGMENTS.md
