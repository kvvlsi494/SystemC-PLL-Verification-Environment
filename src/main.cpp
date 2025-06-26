//
// File: main.cpp
//
// Project: C++/SystemC High-Level Model of a PLL Configuration
//
// Author: Kumar Vedang
//
// Description:
// This file represents the top-level test harness for the entire simulation environment. It acts as the "system integrator's workbench" or the main
// circuit board where all the individual hardware components are placed and wired together. Its primary responsibilities are not to model any specific
// hardware behavior, but to orchestrate the simulation itself.
//
// The execution flow is as follows:
//   1.  Instantiation: It creates C++ objects for our hardware modules, specifically the 'pll' (the Device Under Test or DUT) and the 'pmu_tb'
//       (the smart testbench).
//   2.  Elaboration: It declares all the necessary "wires" (SystemC signals and clocks) that will connect these modules.
//   3.  Connection (Port Binding): It performs the crucial step of connecting the ports of the instantiated modules to the declared signals,
//       establishing the communication pathways.
//   4.  Configuration: It sets up simulation-wide utilities, most importantly the VCD (Value Change Dump) waveform tracing to enable visual debugging.
//   5.  Execution: It hands over control to the SystemC simulation kernel by calling sc_start(), which runs the simulation until a stop
//       request is encountered.
//   6.  Cleanup: After the simulation finishes, it performs necessary cleanup, like closing the trace file and freeing memory.
//
// This file is the entry point of the program, defined by the sc_main function, and it is what brings the separate PLL and PMU models together
// into a single, functional system.
//





//================================================================================================================================
// Library and Header Inclusions
//================================================================================================================================



// What is it: The master header file for the Accellera SystemC library.
// Role: It provides all the core functionalities needed for hardware simulation. This includes the simulation kernel, the definition
//       of modules (sc_module), signals (sc_signal), ports (sc_in, sc_out), data types (sc_uint), and the concept of time (sc_time).
// Purpose: We include it here to access the fundamental building blocks required to declare our signals, clocks, and to control the
//          simulation itself with sc_start(). It also includes the VCD tracing functions like sc_create_vcd_trace_file.

#include "systemc.h"




// What is it: The header file that contains the class declaration for our PLL model.
// Role: It acts as the "datasheet" for the PLL component. By including it, this main.cpp file knows what the 'pll' class is,
//       what ports it has, and how to create an instance of it.
// Purpose: This inclusion is necessary so we can instantiate the PLL module (our Device Under Test).

#include "pll.h"




// What is it: The header file containing the class declaration for our Power Management Unit (PMU) testbench model.
// Role: Similar to pll.h, this acts as the "datasheet" for our testbench component. It tells main.cpp what the 'pmu_tb' class is
//       and what its interface (ports) looks like.
// Purpose: This is required so we can instantiate the PMU testbench, which is the driver of our entire simulation.

#include "pmu_tb.h"





// What is it: This is the mandatory entry point for any SystemC simulation. It is the SystemC equivalent of the standard C++ main() function.
// Role: The SystemC simulation kernel automatically calls this function to begin the entire simulation setup and execution process.
// Parameters:
//   - 'argc' (argument count): An integer that holds the number of command-line arguments passed to the program when it was run.
//   - 'argv' (argument vector): An array of C-style strings, where each string is one of the command-line arguments.
// Purpose: While we are not using command-line arguments in this project, it's standard practice to include them. They could be used in the future
//          to make the simulation more flexible, for example, to pass in a target frequency instead of having it hard-coded.


int sc_main(int argc, char* argv[]) {

    // This message just provides a human-readable marker in the console output to indicate which phase of setup we are in.
    cout << "Instantiating modules..." << endl;





    //================================================================================================================================
    // OOP Concept: Instantiation of Objects from Classes
    //================================================================================================================================
    // What is it: Instantiation is the process of creating a specific, concrete object (an "instance") from a class blueprint.
    // Why is it used: Our 'pmu_tb' and 'pll' classes defined in their respective .h files are just templates. To actually have a PMU and a PLL
    //               in our simulation, we must create objects of these classes in memory.
    // How is it used: We use the C++ 'new' keyword to dynamically allocate memory for these objects. We store the memory address of each
    //                 new object in a pointer variable ('pmu_inst', 'pll_inst'). This allows us to interact with these objects later.
    //
    // Line-by-line breakdown:
    //   - 'pmu_tb* pmu_inst': Declares a pointer named 'pmu_inst' that can hold the memory address of a 'pmu_tb' object.
    //   - '= new pmu_tb(...)': The 'new' operator allocates memory for one 'pmu_tb' object and calls its constructor.
    //   - '"pmu_inst"': This string is passed to the constructor. SystemC uses this unique name to identify the instance in simulation logs
    //                   and waveform viewers, which is crucial for debugging complex systems.
    pmu_tb* pmu_inst = new pmu_tb("pmu_inst");



    //   - 'pll* pll_inst': Declares a pointer named 'pll_inst' for a 'pll' object.
    //   - '= new pll("pll_inst")': Creates an instance of our PLL Device Under Test (DUT), giving it a unique name.
    pll* pll_inst = new pll("pll_inst");




    // --- SIGNAL DECLARATION ("Wires") ---
    cout << "Creating signals and clocks..." << endl;



    
    //================================================================================================================================
    // Signal and Clock Declarations (The "System Wires")
    //================================================================================================================================
    // What is it: This block of code declares the C++ objects that model the physical wires and clock lines on a circuit board.
    // Why is it used: In SystemC, modules are isolated components. They cannot communicate directly. Signals act as the shared channels
    //               that one module can write to and another can read from, enabling communication and synchronization.
    
    // What is it: This declares an 'sc_clock' object, which is a specialized SystemC signal that automatically toggles between 0 and 1.
    // How is it used:
    //   - 'sc_clock clk': Declares a variable named 'clk' of type 'sc_clock'.
    //   - '"clk"': This is the name of the signal for debugging and waveform viewing.
    //   - '10, SC_NS': This sets the clock's period. '10' is the duration, and 'SC_NS' specifies the unit as nanoseconds.
    //                  This creates a 100 MHz clock (Frequency = 1 / 10 ns).
    // Purpose: This will be the main system clock that drives the sequential logic in both the PMU and the PLL.

    sc_clock clk("clk", 10, SC_NS);




    // What is it: This declares an 'sc_signal' object that can hold a boolean (true/false or 1/0) value.
    // Data Type: 'bool' is a fundamental C++ data type representing true or false. It's perfect for modeling single-bit digital signals.
    // Purpose: This signal will act as the master reset line for the entire system, connecting the testbench to the PLL.

    sc_signal<bool> reset_sig;



    // What is it: This declares two 'sc_signal' objects that can each hold a 32-bit unsigned integer value.
    // Data Type: 'sc_uint<32>' is a SystemC fixed-width unsigned integer type. It's used here to accurately model a 32-bit address bus
    //            and a 32-bit data bus, which is a very common architecture in digital systems.
    // Purpose:
    //   - 'bus_addr_sig': This wire will carry the memory address of the PLL register that the PMU wants to access.
    //   - 'bus_wdata_sig': This wire will carry the 32-bit data value that the PMU wants to write to that register.

    sc_signal<sc_uint<32>> bus_addr_sig, bus_wdata_sig;




     // What is it: This declares two more boolean 'sc_signal' objects.
    // Purpose:
    //   - 'bus_we_sig': This will act as the "Write Enable" signal for our simple bus protocol. When it's high (true), it signals
    //                   to the PLL that the address and data on the bus are valid and a write operation should occur.
    //   - 'locked_sig': This wire carries the status signal from the PLL back to the PMU testbench, indicating whether the PLL has
    //                   successfully achieved frequency lock. The testbench will monitor this signal to verify success.

    sc_signal<bool> bus_we_sig, locked_sig;





    // --- PORT BINDING ("Wiring Up") ---
    cout << "Connecting modules..." << endl;


    //================================================================================================================================
    // Port Binding (The "Wiring Up" Phase)
    //================================================================================================================================
    // What is it: This block of code performs port binding. It connects the ports (the 'sc_in' and 'sc_out' pins) of our module instances
    //             to the 'sc_signal' channels we declared above. This is analogous to physically placing wires on a circuit board
    //             to connect different chips together.
    // Why is it used: This is the fundamental way that separate modules are connected to form a system. Without port binding, the 'pmu_inst'
    //               and 'pll_inst' would be isolated objects unable to communicate. This step defines the system's static connectivity.
    // How is it used: The syntax is 'instance_pointer->port_name(signal_name);'. This establishes a permanent connection for the duration
    //                 of the simulation.

    // Binding the PMU Testbench (the driver) to the signals:
    // Connects the 'clk' input port of the PMU to our global 100 MHz clock signal.

    pmu_inst->clk(clk);


    // Connects the 'reset' output port of the PMU to the global reset signal. The PMU will drive this signal.
    pmu_inst->reset(reset_sig);


    // Connects the bus master output ports of the PMU to their respective bus signals.
    pmu_inst->bus_addr(bus_addr_sig);
    pmu_inst->bus_wdata(bus_wdata_sig);
    pmu_inst->bus_we(bus_we_sig);


    // Connects the 'pll_locked' input port of the PMU to the lock status signal. The PMU will monitor this signal.
    pmu_inst->pll_locked(locked_sig);



    // Binding the PLL (the DUT) to the very same signals:
    // Connects the 'clk' input port of the PLL to the same global clock, ensuring both modules are synchronized.
    pll_inst->clk(clk);


    // Connects the 'reset' input port of the PLL to the same reset signal that the PMU drives.
    pll_inst->reset(reset_sig);



    // Connects the bus slave input ports of the PLL to the same bus signals that the PMU drives.
    // An 'sc_out' on the PMU connects to an 'sc_in' on the PLL.
    pll_inst->bus_addr(bus_addr_sig);
    pll_inst->bus_wdata(bus_wdata_sig);
    pll_inst->bus_we(bus_we_sig);



    // Connects the 'locked' output port of the PLL to the lock status signal. The PLL drives this, and the PMU monitors it.
    pll_inst->locked(locked_sig);





    cout << "Starting simulation..." << endl;


    // --- VCD WAVEFORM TRACING SETUP ---
    cout << "Setting up VCD waveform tracing..." << endl;




    //================================================================================================================================
    // Waveform Tracing Setup (Visual Debugging)
    //================================================================================================================================
    // What is it: This block of code configures the generation of a Value Change Dump (VCD) file. A VCD file is a standard, text-based
    //             format that records every time a signal's value changes throughout the simulation.
    // Why is it used: This is one of the most critical debugging techniques in the entire VLSI industry. It allows engineers to load the
    //               VCD file into a waveform viewer (like GTKWave) and get a visual, graphical representation of all signal activity
    //               over time. This provides irrefutable, bit-level proof of the system's behavior and is essential for finding
    //               subtle bugs that are difficult to spot in text-based logs alone.

    // What is it: This function is provided by the SystemC library to create and open a VCD trace file.
    // How is it used:
    //   - 'sc_trace_file* wf': Declares a pointer 'wf' to an 'sc_trace_file' object. This pointer will act as our handle to the file.
    //   - 'sc_create_vcd_trace_file("waveform")': This function creates a file named "waveform.vcd" in the project directory and returns
    //                                             a pointer to it, which we store in 'wf'.

    sc_trace_file* wf = sc_create_vcd_trace_file("waveform");




    // What is it: This function sets the time resolution for the VCD file.
    // Why is it used: It ensures that the timescale shown in the waveform viewer matches our simulation's timescale, making analysis easier.
    // How is it used: '1, SC_NS' means the fundamental time unit in the VCD will be 1 nanosecond.

    wf->set_time_unit(1, SC_NS); // Set the time unit for the VCD file




    // Add the signals we want to record to the trace file


    // What is it: The 'sc_trace' function tells the simulator which specific signals to record in the VCD file.
    // Why is it used: We typically don't need to trace every single internal signal. We select the most important top-level signals
    //                 that represent the communication between modules.
    // How is it used:
    //   - sc_trace(file_handle, signal_object, "name_in_waveform");
    //   - We call this function for each signal we want to see, passing our file handle ('wf'), the signal object itself ('clk', 'reset_sig', etc.),
    //     and a string that will be the human-readable name of the signal inside the waveform viewer.
    sc_trace(wf, clk, "clk");
    sc_trace(wf, reset_sig, "reset");
    sc_trace(wf, bus_we_sig, "bus_we");
    sc_trace(wf, bus_addr_sig, "bus_addr");
    sc_trace(wf, bus_wdata_sig, "bus_wdata");
    sc_trace(wf, locked_sig, "locked");
    // --- END OF TRACING SETUP ---




    // --- SIMULATION START ---

    //================================================================================================================================
    // Simulation Execution and Cleanup
    //================================================================================================================================
    
    // What is it: This is the function that starts the SystemC simulation kernel.
    // How is it used: Up to this point, we have only been setting up the simulation (the "elaboration" phase). Calling sc_start() hands
    //               control over to the kernel, which then begins executing the scheduled processes (like the testbench's 'run_test' thread).
    // Why is it used: The simulation runs, advancing time and executing processes based on events, until a process calls sc_stop(). In our
    //                 project, the testbench ('pmu_tb') calls sc_stop() when its test scenario is complete. Control then returns to the line
    //                 immediately following sc_start().

    sc_start();




    // --- CLEANUP ---
    // This line simply prints the final simulation time to the console, providing a summary of how long the simulation ran.
    cout << "Simulation finished at " << sc_time_stamp() << endl;




    // What is it: This function closes the VCD trace file handle.
    // Why is it used: This is a critical step. If the trace file is not properly closed, it may be corrupted or incomplete, making it
    //                 unreadable by waveform viewers. This ensures all buffered trace data is written to disk and the file is finalized.

    sc_close_vcd_trace_file(wf); // Close the VCD file to save it properly



    // What is it: The C++ 'delete' operator is used to de-allocate memory that was previously allocated with the 'new' operator.
    // Why is it used: Since we created our 'pll_inst' and 'pmu_inst' objects dynamically on the heap, it is good programming practice
    //                 to explicitly free that memory when we are done with them. This prevents memory leaks in larger, more complex programs
    //                 where objects might be created and destroyed multiple times.
    delete pll_inst;
    delete pmu_inst;


    // This returns an exit code of 0 to the operating system, indicating that the program ran and terminated successfully without errors.
    return 0;
}



//================================================================================================================================
//================================================================================================================================
//
//                               DETAILED PROJECT DOCUMENTATION AND ANALYSIS
//
//================================================================================================================================
//================================================================================================================================
//
// -------------------------------------------------------------------------------------------------------------------------------
// I. RELEVANCE OF THIS CODE FILE (main.cpp)
// -------------------------------------------------------------------------------------------------------------------------------
//
// This `main.cpp` file, while containing no hardware logic itself, is arguably one of the most important files in any SystemC project.
// Its relevance is that it serves as the "Top-Level Test Harness" or "System Scaffolding". In a real VLSI project, different engineers or teams
// might work on different blocks (like the PLL, a CPU core, a memory controller, etc.). This file represents the final integration point
// where all those individual blocks are brought together, connected, and simulated as a complete system.
//
// It helps by:
//   - Defining the System Architecture: The connections made in this file (the port binding) explicitly define the static hardware
//     architecture of the system being simulated.
//   - Centralizing Simulation Control: It is the single place where simulation-wide settings, like waveform tracing, are configured.
//   - Providing the Entry Point: It contains the `sc_main` function, which is the starting point for the entire verification task.
//
// -------------------------------------------------------------------------------------------------------------------------------
// II. MAIN CONCEPTS IMPLEMENTED AND UNDERSTOOD
// -------------------------------------------------------------------------------------------------------------------------------
//
// This file demonstrates a solid understanding of the following key C++ and SystemC concepts:
//
//   1. Instantiation (Object-Oriented Programming):
//      - What is it: The process of creating a tangible object in memory from a class definition.
//      - Where is it used: The lines `pmu_tb* pmu_inst = new pmu_tb(...)` and `pll* pll_inst = new pll(...)`.
//      - Why is it used: To create the actual, usable instances of our testbench and DUT modules that will participate in the simulation.
//        We use pointers and dynamic allocation (`new`) because it's a flexible way to manage the lifetime of these potentially large objects.
//
//   2. SystemC Elaboration and Simulation Phases:
//      - What is it: SystemC simulation has distinct phases. This file clearly shows the "Elaboration" phase (where modules are instantiated,
//        signals are created, and ports are bound) and the "Execution" phase (which begins with `sc_start()`).
//      - Why is it important: Understanding this separation is crucial for writing correct SystemC code. All structural connections must be
//        made before the simulation starts running and time begins to advance.
//
//   3. Hardware Abstraction (Signals and Ports):
//      - What is it: Modeling physical hardware components (wires, pins) using software constructs.
//      - Where is it used: The declaration of `sc_signal` and `sc_clock` objects represents the creation of physical wires. The subsequent
//        port binding (`pmu_inst->clk(clk)`) represents connecting a chip's pin to one of those wires.
//      - Why is it used: This is the core of SystemC. It allows us to build and connect a virtual prototype of a hardware system in software.
//
//   4. Simulation Artifact Generation (VCD Tracing):
//      - What is it: The process of instructing the simulator to record signal activity into a standardized file format for later analysis.
//      - Where is it used: The block of code dedicated to `sc_create_vcd_trace_file` and `sc_trace`.
//      - Why is it used: This is a non-negotiable part of modern verification. It provides the "ground truth" for debugging. A text log might say
//        something happened, but the waveform provides the visual, bit-level proof, which is essential for finding and fixing bugs.
//



//
// -------------------------------------------------------------------------------------------------------------------------------
// III. IMPLEMENTATION ETHIC AND INTEGRATION STRATEGY
// -------------------------------------------------------------------------------------------------------------------------------
//
// How I Started:
// The idea for this project came from studying standard SoC architectures. I noticed that clock management via a PLL, controlled by a
// power management unit, is a fundamental and universal task. I wanted to model this high-level interaction as it's a perfect showcase
// for SystemC's strengths in architectural exploration.
//
// My Coding Ethic and Process:
// I approached this project with a structured, bottom-up then top-down integration methodology, which is common in the industry.
//   1.  Outlining and Interface Definition: First, I defined the "contracts" or interfaces for each block. I decided what ports the PLL
//       module would need (bus inputs, lock output) and what ports the PMU testbench would need to drive it. This was done in the `.h`
//       header files.
//   2.  Unit Implementation: I then implemented the internal logic for each module separately (`pll.cpp`, `pmu_tb.cpp`), focusing on one
//       block at a time. This is akin to unit-level development. For example, I first ensured the PLL's register write logic was sound before
//       moving on to the timed locking behavior.
//   3.  Top-Down Integration: Finally, I created this `main.cpp` file to integrate the two completed units. This top-down approach ensures
//       that the individual, tested blocks can be connected into a larger, functional system. I chose clear and consistent names for signals
//       (e.g., `reset_sig`) to make the connectivity obvious and less error-prone.
//
// How this File Integrates the System:
// This file is the "glue" that holds the entire project together. The `pll` and `pmu_tb` modules are completely independent and unaware of
// each other's existence. This `main.cpp` file integrates them by:
//   - Instantiating both as objects within the same `sc_main` scope.
//   - Declaring a common set of `sc_signal` objects that act as the shared communication bus.
//   - Explicitly binding the output ports of the `pmu_tb` to these signals, and the input ports of the `pll` to the very same signals. This
//     creates the directed data flow: PMU -> Wires -> PLL. It does the same for the `locked` signal in the reverse direction.
//
// How I Ensured it Works Correctly:
// Verification of this top-level file was done by observing the output of the complete system.
//   1.  Successful Compilation: The first check was that the project compiled and linked without errors. A linker error at this stage would
//       have indicated a problem with how I was integrating the modules or linking the SystemC library.
//   2.  Runtime Execution: I ran the simulation and checked for SystemC runtime errors. An error here (like the "multiple driver" error
//       I encountered) would point to a logical flaw in my system connection or module design.
//   3.  Waveform Analysis: The ultimate verification was done using the `waveform.vcd` file. I specifically checked that the signals
//       being driven by the PMU testbench (e.g., `bus_we_sig`) were identical to the signals being received by the PLL module, proving
//       that my wiring in this file was correct.
//



//
// -------------------------------------------------------------------------------------------------------------------------------
// IV. INDUSTRIAL RELEVANCE AND PRACTICAL APPLICATIONS
// -------------------------------------------------------------------------------------------------------------------------------
//
// Where this concept is used in the VLSI Industry:
// This `main.cpp` file and the methodology it represents are fundamental to the concept of a "Virtual Platform" or "Virtual Prototype".
// In large semiconductor companies like Intel, Qualcomm, or NVIDIA, designing a full System-on-Chip (SoC) involves hundreds of engineers
// working on different IP (Intellectual Property) blocks concurrently. This file acts as the top-level integration framework where early,
// high-level models of these different IPs can be connected and simulated together long before the detailed RTL for each block is complete.
//
// Practical Applications:
//
//   1.  Pre-RTL Architectural Validation: Before committing to Verilog, system architects use a virtual platform like this to validate key
//       architectural decisions. For instance, they could simulate the entire boot-up sequence of a chip. My project models a small piece
//       of this: it answers the question, "Does our proposed PMU-to-PLL control flow work correctly?" and "Is the 500ns lock-time budget
//       achievable and sufficient for the system's power-on requirements?". Finding a logic error here—like realizing the PMU needs to
//       poll a status bit before proceeding—is orders of magnitude cheaper to fix than finding it in silicon.
//
//   2.  Early Software and Firmware Development ("Shift-Left"): The compiled executable (`pll_sim.exe`) is a functional model of the hardware.
//       A software team can use this executable as a "Hardware Abstraction Layer (HAL)" to start developing and testing their low-level
//       drivers and firmware. They can write the exact C code that will eventually run on the chip's CPU to configure the PLL, and test it
//       against this simulation. This "shift-left" approach allows hardware and software development to happen in parallel, dramatically
//       reducing the total time-to-market for the product.
//
//   3.  Performance Modeling and Bottleneck Analysis: At the system level, architects need to model and analyze performance. By assigning
//       realistic time values to operations (like the `wait(500, SC_NS)` in my PLL), a full-chip simulation can provide early estimates for
//       critical metrics like boot time, memory bandwidth, or power state transition latency. If a boot sequence is taking too long,
//       engineers can analyze the simulation results to identify the bottleneck—perhaps a PLL that takes too long to lock—and address it
//       at the architectural stage.
//
// Industrially Relevant Insights:
//
//   - Technical Insight: A key insight is the importance of a well-defined "Transaction-Level Model" (TLM) interface. My simple bus protocol
//     is a basic form of TLM. It abstracts away the cycle-by-cycle wiggling of physical wires into a single function call (a "transaction"),
//     which makes the simulation run incredibly fast and makes the testbench code much cleaner and more intuitive. This is the core principle
//     behind industry standards like TLM-2.0.
//
//   - Non-Technical Insight: This project highlighted the collaborative nature of SoC design. The clear separation between the testbench (`pmu_tb`),
//     the DUT (`pll`), and the integrator (`main.cpp`) mirrors how organizational teams are structured. One team provides the DUT, another provides
//     the verification environment, and a third (system integration) is responsible for putting them together. A clean interface, defined in
//     the header files, is the "contract" between these teams that allows them to work independently.
//



//
// -------------------------------------------------------------------------------------------------------------------------------
// V. DEBUGGING, LEARNINGS, AND KEY INSIGHTS
// -------------------------------------------------------------------------------------------------------------------------------
//
// Most Problematic Debug I Faced:
// The most challenging bug I encountered happened at this integration stage and was only visible by analyzing the waveform. The simulation
// ran without any errors, but the console log showed the PLL locking almost instantaneously, far faster than the specified 500ns. It was
// reporting a lock at around 100ns, which didn't make sense.
//
//   - The Symptom: The testbench reported `✅ SUCCESS!` but at a time that was logically impossible.
//   - The Investigation: I immediately opened the `waveform.vcd` file in GTKWave. I saw the `reset` and `bus` traffic were correct up to 100ns.
//     However, the `locked` signal, which was supposed to stay low until 600ns, was going high almost immediately after the last bus write.
//     My first thought was a bug in the PLL's `wait(500, SC_NS)` statement. I checked `pll.cpp` and it was correct.
//   - Finding the Root Cause: This forced me to look at the integration code in this `main.cpp` file with extreme care. After tracing every
//     connection, I found the bug: a classic "copy-paste" error in the port binding. My code had looked like this:
//
//       // BUGGY WIRING:
//       pmu_inst->bus_we(bus_we_sig);
//       pmu_inst->pll_locked(bus_we_sig); // <-- ERROR! 'pll_locked' port was accidentally connected to the 'bus_we' signal!
//
//     Because the `pll_locked` input port of the testbench was mistakenly wired to the `bus_we` signal, the testbench saw a "high" signal
//     on its lock input every time it performed a bus write. The final write at 100ns made `bus_we` go high, which the testbench interpreted
//     as the PLL achieving lock, so it immediately reported success. The actual `locked_sig` was working correctly, but the testbench was
//     "looking" at the wrong wire.
//
//   - The Solution: The fix was a one-line change in this `main.cpp` file: correcting the connection to use the proper signal.
//
//       // CORRECTED WIRING:
//       pmu_inst->pll_locked(locked_sig);
//
//     This debug was invaluable because it taught me that system-level bugs are often not in the modules themselves, but in the connections
//     between them. It reinforced the importance of careful integration and using the waveform as the ultimate source of truth when the
//     console log seems misleading.
//
// What I Learned From This Project:
//
//   1.  The Power of Abstraction: I learned that modeling hardware at a high level (like TLM) allows you to focus on functionality and
//       system-level interactions without getting bogged down in cycle-accurate, bit-level details. This makes for much faster simulation
//       and earlier bug detection.
//
//   2.  Verification is a Mindset, Not Just Code: I learned that building a testbench requires thinking like an adversary. You must anticipate
//       how a design might fail and write checks to catch those failures. The goal isn't just to make it work, but to prove it can't fail
//       under the tested conditions.
//
//   3.  The Build System is a First-Class Citizen: A significant portion of my time was spent correctly setting up the environment (compiler,
//       libraries) and the `Makefile`. I learned that a robust, automated build system is not an afterthought; it is a critical piece of
//       infrastructure for any serious project.
//
//   4.  Debugging is a Multi-Tool Process: I became proficient at correlating information from four distinct sources to find a single bug:
//       the compiler's static analysis, the SystemC kernel's runtime error messages, the high-level story from the console log, and the
//       low-level ground truth from the VCD waveform.
//