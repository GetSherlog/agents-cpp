# Logging in agents-cpp

This document describes the logging system in agents-cpp, which uses [spdlog](https://github.com/gabime/spdlog) as its backend.

## Overview

agents-cpp provides a logging utility class called `Logger` that wraps spdlog functionality. This provides several advantages over using `std::cout`:

- Log levels (trace, debug, info, warn, error, critical)
- Structured formatting
- Colorized console output
- Common API that can be extended to file logging, rotating logs, etc.
- Better performance than iostream

## Basic Usage

```cpp
#include <agents-cpp/logger.h>

using namespace agents;

int main() {
    // Initialize the logger (do this at the start of your program)
    Logger::init(Logger::Level::INFO); // or any other level
    
    // Log messages at different levels
    Logger::trace("This is a trace message");
    Logger::debug("This is a debug message");
    Logger::info("This is an info message");
    Logger::warn("This is a warning message");
    Logger::error("This is an error message");
    Logger::critical("This is a critical message");
    
    // Log with formatting (similar to printf)
    Logger::info("Hello, {}!", "world");
    Logger::info("Value = {}, String = {}", 42, "text");
    
    return 0;
}
```

## Log Levels

The logger supports the following log levels (in increasing order of severity):

- `TRACE`: Very detailed information, typically useful only when debugging specific issues
- `DEBUG`: Detailed information, typically useful for debugging
- `INFO`: Informational messages that highlight the progress of the application (default)
- `WARN`: Potentially harmful situations that might require attention
- `ERROR`: Error events that might still allow the application to continue running
- `CRITICAL`: Very severe error events that will likely lead to application failure
- `OFF`: Turns off all logging

## Migrating from std::cout

When migrating from `std::cout` to the Logger, here's a quick reference for replacement:

| std::cout usage | Logger equivalent |
|-----------------|-------------------|
| `std::cout << "Message" << std::endl;` | `Logger::info("Message");` |
| `std::cout << "Value: " << value << std::endl;` | `Logger::info("Value: {}", value);` |
| `std::cout << "Val1: " << val1 << ", Val2: " << val2 << std::endl;` | `Logger::info("Val1: {}, Val2: {}", val1, val2);` |

For error messages that were previously sent to `std::cerr`, use `Logger::error()` or `Logger::critical()`.

## Setting the Log Level

You can set the log level at any time using:

```cpp
Logger::setLevel(Logger::Level::DEBUG);
```

This is useful for adjusting the verbosity of logging during runtime or based on command-line arguments. 