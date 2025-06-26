#
# Makefile for the C++/SystemC PLL Project
#
# Project: C++/SystemC High-Level Model of a PLL Configuration
#
# Author: Kumar Vedang
#
# Description:
# This file is a `Makefile`, which acts as the automated build system for the entire project. Its purpose is to orchestrate the entire
# process of turning the C++ source code files (`.cpp`) into a final, runnable simulation executable (`pll_sim`). It manages dependencies,
# specifies compiler options, and provides simple, high-level commands (like `make`, `make run`, and `make clean`) to perform complex
# compilation and linking tasks.
#
# In any serious software or hardware modeling project, a robust build system like this is not an optional extra; it is a fundamental
# piece of infrastructure. It ensures that the build process is repeatable, reliable, and efficient, saving countless hours of manual effort
# and reducing the potential for human error. It automates the "paperwork" of compilation so I can focus on the design and verification logic.
#

#================================================================================================================================
# Build Environment Sanity Check
#================================================================================================================================
# What is it: This is a conditional block in GNU Make syntax. `ifeq` checks if the two arguments are equal. Here, it checks if the
#             value of the `SYSTEMC_HOME` variable is an empty string.
# How it works: When I run `make`, the first thing it does is evaluate this condition. If the `SYSTEMC_HOME` environment variable has
#               not been set in my terminal session, `$(SYSTEMC_HOME)` will be empty. The condition becomes true, and `make` will
#               execute the `$(error ...)` function.
# Purpose: This is a critical "fail-fast" or "sanity-check" mechanism. The rest of this Makefile depends on `SYSTEMC_HOME` to find the
#          SystemC header files and libraries. If it's not set, the build would fail later with a series of confusing "file not found"
#          errors. This check stops the build immediately and provides a clear, human-readable error message, telling me exactly what
#          I need to do to fix the problem. This is a hallmark of a robust and user-friendly build script.

ifeq ($(SYSTEMC_HOME),)
    $(error Please set the SYSTEMC_HOME environment variable)
endif




#
# Makefile for the C++/SystemC PLL Project
#
# What is it: This line defines a Makefile variable named `CXX` and assigns it the value `g++`. In Makefiles, variables are typically
#             used to hold strings that will be substituted into commands later.
# Purpose: `CXX` is the conventional variable name for the C++ compiler. By defining it here, I can use `$(CXX)` throughout the file.
#          This makes the Makefile more portable. If I ever wanted to switch to a different compiler (like `clang++`), I would only
#          need to change this single line. `g++` is the GNU C++ Compiler, which I chose because it's a powerful, standard-compliant,
#          and freely available compiler that is part of the MinGW-w64 toolchain I used to set up my Windows development environment.
CXX = g++


# What is it: This line defines the `CXXFLAGS` variable, which holds the set of command-line flags (options) that will be passed to
#             the C++ compiler (`g++`) every time it is invoked. This is the central location for all compiler settings.
# Purpose: Each flag serves a specific purpose to ensure the code compiles and links correctly with the SystemC library.
#   - `-I$(SYSTEMC_HOME)/include`: The `-I` flag tells the compiler to "Include" an additional directory when searching for header files
#     (like `<systemc.h>`). This flag is essential for the compiler to find the SystemC header files.
#   - `-L$(SYSTEMC_HOME)/lib-mingw64`: The `-L` flag tells the *linker* (the part of the compiler that combines object files) to "Look"
#     in an additional directory when searching for library files. This is necessary for the linker to find the `libsystemc.a` file.
#   - `-Wl,-rpath=$(SYSTEMC_HOME)/lib-mingw64`: This is a more advanced linker flag. It embeds the path to the SystemC library into the
#     final executable. This helps the operating system find the required SystemC dynamic library (`.dll` or `.so`) at runtime, although
#     for my static build, it's more of a good practice for potential dynamic linking scenarios.
#   - `-g`: This flag tells the compiler to include debugging information (like line numbers and variable names) in the final executable.
#     This is absolutely critical for being able to use a debugger like GDB to step through the code and inspect variables.
#   - `-std=c++17`: This explicitly tells the compiler to use the C++17 standard of the C++ language. This ensures that I can use modern
#     C++ features and that the code is compiled with a consistent language version.
# NOTE: The note is a reminder for myself or others that the library directory name might differ based on the specific SystemC build.

CXXFLAGS = -I$(SYSTEMC_HOME)/include -L$(SYSTEMC_HOME)/lib-mingw64 -Wl,-rpath=$(SYSTEMC_HOME)/lib-mingw64 -g -std=c++17




# What is it: This defines the `LIBS` variable.
# Purpose: This variable holds the list of libraries that need to be linked into the final executable. The `-l` flag is a directive
#          to the linker. `-lsystemc` specifically tells the linker: "Find and link the library named 'systemc'". The linker will
#          automatically look for a file named `libsystemc.a` (for static linking) or `libsystemc.so`/`.dll` (for dynamic linking)
#          in the standard library paths and any paths specified with the `-L` flag.
LIBS = -lsystemc



#================================================================================================================================
# Project Directory Structure Variables
#================================================================================================================================
# What is it: These lines define several Makefile variables that specify the directory structure of my project. This is a key
#             practice for keeping a project clean and organized, separating source code from intermediate build files and final outputs.
# Purpose: By defining these paths as variables, I can easily change the project layout in the future by editing only these lines. It
#          also makes the rules at the bottom of the Makefile much more readable and generic.

# `SRC_DIR`: This variable holds the name of the directory where all my C++ source files (`.cpp`) are located.
#            This keeps the core logic of the project neatly contained in one place.
SRC_DIR = src



# `OBJ_DIR`: This variable holds the name of the directory where the compiler will place intermediate object files (`.o`). Object files
#            are the result of compiling a single source file but before it has been linked into a final program. Separating them
#            prevents cluttering up the source directory.
OBJ_DIR = obj


# `BIN_DIR`: This variable holds the name of the directory where the final, compiled executable (`pll_sim`) will be placed. 'bin' is
#            a standard convention for "binary" executables.
BIN_DIR = bin



# What is it: This defines the `TARGET` variable, which holds the full path and name of the final executable file we want to build.
# Purpose: This is the ultimate goal of the Makefile. The primary `all` target will depend on this `TARGET` variable. By defining it
#          here, the rest of the Makefile knows exactly what file it is trying to create. It uses the `$(BIN_DIR)` variable to ensure
#          the executable is placed in the correct output directory.
TARGET = $(BIN_DIR)/pll_sim




#================================================================================================================================
# Automatic File Discovery (Scalability)
#================================================================================================================================

# What is it: This line defines the `SOURCES` variable by using the built-in GNU Make function `wildcard`.
# How it works: The `wildcard` function scans the file system for files that match the given pattern. In this case, `$(SRC_DIR)/*.cpp`
#               expands to `src/*.cpp`, telling Make to find all files in the `src` directory that end with the `.cpp` extension. The
#               result is a space-separated list of all my source filenames (e.g., `src/main.cpp src/pll.cpp src/pmu_tb.cpp`).
# Purpose: This is the key to making the Makefile scalable and automatic. If I add a new source file (`new_module.cpp`) to the `src`
#          directory, I don't need to edit this Makefile at all. The next time I run `make`, this `wildcard` function will automatically
#          discover the new file and add it to the build process.

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)




# What is it: This line defines the `OBJECTS` variable by using the built-in `patsubst` (pattern substitution) function.
# How it works: `patsubst` performs a text-based find-and-replace on a list of strings. It takes three arguments:
#               1. The pattern to find: `$(SRC_DIR)/%.cpp` (e.g., `src/%.cpp`)
#               2. The pattern to replace it with: `$(OBJ_DIR)/%.o` (e.g., `obj/%.o`)
#               3. The list of strings to operate on: `$(SOURCES)`
#             It goes through the `SOURCES` list and for each filename (e.g., `src/main.cpp`), it replaces the source path and extension
#             with the object path and extension, resulting in a new list: `obj/main.o obj/pll.o obj/pmu_tb.o`.
# Purpose: This automatically generates the list of all the intermediate object files that need to be created, one for each source file.
#          This list is crucial, as it becomes the list of dependencies for the final linking step. This, combined with `wildcard`,
#          completely automates the project's file management.

OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))



#================================================================================================================================
# Main Build Target & Linking Rule
#================================================================================================================================

# What is it: This line defines a target named `all`. In GNU Make, the first target in the file is the default target that runs when
#             you simply type `make` with no arguments.
# How it works: This target has one dependency: `$(TARGET)`, which is our final executable `bin/pll_sim`. This means that to satisfy the
#               `all` target, `make` must first satisfy the `$(TARGET)` target. This creates a chain of dependencies that drives the
#               entire build process.
# Purpose: It provides a standard, conventional name for the primary action of the Makefile, which is to build the entire project.
all: $(TARGET)


# What is it: This is the rule that defines how to build the final executable (`$(TARGET)`).
# How it works:
#   - Target: `$(TARGET)` is the file this rule is responsible for creating (e.g., `bin/pll_sim`).
#   - Dependencies: `$(OBJECTS)` is the list of files that must exist and be up-to-date *before* this rule can run. This means `make`
#     will first ensure all the `.o` files (e.g., `obj/main.o`, `obj/pll.o`) are compiled.
#   - Commands: These are the shell commands that are executed to create the target.
#     - `-if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"`: This is a Windows-specific command to create the `bin` directory if it doesn't
#       already exist. The leading `-` tells `make` to ignore any errors from this command (e.g., if the directory already exists).
#     - `@echo "==> Linking..."`: The `@` symbol at the beginning of a command tells `make` not to print the command itself to the
#       console, only its output. This creates a cleaner build log. This line just prints a status message.
#     - `$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)`: This is the core **linking command**.
#       - `$(CXX)`: The C++ compiler (`g++`).
#       - `$(CXXFLAGS)`: All our compiler/linker flags.
#       - `-o $@`: The `-o` flag specifies the output filename. `$@` is a special "automatic variable" in Make that expands to the
#         name of the current target (in this case, `bin/pll_sim`).
#       - `$^`: This is another automatic variable that expands to the full list of all dependencies (in this case, all the `.o` files).
#       - `$(LIBS)`: Our required libraries (`-lsystemc`).
#     - `@echo "==> Build finished..."`: Prints a final success message.
# Purpose: This rule combines all the separately compiled object files and links them with the SystemC library to create the single,
#          final executable program.
$(TARGET): $(OBJECTS)
	-if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	@echo "==> Linking..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "==> Build finished. Executable is at: $(TARGET)"



#================================================================================================================================
# Compilation Pattern Rule
#================================================================================================================================
# What is it: This is a "pattern rule" in GNU Make. It's a powerful feature that defines a generic template for how to create one
#             type of file from another.
# How it works:
#   - Target: `$(OBJ_DIR)/%.o` defines the pattern for the files this rule can create. The `%` is a wildcard that can match any string.
#     So this matches any file in the `obj` directory that ends with `.o` (e.g., `obj/main.o`).
#   - Dependency: `$(SRC_DIR)/%.cpp` defines the pattern for the corresponding source file. Make is smart enough to use the same string
#     that matched the `%` in the target. So, if Make needs to create `obj/main.o`, it knows the dependency is `src/main.cpp`.
#   - Commands: These are the shell commands executed to perform the compilation.
#     - `-if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"`: Creates the `obj` directory if needed.
#     - `@echo "==> Compiling $<..."`: Prints a status message. `$<` is an automatic variable that expands to the name of the *first*
#       dependency (in this case, the specific `.cpp` file being compiled, like `src/main.cpp`).
#     - `$(CXX) $(CXXFLAGS) -c -o $@ $<`: This is the core **compilation command**.
#       - `$(CXX) $(CXXFLAGS)`: The compiler and its flags.
#       - `-c`: This is a crucial flag. It tells the compiler to "compile only". It will generate an object file (`.o`) but will *not*
#         attempt to link it into a final executable. This is what allows for separate compilation.
#       - `-o $@`: The `-o` flag specifies the output filename. `$@` expands to the name of the current target (e.g., `obj/main.o`).
#       - `$<`: Expands to the name of the source file being compiled (e.g., `src/main.cpp`).
# Purpose: This single, elegant rule provides a generic recipe for compiling *any* `.cpp` file in my project. I do not need to write a
#          separate rule for `main.cpp`, `pll.cpp`, etc. This makes the Makefile extremely concise and maintainable. It's the engine that
#          creates all the object file dependencies needed by the final linking rule.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	-if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
	@echo "==> Compiling $<..."
	$(CXX) $(CXXFLAGS) -c -o $@ $<



#================================================================================================================================
# Utility Targets
#================================================================================================================================

# What is it: This defines a utility target named `run`.
# How it works: It has a dependency on the `all` target. This means if I type `make run`, `make` will first ensure that the `all`
#               target is satisfied (i.e., the entire project is compiled and up-to-date). Only after the build is successful will it
#               proceed to execute the commands for the `run` target.
#   - `@echo "==> Running simulation..."`: Prints a status message.
#   - `./$(TARGET)`: This command executes the compiled program. `./` is the standard way to run an executable in the current
#     directory's path structure.
# Purpose: This provides a convenient, single command (`make run`) to both build and execute the simulation, streamlining my entire
#          workflow.

run: all
	@echo "==> Running simulation..."
	./$(TARGET)




# What is it: This defines a utility target named `clean`.
# How it works: It has no dependencies. It simply executes shell commands to remove the directories containing all the generated
#               build artifacts (the object files and the final executable).
#   - `@-if exist "$(OBJ_DIR)" rmdir /s /q "$(OBJ_DIR)"`: The `@` suppresses printing of the command. The `-` before the command tells
#     `make` to ignore any errors (e.g., if the directory doesn't exist). `rmdir /s /q` is the Windows command to silently and
#     recursively remove a directory.
# Purpose: This is a standard and essential target in any professional Makefile. It allows me to easily clean up my project directory
#          and return it to a pristine state, which is crucial for ensuring a fresh, complete rebuild from scratch.
clean:
	@echo "==> Cleaning up build files..."
	@-if exist "$(OBJ_DIR)" rmdir /s /q "$(OBJ_DIR)"
	@-if exist "$(BIN_DIR)" rmdir /s /q "$(BIN_DIR)"



# What is it: This is a "special target" in Make. It declares that the listed targets (`all`, `clean`, `run`) are "phony".
# Purpose: A phony target is one that does not represent an actual file on the disk. This declaration prevents `make` from getting
#          confused if I ever create a file or directory with the same name as one of these targets (e.g., a file named `clean`).
#          It tells `make`, "When I ask for `clean`, always run the commands associated with it, regardless of whether a file named
#          `clean` exists." This ensures the utility targets always work as expected.
.PHONY: all clean run





#================================================================================================================================
#================================================================================================================================
#
#                               DETAILED PROJECT DOCUMENTATION AND ANALYSIS
#
#================================================================================================================================
#================================================================================================================================
#
# -------------------------------------------------------------------------------------------------------------------------------
# I. RELEVANCE OF THIS CODE FILE (Makefile)
# -------------------------------------------------------------------------------------------------------------------------------
#
# This `Makefile` is the operational backbone of the entire project. While it contains no C++ or hardware logic, its relevance is
# paramount because it provides a robust, repeatable, and automated build process. In the VLSI and software industries, source code
# is useless until it can be reliably compiled into a working executable. This build system is what makes that possible.
#
# This file helps by:
#   - Automating Compilation and Linking: It replaces a long, error-prone series of manual g++ commands with a single command: `make`.
#   - Managing Dependencies: It intelligently understands that if a single `.cpp` file is changed, only that file needs to be
#     recompiled before re-linking, saving significant time on large projects.
#   - Enforcing Consistency: It ensures that every module is compiled with the exact same set of flags (`CXXFLAGS`), preventing
#     subtle bugs that can arise from inconsistent build settings.
#   - Streamlining the Workflow: It provides simple, high-level targets like `make run` and `make clean` that combine multiple steps
#     into one, dramatically improving my productivity as a developer.
#
# -------------------------------------------------------------------------------------------------------------------------------
# II. MAIN CONCEPTS IMPLEMENTED AND UNDERSTOOD
# -------------------------------------------------------------------------------------------------------------------------------
#
# This file demonstrates a practical understanding of essential software build system concepts:
#
#   1.  Declarative Programming:
#       - What is it: A programming paradigm where you describe *what* you want to achieve, not *how* to achieve it.
#       - Where is it used: The entire Makefile is declarative. I state that the `TARGET` depends on the `OBJECTS`, and the `OBJECTS`
#         are derived from the `SOURCES`. I don't write a sequential script; I declare relationships, and the `make` utility figures
#         out the correct sequence of commands to execute.
#
#   2.  Dependency Management:
#       - What is it: The process of defining prerequisites for build steps.
#       - Where is it used: The lines `all: $(TARGET)` and `$(TARGET): $(OBJECTS)` explicitly define a dependency graph. `make` will
#         not attempt to link the final executable until all the necessary object files have been successfully compiled.
#
#   3.  Scalability through Automation:
#       - What is it: Designing a system that can handle growth without requiring significant manual changes.
#       - Where is it used: The use of the `wildcard` and `patsubst` functions is the key to scalability. These functions automatically
#         discover all source files and generate the corresponding object file names. This means I can add new C++ modules to the
#         project, and this Makefile will automatically include them in the build without a single edit.
#
#   4.  Separate Compilation and Linking:
#       - What is it: The two-stage process of first compiling each source file into an intermediate "object file" (`.o`), and then
#         linking all the object files together into a final executable.
#       - Where is it used: The pattern rule (`%.o: %.cpp`) with the `-c` flag handles the compilation stage. The final target rule
#         (`$(TARGET): $(OBJECTS)`) handles the linking stage.
#       - Why is it used: This is vastly more efficient for large projects. If I only change one `.cpp` file, `make` is smart enough
#         to only re-run the compilation for that single file, and then re-link. It doesn't need to recompile all the other unchanged files.
#
# -------------------------------------------------------------------------------------------------------------------------------
# III. IMPLEMENTATION ETHIC AND INTEGRATION STRATEGY
# -------------------------------------------------------------------------------------------------------------------------------
#
# How I Started:
# I knew from the beginning that I didn't want to rely on a complex IDE's "magic" build button. I wanted to understand and control the
# build process from the ground up, as this is a fundamental skill. My goal was to create a build system that was as professional and
# robust as the C++ code it was compiling.
#
# My Coding Ethic and Process:
# I built this Makefile iteratively, adding features and robustness step-by-step.
#   1.  Start Simple: My first version was very basic, with hard-coded filenames and a single rule to compile and link everything
#       at once. This got the project working but was not maintainable.
#   2.  Introduce Variables: I then refactored the file to use variables for the compiler (`CXX`), flags (`CXXFLAGS`), and filenames.
#       This was the first step towards making it clean and configurable.
#   3.  Separate Compilation: I then implemented separate compilation by adding the pattern rule for creating `.o` files. This was a
#       major step in making the build system efficient.
#   4.  Automate File Discovery: The final and most important step was to add the `wildcard` and `patsubst` functions. This is what
#       transformed the Makefile from a static script into a dynamic, scalable build system.
#   5.  Add User-Friendliness: Finally, I added features like the `SYSTEMC_HOME` check, the clean and run targets, and the `@echo`
#       messages to make the build process easy to use and debug.
#
# How this File Integrates the System:
# This Makefile is the master integrator for the entire project. It is the tool that reads all the separate `.cpp` and `.h` files
# and understands how to combine them, along with the external SystemC library, into a single, cohesive program. It is the "factory"
# that assembles the final product from all the individual parts I designed.
#
# -------------------------------------------------------------------------------------------------------------------------------
# IV. INDUSTRIAL RELEVANCE AND PRACTICAL APPLICATIONS
# -------------------------------------------------------------------------------------------------------------------------------
#
# Where this concept is used in the VLSI Industry:
# A robust, command-line-driven build system is absolutely central to the entire VLSI industry. While more modern and complex build
# systems like CMake are now common, the fundamental principles of dependency management and automated scripting embodied in this
# Makefile are universal. Engineers working at any major semiconductor company use these systems daily.
#
# Practical Applications:
#
#   1.  **Continuous Integration (CI) / Regression Environments:** In a professional setting, there are automated servers that constantly
#       check out the latest code, build it, and run a massive suite of verification tests (a "regression"). This entire process is
#       driven by scripts, and a Makefile like this is often the core engine of the build step. The ability to simply call `make` from
#       a script is essential for this automation.
#
#  **Reproducible Builds:** A key requirement in any engineering discipline is reproducibility. This Makefile guarantees that anyone
#     with the same toolchain and source code can produce the exact same executable, because all the build settings are codified
#     within the file. This is critical for debugging issues that might appear on one engineer's machine but not another's.
#
#   - **Cross-Platform Development:** While my Makefile has some Windows-specific commands, the use of variables and standard targets makes
#     it relatively easy to adapt for a Linux environment. A production Makefile would use more advanced techniques to make it fully
#     platform-independent, but the core structure would be the same.
#
# Industrially Relevant Insights:
#
#   - Technical Insight: I learned that the build system is a "first-class citizen" in a project, just as important as the source code
#     itself. A poorly designed build system can waste hundreds of engineering hours through slow, unreliable, or confusing builds.
#     Investing time in a clean, automated Makefile pays for itself many times over.
#
#   - Non-Technical Insight: This experience highlighted the importance of infrastructure in engineering. The most brilliant design is
#     useless if you can't build it. The Makefile represents the "factory floor"—the tools and processes that turn raw materials (source
#     code) into a finished product (the simulation). A good engineer must understand and be able to improve not just the product, but
#     also the process used to create it.
#
# -------------------------------------------------------------------------------------------------------------------------------
# V. EXECUTION ENVIRONMENT AND COMMANDS
# -------------------------------------------------------------------------------------------------------------------------------
#
# This project was developed and executed entirely from the command line within the integrated terminal of Visual Studio Code, running
# on a Windows operating system. The MSYS2 environment provided the necessary GNU/Linux-like tools.
#
# The typical execution flow from the project's root directory was:
#
#   1. `set SYSTEMC_HOME=C:\systemc-3.0.1`
#      - This command sets the environment variable for the current terminal session, pointing the Makefile to my SystemC installation.
#
#   2. `make clean`
#      - This command invokes the `clean` target, removing any previous build artifacts to ensure a fresh start.
#
#   3. `make` (or `make all`)
#      - This invokes the default `all` target, which triggers the full compilation and linking process, creating the `pll_sim` executable.
#
#   4. `make run`
#      - This is my primary workflow command. It automatically ensures the build is up-to-date and then immediately executes the
#        simulation program, which prints the test results to the console and generates the `waveform.vcd` file.
#
#   5. `gtkwave waveform.vcd`
#      - After a simulation run, I would use this command to launch the GTKWave viewer to visually analyze the generated waveform, which
#        was my primary method for debugging and final verification.
#