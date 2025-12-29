# OSCompatible

**OSCompatible** is a cross-platform **C++ threading library** that extends the familiar `std::thread` style API with **operating-system–specific thread controls**:

- Thread **priority**
- Thread **scheduling policy**
- **CPU affinity** (core binding)

It currently supports **Windows** and **Linux**, while keeping your code portable and easy to read.

---

## Why OSCompatible?

The C++ standard library intentionally keeps `std::thread` minimal. As a result, important system-level features are missing or platform-specific:

- You cannot set thread priority portably
- You cannot select a scheduling policy
- You cannot bind a thread to specific CPU cores

**OSCompatible** solves this by providing a small, lightweight wrapper that:
- Preserves a `std::thread`-like construction style
- Adds optional per-thread properties
- Uses native OS APIs internally

---

## Features

- ✅ Cross-platform (**Windows & Linux**)
- ✅ `std::thread`-style API
- ✅ Thread priority control
- ✅ Scheduling policy support
- ✅ CPU affinity (core pinning)
- ✅ CMake-based build system
- ✅ No external dependencies

---

# Basic Usage

## Simple usage (similar to `std::thread`)

```cpp
#include <OSCompatible/thread.hpp>

void worker(int value)
{
    // do work
}

int main()
{
    OSCompatible::thread t(worker, 42);
    t.join();
}
```

## Using thread properties (priority / policy / affinity)

```cpp
#include <OSCompatible/thread.hpp>

void realtime_task()
{
    // time-critical work
}

int main()
{
    OSCompatible::thread::Properties props = {
        OSCompatible::thread::DEFAULT_PRIORITY,
        OSCompatible::thread::DEFAULT_POLICY,
        OSCompatible::thread::DEFAULT_AFFINITY
    };

    OSCompatible::thread t(props, realtime_task);
    t.join();
}
```
Thread behavior depends on the underlying operating system and user permissions.



## Building

The project uses CMake.

```bash
git clone https://github.com/rostik36/OSCompatible.git
cd OSCompatible
cmake -S . -B build
cmake --build build
```



## Integration

### Add as a subdirectory

```cmake
add_subdirectory(OSCompatible)
target_link_libraries(your_target PRIVATE OSCompatible)
```

### Or use FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
  OSCompatible
  GIT_REPOSITORY https://github.com/rostik36/OSCompatible.git
  GIT_TAG main
)

FetchContent_MakeAvailable(OSCompatible)
target_link_libraries(your_target PRIVATE OSCompatible)
```


## Platform Notes
### Linux

- Scheduling policies such as SCHED_FIFO or SCHED_RR may require:

   - elevated privileges

   - CAP_SYS_NICE

   - appropriate system limits

 - Windows

   - Thread priority and CPU affinity are implemented using native Windows thread APIs.

---

### Project Goals

 - Keep the API simple and familiar

 - Avoid hiding OS limitations

 - Make advanced threading features explicit and portable

 - Stay lightweight and dependency-free

## License

MIT License
