//
// File: pmu_tb.h
//
// Project: C++/SystemC High-Level Model of a PLL Configuration
//
// Author: Kumar Vedang
//
// Description:
// This header file serves as the public "datasheet" or "blueprint" for our Power Management Unit (PMU) Testbench module. In the context
// of this verification environment, the PMU acts as the "Test Host" or "Bus Master"—the intelligent, active component that drives the
// simulation forward. Its role is to control the Device Under Test (the `pll` module), send it a specific sequence of commands, and then
// monitor its response to verify if it behaved correctly according to its specification.
//
// This file's primary purpose is to define the public interface (the input and output ports) and the internal structure of the testbench.
// It declares the behavioral functions that will be implemented in `pmu_tb.cpp` but does not contain the logic itself. This separation
// of declaration from implementation is a cornerstone of good C++ and modular system design.
//

// What is it: This is a C++ preprocessor directive known as an "include guard". '#ifndef' stands for "if not defined".
// Why is it used: This is a standard and critical C++ mechanism to prevent the contents of this header file from being included more than
//               once within a single compilation unit. If a file like `main.cpp` were to include this header multiple times (perhaps
//               indirectly), the compiler would see the definition of the `pmu_tb` class twice, resulting in a fatal "redeclaration"
//               error. This guard ensures the code between `#ifndef` and `#endif` is processed only the first time it is encountered.

#ifndef PMU_TB_H



// What is it: This C++ preprocessor directive defines the macro `PMU_TB_H`.
// How it works: The first time the compiler processes this file, `PMU_TB_H` is not yet defined, so the `#ifndef` condition is true. The
//               compiler then immediately executes this line, which defines `PMU_TB_H`. On any subsequent attempt to include this file
//               within the same compilation, the `#ifndef` check will be false, and the compiler will skip directly to the `#endif` line,
//               thus avoiding the redefinition error. `PMU_TB_H` is a standard naming convention, matching the filename.
#define PMU_TB_H





//================================================================================================================================
// Library and Header Inclusions
//================================================================================================================================

// What is it: This is a C++ preprocessor directive that includes the master header file for the Accellera SystemC library.
// Role: It provides all the core functionalities needed for hardware simulation. This includes the simulation kernel and the definitions
//       for modules (SC_MODULE), ports (sc_in, sc_out), data types (sc_uint), and other simulation constructs.
// Purpose: This inclusion is essential for this file. It allows me to use SystemC-specific features like `SC_MODULE` to declare my
//          testbench, `sc_in`/`sc_out` to define its ports, and `SC_CTOR` for its constructor. Without this line, the compiler would
//          not recognize any of the SystemC syntax and would fail.

#include <systemc.h>





// What is it: This includes the local header file for the PLL module, which is our Device Under Test (DUT).
// Role: It provides the "datasheet" or public interface definition for the `pll` class.
// Why is it used here: This is a critical inclusion for a testbench. A testbench needs to "know" how to communicate with the DUT.
//                   By including `pll.h`, this testbench gains access to the PLL's register address map (the `PLL_REG_..._ADDR`
//                   macros). This allows my test sequence in `pmu_tb.cpp` to use clear, symbolic names when writing to the PLL's
//                   registers, rather than relying on hard-coded "magic numbers". This makes the testbench code far more readable,
//                   maintainable, and robust against future changes in the DUT's memory map.
#include "pll.h"





//================================================================================================================================
// C++ Concept: Data Structures (`struct`)
//================================================================================================================================
// What is it: This line declares a C++ `struct` named `PllConfig`. A struct (short for structure) is a composite data type that
//             groups together variables of potentially different types under a single name. In this case, it groups three standard
//             C++ `int` (integer) variables. By default, members of a struct are public.
// Why is it used: While not used in the final implementation of my simple directed test case, I included this structure during my
//               initial design phase. The intent was to create a clean, organized way to store a complete set of PLL configuration
//               parameters (M, N, and OD dividers).
// How it would be used: In a more advanced, coverage-driven verification scenario, I would use this struct to create objects representing
//                    different test configurations. For example, a constrained-random test could generate a `PllConfig` object with
//                    randomized (but legal) values for `m`, `n`, and `od`. This object could then be passed to a function to program
//                    the PLL. This is a much cleaner and more scalable approach than passing multiple loose variables, and it's a
//                    common practice in complex testbenches.

struct PllConfig { int m; int n; int od; };



// What is it: This line declares our testbench module. `SC_MODULE` is a SystemC macro that creates a C++ class named `pmu_tb` which
//             inherits the standard `sc_module` functionality, allowing it to have ports and processes.
SC_MODULE(pmu_tb) {


    // === PORTS to connect to the DUT ===

    //================================================================================================================================
    // Hardware Abstraction: Port Declarations
    //================================================================================================================================
    // The following lines declare the ports of the testbench module. These ports define the physical "pins" that will be connected
    // to the rest of the system in `main.cpp`. The direction of each port (`sc_in` or `sc_out`) is defined from the perspective of the
    // testbench itself, which acts as the "Test Host" or system controller.

    // --- Control and Status Ports ---

    // `sc_in<bool> clk`: Declares a single-bit input port for the main system clock. The testbench receives the clock signal and uses
    // its rising edge to time all its cycle-accurate operations.
    sc_in<bool> clk;


    // `sc_out<bool> reset`: Declares a single-bit output port for the system reset signal. The testbench will *drive* this signal,
    // asserting it high to reset the DUT and then de-asserting it to start the test.
    sc_out<bool> reset;



    // --- Bus Master Ports (Driving the DUT) ---

    // `sc_out<sc_uint<32>> bus_addr`: Declares a 32-bit output port for the address bus. The testbench drives this port to tell the DUT
    // which internal register it wants to access.
    sc_out<sc_uint<32>> bus_addr;


    // `sc_out<sc_uint<32>> bus_wdata`: Declares a 32-bit output port for the write data bus. The testbench drives this port with the
    // data it wants to write into the selected register.
    sc_out<sc_uint<32>> bus_wdata;


    // `sc_out<bool> bus_we`: Declares a single-bit "Write Enable" output port. The testbench asserts this signal high to indicate that
    // the address and data on the bus are valid for one clock cycle, signaling a write transaction.
    sc_out<bool>          bus_we;


    // --- Monitoring Port (Checking the DUT) ---

    // `sc_in<bool> pll_locked`: Declares a single-bit input port to monitor the DUT's status. The testbench will "listen" to the signal
    // connected to this port. When it sees a rising edge (0 to 1), it knows the PLL has successfully locked, which is the pass
    // condition for my test case.
    sc_in<bool> pll_locked;



//================================================================================================================================
// OOP Concept: Data Encapsulation & Private Member Functions
//================================================================================================================================
// What is it: This is the C++ `private:` access specifier. It ensures that the member functions declared below it can only be
//             called from within the `pmu_tb` class itself (i.e., from the code in `pmu_tb.cpp`). They are hidden from the outside
//             world and cannot be called directly by other modules like `main.cpp`.
// Why is it used: This enforces good encapsulation for the testbench. The testbench's behavior should be self-contained and managed
//               by the SystemC kernel via its registered processes. There is no reason for an external module to ever need to call
//               these internal logic functions directly. Keeping them private makes the design cleaner and less prone to misuse.
private:


    // What is it: This declares a private member function named `write_to_pll`. A member function (or method) defines a behavior
    //             that an object of the class can perform. This is just the declaration; the function's actual logic is defined in
    //             `pmu_tb.cpp`.
    // Role in the project: This function acts as a helper task or a basic "Bus Functional Model" (BFM). Its purpose is to abstract
    //                    the low-level details of performing a single bus write transaction. It will be called from within the
    //                    `run_test` sequence to drive the bus signals (`bus_addr`, `bus_wdata`, `bus_we`) with the correct timing.
    void write_to_pll(sc_uint<32> addr, sc_uint<32> data);


    // What is it: This declares the main private member function that will contain the entire test case logic.
    // Role in the project: This is the "brain" of the testbench. It will be registered with the SystemC kernel as an `SC_THREAD` process.
    //                    It contains the complete, ordered sequence of actions: asserting reset, programming the PLL registers (by
    //                    calling `write_to_pll`), monitoring the `pll_locked` signal, and finally determining a pass/fail result.
    void run_test();




// What is it: This is a C++ access specifier. The `public:` keyword means that all members declared after this point are
//             accessible from outside the `pmu_tb` class. In SystemC, the constructor is always public so that the module can
//             be instantiated in the top-level file.
public:


    //================================================================================================================================
    // SystemC Concept: The Constructor (`SC_CTOR`)
    //================================================================================================================================
    // What is it: This is the constructor for the `pmu_tb` module. A constructor is a special C++ function that is automatically
    //             called once when an object of this class is created. `SC_CTOR` is a SystemC macro that simplifies the constructor
    //             declaration.
    // Role: The constructor's role in SystemC is to perform all one-time setup for the module during the simulation's "elaboration"
    //       phase (before time starts). For this testbench, its sole responsibility is to register its main behavioral process,
    //       `run_test`, with the simulation kernel and define its sensitivity.
    // How it impacts execution: The code inside this constructor runs exactly once when `new pmu_tb(...)` is called in `main.cpp`. It
    //                         sets up the testbench so that it's ready to start executing its test sequence as soon as the simulation
    //                         begins.

    SC_CTOR(pmu_tb) {



        // This is a C++ `cout` statement that prints a message to the console. It's a simple and effective debug technique to confirm
        // in the simulation log that the testbench instance was successfully created at the start of elaboration.
        cout << "PMU Testbench module constructed." << endl;



        //================================================================================================================================
        // SystemC Concept: Process Registration and Sensitivity
        //================================================================================================================================

        // What is it: This line registers the `run_test` member function with the SystemC kernel as an `SC_THREAD` process.
        // What is an SC_THREAD: An `SC_THREAD` is a type of SystemC process ideal for modeling test sequences because it can be suspended
        //                      and resumed using `wait()` statements. This allows me to model actions that take a specific number of
        //                      clock cycles or wait for specific events from the DUT.
        // Why is it used here: The entire test case is a sequence of timed actions (reset for 5 cycles, write for 1 cycle, wait for
        //                    lock, etc.). `SC_THREAD` is the only process type that supports this kind of temporal behavior, making it
        //                    the natural and correct choice for implementing the main test sequence.
        SC_THREAD(run_test);



        // What is it: This line defines the "sensitivity list" for the `run_test` thread.
        // How it works: This tells the SystemC kernel that the `run_test` thread should only be sensitive to the *positive edge* (a
        //               transition from 0 to 1) of the signal connected to the `clk` port.
        // Why is it used: This is the key to creating a cycle-accurate testbench. By making the main thread sensitive only to the rising
        //               clock edge, every parameter-less `wait()` call inside `run_test` will mean "suspend execution and resume at
        //               the *next* rising clock edge". This allows me to precisely control the timing of my bus transactions and reset
        //               pulses in terms of clock cycles, which is how real digital systems are controlled.
        sensitive << clk.pos(); // Sensitive to clock for cycle-accurate waits
    }
};


// What is it: This preprocessor directive marks the end of the conditional block started by `#ifndef`. All code between `#ifndef` and `#endif`
//             is subject to the include guard logic.
#endif // PMU_TB_H







//================================================================================================================================
//================================================================================================================================
//
//                               DETAILED PROJECT DOCUMENTATION AND ANALYSIS
//
//================================================================================================================================
//================================================================================================================================
//
// -------------------------------------------------------------------------------------------------------------------------------
// I. RELEVANCE OF THIS CODE FILE (pmu_tb.h)
// -------------------------------------------------------------------------------------------------------------------------------
//
// This `pmu_tb.h` file is the blueprint for the most critical component of any verification environment: the Testbench. Its relevance
// is that it defines the structure of the "Test Host," the intelligent agent responsible for driving stimulus and checking results.
// In a verification context, the DUT (`pll`) is passive; it's the testbench that actively controls the entire simulation.
//
// This file helps by:
//   - Defining the Testbench Interface: It specifies the ports the testbench uses to connect to and control the DUT. The port directions
//     (`sc_out` for bus signals, `sc_in` for status) clearly define its role as a bus master.
//   - Providing Access to the DUT: By including `pll.h`, it gains the necessary knowledge (the register address map) to communicate
//     intelligently with the DUT, a fundamental requirement for any testbench.
//   - Structuring the Verification Logic: It declares the functions (`run_test`, `write_to_pll`) that will contain the test case logic,
//     separating the interface definition from the behavioral implementation.
//
// -------------------------------------------------------------------------------------------------------------------------------
// II. MAIN CONCEPTS IMPLEMENTED AND UNDERSTOOD
// -------------------------------------------------------------------------------------------------------------------------------
//
// This file demonstrates a solid understanding of fundamental verification and software engineering concepts:
//
//   1.  Testbench Architecture:
//       - What is it: A standard, structured approach to building a verification component.
//       - Where is it used: The declaration of a main test sequence (`run_test`) and a helper/driver task (`write_to_pll`) in this
//         header establishes a clean, layered architecture. This is a basic form of the more complex driver/monitor/scoreboard
//         architectures found in industrial frameworks like UVM.
//       - Why is it used: This structure makes the testbench code more readable, reusable, and maintainable.
//
//   2.  Bus Functional Model (BFM) / Driver Abstraction:
//       - What is it: A BFM is a piece of code that translates high-level transaction commands into low-level signal wiggling.
//       - Where is it used: The declaration of the `write_to_pll` function is the first step in creating our BFM. This function's
//         purpose is to abstract the complexity of a bus write.
//       - Why is it used: Abstraction is key to productivity. It allows the main test sequence to be written as a clean, high-level
//         script of commands (e.g., "write this data to this address") rather than being cluttered with repetitive, low-level signal
//         assignments.
//
//   3.  Cycle-Accurate Timing and Control:
//       - What is it: The ability to control and synchronize actions based on discrete clock cycles.
//       - Where is it used: Registering the `run_test` process as an `SC_THREAD` and making it sensitive to `clk.pos()` is a deliberate
//         design choice to enable cycle-accurate control.
//       - Why is it used: This allows the testbench to generate stimulus with precise timing (e.g., holding a signal high for exactly
//         one clock cycle), which is essential for testing synchronous digital logic correctly.
//


//
// -------------------------------------------------------------------------------------------------------------------------------
// III. IMPLEMENTATION ETHIC AND INTEGRATION STRATEGY
// -------------------------------------------------------------------------------------------------------------------------------
//
// How I Started:
// My goal was to create a complete, closed-loop verification environment, not just an isolated piece of logic. This led me to the
// PMU-to-PLL scenario, as it's a perfect, self-contained example of a controller-device relationship common in all SoCs.
//
// My Coding Ethic and Process:
// I adopted a "test-plan-first" mentality for the testbench, which is a core tenet of professional verification engineering.
//   1.  Define the Test Plan: Before writing any implementation code, I outlined the exact test sequence in plain English. This plan
//       (reset, configure, check) became the direct skeleton for the `run_test` function.
//   2.  Identify Reusable Logic: I recognized that the bus write sequence would be performed multiple times. To follow the D.R.Y.
//       (Don't Repeat Yourself) principle, I decided from the start to encapsulate this logic into the `write_to_pll` helper function,
//       which I declared here in the header. This makes the final test code cleaner and much easier to modify.
//   3.  Interface-First Design: This header file itself is a product of this ethic. I defined all the necessary ports and internal
//       functions for the testbench *before* implementing them, ensuring I had a clear plan for the module's structure.
//
// How this File Integrates with the System:
// This header is included by `pmu_tb.cpp` (which implements its declarations) and by `main.cpp`. The `main.cpp` file uses the
// port declarations made here to correctly wire up the testbench instance to the top-level system signals, thereby connecting it
// to the PLL.
//
// How I Ensured it Works Correctly:
// The verification of a testbench header is a meta-process. Its correctness is proven when the testbench, as a whole, functions
// as intended.
//   1.  Compile-Time Checks: The project compiling without errors confirmed that the function and port declarations in this file were
//       syntactically correct and matched the implementation.
//   2.  Stimulus Verification: The ultimate proof came from the VCD waveform. By inspecting the waveform, I could visually confirm that
//       the testbench was driving the bus signals with the correct values and timing, exactly as intended by the logic I declared here
//       and implemented in `pmu_tb.cpp`.
//   3.  Checker Verification: I performed a crucial "sanity check" by intentionally introducing a bug into the `pll.cpp` file (e.g.,
//       disabling the `locked.write(true)` line). I then re-ran the simulation and confirmed that my testbench correctly timed out and
//       reported a `❌ FAILED!` message. This proved that the testbench's monitoring and checking logic, structured by this header,
//       was working correctly.
//



//
// -------------------------------------------------------------------------------------------------------------------------------
// IV. INDUSTRIAL RELEVANCE AND PRACTICAL APPLICATIONS
// -------------------------------------------------------------------------------------------------------------------------------
//
// Where this concept is used in the VLSI Industry:
// The structure defined in this file is the foundation of modern digital verification. Every semiconductor company employs large teams
// of verification engineers to build sophisticated testbenches based on these exact principles. While I used C++/SystemC, the concepts
// directly map to industry-standard frameworks like SystemVerilog/UVM, where `pmu_tb` would be analogous to a UVM `test` or `sequence`
// and `write_to_pll` would be part of a UVM `driver`.
//
// Practical Applications:
//
//   1.  **Regression Testing:** A real project would have dozens of testbenches like this, each testing a different feature. These are
//       run automatically every night in a "regression suite." A testbench that is structured, self-checking, and robust (as defined
//       by this header) is the key enabling component for this automated process, allowing teams to quickly catch bugs introduced
//       by new code.
//
//   2.  **Bug Reproduction:** When a complex random test finds a bug, a verification engineer's job is to create a small, targeted
//       "directed test" that can reliably reproduce the failure. The structure I've defined here is perfect for creating such a test,
//       which can then be handed to a designer for efficient debugging.
//
//   3.  **Coverage-Driven Verification:** The `PllConfig` struct I declared hints at a more advanced application. In coverage-driven
//       verification, testbenches are designed to randomly generate legal stimulus to explore the design's state space and find
//       unexpected corner-case bugs. A well-structured testbench is essential for managing this complexity.
//
// Industrially Relevant Insights:
//
//   - Technical Insight: A key insight is that a testbench must be more robust than the DUT it tests. The logic implemented based on
//     this header will include a timeout mechanism. This reflects a core principle: a verification environment cannot assume the DUT
//     will work; it must anticipate failures and handle them gracefully without getting stuck.
//   - Non-Technical Insight: This project highlights the economic imperative of pre-silicon verification. Building a testbench costs
//     engineering time, but finding a bug with it is vastly cheaper than finding that same bug after the chip has been manufactured, which
//     can cost millions in silicon re-spins and product delays. The testbench is the primary tool for mitigating this enormous financial risk.
//
// -------------------------------------------------------------------------------------------------------------------------------
// V. DEVELOPMENT AND EXECUTION ENVIRONMENT
// -------------------------------------------------------------------------------------------------------------------------------
//
//   - Language: C++ (using the C++17 standard)
//   - Core Library (EDA Tool): SystemC 3.0.1 from Accellera. I built this library from source to create my simulation environment.
//   - Editor/IDE: Visual Studio Code. I used it for its lightweight interface and powerful integrated terminal.
//   - Compiler/Simulator: G++ (MinGW-w64). This C++ compiler, combined with the linked SystemC library, forms the complete simulator.
//   - Build System: GNU Make. I created a custom `Makefile` to fully automate the compile-link-run-clean workflow.
//   - Waveform Viewer: GTKWave, for visual analysis of the `waveform.vcd` output.
//















