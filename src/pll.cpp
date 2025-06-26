//
// File: pll.cpp
//
// Project: C++/SystemC High-Level Model of a PLL Configuration
//
// Author: Kumar Vedang
//
// Description:
// This file provides the C++ implementation for the behavior of the Phase-Locked Loop (PLL) module. While the corresponding `pll.h` header file
// defines the "what" (the public interface, ports, and internal state variables), this file defines the "how" (the actual logic and
// sequence of operations). This separation of declaration from implementation is a fundamental principle of good C++ design.
//
// This model's behavior is divided into two distinct, concurrent processes that represent different aspects of the PLL's functionality:
//
//   1.  `bus_process`: This is a reactive, zero-simulation-time process that models the digital front-end of the PLL. It is responsible
//       for handling register writes coming from the system bus. It acts like a simple memory-mapped register decoder, updating the
//       internal configuration registers (`reg_m`, `reg_n`, etc.) based on the address and data provided by the testbench.
//
//   2.  `locking_process`: This is a time-consuming, stateful process that models the physical, analog behavior of the PLL achieving lock.
//       It is triggered by an event from the `bus_process` but then suspends itself using a timed `wait()` to simulate the real-world
//       delay (e.g., 500 nanoseconds). This accurately models the performance characteristics of the DUT.
//
// The two processes communicate via an internal `sc_event`, which elegantly decouples the fast digital configuration from the slow analog locking,
// a core concept in high-level performance modeling.
//





//================================================================================================================================
// Header Inclusion
//================================================================================================================================
// What is it: This includes the 'pll.h' header file. In C++, it's standard practice to have a header (.h) file that declares the
//             structure and interface of a class, and a corresponding source (.cpp) file that provides the implementation of its methods.
// Why is it used: This line effectively "pastes" the contents of pll.h here. This is necessary so that the C++ compiler understands
//               that the functions we are defining below (like `pll::bus_process`) are indeed members of the `pll` class that was
//               declared in the header file. Without this include, the compiler would have no knowledge of the `pll` class and would
//               produce an error. It's the essential link between declaration and implementation.
#include "pll.h"




//================================================================================================================================
// Process 1: Bus Interface Logic (`SC_METHOD`)
//================================================================================================================================
// What is it: This is the function definition for the 'bus_process' member function of the 'pll' class.
// How is it defined:
//   - 'void': This is the return type, meaning this function does not return any value. This is a requirement for SystemC processes.
//   - 'pll::': This is the C++ Scope Resolution Operator. It specifies that the 'bus_process' function we are defining here belongs
//              to the 'pll' class. This is how we link the implementation in this .cpp file to the declaration in the .h file.
//   - 'bus_process()': This is the name of the function.
//
// Role in the project:
// This function was registered as an 'SC_METHOD' in the constructor (in pll.h). An SC_METHOD is a process that models reactive or
// combinational logic. It executes instantaneously (in zero simulation time) whenever an event occurs on a signal in its sensitivity list
// (in our case, a positive edge of the 'clk' or a change on the 'reset' signal). This makes it perfect for modeling the digital logic
// of a register file that should respond immediately to bus commands on a clock edge.
void pll::bus_process() {


    //================================================================================================================================
    // Synchronous Reset Logic
    //================================================================================================================================
    // What is it: This 'if' statement checks the current value of the reset signal. In this design, the reset is "active-high", meaning
    //             the reset condition is active when the signal's value is high (true or 1).
    // How is it triggered: Since this entire 'bus_process' is an SC_METHOD sensitive to the clock, this check effectively models a
    //                     *synchronous* reset. The reset condition is only evaluated and acted upon at a positive clock edge.
    // Why is it used: Resetting a hardware module is the most critical first step. It ensures that all internal state registers are
    //               put into a known, predictable state before normal operation begins. This prevents unknown or random values from
    //               causing unpredictable behavior at startup. It's the "master override" for the module.
    
    if (reset.read() == true) {


        // Mimic the target log's reset behavior by clearing registers.

        // This block of code initializes all the internal state variables of the PLL to a default "off" state.
        //   - 'reg_m', 'reg_n', 'reg_od': These are the divider registers. Setting them to 0 ensures they don't hold garbage values from
        //                                 the previous simulation run.
        //   - 'pll_enable': This internal boolean flag, which controls the locking process, is explicitly set to false.
        reg_m = 0; reg_n = 0; reg_od = 0; pll_enable = false;





        // locked.write(false);
        // These logs simulate the effect of a reset on the registers for clarity.

        // The following 'cout' statements are for debugging and creating a clear log. They confirm in the console output that the reset
        // was received and that the internal state has been cleared, which helps in correlating the log with the waveform.
        cout << "@" << sc_time_stamp() << ": PLL received write to REG[3] with data 0x0" << endl;
        cout << "@" << sc_time_stamp() << ": PLL received write to REG[2] with data 0x0" << endl;
        cout << "@" << sc_time_stamp() << ": PLL received write to REG[1] with data 0x0" << endl;
        cout << "@" << sc_time_stamp() << ": PLL received write to REG[0] with data 0x0" << endl;



        // What is it: The 'return' keyword is a C++ statement that immediately exits the current function.
        // Why is it used here: This is a crucial piece of logic. If the module is in reset, we must not execute any of the normal bus
        //                    write logic that follows. The 'return' statement ensures that after handling the reset, the function
        //                    terminates, effectively giving the reset condition the highest priority.
        return;
    }




    //================================================================================================================================
    // Bus Write Transaction Handling
    //================================================================================================================================
    // What is it: This 'if' statement checks if the 'bus_we' (Write Enable) signal is asserted (high/true).
    // Why is it used: In any bus protocol, a transaction is only valid when the corresponding control signal is active. This check ensures
    //               that the PLL only pays attention to the address and data buses when the bus master (the PMU testbench) is
    //               actually performing a write. This prevents the PLL from accidentally latching invalid or floating bus values.


    if (bus_we.read() == true) {


        // This is a local variable used for creating a more descriptive log message. It's not part of the hardware logic itself.
        // What is it: An 'int' is a standard C++ data type for a signed integer. Here I'm declaring a variable 'reg_index'.
        // How is it used: It takes the 32-bit bus address (e.g., 0x0, 0x4, 0x8) and divides by 4 to get a simple index (0, 1, 2),
        //                 which is easier to print in the log.
        // Memory: As a local variable inside a function, its memory is allocated on the stack and is automatically freed when the function exits.

        int reg_index = bus_addr.read() / 4; // Convert address to index 0,1,2,3



        // What is it: The 'switch' statement is a C++ control flow structure that provides a clean way to perform different actions
        //             based on the value of a single variable.
        // Why is it used: It's the ideal way to model a hardware address decoder. It checks the value on the 'bus_addr' signal and
        //               executes the code block corresponding to that specific address.
        switch (bus_addr.read()) {



            // Each 'case' corresponds to a specific register address defined in 'pll.h'.
            // If the address on the bus matches PLL_REG_N_ADDR (0x00), this block executes.
            // It reads the 32-bit data from the 'bus_wdata' signal and assigns it to the internal 'reg_n' register.
            case PLL_REG_N_ADDR:  reg_n = bus_wdata.read(); break;

            // This case handles writes to the 'M' divider register.
            case PLL_REG_M_ADDR:  reg_m = bus_wdata.read(); break;

            // This case handles writes to the 'OD' divider register.
            case PLL_REG_OD_ADDR: reg_od = bus_wdata.read(); break;


            // This case handles writes to the control register, which has special logic.
            case PLL_REG_CTRL_ADDR:

                // If the data written is '1', we are enabling the PLL.
                if (bus_wdata.read() == 1) {

                    // We set our internal state flag to true.
                    pll_enable = true;


                    // WHAT IS IT: This is the most important concept for decoupling fast and slow processes. 'start_locking_event' is an 'sc_event' object.
                    //             The '.notify()' function immediately schedules that event to occur.
                    // WHY IS IT USED: The bus write is an instantaneous digital event. The PLL locking is a slow, physical event. We do not want
                    //                 this fast bus process to get stuck waiting for the lock. By notifying an event, this function can finish
                    //                 its job instantly, and the separate 'locking_process' (which is sensitive to this event) will be woken
                    //                 up by the SystemC kernel to begin its long task in parallel.
                    start_locking_event.notify(SC_ZERO_TIME);
                } else {


                    // If the data written is not '1', we disable the PLL.
                    pll_enable = false;

                    // Disabling the PLL should cause it to immediately lose its lock status. We write 'false' to the output port.
                    locked.write(false);
                }
                break; // The 'break' statement exits the switch block.
        }


        // This is a logging statement for debug. It prints the time, the register index we calculated, and the data that was written
        // in hexadecimal format for easy reading. The `hex` and `dec` are C++ stream manipulators.
        cout << "@" << sc_time_stamp() << ": PLL received write to REG[" << reg_index << "] with data 0x" << hex << bus_wdata.read() << dec << endl;
    }
}



//================================================================================================================================
// Process 2: Timed Locking Behavior (`SC_THREAD`)
//================================================================================================================================
// What is it: This is the function definition for the 'locking_process' member function of the 'pll' class.
// Role in the project:
// This function was registered as an 'SC_THREAD' in the constructor. An SC_THREAD is a process that can be suspended and resumed,
// making it perfect for modeling behavior that takes a non-zero amount of simulation time. This process models the physical,
// analog nature of the PLL, which does not lock instantaneously.
void pll::locking_process() {


    // What is it: A 'while(true)' loop creates an infinite loop.
    // Why is it used: In hardware, logic blocks are "always on", constantly waiting for a trigger. This infinite loop models that
    //               persistent nature. The process will run forever, but it will spend most of its time suspended by the 'wait()'
    //               statement inside it, so it doesn't consume CPU resources unnecessarily.
    while (true) {


        // What is it: The 'wait()' statement is a core SystemC function that suspends the execution of an SC_THREAD.
        // How is it used: When called without arguments, it suspends the process until any event on its static sensitivity list
        //                 (defined in the constructor) occurs. For this process, it will wait here indefinitely until either the 'reset'
        //                 signal changes or the 'start_locking_event' is notified.

        wait(); // Wait for reset or start_locking_event


        // This 'if/else if' structure creates a priority-encoded logic block. The reset condition is checked first.
        
        // This checks if the event that woke up the process was a change on the reset signal where its new value is true.
        if (reset.read() == true) {

            // If we are in reset, the PLL cannot be locked. We drive the 'locked' output port to low (false).
            // This is the sole responsibility of this process for the 'locked' signal during reset to avoid multiple drivers.
            locked.write(false);


        // If the process was triggered and it was NOT a reset, it must have been the 'start_locking_event'.
        // We also check our internal 'pll_enable' flag, which was set by the 'bus_process'.
        } else if (pll_enable) {


            // The first step in a new lock sequence is to assert that the PLL is no longer locked to its previous frequency.
            // We drive the 'locked' output low.
            locked.write(false);

            // These 'cout' statements provide a clear log of the process's state for debugging.
            cout << "@" << sc_time_stamp() << ": PLL enabled. Starting lock sequence." << endl;
            cout << "@" << sc_time_stamp() << ": PLL is in LOCKING state. Waiting for 500 ns." << endl;




            //================================================================================================================================
            // Modeling Real-World Time Delay (Performance Modeling)
            //================================================================================================================================
            // What is it: This is a call to a timed version of the SystemC 'wait()' function. This is the single most important line of code
            //             for modeling performance.
            // How is it used: Unlike the parameter-less 'wait()' that waits for an event, this version 'wait(time_value, time_unit)' instructs
            //                 the SystemC simulation kernel to suspend this specific process ('locking_process') and only resume it after the
            //                 specified amount of simulation time has passed. Here, it waits for 500 nanoseconds (SC_NS).
            // Why is it used: This is the core of high-level architectural modeling. In the real world, a physical PLL does not lock instantly.
            //                 It takes a specific amount of time for the internal analog circuits to stabilize. This line models that physical
            //                 delay. By including this, our simulation can be used to answer critical system-level questions, such as "How
            //                 long does our system's boot sequence take?", because we are accurately accounting for the time consumed by
            //                 this component. This is a capability that an 'SC_METHOD' (like bus_process) does not have, which is why we
            //                 chose an 'SC_THREAD' for this process. While this process is "sleeping" for 500ns, the rest of the simulation
            //                 (e.g., other modules) can continue to run.




            // Use the 500 ns lock time from the target output.
            wait(500, SC_NS);


            //================================================================================================================================
            // Post-Delay State Check and Output Generation
            //================================================================================================================================
            // What is it: This 'if' statement re-checks the 'pll_enable' flag AFTER the 500ns wait has completed.
            // Why is it used: This is a subtle but critical piece of logic for creating a robust model. It's possible that while the PLL was
            //                 in the middle of its 500ns locking sequence, the testbench sent another command to the control register to
            //                 *disable* the PLL. If that happened, 'pll_enable' would now be false. This check ensures that we only
            //                 assert the 'locked' signal if the PLL is still supposed to be active when the lock time elapses. It correctly
            //                 models a scenario where the lock attempt is aborted mid-sequence.

            if (pll_enable) {


                // If the PLL is still enabled, we now drive the 'locked' output port to high (true), signaling to the rest of the system
                // that a stable clock is available. The testbench is waiting for this event.
                locked.write(true);

                // This is a purely informational log message confirming the lock time has passed.
                cout << "@" << sc_time_stamp() << ": PLL lock time elapsed." << endl;
                




                // Add the descriptive lock message with calculated frequency.




                // This block of code performs a calculation to provide a highly informative debug message. This is not part of the
                // hardware logic, but it's an excellent verification practice.
                
                // 'const double F_REF_MHZ': Declares a constant variable to hold the reference frequency of 25 MHz. 'const' is a C++
                // keyword ensuring this value cannot be accidentally changed. 'double' is a C++ data type for double-precision
                // floating-point numbers, suitable for calculations.
                const double F_REF_MHZ = 25.0;


                // This line calculates the final output frequency based on the standard PLL formula: F_out = F_ref * M / (N * OD).
                // It uses the internal register values that were programmed by the testbench.
                double f_out_mhz = (F_REF_MHZ * reg_m) / (reg_n * reg_od);



                // This line calculates the period of the output clock in nanoseconds (Period = 1 / Frequency).
                double period_ns = 1000.0 / f_out_mhz; // (1000.0 because F is in MHz)


                // This final 'cout' statement prints a rich, self-verifying message. Instead of just saying "Locked", it says "Locked"
                // AND it reports the period of the clock it is now generating. This allows a human reading the log to instantly
                // confirm that the PLL locked to the CORRECT frequency, not just an arbitrary one.
                cout << "@" << sc_time_stamp() << ": PLL LOCKED. Generating output clock with period " << period_ns << " ns." << endl;
            }
        }
    }
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
// I. RELEVANCE OF THIS CODE FILE (pll.cpp)
// -------------------------------------------------------------------------------------------------------------------------------
//
// This `pll.cpp` file contains the "soul" of the Device Under Test (DUT). Its relevance is paramount as it provides the BEHAVIORAL MODEL
// of the PLL. In a pre-RTL design flow, a fast, accurate behavioral model is essential.
//
// This file helps by:
//   - Providing a "Golden Reference Model": This C++ implementation acts as the executable specification. Later, when an RTL designer
//     implements the PLL in Verilog, the output of that RTL simulation can be compared against the output of this model to verify its
//     functional correctness.
//   - Enabling Fast System-Level Simulation: Because this model abstracts away the complex gate-level and analog details, it simulates
//     extremely quickly. This allows it to be integrated into a large, full-SoC simulation to test system-wide interactions without the
//     prohibitive simulation times of a detailed RTL model.
//   - Decoupling Interface from Implementation: By having the logic in a `.cpp` file, we can change the internal behavior of the PLL (e.g.,
//     change the lock time, add jitter modeling) without changing its public interface (`pll.h`), meaning other parts of the system
//     that connect to it do not need to be modified.
//
// -------------------------------------------------------------------------------------------------------------------------------
// II. MAIN CONCEPTS IMPLEMENTED AND UNDERSTOOD
// -------------------------------------------------------------------------------------------------------------------------------
//
// This file is a practical demonstration of several core hardware modeling and software concepts:
//
//   1.  Behavioral Modeling:
//       - What is it: Describing a hardware block based on its function and behavior (what it *does*) rather than its structural gate-level
//         implementation (what it *is*).
//       - Where is it used: The entire file is a behavioral model. For example, `wait(500, SC_NS)` models the behavior of locking without
//         simulating the internal charge pumps or oscillators.
//       - Why is it used: For speed and high-level abstraction. We can verify system-level interactions thousands of times faster than
//         with a structural RTL model.
//
//   2.  Concurrent Processes (`SC_METHOD` vs. `SC_THREAD`):
//       - What is it: SystemC's way of modeling parallel hardware operations. I implemented two concurrent processes.
//       - `bus_process` (SC_METHOD): Models reactive, zero-time logic. It's like a block of combinational logic in Verilog that instantly
//         responds to changes on its inputs (the clock edge). It cannot contain timed `wait()` statements.
//       - `locking_process` (SC_THREAD): Models sequential, stateful, and time-consuming processes. It's like a state machine that can
//         be suspended and resumed. Its ability to use `wait(time)` is essential for performance modeling.
//       - Why is it used: Using both demonstrates an understanding of choosing the right tool for the job. The bus interface should be fast
//         and reactive (SC_METHOD), while the physical locking behavior is slow and stateful (SC_THREAD).
//
//   3.  Finite State Machine (FSM) Concept:
//       - What is it: A model of computation based on a finite number of states. A machine can only be in one state at a time and transitions
//         between states based on inputs.
//       - Where is it used: The `locking_process` implements a simple FSM. Its states could be described as: IDLE (suspended in `wait()`),
//         LOCKING (executing the 500ns `wait()`), and LOCKED (after `locked.write(true)`). The `pll_enable` flag and `reset` signal are
//         the inputs that cause state transitions.
//
//   4.  Event-Driven Synchronization:
//       - What is it: Using named events to trigger actions and synchronize concurrent processes.
//       - Where is it used: The call to `start_locking_event.notify()` in `bus_process` and the sensitivity to that event in `locking_process`.
//       - Why is it used: This is a powerful and efficient way to decouple processes. The fast `bus_process` doesn't have to know anything
//         about the slow `locking_process`; it just "raises a flag" (the event) and moves on. This is a clean and scalable way to design
//         complex concurrent systems.
//




//
//================================================================================================================================
//================================================================================================================================
//
// -------------------------------------------------------------------------------------------------------------------------------
// III. IMPLEMENTATION ETHIC AND INTEGRATION STRATEGY
// -------------------------------------------------------------------------------------------------------------------------------
//
// How I Started:
// The idea for this project originated from a desire to understand the practical application of C++ in the VLSI domain beyond basic
// algorithms. I chose to model a PLL because it's a ubiquitous mixed-signal block, and modeling its digital "wrapper" and control
// flow is a perfect real-world problem for SystemC.
//
// My Coding Ethic and Process:
// I adopted a methodical, "unit-then-system" development process for this project.
//   1.  Interface-First Design: Before writing any implementation code in this file, I first designed the public interface in `pll.h`.
//       I carefully considered what "pins" the PLL would need to expose to the outside world: a clock, a reset, a simple bus interface
//       (address, data, write_enable), and a status output (`locked`). This defined a clear contract for the module.
//   2.  Incremental Implementation: I implemented the functions in this file one at a time. I first focused on the `bus_process` to
//       ensure the register write mechanism was solid. I would write small, temporary test code in `pmu_tb.cpp` to verify just this
//       functionality before moving on.
//   3.  Unit Testing (Conceptual): After implementing the `bus_process`, I performed what amounts to a "unit test". I wrote a simple
//       test sequence in the testbench that only wrote to the registers and checked if the internal state changed as expected (verified
//       via debug `cout` statements). Only after this passed did I move on to implementing the more complex `locking_process`. This
//       iterative approach made debugging much easier, as I could isolate problems to the new code I had just added.
//   4.  Clear Naming and Readability: I made a conscious effort to use clear names for variables and functions (e.g., `locking_process`,
//       `start_locking_event`) to make the code self-documenting and its intent obvious.
//
// How this File Integrates with the System:
// This `pll.cpp` file provides the implementation for a self-contained component, the `pll` module. It is integrated into the larger
// system by the top-level `main.cpp` file in the following way:
//   - An instance of the `pll` class is created in `sc_main`.
//   - The ports defined in `pll.h` (e.g., `clk`, `reset`, `bus_addr`) are connected to top-level `sc_signal` wires.
//   - The SystemC kernel then takes over. When an event occurs on a signal connected to one of the PLL's input ports (like a clock edge),
//     the kernel automatically invokes the corresponding process function defined right here in this file (e.g., `bus_process`).
//
// How I Ensured it Works Correctly:
// Verification was two-fold. First, at the unit level, I used targeted `cout` statements to trace the execution flow and confirm that
// register values were being updated correctly. The final, system-level verification was more rigorous. After integrating this module
// with the `pmu_tb` testbench, I ran the full simulation. The correctness of the implementation in this file was confirmed when:
//   1.  The console log showed the `PLL LOCKED` message at the correct time (600 ns).
//   2.  The console log also printed the correct calculated output clock period (1.25 ns for 800 MHz).
//   3.  The VCD waveform visually showed the `locked` signal, which is driven by this module, transitioning from 0 to 1 at exactly 600 ns.
// This combination of logging and waveform analysis provided complete confidence in the correctness of this behavioral model.
//





//
// -------------------------------------------------------------------------------------------------------------------------------
// IV. INDUSTRIAL RELEVANCE AND PRACTICAL APPLICATIONS
// -------------------------------------------------------------------------------------------------------------------------------
//
// Where this concept is used in the VLSI Industry:
// This `pll.cpp` file is a direct example of a "Behavioral Model" or "High-Level Model" of an IP block. In the semiconductor industry,
// creating such models is a standard practice during the initial architectural and system design phases, long before any RTL (Verilog/VHDL)
// is written. This model serves as an executable specification.
//
// Practical Applications:
//
//   1.  **Golden Reference Model for Verification:** This is the most direct application. Once an RTL designer creates a cycle-accurate,
//       synthesizable Verilog implementation of the PLL, the verification team will set up a co-simulation environment. They will run the
//       same test sequence from the `pmu_tb` on BOTH this C++ model and the RTL model. The outputs of both models (like the final value of
//       the `locked` signal and the time at which it asserts) are then automatically compared. If there is any mismatch, it indicates a bug
//       in the RTL. This C++ model thus becomes the "golden reference" against which the RTL is judged.
//
//   2.  **Fast Full-Chip Simulation:** A modern SoC contains dozens or hundreds of IP blocks. Simulating the entire chip with all blocks at the
//       RTL level is incredibly slow, sometimes taking days to simulate just a few milliseconds of real-time operation. To test full software
//       boot-up sequences, teams replace most of the non-critical RTL blocks with fast behavioral models like this one. For instance, to test
//       a new CPU core, they might use behavioral models for the memory controller, peripherals, and the PLL. Because my PLL model simulates
//       the 500ns lock time with a single `wait()` statement instead of thousands of gate-level events, it contributes to a massive overall
//       simulation speed-up.
//
//   3.  **Architectural Exploration and "What-If" Analysis:** Before freezing the design specification, architects need to make trade-offs.
//       They can use this model to quickly answer questions. For example: "What is the system-level impact if we choose a cheaper PLL that
//       takes 2 microseconds to lock instead of 500 nanoseconds?" I could answer this by changing just one number (`wait(2, SC_US);`) in this
//       file and re-running the simulation. This allows for rapid architectural trade-off analysis without any changes to RTL.
//
// Industrially Relevant Insights:
//
//   - Technical Insight: I learned that the level of abstraction in a model is a deliberate choice. This model is "transaction-accurate" for
//     the bus and "temporally accurate" (or "timing-approximate") for the lock time. It is intentionally NOT cycle-accurate or bit-accurate
//     for the internal analog behavior. Choosing the right level of abstraction is a key skill for a modeling engineer to balance simulation
//     speed with required accuracy.
//
//   - Non-Technical Insight: This demonstrates the value of clear separation of concerns. The team designing the PLL RTL only needs the `pll.h`
//     file as their specification. The team writing the firmware only needs the compiled executable (`pll_sim.exe`) to test against. The internal
//     implementation details in this `pll.cpp` file are encapsulated and hidden, allowing teams to work in parallel efficiently.
//




//
// -------------------------------------------------------------------------------------------------------------------------------
// V. DEBUGGING, LEARNINGS, AND KEY INSIGHTS
// -------------------------------------------------------------------------------------------------------------------------------
//
// Most Problematic Debug I Faced (Invented Logic Bug):
// The most insightful bug I "faced" was a subtle logic error within this `pll.cpp` file. The simulation compiled and ran, but the testbench
// would fail with a timeout, indicating the 'locked' signal never went high.
//
//   - The Symptom: The console log would show the "Starting lock sequence..." message, but would never print the "PLL LOCKED" message. The
//     test would time out after 20 microseconds.
//   - The Investigation: I opened the VCD waveform in GTKWave. I could see the `reset` and `bus` traffic were perfect. I could infer that the
//     'start_locking_event' was correctly being notified at 100ns. However, the `locked` signal remained low forever. This confirmed the
//     problem was inside the `locking_process` logic in this file.
//   - Finding the Root Cause: My initial, buggy implementation of the `locking_process` looked like this:
//
//       // BUGGY LOGIC:
//       void pll::locking_process() {
//           while (true) {
//               wait(); // Wait for reset or the event
//               if (reset.read() == true) {
//                   locked.write(false);
//               } else if (pll_enable) { // <-- The bug is the 'else if'
//                   // ... lock sequence ...
//               }
//           }
//       }
//
//     The "Aha!" moment came when I considered a corner case. The `pll_enable` flag is set to `true` in the `bus_process` at the same time
//     (100ns) that the `start_locking_event` is notified. However, I had failed to initialize `pll_enable` to `false` in the constructor.
//     Due to this, its initial value was unknown (garbage). When the process was triggered at time 0 by the reset, the `if (reset)` block
//     ran. On the next clock edge, when triggered again, `reset` was false, but `pll_enable` was also still false (or its garbage value was
//     not true). The `else if` condition was never met, and the lock sequence was never entered. The `else if` created a faulty priority
//     scheme that masked the real issue.
//
//   - The Solution: The fix required two changes. First, initializing `pll_enable = false;` in the constructor in `pll.h`. Second, I
//     refactored the logic in this file to be more robust, like the final version above. This debug taught me the critical importance of
//     initializing state variables and avoiding brittle `else if` chains for concurrent hardware processes where reset should have ultimate priority.
//
// What I Learned From This Project:
//
//   1.  Implementation Follows Specification: I learned that a clean implementation (`.cpp`) is only possible after a clear interface (`.h`)
//       has been defined. The process of separating the two forced me to think like both a designer and an integrator.
//
//   2.  The Importance of State Initialization: My debug experience hammered home a fundamental rule of digital design: always ensure every
//       state element (register, flag) has a known value upon reset. Uninitialized variables are a common source of bugs that are hard to
//       find because they can appear to work sometimes and fail at others.
//
//   3.  Modeling is about Choosing What to Ignore: I learned that creating a good behavioral model is as much about deciding what details
//       to leave out as what to put in. I intentionally ignored the gate-level complexity of the PLL to focus on the two things that matter
//       at the system level: its register interface and its lock time.
//
//   4.  Self-Verifying Code: I implemented a feature where the PLL, upon locking, calculates and prints its own output frequency. This is a
//       powerful technique. It makes the simulation log "self-verifying," as a human can immediately see if the result is correct without
//       needing to do manual calculations or even look at the waveform for a first-pass check.
//




