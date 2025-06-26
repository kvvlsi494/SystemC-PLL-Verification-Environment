
//
// File: pll.h
//
// Project: C++/SystemC High-Level Model of a PLL Configuration
//
// Author: Kumar Vedang
//
// Description:
// This header file acts as the public "datasheet" or "blueprint" for the Phase-Locked Loop (PLL) module. Its primary role is to define the
// *interface* and *structure* of the PLL component, not its internal behavior (which is implemented in `pll.cpp`).
//
// In a typical VLSI project workflow, this file serves as the "contract" between different teams or components:
//   - To the System Integrator (`main.cpp`): It shows what ports are available for connection (e.g., `clk`, `reset`, bus ports).
//   - To the RTL Implementer (`pll.cpp`): It defines the class structure, ports, and internal variables that need to be implemented.
//   - To the Verification Engineer (`pmu_tb.h`): It provides the register address map and port names needed to control and monitor the DUT.
//
// By separating the declaration (in this .h file) from the implementation (in the .cpp file), we follow a fundamental principle of C++
// that promotes modularity, readability, and parallel development.
//

// What is it: This is a C++ preprocessor directive known as an "include guard". '#ifndef' stands for "if not defined".
// Why is it used: This is a standard and critical C++ mechanism to prevent the contents of this header file from being included more than
//               once within a single source file compilation. If a file like `main.cpp` included both `pll.h` and `pmu_tb.h`, and `pmu_tb.h`
//               also included `pll.h`, without this guard, the compiler would see the definition of the `pll` class twice, leading to a
//               "redeclaration" error. This guard ensures the code between `#ifndef` and `#endif` is processed only the first time it is encountered.

#ifndef PLL_H





// What is it: This C++ preprocessor directive defines the macro `PLL_H`.
// How it works: The first time the compiler sees this file, `PLL_H` is not yet defined, so the `#ifndef` condition is true. The compiler then
//               immediately processes this line, which defines `PLL_H`. On any subsequent attempt to include this file within the same
//               compilation unit, the `#ifndef PLL_H` check will be false, and the compiler will skip directly to the `#endif` line,
//               thus avoiding the redefinition error. The name `PLL_H` is a convention, typically matching the filename.

#define PLL_H






// What is it: This is a C++ preprocessor directive that includes the master header file for the Accellera SystemC library.
// Role: It provides all the core functionalities needed for hardware simulation. This includes the simulation kernel, the definition
//       of modules (SC_MODULE), signals (sc_signal), ports (sc_in, sc_out), data types (sc_uint), and the concept of time (sc_time).
// Purpose: This inclusion is fundamental. Without it, the compiler would not understand any of the SystemC-specific keywords used in this
//          file, such as `SC_MODULE`, `sc_in`, `sc_out`, `sc_uint`, `sc_event`, or `SC_CTOR`. It is the entry point to the entire
//          SystemC feature set.

#include <systemc.h>




// What is it: These are C++ preprocessor directives that define macros. A macro is a fragment of code which has been given a name.
//             Whenever the name is used, it is replaced by the contents of the macro. Here, I'm using them to create symbolic, human-readable
//             names for the memory-mapped register addresses of our PLL.
// Why is it used: This is a critical design practice for maintainability and readability. Instead of scattering "magic numbers" like 0x00, 0x04,
//               etc., throughout the project (in both `pll.cpp` and `pmu_tb.cpp`), I have centralized them here. If the memory map ever
//               needs to be changed, I only have to edit these four lines, and the change will automatically propagate throughout the entire
//               project upon recompilation. This dramatically reduces the risk of errors.
// How it impacts execution: Before the C++ compiler even begins its work, the preprocessor scans the code and performs a direct text replacement.
//                          Every instance of `PLL_REG_N_ADDR` will be replaced with `0x00`, and so on.

// Defines the address for the 'N' divider register. The testbench will write to this address to set the 'N' value.
#define PLL_REG_N_ADDR    0x00

// Defines the address for the 'M' (multiplier) feedback divider register.
#define PLL_REG_M_ADDR    0x04

// Defines the address for the 'OD' (output divider) register.
#define PLL_REG_OD_ADDR   0x08

// Defines the address for the main control register, which is used to enable or disable the PLL's locking sequence.
#define PLL_REG_CTRL_ADDR 0x0C






//================================================================================================================================
// OOP Concept: Class Declaration and Inheritance
//================================================================================================================================
// What is it: This line declares the beginning of our PLL module. `SC_MODULE` is a special SystemC macro that simplifies the creation
//             of a hardware module. Underneath, it expands into a standard C++ class declaration.
//
//   - C++ Class: A class is a fundamental concept in Object-Oriented Programming (OOP). It serves as a blueprint for creating objects.
//                In this project, `pll` is the name of our class, and it will contain all the data (ports, internal registers) and
//                behaviors (processes) that define what a PLL is and does.
//
//   - SystemC `sc_module`: In SystemC, every hardware block is modeled as a class that inherits from the base `sc_module` class.
//                        The `SC_MODULE(pll)` macro is a convenient shorthand for `class pll : public sc_module`.
//
//   - Inheritance: This is another core OOP concept where a new class (our `pll`) derives properties and behaviors from an existing
//                  class (`sc_module`). By inheriting from `sc_module`, our `pll` class automatically gets all the necessary machinery
//                  to have ports, contain processes, and participate in the SystemC event-driven simulation.
//
// Why is it used: This is the standard way to define a hardware component in SystemC. It encapsulates all related hardware properties
//               (ports, registers, logic) into a single, reusable software object. The `main.cpp` file will later create an *instance*
//               (a specific object) of this `pll` class to include it in the simulation.

SC_MODULE(pll) {



    //================================================================================================================================
    // Hardware Abstraction: Input Port Declarations
    //================================================================================================================================
    // The following lines declare the input ports of the PLL module. In SystemC, an `sc_in` is a special object that models a physical
    // input pin on a hardware chip. It defines a channel through which the module can read values from signals connected to it by the
    // top-level `main.cpp` file. The data type specified inside the angle brackets `<...>` dictates the kind of data the pin expects.

    // What is it: This declares an input port named 'clk' that can receive a boolean value (`bool`).
    // Data Type: `bool` is a fundamental C++ data type representing true/false. In digital design, this is the perfect software
    //            representation for a single-bit signal like a clock, which toggles between a logic '1' (true) and '0' (false).
    // Purpose: This will be the main system clock input for the PLL. All synchronous operations within this module, such as sampling
    //          the bus values, will be timed relative to the rising edge of the signal connected to this port.


    sc_in<bool> clk;


    // What is it: This declares an input port named 'reset' that also receives a boolean value.
    // Purpose: This port will be connected to the global system reset line. In my design, this reset is "active-high". When the signal
    //          connected to this port is high (true), it will trigger the PLL's internal reset logic, forcing all its state registers
    //          into a known, default state. This is a critical pin for system initialization and error recovery.

    sc_in<bool> reset;




    // What is it: This declares an input port named 'bus_addr' that can receive a 32-bit unsigned integer (`sc_uint<32>`).
    // Data Type: `sc_uint<N>` is a special data type provided by the SystemC library specifically for modeling fixed-width hardware
    //            registers and buses. Using `<32>` ensures that this port accurately models a standard 3_2-bit address bus found in many
    //            microprocessor systems.
    // Purpose: This port receives the memory address of the internal PLL register that the testbench (PMU) wants to access. The `bus_process`
    //          logic inside `pll.cpp` will read the value from this port to determine which internal register to target for a write operation.


    sc_in<sc_uint<32>> bus_addr;



    // What is it: This declares an input port named 'bus_wdata' for the 32-bit write data bus, also of type `sc_uint<32>`.
    // Purpose: This port receives the 32-bit data value that the PMU wants to write into the register specified by the `bus_addr` port.
    //          It works in tandem with the address bus to perform a complete register write.


    sc_in<sc_uint<32>> bus_wdata;


    // What is it: This declares a single-bit boolean input port named 'bus_we' which stands for "Write Enable".
    // Purpose: This is the primary control signal for my simple bus protocol. The PLL's bus logic will only consider the values on
    //          `bus_addr` and `bus_wdata` to be valid and will only perform a write operation on a clock edge when this signal is
    //          asserted high (true). This prevents the PLL from accidentally latching invalid data when the bus master is not actively
    //          writing to it.
    sc_in<bool>          bus_we;


    //================================================================================================================================
    // Hardware Abstraction: Output Port Declaration
    //================================================================================================================================
    // The following line declares the output port of the PLL module. In SystemC, an `sc_out` is a special object that models a physical
    // output pin on a chip. It provides a channel for the module to *drive* or *write* a value onto a signal, which can then be read
    // by other modules connected to that same signal.

    // What is it: This declares an output port named 'locked' that drives a boolean (`bool`) value.
    // Data Type: The `bool` type is used to model a single-bit status signal. It will be driven to `true` (logic '1') when the PLL
    //            has successfully locked to the target frequency, and `false` (logic '0') otherwise.
    // Purpose: This is the primary status output of the PLL. The PMU testbench will monitor the signal connected to this port to verify
    //          that the DUT has completed its task successfully. The `locking_process` inside `pll.cpp` is responsible for controlling
    //          the value that is written to this port. In a real system, this signal would be used by other parts of the SoC to know
    //          when it is safe to use the clock generated by the PLL.


    sc_out<bool> locked;



//================================================================================================================================
// OOP Concept: Data Encapsulation
//================================================================================================================================
// What is it: This is a standard C++ access specifier. The `private:` keyword dictates that all member variables and functions
//             declared after this point (until a `public:` or `protected:` specifier is encountered) are only accessible from
//             within the `pll` class itself (i.e., from the code inside `pll.cpp`). They are completely hidden from the outside
//             world, including `main.cpp` and the `pmu_tb` module.
// Why is it used: This is the core principle of **Data Encapsulation**, a fundamental pillar of Object-Oriented Programming. By
//               making the internal state of the PLL private, I am enforcing a clean and robust design. The only way to interact
//               with the PLL's internal state is through its well-defined public interface (the bus ports). This prevents external
//               modules from accidentally or intentionally corrupting the PLL's state, making the overall system more reliable and
//               easier to debug.

private:

    
    // What is it: These lines declare three member variables of type `sc_uint<8>` to model the internal hardware registers of the PLL.
    // Data Type: I chose `sc_uint<8>` (an 8-bit unsigned integer) because PLL divider values typically do not require a full 32 bits.
    //            This is a form of micro-optimization; it more accurately reflects the likely size of the physical registers and uses
    //            slightly less memory in the simulation compared to using a full `int` or `sc_uint<32>`.
    // Purpose: These variables will store the configuration values (M, N, and OD dividers) that are written by the testbench via the
    //          bus. The `bus_process` in `pll.cpp` will write to these variables, and the `locking_process` will use their values
    //          to calculate the final output frequency for a logging message. They represent the persistent state of the module.

    sc_uint<8> reg_m, reg_n, reg_od;



    // What is it: This declares a standard C++ boolean member variable named `pll_enable`.
    // Purpose: This variable acts as an internal state flag. It is set to `true` when the testbench writes a '1' to the control
    //          register (`PLL_REG_CTRL_ADDR`). It serves as a crucial link between the two concurrent processes: the fast `bus_process`
    //          sets this flag, and the slow `locking_process` checks it to know when to start its timed sequence and whether to
    //          assert the `locked` output after the lock time has elapsed.
    bool       pll_enable;



    //================================================================================================================================
    // SystemC Concept: Inter-Process Synchronization
    //================================================================================================================================

    // What is it: This declares a member variable of type `sc_event`. An `sc_event` is a fundamental SystemC object used for
    //             synchronizing concurrent processes. It acts like a software-only notification flag that has no direct hardware
    //             equivalent but is essential for creating efficient high-level models.
    // Purpose: In my design, this event serves as the primary communication channel between the fast `bus_process` and the slow
    //          `locking_process`. When the `bus_process` receives the command to enable the PLL, it will `.notify()` this event.
    //          The `locking_process`, which is sensitive to this event, will wake up from its suspended state and begin the
    //          time-consuming lock sequence. This elegant mechanism decouples the two processes, allowing the bus interface to
    //          remain responsive while the slow analog behavior happens in the background.

    sc_event   start_locking_event;



     // What is it: These lines declare two private member functions of the `pll` class. In C++, a member function (or method)
    //             defines a behavior or operation that an object of the class can perform. These are just the declarations; the
    //             actual implementation (the code that defines what they do) is located in the corresponding `pll.cpp` file.
    // Purpose: These functions contain the core behavioral logic of the PLL. They will be registered with the SystemC kernel as
    //          concurrent processes that run in parallel during the simulation.

    // This declares the function that will model the reactive, digital bus interface logic. It will be registered as an `SC_METHOD`.

    void bus_process();


    // This declares the function that will model the stateful, time-consuming analog locking behavior. It will be registered as an `SC_THREAD`.
    void locking_process();



// What is it: This is a C++ access specifier. The `public:` keyword means that all members declared after this point are
//             accessible from outside the `pll` class. In SystemC, the constructor and ports are typically public.
public:




    //================================================================================================================================
    // SystemC Concept: The Constructor (`SC_CTOR`)
    //================================================================================================================================
    // What is it: This is the constructor for the `pll` module. A constructor is a special member function in C++ that is
    //             automatically called when an object of the class is created (instantiated). `SC_CTOR` is a SystemC macro that
    //             simplifies the declaration of a module's constructor. It is equivalent to writing `pll(sc_module_name name) : sc_module(name)`.
    // Role: The constructor's primary role in SystemC is to perform one-time setup and initialization for the module *before* the
    //       simulation starts. This includes:
    //         1. Initializing internal member variables.
    //         2. Registering the module's behavioral processes (like `bus_process` and `locking_process`) with the simulation kernel.
    //         3. Defining the sensitivity lists for those processes.
    // How it impacts execution: The code inside the constructor runs only once during the "elaboration" phase of the simulation, when the
    //                         `pll` object is created in `main.cpp`. It sets up the static structure and behavior of the module.

    SC_CTOR(pll) {


        // This is a simple C++ `cout` statement that prints a message to the console when the constructor is called. It's a useful
        // debugging technique to confirm that the module instance has been successfully created by the simulator.
        cout << "PLL module constructed." << endl;


        //================================================================================================================================
        // Digital Design Concept: State Initialization
        //================================================================================================================================
        // What is it: This line initializes the `pll_enable` boolean member variable to `false`.
        // Why is it used: This is a critically important step in robust digital design and C++ programming. In C++, if a member
        //               variable is not explicitly initialized in the constructor, its initial value is undefined (it could contain
        //               random "garbage" data from whatever was in that memory location previously). In a hardware model, an
        //               uninitialized state flag can lead to unpredictable, non-deterministic behavior that is very difficult to
        //               debug. By explicitly setting `pll_enable` to `false` here, I am ensuring that the PLL module always starts
        //               in a known, predictable "off" state from simulation time zero.

        pll_enable = false;



        //================================================================================================================================
        // SystemC Concept: Process Registration (`SC_METHOD`)
        //================================================================================================================================
        // What is it: This line registers the `bus_process` member function with the SystemC simulation kernel as an `SC_METHOD` process.
        // What is an SC_METHOD: An `SC_METHOD` is a type of SystemC process used to model reactive or combinational logic. Its key
        //                      characteristics are:
        //                        - It executes instantaneously (in zero simulation time) whenever an event occurs on its sensitivity list.
        //                        - It cannot be suspended with timed `wait()` statements. It runs to completion every time it's triggered.
        // Why is it used here: I chose `SC_METHOD` for the `bus_process` because it perfectly models the behavior of a digital register
        //                    file. The register write logic should be fast and reactive, responding immediately to a clock edge
        //                    when the write enable signal is active. It does not need to model the passage of time itself.
        SC_METHOD(bus_process);





        //================================================================================================================================
        // SystemC Concept: The Sensitivity List
        //================================================================================================================================
        // What is it: This line defines the "sensitivity list" for the `bus_process` that was registered just above. The `sensitive`
        //             keyword tells the SystemC kernel which events should trigger the execution of this specific process.
        // How it works: The SystemC scheduler continuously monitors all signals and events. When it detects an event that is in a
        //               process's sensitivity list, it schedules that process to run.
        // Breakdown of this specific list:
        //   - `<< clk.pos()`: This makes the `bus_process` sensitive to the *positive edge* (a transition from 0 to 1) of the signal
        //                    connected to the `clk` port. This is the standard way to model synchronous digital logic that only
        //                    updates its state on a rising clock edge.
        //   - `<< reset`: This makes the `bus_process` sensitive to *any change* on the signal connected to the `reset` port. This is
        //                important for modeling an asynchronous reset behavior at the process level. Even though my reset logic inside
        //                the process is synchronous (checked on a clock edge), making the process itself sensitive to any change on
        //                `reset` ensures it wakes up promptly to evaluate the reset condition.
        // Why is it used: The sensitivity list is fundamental to event-driven simulation. It prevents the simulator from having to
        //               re-evaluate every process at every time step. Instead, a process only consumes CPU resources when one of its
        //               specific trigger events occurs, making the simulation highly efficient.
        sensitive << clk.pos() << reset;





        //================================================================================================================================
        // SystemC Concept: Process Registration (`SC_THREAD`)
        //================================================================================================================================
        // What is it: This line registers the `locking_process` member function with the SystemC kernel as an `SC_THREAD` process.
        // What is an SC_THREAD: An `SC_THREAD` is another type of SystemC process, used to model sequential or stateful logic that
        //                      takes a non-zero amount of simulation time to complete. Its key characteristics are:
        //                        - It can be suspended and resumed using `wait()` statements. This is its most important feature.
        //                        - It typically runs within an infinite loop (`while(true)`) to model hardware that is "always on".
        // Why is it used here: I chose `SC_THREAD` for the `locking_process` because it is the only process type that can model the
        //                    passage of time. The physical, analog behavior of a PLL achieving lock is not instantaneous; it takes a
        //                    specific duration (e.g., 500 nanoseconds). An `SC_THREAD` allows me to use a timed `wait(500, SC_NS)`
        //                    statement to accurately model this real-world performance characteristic, which is essential for any
        //                    meaningful system-level performance analysis. An `SC_METHOD` cannot do this.
        SC_THREAD(locking_process);




        //================================================================================================================================
        // SystemC Concept: The Sensitivity List (for SC_THREAD)
        //================================================================================================================================
        // What is it: This line defines the sensitivity list for the `locking_process` thread. Similar to the `SC_METHOD`, this tells
        //             the SystemC kernel which events should wake up this process from a suspended state.
        // How it works: Inside the `locking_process` function (in `pll.cpp`), there is a `wait()` statement inside an infinite loop.
        //               The process will pause at that `wait()` statement indefinitely until one of the events specified here occurs.
        // Breakdown of this specific list:
        //   - `<< reset`: This makes the `locking_process` sensitive to *any change* on the reset signal. This is critical for modeling
        //                a high-priority, asynchronous reset. If the reset signal is asserted at any time, even while the process is in
        //                the middle of its timed 500ns wait, this sensitivity ensures the process can be "woken up" to handle the reset
        //                condition immediately.
        //   - `<< start_locking_event`: This makes the process sensitive to the notification of our custom `sc_event`. This is the
        //                             primary trigger for the normal operation. When the `bus_process` notifies this event, the
        //                             `locking_process` will wake up from its `wait()` statement and begin the locking sequence.
        // Why is it used: This sensitivity list defines the specific triggers that can start or interrupt the `locking_process`. It
        //               ensures the process is responsive to both normal control flow (the event) and high-priority interrupts (the reset).
        sensitive << reset << start_locking_event;
    }
    
};// The closing curly brace for the class definition will be at the end of the file.



// What is it: This preprocessor directive marks the end of the conditional block started by `#ifndef`. All code between `#ifndef` and `#endif`
//             is subject to the include guard logic.
#endif






//================================================================================================================================
//================================================================================================================================
//
//                               DETAILED PROJECT DOCUMENTATION AND ANALYSIS
//
//================================================================================================================================
//================================================================================================================================
//
// -------------------------------------------------------------------------------------------------------------------------------
// I. RELEVANCE OF THIS CODE FILE (pll.h)
// -------------------------------------------------------------------------------------------------------------------------------
//
// This `pll.h` header file, while containing no active logic, is one of the most structurally important files in the project. Its
// relevance is that it serves as the formal "contract" or "datasheet" for the PLL module. In the collaborative world of VLSI design,
// where different engineers work on different parts of a system, having a well-defined interface is paramount.
//
// This file helps by:
//   - Defining the Public Interface: It explicitly declares all the input and output "pins" (the SystemC ports) that the PLL module
//     exposes to the outside world. This allows a system integrator, working in `main.cpp`, to correctly connect the PLL without needing
//     to know anything about its internal workings.
//   - Centralizing Configuration: By defining the register address map (`#define PLL_REG_...`) in this single location, it provides a
//     single source of truth for communication. The verification engineer writing the testbench (`pmu_tb`) can include this file to
//     use symbolic, readable names for addresses, making the test code robust and easy to maintain.
//   - Enabling Parallel Development: By separating the interface declaration (.h) from the behavioral implementation (`pll.cpp`), this
//     file allows different tasks to happen concurrently. An RTL designer can start implementing the PLL's logic based on this contract,
//     while a verification engineer can simultaneously start writing a testbench that targets this same contract.
//
// -------------------------------------------------------------------------------------------------------------------------------
// II. MAIN CONCEPTS IMPLEMENTED AND UNDERSTOOD
// -------------------------------------------------------------------------------------------------------------------------------
//
// This file is a practical demonstration of several core software engineering and hardware modeling concepts:
//
//   1.  Hardware Abstraction:
//       - What is it: The process of modeling physical hardware components and concepts using software constructs.
//       - Where is it used: The `sc_in<...>` and `sc_out<...>` port declarations are a direct abstraction of the physical input/output
//         pins on an integrated circuit. The `sc_uint<32>` data type is an abstraction of a 32-bit wide hardware bus.
//       - Why is it used: This is the fundamental principle of SystemC. It allows us to build a "virtual prototype" of a hardware
//         system in software, enabling simulation and verification long before any physical hardware is manufactured.
//
//   2.  Class-Based Modeling and Inheritance (Object-Oriented Programming):
//       - What is it: Using C++ classes to represent self-contained objects. Inheritance allows a new class to acquire the properties
//         of an existing one.
//       - Where is it used: The `SC_MODULE(pll)` macro declares a C++ class named `pll` that inherits from the base `sc_module` class.
//       - Why is it used: This encapsulates all the data (ports, registers) and behaviors (processes) of the PLL into a single, cohesive
//         unit. Inheritance from `sc_module` gives our `pll` class the built-in capabilities to be part of a SystemC simulation.
//
//   3.  Data Encapsulation (Object-Oriented Programming):
//       - What is it: The practice of bundling data and the methods that operate on that data together, and restricting direct access
//         to some of the object's components.
//       - Where is it used: The `private:` access specifier hides the internal state variables (`reg_m`, `pll_enable`, etc.) and process
//         functions from the outside world.
//       - Why is it used: This enforces a robust design. Other modules cannot accidentally corrupt the PLL's internal state. They are
//         forced to interact with the PLL only through its well-defined public interface (its ports), making the system more predictable
//         and easier to debug.
//
//   4.  Event-Driven Simulation Principles:
//       - What is it: A simulation paradigm where the flow of the program is determined by events (e.g., a clock edge, a signal change).
//       - Where is it used: The registration of processes (`SC_METHOD`, `SC_THREAD`) and the definition of their `sensitive` lists in the
//         constructor (`SC_CTOR`) establish the event-driven nature of this module.
//       - Why is it used: This is how hardware works and is vastly more efficient for simulation than a time-stepped approach. A process
//         only executes when one of its specific trigger events occurs, saving enormous amounts of computation.
//





//
// -------------------------------------------------------------------------------------------------------------------------------
// I. RELEVANCE OF THIS CODE FILE (pll.h)
// -------------------------------------------------------------------------------------------------------------------------------
//
// This `pll.h` header file, while containing no active logic, is one of the most structurally important files in the project. Its
// relevance is that it serves as the formal "contract" or "datasheet" for the PLL module. In the collaborative world of VLSI design,
// where different engineers work on different parts of a system, having a well-defined interface is paramount.
//
// This file helps by:
//   - Defining the Public Interface: It explicitly declares all the input and output "pins" (the SystemC ports) that the PLL module
//     exposes to the outside world. This allows a system integrator, working in `main.cpp`, to correctly connect the PLL without needing
//     to know anything about its internal workings.
//   - Centralizing Configuration: By defining the register address map (`#define PLL_REG_...`) in this single location, it provides a
//     single source of truth for communication. The verification engineer writing the testbench (`pmu_tb`) can include this file to
//     use symbolic, readable names for addresses, making the test code robust and easy to maintain.
//   - Enabling Parallel Development: By separating the interface declaration (.h) from the behavioral implementation (`pll.cpp`), this
//     file allows different tasks to happen concurrently. An RTL designer can start implementing the PLL's logic based on this contract,
//     while a verification engineer can simultaneously start writing a testbench that targets this same contract.
//
// -------------------------------------------------------------------------------------------------------------------------------
// II. MAIN CONCEPTS IMPLEMENTED AND UNDERSTOOD
// -------------------------------------------------------------------------------------------------------------------------------
//
// This file is a practical demonstration of several core software engineering and hardware modeling concepts:
//
//   1.  Hardware Abstraction:
//       - What is it: The process of modeling physical hardware components and concepts using software constructs.
//       - Where is it used: The `sc_in<...>` and `sc_out<...>` port declarations are a direct abstraction of the physical input/output
//         pins on an integrated circuit. The `sc_uint<32>` data type is an abstraction of a 32-bit wide hardware bus.
//       - Why is it used: This is the fundamental principle of SystemC. It allows us to build a "virtual prototype" of a hardware
//         system in software, enabling simulation and verification long before any physical hardware is manufactured.
//
//   2.  Class-Based Modeling and Inheritance (Object-Oriented Programming):
//       - What is it: Using C++ classes to represent self-contained objects. Inheritance allows a new class to acquire the properties
//         of an existing one.
//       - Where is it used: The `SC_MODULE(pll)` macro declares a C++ class named `pll` that inherits from the base `sc_module` class.
//       - Why is it used: This encapsulates all the data (ports, registers) and behaviors (processes) of the PLL into a single, cohesive
//         unit. Inheritance from `sc_module` gives our `pll` class the built-in capabilities to be part of a SystemC simulation.
//
//   3.  Data Encapsulation (Object-Oriented Programming):
//       - What is it: The practice of bundling data and the methods that operate on that data together, and restricting direct access
//         to some of the object's components.
//       - Where is it used: The `private:` access specifier hides the internal state variables (`reg_m`, `pll_enable`, etc.) and process
//         functions from the outside world.
//       - Why is it used: This enforces a robust design. Other modules cannot accidentally corrupt the PLL's internal state. They are
//         forced to interact with the PLL only through its well-defined public interface (its ports), making the system more predictable
//         and easier to debug.
//
//   4.  Event-Driven Simulation Principles:
//       - What is it: A simulation paradigm where the flow of the program is determined by events (e.g., a clock edge, a signal change).
//       - Where is it used: The registration of processes (`SC_METHOD`, `SC_THREAD`) and the definition of their `sensitive` lists in the
//         constructor (`SC_CTOR`) establish the event-driven nature of this module.
//       - Why is it used: This is how hardware works and is vastly more efficient for simulation than a time-stepped approach. A process
//         only executes when one of its specific trigger events occurs, saving enormous amounts of computation.
//
// -------------------------------------------------------------------------------------------------------------------------------
// III. IMPLEMENTATION ETHIC AND INTEGRATION STRATEGY
// -------------------------------------------------------------------------------------------------------------------------------
//
// How I Started:
// The idea for this project came from a desire to model a real-world, fundamental piece of an SoC. A PLL is a ubiquitous mixed-signal
// block, and modeling its digital control interface is a classic problem perfectly suited for SystemC. I wanted to build a complete
// system, not just an isolated block, so the interaction between a PMU and a PLL was the natural choice.
//
// My Coding Ethic and Process:
// I approached this project with an "interface-first" design philosophy, which is a disciplined and professional way to build complex systems.
//   1.  Outlining and Interface Definition: Before writing a single line of logic in `pll.cpp` or `pmu_tb.cpp`, I created this `pll.h`
//       file. I carefully thought about the "contract" for the PLL module. What "pins" does it need to interact with the outside world?
//       I decided it needed a clock, a reset, a simple bus interface (address, data, write enable), and a status output (`locked`).
//       This defined the public interface. I did the same for the `pmu_tb.h` file.
//   2.  Centralized Definitions: I made a conscious decision to place the register address map here using `#define`. This follows the
//       D.R.Y. (Don't Repeat Yourself) principle. This ensured consistency and made the project far easier to maintain.
//   3.  Clear Naming Conventions: I used clear and descriptive names for all ports and internal variables (e.g., `bus_we` for Write
//       Enable, `start_locking_event`). This makes the code largely self-documenting and easier to understand for anyone reading it.
//
// How this File Integrates the System:
// This header file is the lynchpin for the PLL component. It's integrated via the C++ `#include` directive:
//   - `pll.cpp` includes it to get the class definition it needs to implement.
//   - `pmu_tb.h` includes it to get the register address definitions, enabling it to "talk" to the PLL.
//   - `main.cpp` includes it (indirectly through `pll.h` or `pmu_tb.h`) to know how to instantiate the `pll` object and what ports
//     are available for connection.
//
// How I Ensured it Works Correctly:
// The correctness of a header file is primarily validated at compile-time and link-time.
//   1.  Successful Compilation: The first and most important check was that the entire project compiled without errors. A "redeclaration"
//       error would have indicated a problem with my include guards. A "member not found" error would have pointed to a typo or a
//       missing declaration in this file.
//   2.  Linker Success: The fact that the project linked successfully proved that the function declarations I made here (like `bus_process`)
//       correctly matched their implementations in `pll.cpp`.
//   3.  Functional Verification (Indirect): The ultimate proof came from the successful run of the full simulation. The fact that the
//       testbench could write to the correct register addresses and that the PLL responded appropriately proved that the "contract"
//       defined in this header file was correct and honored by all components.
//





//
//                               DETAILED PROJECT DOCUMENTATION AND ANALYSIS
//
//================================================================================================================================
//================================================================================================================================
//
// -------------------------------------------------------------------------------------------------------------------------------
// I. RELEVANCE OF THIS CODE FILE (pll.h)
// -------------------------------------------------------------------------------------------------------------------------------
//
// This `pll.h` header file, while containing no active logic, is one of the most structurally important files in the project. Its
// relevance is that it serves as the formal "contract" or "datasheet" for the PLL module. In the collaborative world of VLSI design,
// where different engineers work on different parts of a system, having a well-defined interface is paramount.
//
// This file helps by:
//   - Defining the Public Interface: It explicitly declares all the input and output "pins" (the SystemC ports) that the PLL module
//     exposes to the outside world. This allows a system integrator, working in `main.cpp`, to correctly connect the PLL without needing
//     to know anything about its internal workings.
//   - Centralizing Configuration: By defining the register address map (`#define PLL_REG_...`) in this single location, it provides a
//     single source of truth for communication. The verification engineer writing the testbench (`pmu_tb`) can include this file to
//     use symbolic, readable names for addresses, making the test code robust and easy to maintain.
//   - Enabling Parallel Development: By separating the interface declaration (.h) from the behavioral implementation (`pll.cpp`), this
//     file allows different tasks to happen concurrently. An RTL designer can start implementing the PLL's logic based on this contract,
//     while a verification engineer can simultaneously start writing a testbench that targets this same contract.
//
// -------------------------------------------------------------------------------------------------------------------------------
// II. MAIN CONCEPTS IMPLEMENTED AND UNDERSTOOD
// -------------------------------------------------------------------------------------------------------------------------------
//
// This file is a practical demonstration of several core software engineering and hardware modeling concepts:
//
//   1.  Hardware Abstraction:
//       - What is it: The process of modeling physical hardware components and concepts using software constructs.
//       - Where is it used: The `sc_in<...>` and `sc_out<...>` port declarations are a direct abstraction of the physical input/output
//         pins on an integrated circuit. The `sc_uint<32>` data type is an abstraction of a 32-bit wide hardware bus.
//       - Why is it used: This is the fundamental principle of SystemC. It allows us to build a "virtual prototype" of a hardware
//         system in software, enabling simulation and verification long before any physical hardware is manufactured.
//
//   2.  Class-Based Modeling and Inheritance (Object-Oriented Programming):
//       - What is it: Using C++ classes to represent self-contained objects. Inheritance allows a new class to acquire the properties
//         of an existing one.
//       - Where is it used: The `SC_MODULE(pll)` macro declares a C++ class named `pll` that inherits from the base `sc_module` class.
//       - Why is it used: This encapsulates all the data (ports, registers) and behaviors (processes) of the PLL into a single, cohesive
//         unit. Inheritance from `sc_module` gives our `pll` class the built-in capabilities to be part of a SystemC simulation.
//
//   3.  Data Encapsulation (Object-Oriented Programming):
//       - What is it: The practice of bundling data and the methods that operate on that data together, and restricting direct access
//         to some of the object's components.
//       - Where is it used: The `private:` access specifier hides the internal state variables (`reg_m`, `pll_enable`, etc.) and process
//         functions from the outside world.
//       - Why is it used: This enforces a robust design. Other modules cannot accidentally corrupt the PLL's internal state. They are
//         forced to interact with the PLL only through its well-defined public interface (its ports), making the system more predictable
//         and easier to debug.
//
//   4.  Event-Driven Simulation Principles:
//       - What is it: A simulation paradigm where the flow of the program is determined by events (e.g., a clock edge, a signal change).
//       - Where is it used: The registration of processes (`SC_METHOD`, `SC_THREAD`) and the definition of their `sensitive` lists in the
//         constructor (`SC_CTOR`) establish the event-driven nature of this module.
//       - Why is it used: This is how hardware works and is vastly more efficient for simulation than a time-stepped approach. A process
//         only executes when one of its specific trigger events occurs, saving enormous amounts of computation.
//
// -------------------------------------------------------------------------------------------------------------------------------
// III. IMPLEMENTATION ETHIC AND INTEGRATION STRATEGY
// -------------------------------------------------------------------------------------------------------------------------------
//
// How I Started:
// The idea for this project came from a desire to model a real-world, fundamental piece of an SoC. A PLL is a ubiquitous mixed-signal
// block, and modeling its digital control interface is a classic problem perfectly suited for SystemC. I wanted to build a complete
// system, not just an isolated block, so the interaction between a PMU and a PLL was the natural choice.
//
// My Coding Ethic and Process:
// I approached this project with an "interface-first" design philosophy, which is a disciplined and professional way to build complex systems.
//   1.  Outlining and Interface Definition: Before writing a single line of logic in `pll.cpp` or `pmu_tb.cpp`, I created this `pll.h`
//       file. I carefully thought about the "contract" for the PLL module. What "pins" does it need to interact with the outside world?
//       I decided it needed a clock, a reset, a simple bus interface (address, data, write enable), and a status output (`locked`).
//       This defined the public interface. I did the same for the `pmu_tb.h` file.
//   2.  Centralized Definitions: I made a conscious decision to place the register address map here using `#define`. This follows the
//       D.R.Y. (Don't Repeat Yourself) principle. This ensured consistency and made the project far easier to maintain.
//   3.  Clear Naming Conventions: I used clear and descriptive names for all ports and internal variables (e.g., `bus_we` for Write
//       Enable, `start_locking_event`). This makes the code largely self-documenting and easier to understand for anyone reading it.
//
// How this File Integrates the System:
// This header file is the lynchpin for the PLL component. It's integrated via the C++ `#include` directive:
//   - `pll.cpp` includes it to get the class definition it needs to implement.
//   - `pmu_tb.h` includes it to get the register address definitions, enabling it to "talk" to the PLL.
//   - `main.cpp` includes it (indirectly through `pll.h` or `pmu_tb.h`) to know how to instantiate the `pll` object and what ports
//     are available for connection.
//
// How I Ensured it Works Correctly:
// The correctness of a header file is primarily validated at compile-time and link-time.
//   1.  Successful Compilation: The first and most important check was that the entire project compiled without errors. A "redeclaration"
//       error would have indicated a problem with my include guards. A "member not found" error would have pointed to a typo or a
//       missing declaration in this file.
//   2.  Linker Success: The fact that the project linked successfully proved that the function declarations I made here (like `bus_process`)
//       correctly matched their implementations in `pll.cpp`.
//   3.  Functional Verification (Indirect): The ultimate proof came from the successful run of the full simulation. The fact that the
//       testbench could write to the correct register addresses and that the PLL responded appropriately proved that the "contract"
//       defined in this header file was correct and honored by all components.
//
// -------------------------------------------------------------------------------------------------------------------------------
// IV. INDUSTRIAL RELEVANCE AND PRACTICAL APPLICATIONS
// -------------------------------------------------------------------------------------------------------------------------------
//
// Where this concept is used in the VLSI Industry:
// The concepts embodied in this `pll.h` file are fundamental to nearly all large-scale digital design projects. This file is a textbook
// example of an IP (Intellectual Property) block's header. In companies like Intel, NVIDIA, or Qualcomm, teams designing complex SoCs
// work with hundreds of such IP blocks. The header file for each block is the non-negotiable contract that governs how it integrates
// into the larger system.
//
// Practical Applications:
//
//   1.  **IP Integration:** When a company licenses a third-party IP block (e.g., a USB controller), they receive a set of files that
//       always includes a header like this one. It tells their system architects exactly which ports need to be connected to the system bus
//       and which signals (like interrupts or status flags) need to be routed.
//
//   2.  **Architectural Specification:** In the very early stages of chip design, system architects will often write header files like this
//       *before any logic is implemented*. These headers serve as an executable specification. They can be used to compile a "scaffolding"
//       of the chip to ensure the high-level connectivity is sound, long before the detailed behavior of each block is defined.
//
//   3.  **API for Firmware/Driver Development ("Shift-Left"):** The register address definitions in this file form a basic Application
//       Programming Interface (API). A firmware engineer can take these addresses and start writing the C code for the low-level drivers
//       that will eventually run on the chip's CPU to configure the PLL. They can do this in parallel with the hardware design, which
//       dramatically reduces the overall product development time.
//
// Industrially Relevant Insights:
//
//   - Technical Insight: A key insight I gained is that a well-defined interface is a form of abstraction that hides complexity. By
//     interacting with the PLL only through the ports defined here, the rest of the system doesn't need to care about the complex
//     analog behavior happening inside. This separation of concerns is what makes it possible to build and reason about systems with
//     billions of transistors.
//
//   - Non-Technical Insight: This project highlighted the importance of clear communication and standardization in a team environment.
//     This header file is a form of standardized communication. It's an unambiguous document that tells every other engineer on the
//     project, "This is how you talk to my block." Without such formal contracts, large-scale collaborative engineering would be impossible.
//
// -------------------------------------------------------------------------------------------------------------------------------
// V. DEVELOPMENT AND EXECUTION ENVIRONMENT
// -------------------------------------------------------------------------------------------------------------------------------
//
//   - Language: C++ (using the C++17 standard)
//   - Core Library (EDA Tool): SystemC 3.0.1 from Accellera. This is the core library that provides the event-driven simulation kernel
//     and hardware modeling capabilities. I built it from source using the MSYS2 toolchain.
//   - Editor/IDE: Visual Studio Code (VS Code). I chose it because it's lightweight, highly extensible, and has an excellent integrated
//     terminal, which allowed me to perform all build and run tasks within a single window.
//   - VS Code Extensions: I used the "C/C++ Extension Pack" from Microsoft, which provided syntax highlighting, code completion (IntelliSense),
//     and basic error checking that made writing the code much more efficient.
//   - Compiler/Simulator: G++ (GNU C++ Compiler) from the MinGW-w64 toolchain, installed via MSYS2. In this project, the G++ compiler
//     and the linked SystemC library together form the "simulator". G++ compiles the C++ code, and the linked SystemC library provides the
//     runtime kernel that executes the simulation.
//   - Build System: GNU Make. I created a custom `Makefile` to automate the entire workflow.
//   - Waveform Viewer: GTKWave. I used this free, standard tool to visualize the `waveform.vcd` output file, which was essential for debugging.
//




//
// -------------------------------------------------------------------------------------------------------------------------------
// IV. INDUSTRIAL RELEVANCE AND PRACTICAL APPLICATIONS
// -------------------------------------------------------------------------------------------------------------------------------
//
// Where this concept is used in the VLSI Industry:
// The concepts embodied in this `pll.h` file are fundamental to nearly all large-scale digital design projects. This file is a textbook
// example of an IP (Intellectual Property) block's header. In companies like Intel, NVIDIA, or Qualcomm, teams designing complex SoCs
// work with hundreds of such IP blocks. The header file for each block is the non-negotiable contract that governs how it integrates
// into the larger system.
//
// Practical Applications:
//
//   1.  **IP Integration:** When a company licenses a third-party IP block (e.g., a USB controller), they receive a set of files that
//       always includes a header like this one. It tells their system architects exactly which ports need to be connected to the system bus
//       and which signals (like interrupts or status flags) need to be routed.
//
//   2.  **Architectural Specification:** In the very early stages of chip design, system architects will often write header files like this
//       *before any logic is implemented*. These headers serve as an executable specification. They can be used to compile a "scaffolding"
//       of the chip to ensure the high-level connectivity is sound, long before the detailed behavior of each block is defined.
//
//   3.  **API for Firmware/Driver Development ("Shift-Left"):** The register address definitions in this file form a basic Application
//       Programming Interface (API). A firmware engineer can take these addresses and start writing the C code for the low-level drivers
//       that will eventually run on the chip's CPU to configure the PLL. They can do this in parallel with the hardware design, which
//       dramatically reduces the overall product development time.
//
// Industrially Relevant Insights:
//
//   - Technical Insight: A key insight I gained is that a well-defined interface is a form of abstraction that hides complexity. By
//     interacting with the PLL only through the ports defined here, the rest of the system doesn't need to care about the complex
//     analog behavior happening inside. This separation of concerns is what makes it possible to build and reason about systems with
//     billions of transistors.
//
//   - Non-Technical Insight: This project highlighted the importance of clear communication and standardization in a team environment.
//     This header file is a form of standardized communication. It's an unambiguous document that tells every other engineer on the
//     project, "This is how you talk to my block." Without such formal contracts, large-scale collaborative engineering would be impossible.
//
// -------------------------------------------------------------------------------------------------------------------------------
// V. DEVELOPMENT AND EXECUTION ENVIRONMENT
// -------------------------------------------------------------------------------------------------------------------------------
//
//   - Language: C++ (using the C++17 standard)
//   - Core Library (EDA Tool): SystemC 3.0.1 from Accellera. This is the core library that provides the event-driven simulation kernel
//     and hardware modeling capabilities. I built it from source using the MSYS2 toolchain.
//   - Editor/IDE: Visual Studio Code (VS Code). I chose it because it's lightweight, highly extensible, and has an excellent integrated
//     terminal, which allowed me to perform all build and run tasks within a single window.
//   - VS Code Extensions: I used the "C/C++ Extension Pack" from Microsoft, which provided syntax highlighting, code completion (IntelliSense),
//     and basic error checking that made writing the code much more efficient.
//   - Compiler/Simulator: G++ (GNU C++ Compiler) from the MinGW-w64 toolchain, installed via MSYS2. In this project, the G++ compiler
//     and the linked SystemC library together form the "simulator". G++ compiles the C++ code, and the linked SystemC library provides the
//     runtime kernel that executes the simulation.
//   - Build System: GNU Make. I created a custom `Makefile` to automate the entire workflow.
//   - Waveform Viewer: GTKWave. I used this free, standard tool to visualize the `waveform.vcd` output file, which was essential for debugging.
//