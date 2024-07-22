# Process-Management-System-in-C
A process management system in C, which includes functionalities for creating, managing, and scheduling processes.
Features
- Process Creation: Create processes with specified PID, PPID, size, burst time, priority, and I/O instructions.
- Ready Queues Management: Handle multiple Ready Queues (RQ0, RQ1, RQ2) for different priorities and burst times.
- File Operations: Read processes from a file and write processes to a file in a specific format.
- Process Termination: Terminate processes and update the Ready Queues and file accordingly.
- Process Status Display: Display the status of processes in the Ready Queues.
- Memory Status Display: Display the status of virtual and real memory.
- Fragmentation Display: Display fragmentation information for the Ready Queues.
- Process Scheduling: Schedule processes based on various criteria.
Functions
- writeProccessesToFile: Writes processes from Ready Queues and completed processes to a file.
- ReadProcessesFromFile: Reads processes from a file and creates them in the Ready Queues.
- createProcess: Creates a process and adds it to the appropriate Ready Queue.
- delete_process: Deletes a process from the Ready Queues.
- displayProcessStatus: Displays the status of a specific process.
- display_virtual_memory_status: Displays the status of virtual memory.
- display_real_memory_status: Displays the status of real memory.
- displayFragmentation: Displays fragmentation information for the Ready Queues.
- displayAllProcesses: Displays all processes in the Ready Queues and completed processes.
File Format
The file containing process information should have each process represented by a line containing:

- PID
- PPID
- Size
- Burst Time
- Priority
- I/O Instructions (3 sets of 3 integers)
- Usage
- Compile the Code: Use a C compiler to compile the code.

sh
Copy code
gcc -o process_management_system SourceCode.c
Run the Program: Execute the compiled binary.

sh
Copy code
./process_management_system
Follow the Menu Options: The program provides a menu for different operations such as creating processes, displaying status, scheduling, etc.

Note
- Ensure the file containing process information is in the same directory as the executable.
- The program prints error messages to the console if file operations fail or invalid choices are made.
