#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Define constants
#define REAL_MEMORY_SIZE 20   // Size of real memory in KB
#define VIRTUAL_MEMORY_SIZE 200  // Size of virtual memory in KB
#define PAGE_SIZE 1.0   // Size of each page in KB
#define FRAME_SIZE 1  // Size of each frame in KB

// Constants for virtual memory management
#define ALL_PAGES 220
#define NUM_PAGES (VIRTUAL_MEMORY_SIZE / PAGE_SIZE)
#define NUM_FRAMES (REAL_MEMORY_SIZE / FRAME_SIZE)
#define NUM_REAL_FRAMES (REAL_MEMORY_SIZE / FRAME_SIZE)  // Total number of frames in real memory
#define NUM_VIRTUAL_FRAMES (VIRTUAL_MEMORY_SIZE / FRAME_SIZE)  // Total number of frames in virtual memory

// Structure to represent a page table entry
typedef struct {
    int frame_num;
    int valid;
    int referenced;
    int modified;
    int process_id; // Add this field to store the process ID
    int is_real_memory;
} page_table_entry;

typedef struct {
    int referenced; // Reference bit for Second Chance algorithm
    int modified;
    // Add other fields as needed
} Frame;


// Read process credentials from text file
char filename[] = "processes.txt";

// Define a structure to represent a process
typedef struct Process {
int PID;
int PPID;
float size;
int burst;
int ioInstructions[3][3]; // [instruction][location, nature, duration]
char state[10];
int queuePointer;
int programCounter;
int suspensionTime;
int registers[5];
char completionHistory[100];
int priority; // Add priority member here
page_table_entry pageTable[ALL_PAGES]; // Add pageTable member here
} Process;
int TotalProcesses;

// Define a queue to store processes
typedef struct Queue {
    Process processes[10];
    int front;
    int rear;
    int size;
} Queue;


// Define completedProcesses queue
Queue completedProcesses;
// Create queues for RQ0, RQ1, RQ2, diskIO, printer, and internetIO
Queue RQ0 = {{}, 0, 0, 10};
Queue RQ1 = {{}, 0, 0, 10};
Queue RQ2 = {{}, 0, 0, 10};
Queue diskIO = {{}, 0, 0, 10};
Queue printer = {{}, 0, 0, 10};
Queue internetIO = {{}, 0, 0, 10};


int next_free_real_frame = 0; // Index of the next free frame in real memory to allocate
int next_free_virtual_frame = 0; // Index of the next free frame in virtual memory to allocate

Frame real_frames[NUM_REAL_FRAMES]; // Array to represent frames in real memory
Frame virtual_frames[NUM_VIRTUAL_FRAMES]; // Array to represent frames in virtual memory

// Global arrays to represent the free frame tables for real and virtual memory
int real_free_frame_table[NUM_REAL_FRAMES];
int virtual_free_frame_table[NUM_VIRTUAL_FRAMES];

int empty_indexes_in_real_memory[NUM_REAL_FRAMES]; // Initialize empty array for real memory
int empty_indexes_in_virtual_memory[NUM_VIRTUAL_FRAMES]; // Initialize empty array for virtual memory


// Function to initialize the free frame tables and frames for real and virtual memory
void initialize_memory() {
	
	
    for (int i = 0; i < NUM_REAL_FRAMES; i++) {
        real_free_frame_table[i] = 1; // Mark all frames in real memory as free
        real_frames[i].referenced = 0; // Initialize reference bit for Second Chance algorithm
        // Initialize other fields of the frame structure for real memory as needed
    }
    for (int i = 0; i < NUM_VIRTUAL_FRAMES; i++) {
        virtual_free_frame_table[i] = 1; // Mark all frames in virtual memory as free
        virtual_frames[i].referenced = 0; // Initialize reference bit for Second Chance algorithm
        // Initialize other fields of the frame structure for virtual memory as needed
    }
}

//

// This function is without second chance algorithm.

//// Function to allocate a free frame using Second Chance algorithm for both real and virtual memory
//int allocate_frame(int is_virtual) {
//    int num_frames;
//    int *free_frame_table;
//    Frame *frames;
//
//    // Determine the number of frames and select the appropriate free frame table and frames array
//    if (is_virtual) {
//        num_frames = NUM_VIRTUAL_FRAMES;
//        free_frame_table = virtual_free_frame_table;
//        frames = virtual_frames;
//    } else {
//        num_frames = NUM_REAL_FRAMES;
//        free_frame_table = real_free_frame_table;
//        frames = real_frames;
//    }
//	if((is_virtual == 0) && (next_free_real_frame >= NUM_REAL_FRAMES))
//	{
//		return -1;
//	}
//	if((is_virtual == 1) && (next_free_virtual_frame >= NUM_VIRTUAL_FRAMES))
//	{
//		return -1;
//	}
//    while (1) {
//        // Check if there are any free frames available
//        if (is_virtual == 1 && next_free_virtual_frame < NUM_VIRTUAL_FRAMES) {
//        	int first_value = empty_indexes_in_virtual_memory[0];
//		    // Move all values to the back
//		    for (int i = 0; i < NUM_REAL_FRAMES - 1; i++) {
//		        empty_indexes_in_virtual_memory[i] = empty_indexes_in_virtual_memory[i + 1];
//		    }
//            int frame_num = first_value;
//            next_free_virtual_frame++;
//            free_frame_table[frame_num] = 0; // Mark the frame as allocated
//            return frame_num;
//        } else if (is_virtual == 0 && next_free_real_frame < NUM_REAL_FRAMES) {
//        	int first_value = empty_indexes_in_real_memory[0];
//		    // Move all values to the back
//		    for (int i = 0; i < NUM_REAL_FRAMES - 1; i++) {
//		        empty_indexes_in_real_memory[i] = empty_indexes_in_real_memory[i + 1];
//		    }
//            int frame_num = first_value;
//            next_free_real_frame++;
//            free_frame_table[frame_num] = 0; // Mark the frame as allocated
//            return frame_num;
//        }
//
//        // Iterate through frames to find a victim frame for replacement using Second Chance algorithm
//        for (int i = 0; i < num_frames; i++) {
//            if (frames[i].referenced == 0) {
//                // Found a victim frame, replace it
//                frames[i].referenced = 1; // Set the reference bit to give it a second chance
//                return i; // Return the frame number of the victim frame
//            } else {
//                frames[i].referenced = 0; // Give the frame a second chance by resetting the reference bit
//            }
//        }
//    }
//}

int next_frame_to_replace = 0; // Define and initialize next_frame_to_replace


int allocate_frame(int is_virtual) {
    int num_frames;
    int *free_frame_table;
    Frame *frames;

    // Determine the number of frames and select the appropriate free frame table and frames array
    if (is_virtual) {
        num_frames = NUM_VIRTUAL_FRAMES;
        free_frame_table = virtual_free_frame_table;
        frames = virtual_frames;
    } else {
        num_frames = NUM_REAL_FRAMES;
        free_frame_table = real_free_frame_table;
        frames = real_frames;
    }

    // Check if there are any free frames available
    if (is_virtual && next_free_virtual_frame < NUM_VIRTUAL_FRAMES) {
        int frame_num = empty_indexes_in_virtual_memory[next_free_virtual_frame];
        next_free_virtual_frame++;
        free_frame_table[frame_num] = 0; // Mark the frame as allocated
        return frame_num;
    } else if (!is_virtual && next_free_real_frame < NUM_REAL_FRAMES) {
        int frame_num = empty_indexes_in_real_memory[next_free_real_frame];
        next_free_real_frame++;
        free_frame_table[frame_num] = 0; // Mark the frame as allocated
        return frame_num;
    }
	
	
    // Iterate through frames to find a victim frame for replacement using Second Chance algorithm
    int i;
    for (i = next_frame_to_replace; i < num_frames + next_frame_to_replace; i++) {
        int index = i % num_frames;
        if (frames[index].referenced == 0) {
            // Found a victim frame, replace it
            next_frame_to_replace = i + 1 % num_frames; // Update the next frame to replace
            return index; // Return the frame number of the victim frame
        } else {
            frames[index].referenced = 0; // Give the frame a second chance by resetting the reference bit
        }
    }

    return -1; // No free frame available
}

// Function to create a process
int create_process(Queue *RQ, int PID, int PPID, float size, int burst, int ioInstructions[3][3], int priority) {
	
	int isProcessCreated = 0;
	
	if ((RQ->rear + 1) % RQ->size == RQ->front) {
	    printf("Ready Queue is full. Cannot create process %d.\n", PID);
	    return isProcessCreated;
	}
	
	int isAlreadyPresent = 0;
	for (int i = 0; i < RQ->rear; i++) {
	    if (RQ->processes[i].PID == PID) {
	        isAlreadyPresent = 1;
	    }
	}
	
	if(isAlreadyPresent == 1)
	{
	    printf("\n\nProcess with PID: %d already exists.\n\n", PID);
	    return 2;
	}
	
    // Calculate the number of pages required for the process
    int num_pages = (int)(size / PAGE_SIZE);
    if (fmod(size, PAGE_SIZE) != 0.0) {
        num_pages++; // Round up if the process size is not a multiple of the page size
    }

	Process newProcess;
	newProcess.PID = PID;
	newProcess.PPID = PPID;
	newProcess.size = size;
	newProcess.priority = priority;
	newProcess.burst = burst;
	
	for (int i = 0; i < 3; i++) {
	    for (int j = 0; j < 3; j++) {
	        newProcess.ioInstructions[i][j] = ioInstructions[i][j];
	    }
	}
	
    Process new_process;
    new_process.PID = 1; // Assign a unique process ID
    for (int i = 0; i < ALL_PAGES; i++) {
        new_process.pageTable[i].valid = 0; // Mark all pages as invalid initially
        // Initialize other fields of the page table entry as needed
    }

    float float_pages_in_real_memory = size / 3.0;
    float float_pages_in_virtual_memory = size - float_pages_in_real_memory;
	
    int pages_in_real_memory = 0;
    int pages_in_virtual_memory = 0;
	
    if (float_pages_in_real_memory - (int)float_pages_in_real_memory > 0) {
        pages_in_real_memory = (int)(ceil(float_pages_in_real_memory));
    } else {
        pages_in_real_memory = (int)float_pages_in_real_memory;
    }
    
    
    if (float_pages_in_virtual_memory - (int)float_pages_in_virtual_memory > 0) {
        pages_in_virtual_memory = (int)(ceil(float_pages_in_virtual_memory));
    } else {
        pages_in_virtual_memory = (int)float_pages_in_virtual_memory;
    }
    // Check if real memory is full
    if (next_free_real_frame >= NUM_REAL_FRAMES) {
    	printf("Warning - Insufficient Real memory, assigning Virtual Memory. Thank you for understanding. -\n");
        pages_in_real_memory = 0;
        pages_in_virtual_memory = num_pages;
    }

    // Allocate frames for the process in real memory
    int real_memory_index = 0;
    for (real_memory_index = 0; real_memory_index < pages_in_real_memory; real_memory_index++) {
        int frame_num = allocate_frame(0); // Allocate a frame in real memory
        newProcess.pageTable[real_memory_index].valid = -1;
        newProcess.pageTable[real_memory_index].frame_num = -1;
        if (frame_num != -1) {
            // Update the page table entry for the allocated page
            newProcess.pageTable[real_memory_index].frame_num = frame_num;
            newProcess.pageTable[real_memory_index].valid = 1; // Mark the page as valid
            newProcess.pageTable[real_memory_index].referenced = 0; // Initialize other flags
            newProcess.pageTable[real_memory_index].modified = 0;
            newProcess.pageTable[real_memory_index].process_id = new_process.PID; // Update the process ID
            newProcess.pageTable[real_memory_index].is_real_memory = 1;
            
        } else {
            printf("Warning - Insufficient Real memory assigning remaining memory to Virtual Memory. -\n");
            break;
        }
    }
    	
    // Allocate frames for the process in virtual memory
    for (int l = real_memory_index; l < pages_in_virtual_memory + pages_in_real_memory; l++) {
        int frame_num = allocate_frame(1); // Allocate a frame in virtual memory
   		newProcess.pageTable[l].valid = -1;
        if (frame_num != -1) {
            // Update the page table entry for the allocated page
            newProcess.pageTable[l].frame_num = frame_num;
            newProcess.pageTable[l].valid = 1; // Mark the page as valid
            newProcess.pageTable[l].referenced = 0; // Initialize other flags
            newProcess.pageTable[l].modified = 0;
            newProcess.pageTable[l].process_id = new_process.PID; // Update the process ID
            newProcess.pageTable[l].is_real_memory = 0;
        } else {
            printf("Error: Insufficient Virtual memory to create process.\n");
            return 0;
        }
    }
    
   // printf("\n Virtual Memory Size: %d \n",max_length);


	// Add the new process to the Ready Queue
	RQ->processes[RQ->rear] = newProcess;
	RQ->rear = (RQ->rear + 1) % RQ->size;
	isProcessCreated = 1;
	return isProcessCreated;
    //printf("Process created successfully with PID: %d\n", new_process.PID);
    
}

//// Function to delete a process from the Ready Queue based on PID
int delete_process(Queue *RQ, int PID) {
    int isProcessDeleted = 0;
    
    // Check if the Ready Queue is empty
    if (RQ->front == RQ->rear) {
        return isProcessDeleted;
    }
    
    // Find the process with the given PID and remove it
    for (int i = RQ->front; i != RQ->rear; i = (i + 1) % RQ->size) {
        if (RQ->processes[i].PID == PID) {
            // Process found, remove it by shifting elements in the queue
            int j = i;
            
            isProcessDeleted = 1;
            // Free memory allocated to the process
            int total_real_memory_freed = 0;
            int total_virtual_memory_freed = 0;
            for (int l = 0; l < ALL_PAGES; l++) {
                if (RQ->processes[i].pageTable[l].valid == 1) {
                    int frame_num = RQ->processes[i].pageTable[l].frame_num;
                    int is_real_memory = RQ->processes[i].pageTable[l].is_real_memory;
	
                    // Free the frame based on whether it belongs to real or virtual memory
                    if (is_real_memory == 1 && (frame_num >= 0 && frame_num < NUM_REAL_FRAMES)) {
                        empty_indexes_in_real_memory[next_free_real_frame+NUM_REAL_FRAMES] = frame_num;
                        next_free_real_frame--;
                        // Free the frame from real memory
                        real_free_frame_table[frame_num] = 1;
                        total_real_memory_freed++;
                    } else if(is_real_memory == 0 && (frame_num >= 0 && frame_num < NUM_VIRTUAL_FRAMES)) {
                        empty_indexes_in_virtual_memory[next_free_virtual_frame+NUM_VIRTUAL_FRAMES] = frame_num;
                        next_free_virtual_frame--;
                        // Free the frame from virtual memory
                        virtual_free_frame_table[frame_num] = 1;
                        total_virtual_memory_freed++;
                    }
                }
            }
            
            if(total_real_memory_freed > 0) {
                printf("Total %d real memory freed.\n", total_real_memory_freed);
            }
            
            
            do {
                RQ->processes[j] = RQ->processes[(j + 1) % RQ->size];
                j = (j + 1) % RQ->size;
            } while (j != RQ->rear);
            RQ->rear = (RQ->rear - 1 + RQ->size) % RQ->size; // Decrement rear pointer
            
            break;
        }
    }
    
    return isProcessDeleted;
}


// Function to display the status of both real and virtual memory
void display_virtual_memory_status() {
	
    printf("Virtual Memory Status:\n");
    printf("Total Frames: %d\n", NUM_VIRTUAL_FRAMES);
    printf("Free Frames: %d\n", NUM_VIRTUAL_FRAMES - next_free_virtual_frame);
    printf("Occupied Frames: %d\n", next_free_virtual_frame);
    printf("\n");
    
}

// Function to display the status of both real and virtual memory
void display_real_memory_status() {
	
    printf("Real Memory Status:\n");
    printf("Total Frames: %d\n", NUM_REAL_FRAMES);
    printf("Free Frames: %d\n", NUM_REAL_FRAMES - next_free_real_frame);
    printf("Occupied Frames: %d\n", next_free_real_frame);
    printf("\n");
    
}



//Function Name: writeProccessesToFile
//Parameters:
//	RQ0, RQ1, RQ2: Pointers to Queue structures, representing the Ready Queues.
//	
//completedProcesses: 
//	A pointer to a Queue structure, representing the Completed Processes queue.
//	
//Description:
//	The writeProccessesToFile function writes the processes from the Ready Queues and the Completed Processes queue to a file named "processes.txt". 
//	The file is opened in write mode, and the processes are written in a specific format, including their PID, PPID, size, burst time, priority, and I/O instructions.
//Algorithm:
//	-> Open the file "processes.txt" in write mode.
//	-> If the file is opened successfully, write the total number of processes to the file.
//	-> Iterate over each process in the Ready Queues and the Completed Processes queue.
//	-> For each process, write its PID, PPID, size, burst time, priority, and I/O instructions to the file.
//	-> Close the file.
//Note:
//	-> The TotalProcesses variable is assumed to keep track of the total number of processes.
//	-> The ioInstructions array is assumed to store the I/O instructions for each process.
//	-> The file "processes.txt" is assumed to be in the same directory as the executable.
//	-> If the file cannot be opened, an error message is printed to the console.

void writeProccessesToFile(Queue *RQ0, Queue *RQ1, Queue *RQ2, Queue *completedProcesses)
{
	FILE *file = fopen("processes.txt", "w");
	fprintf(file, "%d", TotalProcesses);
	
    if (file != NULL) {
        
		for (int i = RQ0->front; i != RQ0->rear; i = (i + 1) % RQ0->size) {
			fprintf(file, "\n%d %d %f %d %d", RQ0->processes[i].PID, RQ0->processes[i].PPID, RQ0->processes[i].size, RQ0->processes[i].burst,RQ0->processes[i].priority);
			int IOinstructions[3][3] = {{RQ0->processes[i].ioInstructions[0][0], RQ0->processes[i].ioInstructions[0][1], RQ0->processes[i].ioInstructions[0][2]},
                           {RQ0->processes[i].ioInstructions[1][0], RQ0->processes[i].ioInstructions[1][1], RQ0->processes[i].ioInstructions[1][2]},
                           {RQ0->processes[i].ioInstructions[2][0], RQ0->processes[i].ioInstructions[2][1], RQ0->processes[i].ioInstructions[2][2]}};

	        for (int i = 0; i < 3; i++) {
	        	fprintf(file, "\n%d %d %d", IOinstructions[i][0], IOinstructions[i][1], IOinstructions[i][2]);
			}
		}
	    for (int i = RQ1->front; i != RQ1->rear; i = (i + 1) % RQ1->size) {
			fprintf(file, "\n%d %d %f %d %d", RQ1->processes[i].PID, RQ1->processes[i].PPID, RQ1->processes[i].size, RQ1->processes[i].burst, RQ1->processes[i].priority);
			int IOinstructions[3][3] = {{RQ1->processes[i].ioInstructions[0][0], RQ1->processes[i].ioInstructions[0][1], RQ1->processes[i].ioInstructions[0][2]},
                           {RQ1->processes[i].ioInstructions[1][0], RQ1->processes[i].ioInstructions[1][1], RQ1->processes[i].ioInstructions[1][2]},
                           {RQ1->processes[i].ioInstructions[2][0], RQ1->processes[i].ioInstructions[2][1], RQ1->processes[i].ioInstructions[2][2]}};

		    for (int i = 0; i < 3; i++) {
		    	fprintf(file, "\n%d %d %d", IOinstructions[i][0], IOinstructions[i][1], IOinstructions[i][2]);
			}
		}
	    for (int i = RQ2->front; i != RQ2->rear; i = (i + 1) % RQ2->size) {
			fprintf(file, "\n%d %d %f %d %d", RQ2->processes[i].PID, RQ2->processes[i].PPID, RQ2->processes[i].size, RQ2->processes[i].burst, RQ2->processes[i].priority);
			int IOinstructions[3][3] = {{RQ2->processes[i].ioInstructions[0][0], RQ2->processes[i].ioInstructions[0][1], RQ2->processes[i].ioInstructions[0][2]},
                           {RQ2->processes[i].ioInstructions[1][0], RQ2->processes[i].ioInstructions[1][1], RQ2->processes[i].ioInstructions[1][2]},
                           {RQ2->processes[i].ioInstructions[2][0], RQ2->processes[i].ioInstructions[2][1], RQ2->processes[i].ioInstructions[2][2]}};

	        for (int i = 0; i < 3; i++) {
	        	fprintf(file, "\n%d %d %d", IOinstructions[i][0], IOinstructions[i][1], IOinstructions[i][2]);
			}
		}
	    for (int i = completedProcesses->front; i != completedProcesses->rear; i = (i + 1) % completedProcesses->size) {
			fprintf(file, "\n%d %d %f %d %d", completedProcesses->processes[i].PID, completedProcesses->processes[i].PPID, completedProcesses->processes[i].size, completedProcesses->processes[i].burst, completedProcesses->processes[i].priority);
			int IOinstructions[3][3] = {{completedProcesses->processes[i].ioInstructions[0][0], completedProcesses->processes[i].ioInstructions[0][1], completedProcesses->processes[i].ioInstructions[0][2]},
                           {completedProcesses->processes[i].ioInstructions[1][0], completedProcesses->processes[i].ioInstructions[1][1], completedProcesses->processes[i].ioInstructions[1][2]},
                           {completedProcesses->processes[i].ioInstructions[2][0], completedProcesses->processes[i].ioInstructions[2][1], completedProcesses->processes[i].ioInstructions[2][2]}};

	        for (int i = 0; i < 3; i++) {
	        	fprintf(file, "\n%d %d %d", IOinstructions[i][0], IOinstructions[i][1], IOinstructions[i][2]);
			}
		}
        fclose(file);
    } else {
        printf("Error opening file for appending!\n");
    }
}

//Function Name: ReadProcessesFromFile
//Parameters:
//	filename: A string representing the name of the file to read from.
//	
//Description:
//	The ReadProcessesFromFile function reads processes from a file and creates them in the Ready Queues (RQ0, RQ1, RQ2). 
//	The file is assumed to be in a specific format, with each process represented by a line containing its PID, PPID, size, burst time, priority, and I/O instructions.
//	
//Algorithm:
//	-> Open the file in read mode.
//	-> If the file is opened successfully, read the total number of processes (N) from the file.
//	-> Set the TotalProcesses variable to N.
//	-> For each process (from 0 to N-1):
//	-> Read the PID, PPID, size, burst time, and priority from the file.
//	-> Read the I/O instructions (3 sets of 3 integers) from the file.
//	-> Based on the burst time and priority, create the process in the appropriate Ready Queue (RQ0, RQ1, or RQ2) using the createProcess function.
//	-> Close the file.
//	
//Note:
//	-> The file is assumed to be in the same directory as the executable.
//	-> If the file cannot be opened, an error message is printed to the console.
//	-> The createProcess function is assumed to be defined elsewhere in the program.
//	-> The Ready Queues (RQ0, RQ1, RQ2) are assumed to be defined elsewhere in the program.
void ReadProcessesFromFile(const char *filename)
{
	FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }

    int N;
    fscanf(file, "%d", &N);
	TotalProcesses = N;
 	for (int i = 0; i < N; i++) {
	    int PID, PPID, burst, priority;
	    float size;
	    fscanf(file, "%d %d %f %d %d", &PID, &PPID, &size, &burst, &priority);
	    int ioInstructions[3][3];
	    for (int j = 0; j < 3; j++) {
	        fscanf(file, "%d %d %d", &ioInstructions[j][0], &ioInstructions[j][1], &ioInstructions[j][2]);
	    }
	    if(burst <= 4)
	    {
	        create_process(&RQ0, PID, PPID, size, burst, ioInstructions, priority);
	    }
	    else if(priority == 0)
	    {
	        create_process(&RQ1, PID, PPID, size, burst, ioInstructions, priority);
	    }
	    else if(priority == 1)
	    {
			create_process(&RQ2, PID, PPID, size, burst, ioInstructions, priority);
	    }
	    
	}	
    fclose(file);
}


void displayAllProcesses(Queue *RQ0, Queue *RQ1, Queue *RQ2, Queue *completedProcesses) {
	printf("\n");
    printf("Ready Queue 0:\n");
    printf("\n");
    for (int i = RQ0->front; i != RQ0->rear; i = (i + 1) % RQ0->size) {
        printf("Process %d: PID=%d, PPID=%d, Size=%f, Burst=%d,State = %s, Completion History = %s\n", i, RQ0->processes[i].PID, RQ0->processes[i].PPID, RQ0->processes[i].size, RQ0->processes[i].burst,RQ0->processes[i].state, RQ0->processes[i].completionHistory);
    	
	}
    printf("Ready Queue 1:\n");
	printf("\n");
    for (int i = RQ1->front; i != RQ1->rear; i = (i + 1) % RQ1->size) {

        printf("Process %d: PID=%d, PPID=%d, Size=%f, Burst=%d,State = %s, Completion History = %s\n", i, RQ1->processes[i].PID, RQ1->processes[i].PPID, RQ1->processes[i].size, RQ1->processes[i].burst,RQ1->processes[i].state, RQ1->processes[i].completionHistory);

	}

    printf("Ready Queue 2:\n");
	printf("\n");
    for (int i = RQ2->front; i != RQ2->rear; i = (i + 1) % RQ2->size) {

        printf("Process %d: PID=%d, PPID=%d, Size=%f, Burst=%d,State = %s, Completion History = %s\n", i, RQ2->processes[i].PID, RQ2->processes[i].PPID, RQ2->processes[i].size, RQ2->processes[i].burst,RQ2->processes[i].state, RQ2->processes[i].completionHistory);

	}

    printf("Completed Processes:\n");
	printf("\n");
    for (int i = completedProcesses->front; i != completedProcesses->rear; i = (i + 1) % completedProcesses->size) {

        printf("Process %d: PID=%d, PPID=%d, Size=%f, Burst=%d,State = %s, Completion History = %s\n", i, completedProcesses->processes[i].PID, completedProcesses->processes[i].PPID, completedProcesses->processes[i].size, completedProcesses->processes[i].burst, completedProcesses->processes[i].completionHistory);

	}
	printf("\n");
}

void displayFragmentation(Queue *processQueue) {
    if (processQueue->front == processQueue->rear) {
        printf("No processes in the queue.\n");
        return;
    }

    float totalFragmentation = 0;
    float totalAllocatedForRealMemory = 0;

    printf("\n");

    for (int i = processQueue->front; i != processQueue->rear; i = (i + 1) % processQueue->size) {
        int allocatedSize = 0;
        
        // Calculate allocated size for real memory
        for (int j = 0; j < ALL_PAGES; j++) {
            if (processQueue->processes[i].pageTable[j].valid == 1 && processQueue->processes[i].pageTable[j].is_real_memory == 1) {
                allocatedSize++;
            }
        }

        // Calculate real memory size (assuming 1/3 of total process size)
        float realMemorySize = processQueue->processes[i].size / 3.0;
        
        // Calculate fragmentation
        float fragmentation = allocatedSize - realMemorySize;
        totalAllocatedForRealMemory += allocatedSize;

        printf("\nFrame %d: Process ID: %d, Allocated: %d KB, Fragmentation: %.2f KB", NUM_FRAMES, processQueue->processes[i].PID, allocatedSize, fragmentation > 0 ? fragmentation : 0.0);
        
        // Accumulate total fragmentation
        if (allocatedSize != 0) {
            totalFragmentation += fragmentation; 
        }
    }
    
    printf("\n\nTotal Allocated: %.2f KB, Total Fragmentation: %.2f KB\n", totalAllocatedForRealMemory, totalFragmentation);
}

void displayProcessStatus(Queue *RQ0, Queue *RQ1, Queue *RQ2, Queue *completedProcesses, int PID) {
	printf("\n");
	int isProcessFound = 0;
    printf("Process %d Segment Table:\n", PID);
    printf("\n");
    printf("Ready Queue 0:\n");
    printf("\n");
    for (int i = RQ0->front; i != RQ0->rear; i = (i + 1) % RQ0->size) {
        if (RQ0->processes[i].PID == PID) {
            printf("Process %d Segment Table:\n", PID);
            printf("Segment #  Base  Limit\n");
            for (int j = 0; j < RQ0->processes[i].size; j++) {
                printf("%d       %d    %d\n", j, RQ0->processes[i].pageTable[j].frame_num * 1024, (RQ0->processes[i].pageTable[j].frame_num + 1) * 1024);
            }

            printf("Process %d Page Table:\n", PID);
            printf("Page #  Frame #  Valid  Referenced  Modified\n");
            for (int j = 0; j < RQ0->processes[i].size; j++) {
                printf("%d       %d      %d       %d         %d\n", j, RQ0->processes[i].pageTable[j].frame_num, RQ0->processes[i].pageTable[j].valid, RQ0->processes[i].pageTable[j].referenced, RQ0->processes[i].pageTable[j].modified);
            }
			isProcessFound = 1;
        }
	}
	
	if(isProcessFound == 1)
	{
		return;
	}
	
    printf("Ready Queue 1:\n");
	printf("\n");
    for (int i = RQ1->front; i != RQ1->rear; i = (i + 1) % RQ1->size) {
		if (RQ1->processes[i].PID == PID) {
		    printf("Process %d Segment Table:\n", PID);
		    printf("Segment #  Base  Limit\n");
		    for (int j = 0; j < RQ1->processes[i].size; j++) {
		        printf("%d       %d    %d\n", j, RQ1->processes[i].pageTable[j].frame_num * 1024, (RQ1->processes[i].pageTable[j].frame_num + 1) * 1024);
		    }
		
		    printf("Process %d Page Table:\n", PID);
		    printf("Page #  Frame #  Valid  Referenced  Modified\n");
		    for (int j = 0; j < RQ1->processes[i].size; j++) {
		        printf("%d       %d      %d       %d         %d\n", j, RQ1->processes[i].pageTable[j].frame_num, RQ1->processes[i].pageTable[j].valid, RQ1->processes[i].pageTable[j].referenced, RQ1->processes[i].pageTable[j].modified);
		    }
			isProcessFound = 1;
		}
	}

	if(isProcessFound == 1)
	{
		return;
	}
    printf("Ready Queue 2:\n");
	printf("\n");
    for (int i = RQ2->front; i != RQ2->rear; i = (i + 1) % RQ2->size) {
		if (RQ2->processes[i].PID == PID) {
		    printf("Process %d Segment Table:\n", PID);
		    printf("Segment #  Base  Limit\n");
		    for (int j = 0; j < RQ2->processes[i].size; j++) {
		        printf("%d       %d    %d\n", j, RQ2->processes[i].pageTable[j].frame_num * 1024, (RQ2->processes[i].pageTable[j].frame_num + 1) * 1024);
		    }
		
		    printf("Process %d Page Table:\n", PID);
		    printf("Page #  Frame #  Valid  Referenced  Modified\n");
		    for (int j = 0; j < RQ2->processes[i].size; j++) {
		        printf("%d       %d      %d       %d         %d\n", j, RQ2->processes[i].pageTable[j].frame_num, RQ2->processes[i].pageTable[j].valid, RQ2->processes[i].pageTable[j].referenced, RQ2->processes[i].pageTable[j].modified);
		    }
			isProcessFound = 1;
		}
	}

	if(isProcessFound == 1)
	{
		return;
	}
    printf("Completed Processes:\n");
	printf("\n");
    for (int i = completedProcesses->front; i != completedProcesses->rear; i = (i + 1) % completedProcesses->size) {
		if (completedProcesses->processes[i].PID == PID) {
		    printf("Process %d Segment Table:\n", PID);
		    printf("Segment #  Base  Limit\n");
		    for (int j = 0; j < completedProcesses->processes[i].size; j++) {
		        printf("%d       %d    %d\n", j, completedProcesses->processes[i].pageTable[j].frame_num * 1024, (completedProcesses->processes[i].pageTable[j].frame_num + 1) * 1024);
		    }
		
		    printf("Process %d Page Table:\n", PID);
		    printf("Page #  Frame #  Valid  Referenced  Modified\n");
		    for (int j = 0; j < completedProcesses->processes[i].size ; j++) {
		        printf("%d       %d      %d       %d         %d\n", j, completedProcesses->processes[i].pageTable[j].frame_num, completedProcesses->processes[i].pageTable[j].valid, completedProcesses->processes[i].pageTable[j].referenced, completedProcesses->processes[i].pageTable[j].modified);
		    }
			isProcessFound = 1;
		}
	}
	
	if(isProcessFound == 1)
	{
		return;
	}
	else
	{
		printf("No Process found\n");
	}
}


int main()
{
	for (int i = 0; i < NUM_REAL_FRAMES; i++) {
    	empty_indexes_in_real_memory[i] = i;
    }
    for (int i = 0; i < NUM_VIRTUAL_FRAMES; i++) {
    	empty_indexes_in_virtual_memory[i] = i;
    }
    // Initialize memory
    initialize_memory();
    
	
    ReadProcessesFromFile(filename);
	// Example usage of create_process function
	
    
    
	int priority;
    printf("--------------------------------------------------------------------------------------------------------\n");
    printf("-                                                               										-\n");
    printf("-     P R O C E S S    M A N A G E R    W I T H    V I R T U A L    M E M O R Y    M A N A G E M E N T  -\n");
    printf("-                                                               										-\n");
    printf("--------------------------------------------------------------------------------------------------------\n");

    while (1) {
    	
    	
	    printf("\n\n----------------------Choices For User---------------------------\n");
	    printf("\n");
	    printf("CHOICE : 1 -----------  Create a Process\n");
	    printf("CHOICE : 2 -----------  Delete a Process\n");
	    printf("CHOICE : 3 -----------  Display process status\n");
	    printf("CHOICE : 4 -----------  Virtual memory status\n");
	    printf("CHOICE : 5 -----------  Real memory status\n");
	    printf("CHOICE : 6 -----------  Fragmentation\n");
	    printf("CHOICE : 7 -----------  Display All Processes\n");
	    printf("CHOICE : 0 -----------  EXIT\n");
	    printf("\n");
	    printf("----------------------ENTER YOUR CHOICE--------------------------\n");
       
        int choice;
        printf("Choice: ");
        scanf("%d", &choice);
		int PID, PPID, burst, ioInstructions[3][3];
		float size;
		for (int i = 0; i < 3; i++) {
		    for (int j = 0; j < 3; j++) {
		        ioInstructions[i][j] = -1;
		    }
		}

        int terminatePID;
        int processPID;
		int suspendPID;
		int resumePID;
		int suspensionTime;
         
		if(choice == 1)
		{
			// Create Process
	                
//	        printf("Enter PID: ");
//	        scanf("%d", &PID);
//	        printf("Enter PPID: ");
//	        scanf("%d", &PPID);

           int PID, PPID;

    do {
        printf("Enter PID: ");
        scanf("%d", &PID);
        printf("Enter PPID: ");
        scanf("%d", &PPID);

        if (PID < 0 || PPID < 0) {
            printf("PID and PPID cannot be negative.\n");
        } else if (PPID < PID) {
            printf("PPID cannot be smaller than PID.\n");
        } else {
            printf("PID: %d, PPID: %d\n", PID, PPID);
            // Further code execution can follow here
            break; // Break out of the loop if valid values are provided
        }
    } while (1); // Loop indefinitely until valid values are provided
	        printf("Enter size in KB's (eg. 20.0) : ");
	        scanf("%f", &size);
	        // Input validation
		    while (size <= 0 || size > 200.0) { // Assuming a reasonable upper limit of 200KB
			    if (size <= 0) {
			        printf("Size cannot be zero or negative. Please enter a positive value: ");
			    } else if (size > 200.0) {
			        printf("Size is too large. Please enter a value not exceeding 200KB: ");
			    }
			    scanf("%f", &size);
			}

	        do {
        printf("Enter burst time: ");
        scanf("%d", &burst);

        if (burst < 0 || burst > 5000) {
            printf("Burst time must be non-negative and not greater than 5000.\n");
        } else {
            printf("Burst time: %d\n", burst);
            // Further code execution can follow here
            break; // Break out of the loop if valid value is provided
        }
    } while (1); // Loop indefinitely until valid value is provided
	       do {
        printf("Enter priority (0 for low, 1 for high): ");
        scanf("%d", &priority);

        if (priority != 0 && priority != 1) {
            printf("Invalid priority. Please enter 0 for low priority or 1 for high priority.\n");
        } else {
            printf("Priority: %d\n", priority);
            // Further code execution can follow here
            break; // Break out of the loop if valid value is provided
        }
    } while (1); 
	        printf("--Enter I/O instructions for the process--\n");
			for (int i = 0; i < 3; i++) {
			    printf("I/O Instruction %d:\n", i + 1);
			    
			    // Input location
			    printf("Enter location for I/O instruction %d: ", i + 1);
			    scanf("%d", &ioInstructions[i][0]);
			    
			    // Input nature
			    printf("Enter nature for I/O instruction %d (0 for disk IO, 1 for printer, 2 for internet IO, -1 to exit): ", i + 1);
			    scanf("%d", &ioInstructions[i][1]);
			    
			    if(ioInstructions[i][1] == -1)
			    {
			        break; // Exit the loop
			    }
			    
			    if(ioInstructions[i][1] != 0 && ioInstructions[i][1] != 1 && ioInstructions[i][1] != 2)
			    {
			        printf("Invalid nature. Please enter a valid value (0, 1, or 2).\n");
			        continue; // Skip to the next iteration
			    }
			    
			    // Input duration
			    printf("Enter duration for I/O instruction %d (in seconds): ", i + 1);
			    scanf("%d", &ioInstructions[i][2]);
			}
	        printf("--End entering I/O instructions for the process--\n");
	        
	        int isProcessCreated = 0;
        
			// Prompt for the target queue selection
			if(burst <= 4)
		    {
		        isProcessCreated = create_process(&RQ0, PID, PPID, size, burst, ioInstructions, priority);
		    }
		    else if(priority == 0)
		    {
		        isProcessCreated = create_process(&RQ1, PID, PPID, size, burst, ioInstructions, priority);
		    }
		    else if(priority == 1)
		    {
				isProcessCreated = create_process(&RQ2, PID, PPID, size, burst, ioInstructions, priority);
			}
	       // createProcess(&RQ0, PID, PPID, size, burst, ioInstructions, 0);
	        if(isProcessCreated == 1)
	        {
				TotalProcesses = TotalProcesses+1;
	    		writeProccessesToFile(&RQ0,&RQ1,&RQ2,&completedProcesses);
			}
			else if(isProcessCreated == 0)
			{
				printf("\n\nError: Not enough memory to create process %d.\n\n", PID);
			}
		}
		else if(choice == 2)
		{
	        // Terminate Process
	        printf("Enter PID of process to Delete: ");
	        scanf("%d", &terminatePID);
	        int isProcessFound = 0;
	        isProcessFound = delete_process(&RQ0, terminatePID);
	        printf("\nRQ0 PID: %d ",terminatePID);
	        if(isProcessFound == 0)
	        {
	        	isProcessFound = delete_process(&RQ1, terminatePID);
	        	printf("\nRQ1 PID: %d ",isProcessFound);
	        	if(isProcessFound == 0)
	        	{
	        		isProcessFound = delete_process(&RQ2, terminatePID);
	        		printf("\nRQ2 PID: %d ",isProcessFound);
	        		if(isProcessFound == 0)
	        		{
	        			printf("\nNo Process found with PID:%d\n\n",terminatePID);
					}
					else if (isProcessFound == 1)
				    {
				    	TotalProcesses = TotalProcesses - 1;
				    	writeProccessesToFile(&RQ0,&RQ1,&RQ2,&completedProcesses);
					}
				}
				else if (isProcessFound == 1)
			    {
			    	TotalProcesses = TotalProcesses - 1;
			    	writeProccessesToFile(&RQ0,&RQ1,&RQ2,&completedProcesses);
				}
			}
			else if (isProcessFound == 1)
		    {
		    	TotalProcesses = TotalProcesses - 1;
		    	writeProccessesToFile(&RQ0,&RQ1,&RQ2,&completedProcesses);
			}
		}
		else if(choice == 3)
		{
	        // Display process status
	        printf("Enter PID of Process: ");
			scanf("%d", &processPID);
			displayProcessStatus(&RQ0,&RQ1,&RQ2,&completedProcesses,processPID);
		}
		else if(choice == 4)
		{
	        // Resume Process
	        display_virtual_memory_status();
		}
		else if(choice == 5)
		{
	        // Schedule Processes
	        display_real_memory_status();
		}
		else if(choice == 6)
		{
	        displayFragmentation(&RQ0);
	        displayFragmentation(&RQ1);
	        displayFragmentation(&RQ2);
		}
		else if (choice == 7)
		{
			displayAllProcesses(&RQ0,&RQ1,&RQ2,&completedProcesses);
		}
	    else if (choice == 0)
		{
	        // Exit
	        return 0;
		}
		else
		{
			printf("Enter Valid Choice");
		}
	}
    return 0;
}