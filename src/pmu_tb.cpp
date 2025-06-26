//
// File: pmu_tb.cpp
//
// Project: C++/SystemC High-Level Model of a PLL Configuration
//
// Author: Kumar Vedang
//
// Description:
// This file contains the C++ implementation for the behavior of our Power Management Unit (PMU) Testbench. In our simulation environment,
// this module acts as the "Test Host" or the "Bus Master". Its sole purpose is to control the Device Under Test (the 'pll' module) and
// verify that it behaves according to its specification. This is the active, "intelligent" component of the simulation.
//
// The behavior implemented here follows a standard verification methodology:
//
//   1.  **Control and Reset:** The testbench first takes control of the simulation environment by asserting and de-asserting the global
//       reset signal, ensuring the entire system starts from a known state.
//
//   2.  **Stimulus Generation:** It then acts as a stimulus generator. It calculates the necessary register values to achieve a target
//       frequency (800 MHz) and then drives these values onto the system bus using a defined protocol. This is implemented in the
//       `write_to_pll` helper function, which acts as a basic Bus Functional Model (BFM).
//
//   3.  **Response Monitoring and Verification:** After sending the stimulus, the testbench switches to a monitoring role. It waits for a
//       specific response from the DUT (the 'locked' signal going high). It includes a timeout mechanism to ensure the test does not hang
//       if the DUT fails to respond. Based on the outcome, it reports a definitive "PASS" or "FAIL" status to the console.
//
// This file is a practical example of how a testbench is constructed to drive and check a hardware design in a simulation-based
// verification flow.
//





//================================================================================================================================
// Library and Header Inclusions
//================================================================================================================================
// What is it: This includes the 'pmu_tb.h' header file. This is the corresponding header for this implementation file.
// Role: It provides the definition of the 'pmu_tb' class, including the declaration of the member functions ('write_to_pll' and 'run_test')
//       that we are about to implement below.
// Purpose: This link is essential for the C++ compiler to verify that our function implementations match the declarations made in the
//          header. It's the core principle of separating interface from implementation. It also gives this file knowledge of the 'pll.h'
//          header (which is included by pmu_tb.h), so we can use the PLL register address macros like 'PLL_REG_N_ADDR'.
#include "pmu_tb.h"







// What is it: This is a standard C++ library for Input/Output Manipulation.
// Role: It provides "stream manipulators" that allow us to format the data we print to the console using 'cout'.
// Purpose: In this specific file, I am using the 'hex' and 'dec' manipulators from this library inside the 'write_to_pll' function. This
//          allows me to print the bus address and data values in hexadecimal format (e.g., "0x20"), which is the standard and most
//          human-readable format for representing register data in digital design. I then switch back to decimal format with 'dec'.
#include <iomanip> // For std::hex






//================================================================================================================================
// Helper Function: Bus Write Driver (Bus Functional Model - BFM)
//================================================================================================================================
// What is it: This is the implementation of the `write_to_pll` member function of the `pmu_tb` class. It's a helper function, or task,
//             designed to perform one complete bus write transaction.
// Role in the project: This function is a fundamental part of creating a layered and readable testbench. Instead of cluttering our main
//                    test sequence (`run_test`) with repetitive, low-level signal assignments for every register write, we abstract
//                    that logic into this single, reusable function. This is a basic form of a Bus Functional Model (BFM), where a
//                    high-level command ("write this data to this address") is translated into the necessary cycle-accurate signal wiggling.
// Parameters:
//   - 'sc_uint<32> addr': This is the 32-bit address of the target register in the PLL. It is passed by value.
//   - 'sc_uint<32> data': This is the 32-bit data we want to write to that register. It is also passed by value.

void pmu_tb::write_to_pll(sc_uint<32> addr, sc_uint<32> data) {



    // Log the driver action *before* the wait, as seen in the target log.

    // This `cout` statement is for logging and debug. It prints a message to the console *before* the transaction happens, indicating
    // the testbench's intent. Using `hex` from `<iomanip>` formats the integer values into a more readable hexadecimal format.
    cout << "  PMU_DRIVER: Wrote 0x" << hex << data << " to address 0x" << addr << dec << endl;


    // The following lines perform the actual signal driving for the bus write protocol.
    // Drive the address bus with the target address. The PLL's 'bus_addr' port will see this value.
    bus_addr.write(addr);

    // Drive the data bus with the target data.
    bus_wdata.write(data);

    // Assert the Write Enable signal. Driving 'bus_we' to 'true' (or 1) signals to the PLL that the address and data are now valid.
    bus_we.write(true);


    // This is a single-cycle wait. Since the 'run_test' process that calls this function is an SC_THREAD sensitive to the positive
    // edge of the clock, this 'wait()' will suspend execution until the *next* positive clock edge. This holds the bus signals
    // stable for one full clock cycle so the PLL (which is also sensitive to the clock edge) can correctly sample them.
    wait(); // Wait for one positive clock edge



    // De-assert the Write Enable signal. After the clock edge, the transaction is complete. We set 'bus_we' back to 'false' (or 0)
    // to end the write cycle and signify that the bus is now idle.
    bus_we.write(false);
}



//================================================================================================================================
// Main Test Sequence (`SC_THREAD`)
//================================================================================================================================
// What is it: This is the implementation of the `run_test` member function, which was registered as an SC_THREAD in the constructor.
// Role in the project: This function is the "brain" of the testbench and the master controller of the entire simulation. It contains the
//                    step-by-step test plan that will be executed. An SC_THREAD is used because a test sequence inherently involves
//                    actions that take time (like waiting for a reset to propagate, waiting for the DUT to respond, etc.), and only
//                    SC_THREADs can be suspended with timed `wait()` statements.
// How it impacts execution: The SystemC kernel will start executing this thread at the beginning of the simulation. The thread will then
//                          proceed sequentially through its defined steps, driving signals, calling helper functions, and waiting for
//                          specific time delays or events, until it finally calls `sc_stop()` to end the simulation.


void pmu_tb::run_test() {


    // --- Phase 1: Reset ---
    // The initial wait() ensures reset is asserted after simulation starts.


    // This initial 'wait()' is a good practice. It ensures that this thread starts its execution on the first positive clock edge
    // *after* time 0, allowing the simulation to properly initialize before we begin driving signals.
    wait(); 


    //================================================================================================================================
    // Phase 1: System Reset Generation
    //================================================================================================================================
    // What is it: This block of code models the testbench acting as a system controller to generate a reset pulse.
    // Why is it important: In any digital system, it is absolutely critical to begin a test by putting the Device Under Test (DUT) into a
    //                   known, predictable state. A reset pulse achieves this by forcing all internal state machines and registers
    //                   in the DUT to their default values.

    // This is a log message indicating the start of the reset phase.

    cout << "PMU_TEST: Resetting the system..." << endl;


    // Here, the testbench drives the 'reset' output port, which is connected to the top-level 'reset_sig' signal, to 'true' (high).
    // This begins the active-high reset pulse.
    reset.write(true);



    // What is it: This is a call to a special version of the 'wait()' function that waits for a specific number of events.
    // How is it used: Since this SC_THREAD is sensitive to the positive edge of the clock, `wait(5)` instructs the simulator to suspend
    //                 this process and resume it only after 5 positive clock edges have occurred.
    // Purpose: With a 10ns clock period, this creates a reset pulse that is held high for exactly 50 nanoseconds (5 * 10ns). This ensures
    //          the reset signal is asserted for a stable, defined duration, long enough for all components in the system to recognize it.
    wait(5); // Hold reset for 5 clock cycles (50 ns)


    // The testbench now de-asserts the reset signal by driving it to 'false' (low). This ends the reset pulse.
    reset.write(false);


    // Another single-cycle wait is added to allow one clock cycle to pass with reset de-asserted before we begin the actual test stimulus.
    // This ensures a clean separation between the reset phase and the test phase.
    wait();





    //================================================================================================================================
    // Phase 2: Stimulus Generation
    //================================================================================================================================
    // What is it: This block of code represents the core "stimulus" portion of the test case. The testbench is now actively interacting
    //             with the DUT to make it perform a specific task.
    // Why is it important: This demonstrates an "intelligent" testbench. Instead of just sending random data, the testbench has a specific
    //                   goal (configure the PLL for 800 MHz) and takes deliberate actions to achieve it.

    // A log message to clearly state the objective of this specific test case in the console output.

    cout << "PMU_TEST: Starting test case: Configure PLL for 800 MHz." << endl;
    
    // In a more complex testbench, these values would be calculated by a dedicated function. For this targeted test, I've pre-calculated
    // them to keep the test sequence clean. The formula is F_out = F_ref * M / (N * OD), so for a 25MHz reference,
    // 800 MHz = 25 MHz * 32 / (1 * 1).
    // The variables are declared as standard C++ 'int' (signed integer) types. Their memory is allocated locally on the stack.

    int n_val = 1;  // The value for the N divider register.
    int m_val = 32; // The value for the M (multiplier) register.
    int od_val = 1; // The value for the OD (output divider) register.


    // This log message confirms the values that will be used for the test, which is good for debug.
    cout << "PMU_TEST: Calculation successful. N=" << n_val << ", M=" << m_val << ", OD=" << od_val << endl;
    

    // This log message announces the start of the programming sequence.
    cout << "PMU_TEST: Programming PLL registers..." << endl;

    // Here, we call our 'write_to_pll' helper function multiple times. This is where the abstraction pays off. The test sequence
    // is clean and readable, like a high-level script. Each call represents a complete, single-cycle bus transaction.

    // Write the calculated value for 'N' to the N-divider register address.
    write_to_pll(PLL_REG_N_ADDR, n_val);

    // Write the calculated value for 'M' to the M-divider register address.
    write_to_pll(PLL_REG_M_ADDR, m_val);

    // Write the calculated value for 'OD' to the OD-divider register address.
    write_to_pll(PLL_REG_OD_ADDR, od_val);
    
    // This is the final and most important write. We write '1' to the control register. This specific action is what signals
    // the PLL model to begin its locking sequence. This demonstrates testing a control mechanism, not just a data register.
    write_to_pll(PLL_REG_CTRL_ADDR, 1);
    


    //================================================================================================================================
    // Phase 3: Response Monitoring and Verification
    //================================================================================================================================
    // What is it: This block of code represents the checking or verification phase of the test. Having sent the stimulus, the testbench
    //             now passively monitors the DUT's outputs to see if it responded correctly.
    // Why is it important: A test is meaningless without a check. This is where we determine if the test case has passed or failed.



    // A log message to indicate that the testbench has entered the monitoring state.
    cout << "PMU_TEST: Waiting for PLL lock signal..." << endl;



    // What is it: This is a call to a modern, multi-argument version of the `wait()` function that handles a wait-with-timeout scenario.
    // How it works: It instructs the SystemC kernel to suspend this thread until EITHER of two conditions is met, whichever comes first:
    //   1.  `sc_time(20, SC_US)`: A timeout of 20 microseconds of simulation time elapses.
    //   2.  `pll_locked.posedge_event()`: The signal connected to the `pll_locked` port experiences a positive edge (a transition from 0 to 1).
    // Why is it used: This is an essential technique for robust verification. We expect the 'locked' signal to go high within 500ns.
    //                 The 20us timeout acts as a "watchdog". If the DUT has a bug and never asserts the 'locked' signal, this timeout
    //                 ensures the simulation doesn't hang forever. The test will resume after 20us and fail gracefully. The arguments
    //                 must be in the order (time, event).
   wait(sc_time(20, SC_US), pll_locked.posedge_event()); // Wait for lock or timeout

    // After the wait() statement finishes (either by event or timeout), this 'if' statement checks the final state of the 'locked' signal.
    // `pll_locked.read()` gets the current value of the signal.
    if (pll_locked.read() == true) {

        // If the 'locked' signal is high, it means the DUT behaved as expected. The `posedge_event` occurred before the timeout.
        // We print a clear "SUCCESS" message. Using an emoji like the checkmark makes logs visually easy to parse.
        cout << "PMU_TEST: ✅ SUCCESS! PLL lock signal asserted." << endl;
    } else {

        // If the 'locked' signal is still low, it means the wait finished because the 20us timeout was reached. This is a failure condition.
        // We print a clear "FAILED" message so that an engineer or an automated script can immediately identify the test failure.
        cout << "PMU_TEST: ❌ FAILED! PLL did not lock." << endl;
    }
    



    //================================================================================================================================
    // Phase 4: Test and Simulation Termination
    //================================================================================================================================
    
    // A log message to clearly indicate that the active testing phase is complete.
    cout << "PMU_TEST: Test finished." << endl;
    


    // What is it: This is a calculated timed wait.
    // Why is it used: This is purely for making the final waveform easier to read. The test is already finished. This wait statement simply
    //                 advances the simulation time to a round number (850 ns). This ensures that the VCD waveform doesn't end abruptly
    //                 right after the last interesting event, giving a nice, clean tail-end to the visual output. It calculates the
    //                 remaining time needed by subtracting the current simulation time (`sc_time_stamp()`) from the target end time.
    wait(sc_time(850, SC_NS) - sc_time_stamp());
    


    // What is it: This is a global SystemC function that instructs the simulation kernel to stop.
    // Why is it used: The SystemC simulation will run forever unless explicitly stopped. It is the responsibility of the testbench, as the
    //                 master controller, to determine when the test is complete and to terminate the simulation.
    // How it impacts execution: When this line is executed, the SystemC kernel stops processing events and advancing time. Control is then
    //                         returned from the `sc_start()` call back to the `sc_main` function, allowing the program to perform final
    //                         cleanup and exit gracefully. This is the definitive end of the simulation run.
    sc_stop();
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
// I. RELEVANCE OF THIS CODE FILE (pmu_tb.cpp)
// -------------------------------------------------------------------------------------------------------------------------------
//
// This `pmu_tb.cpp` file is the implementation of the Testbench, which is the most critical component in any verification environment.
// Its relevance is that it acts as the "Test Host" or "Verification Controller". While the DUT (`pll.cpp`) models the hardware to be
// tested, this file models the external world that interacts with the DUT. It contains the intelligence to execute a predefined test
// plan and determine a pass/fail result.
//
// This file helps by:
//   - Defining a clear, repeatable Test Case: It codifies a specific test scenario (reset, configure for 800 MHz, check lock) that can
//     be run consistently.
//   - Driving the DUT: It generates the actual stimulus (the register writes) that exercises the functionality of the PLL.
//   - Checking the Results: It actively monitors the output of the DUT (`pll_locked`) against an expected outcome, which is the core
//     purpose of verification.
//
// -------------------------------------------------------------------------------------------------------------------------------
// II. MAIN CONCEPTS IMPLEMENTED AND UNDERSTOOD
// -------------------------------------------------------------------------------------------------------------------------------
//
// This file demonstrates a practical understanding of fundamental verification concepts:
//
//   1.  Testbench Architecture:
//       - What is it: A standard architecture for a testbench that separates different verification tasks.
//       - Where is it used: The `run_test` function clearly shows the three main parts of a test: initialization (reset), stimulus
//         generation (programming), and response checking (monitoring for lock).
//       - Why is it used: This structured approach makes testbenches readable, maintainable, and scalable. In a more complex environment
//         like UVM, these parts would be implemented as separate classes (driver, monitor, scoreboard).
//
//   2.  Bus Functional Model (BFM) / Driver Task:
//       - What is it: A BFM is a piece of code that translates high-level commands into low-level signal activity.
//       - Where is it used: The `write_to_pll` function is a simple BFM. It takes a high-level command ("write data to address") and
//         translates it into the necessary cycle-accurate wiggling of the `bus_addr`, `bus_wdata`, and `bus_we` signals.
//       - Why is it used: Abstraction. It allows the main test sequence to be written in a clean, human-readable way without being
//         cluttered by low-level signal details. This makes tests much easier to write and debug.
//
//   3.  Constrained-Random vs. Directed Testing:
//       - What is it: Two primary methodologies for stimulus generation. Directed testing (which I used) involves writing specific,
//         hand-crafted stimulus to test a known feature. Constrained-random testing involves generating randomized stimulus within
//         legal constraints to find unexpected corner-case bugs.
//       - Why is it used: I used directed testing here because the goal was to verify a single, specific functionality (locking at 800 MHz).
//         This project could be extended to use constrained-random techniques by randomizing the M, N, and OD values and having a more
//         complex checking mechanism.
//
//   4.  Self-Checking Testbenches:
//       - What is it: A testbench that can automatically determine a pass/fail result without requiring a human to manually inspect a
//         waveform.
//       - Where is it used: The `if (pll_locked.read() == true)` block is the self-checking part of this testbench. It checks the condition
//         and prints a definitive "SUCCESS" or "FAILED" message.
//       - Why is it used: This is absolutely essential for regression testing, where hundreds or thousands of tests are run automatically
//         overnight. A simple pass/fail summary allows engineers to quickly identify which specific test broke.
//


//
// -------------------------------------------------------------------------------------------------------------------------------
// III. IMPLEMENTATION ETHIC AND INTEGRATION STRATEGY
// -------------------------------------------------------------------------------------------------------------------------------
//
// How I Started:
// The idea for this project came from wanting to build a complete, closed-loop verification environment. Rather than just designing a
// block in isolation, I wanted to experience the full cycle of driving stimulus and checking a response, which led me to the
// PMU-to-PLL configuration scenario.
//
// My Coding Ethic and Process:
// I approached the implementation of this testbench with a "test-plan-first" mentality.
//   1.  Define the Test Plan: Before writing the `run_test` code, I wrote down the test plan in plain English:
//       - Phase 1: Assert reset for 50ns, then de-assert.
//       - Phase 2: Program the N, M, OD, and CTRL registers with values for 800 MHz.
//       - Phase 3: Monitor the 'locked' signal.
//       - Phase 4: If 'locked' goes high within 20us, PASS. Otherwise, FAIL.
//       - Phase 5: End the simulation.
//   2.  Code the Sequence: The `run_test` function in this file is a direct translation of that test plan into SystemC code. This direct
//       mapping from plan to code makes the test's intent clear and easy to follow.
//   3.  Create Reusable Components: I recognized that the bus write sequence was repetitive. To follow the D.R.Y. (Don't Repeat Yourself)
//       principle, I encapsulated this logic into the `write_to_pll` helper function. This is a key coding ethic that improves
//       maintainability and readability. If the bus protocol were to change, I would only need to update one function, not four separate
//       blocks of code.
//
// How this File Integrates with the System:
// This `pmu_tb.cpp` file provides the implementation for the "master" or "driver" module in the simulation. Its integration with the
// DUT (`pll`) is managed by the top-level `main.cpp` file:
//   - The `pmu_tb`'s output ports (like `reset`, `bus_addr`, `bus_wdata`, `bus_we`) are connected to top-level signals.
//   - The `pll`'s corresponding input ports are connected to those same signals.
//   - Therefore, when a function in this file calls `bus_addr.write(...)`, it is driving a signal that is directly read by the DUT's
//     `bus_process` on the next clock edge.
//   - Conversely, its `pll_locked` input port is connected to the signal being driven by the DUT's `locked` output port. This is how the
//     testbench "sees" the DUT's response.
//
// How I Ensured it Works Correctly:
// The verification of the testbench itself is a meta-process. I ensured it was working correctly by observing its effect on the DUT and the
// overall simulation.
//   1.  Stimulus Verification: I used the VCD waveform to confirm that this testbench was generating the correct stimulus. I could see the
//       `bus_we` pulses and verify that the `bus_addr` and `bus_wdata` signals held the correct values during those pulses, as dictated
//       by the logic in this file.
//   2.  Checker Verification: I intentionally introduced a bug into the `pll.cpp` model (e.g., commenting out the `locked.write(true)` line)
//       and re-ran the simulation. I then verified that this testbench correctly caught the failure and printed the `❌ FAILED!` message,
//       proving that my checking and timeout logic was working as intended.
//





//
// -------------------------------------------------------------------------------------------------------------------------------
// IV. INDUSTRIAL RELEVANCE AND PRACTICAL APPLICATIONS
// -------------------------------------------------------------------------------------------------------------------------------
//
// Where this concept is used in the VLSI Industry:
// The concepts implemented in this `pmu_tb.cpp` file are the bedrock of modern Digital Verification. Every major semiconductor company
// employs teams of verification engineers whose primary job is to create sophisticated testbenches like this one. While this project uses
// C++/SystemC, the exact same principles are used in industry-standard verification languages like SystemVerilog with the Universal
// Verification Methodology (UVM). The 'pmu_tb' is analogous to a UVM 'test' class, and the 'write_to_pll' function is a basic 'driver'.
//
// Practical Applications:
//
//   1.  **Regression Testing:** This test case is a single "directed test". In a real project, there would be hundreds or thousands of
//       similar tests, each targeting a different feature or corner case (e.g., testing different frequencies, testing invalid register writes,
//       testing reset during a lock sequence). These tests are bundled into a "regression suite". Every night, automated servers run this entire
//       suite against the latest version of the RTL design. If a designer makes a change that accidentally breaks an existing feature, the
//       regression test for that feature will fail, and the team is notified immediately. My self-checking testbench (`✅ SUCCESS!` vs.
//       `❌ FAILED!`) is the key component that enables this automation.
//
//   2.  **Coverage-Driven Verification:** While my test is "directed", industrial testbenches also use constrained-random stimulus to try
//       and hit unexpected corner cases. The goal is to achieve 100% "functional coverage," meaning every specified feature has been tested.
//       This testbench could be extended to support this by randomizing the M, N, and OD values within legal ranges and using a more
//       complex checker (a "scoreboard") to predict and verify the results.
//
//   3.  **Bug Reproduction:** When a bug is found, a verification engineer's job is to create the smallest, most targeted test case that can
//       reliably reproduce the failure. This `pmu_tb.cpp` is a perfect example of such a test case. It could be sent to a designer, who can
//       then run this specific test in their own environment to debug the RTL, confident that they are looking at the exact scenario that
//       causes the failure.
//
// Industrially Relevant Insights:
//
//   - Technical Insight: I learned that a good testbench must be more robust than the DUT it is testing. The inclusion of a timeout
//     mechanism in the `wait()` call is a prime example. The testbench must not assume the DUT will work correctly; it must be prepared
//     for failure and handle it gracefully without causing the entire simulation to hang. This is a critical principle in designing
//     industrial-strength verification environments.
//
//   - Non-Technical Insight: This project highlights the economic importance of pre-silicon verification. The cost of finding a bug in a
//     testbench like this is measured in engineer-hours. The cost of finding that same bug after the chip has been manufactured (a "tape-out")
//     can be millions of dollars in mask revisions and product delays. This testbench, and the methodology it represents, is the primary
//     tool used by the industry to mitigate that enormous financial risk.
//




//
// -------------------------------------------------------------------------------------------------------------------------------
// V. DEBUGGING, LEARNINGS, AND ENVIRONMENT
// -------------------------------------------------------------------------------------------------------------------------------
//
// Most Problematic Debug I Faced:
// The most significant C++-specific debug I faced was a "no matching function for call" error that occurred on the main verification line
// in this file. It was a subtle but important API syntax issue.
//
//   - The Symptom: The project failed to compile, and the error pointed directly to the `wait(...)` call intended to monitor the lock signal.
//   - The Investigation: The compiler error log was extremely helpful. It listed all 13 possible "candidate" versions of the `wait()`
//     function that it knew about. My initial code was `wait(pll_locked.posedge_event(), sc_time(20, SC_US));`. I compared this signature
//     (event, time) against the list of candidates provided by the compiler.
//   - Finding the Root Cause: I carefully read the candidate list and found one that looked very similar:
//     `candidate 7: 'void sc_core::sc_module::wait(const sc_core::sc_time&, const sc_core::sc_event&)'`
//     This showed me that while a version of `wait` that takes both a time and an event exists, the SystemC standard requires the arguments
//     to be in a specific order: the time value must come *first*, and the event must come *second*. My arguments were in the wrong order.
//
//   - The Solution: The fix was a simple one-line change in this `pmu_tb.cpp` file. I swapped the arguments in the function call to match
//     the required signature:
//       `wait(sc_time(20, SC_US), pll_locked.posedge_event());`
//     This debug was a valuable lesson in reading API documentation carefully (both from the compiler and from standards) and understanding
//     that function signatures must be matched exactly.
//
// What I Learned From This Project:
//
//   1.  The Power of a Layered Testbench: I learned that abstracting low-level actions (like bus writes) into helper functions (`write_to_pll`)
//       makes the main test sequence (`run_test`) incredibly clean, readable, and easy to modify.
//   2.  Verification Requires Planning: A good test doesn't just happen. It follows a deliberate plan: reset, configure, check, and report.
//       Writing this plan down before coding made the implementation of `run_test` much more straightforward.
//   3.  Testbenches Must Be Robust: The inclusion of a timeout mechanism is non-negotiable. A testbench must be able to gracefully handle
//       a failure in the DUT without getting stuck, which is critical for automated regression environments.
//
// -------------------------------------------------------------------------------------------------------------------------------
// VI. DEVELOPMENT AND EXECUTION ENVIRONMENT
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
//     and the SystemC library together form the "simulator". G++ compiles the C++ code, and the linked SystemC library provides the
//     runtime kernel that executes the simulation.
//   - Build System: GNU Make. I created a custom `Makefile` to automate the entire workflow.
//   - Waveform Viewer: GTKWave. I used this free, standard tool to visualize the `waveform.vcd` output file, which was essential for debugging.
//
//   Execution Flow and Commands:
//   This project was executed entirely from the integrated terminal in VS Code from the root `pll_project` directory on my Desktop.
//   The sequence of commands I used was:
//     1. `set SYSTEMC_HOME=C:\systemc-3.0.1`  (To tell the build system where to find the SystemC library for this session).
//     2. `make clean`                           (To remove any old build artifacts and ensure a fresh build).
//     3. `make`                                 (To compile all .cpp files and link them into the final executable).
//     4. `make run`                             (To execute the compiled simulation program, which then generates the console log and the .vcd file).
//     5. `gtkwave waveform.vcd`                 (Optionally, to launch the waveform viewer with the output file directly from the terminal).
//

