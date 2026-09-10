#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PROCESSES 20
#define MAX_FILES 20
#define MAX_MEMORY_BLOCKS 32
#define MAX_FILE_CONTENT 256
#define RR_QUANTUM 2

enum ProcessState { NEW, READY, RUNNING, WAITING, TERMINATED };

typedef struct {
    int pid;
    char name[50];
    enum ProcessState state;
    int priority;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int first_start;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int response_time;
    int active;
} Process;

typedef struct {
    char name[64];
    char content[MAX_FILE_CONTENT];
    int size;
    int used;
} SimFile;

typedef struct {
    int used;
    int owner_pid;
} MemoryBlock;

Process processTable[MAX_PROCESSES];
SimFile fileTable[MAX_FILES];
MemoryBlock memory[MAX_MEMORY_BLOCKS];
int nextPID = 1;
int currentTime = 0;

const char *stateString(enum ProcessState state) {
    switch (state) {
        case NEW: return "NEW";
        case READY: return "READY";
        case RUNNING: return "RUNNING";
        case WAITING: return "WAITING";
        case TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

void resetProcesses(void) {
    memset(processTable, 0, sizeof(processTable));
    nextPID = 1;
    currentTime = 0;
}

void defaultsForApp(const char *name, int *priority, int *burst) {
    if (strcmp(name, "Finder") == 0) { *priority = 4; *burst = 4; }
    else if (strcmp(name, "Safari") == 0) { *priority = 3; *burst = 7; }
    else if (strcmp(name, "Terminal") == 0) { *priority = 5; *burst = 3; }
    else if (strcmp(name, "Music") == 0) { *priority = 2; *burst = 5; }
    else if (strcmp(name, "Activity Monitor") == 0) { *priority = 5; *burst = 2; }
    else { *priority = 3; *burst = 4; }
}

int createProcess(const char *name, int arrivalTime) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!processTable[i].active) {
            Process *p = &processTable[i];
            int priority, burst;
            defaultsForApp(name, &priority, &burst);
            p->pid = nextPID++;
            strncpy(p->name, name, sizeof(p->name) - 1);
            p->name[sizeof(p->name) - 1] = '\0';
            p->state = NEW;
            p->priority = priority;
            p->arrival_time = arrivalTime;
            p->burst_time = burst;
            p->remaining_time = burst;
            p->first_start = -1;
            p->active = 1;
            p->state = READY;
            return p->pid;
        }
    }
    return -1;
}

void displayProcessTable(void) {
    printf("\n================ PROCESS TABLE ================\n");
    printf("PID  NAME                 STATE       ARR  PRI  BURST  REM\n");
    printf("-----------------------------------------------------------\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processTable[i].active) {
            Process *p = &processTable[i];
            printf("%-4d %-20s %-11s %-4d %-4d %-6d %-4d\n",
                   p->pid, p->name, stateString(p->state), p->arrival_time,
                   p->priority, p->burst_time, p->remaining_time);
        }
    }
    printf("===========================================================\n");
}

void resetMetrics(void) {
    currentTime = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processTable[i].active) {
            Process *p = &processTable[i];
            p->state = READY;
            p->remaining_time = p->burst_time;
            p->first_start = -1;
            p->completion_time = 0;
            p->waiting_time = 0;
            p->turnaround_time = 0;
            p->response_time = 0;
        }
    }
}

int activeCount(void) {
    int n = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) if (processTable[i].active) n++;
    return n;
}

int allFinished(void) {
    for (int i = 0; i < MAX_PROCESSES; i++)
        if (processTable[i].active && processTable[i].state != TERMINATED) return 0;
    return 1;
}

void advanceToNextArrival(void) {
    int next = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processTable[i].active && processTable[i].state == READY && processTable[i].arrival_time > currentTime) {
            if (next == -1 || processTable[i].arrival_time < next) next = processTable[i].arrival_time;
        }
    }
    if (next > currentTime) {
        printf("Time %d-%d: CPU IDLE (waiting for next process)\n", currentTime, next);
        currentTime = next;
    }
}

void calculateMetrics(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!processTable[i].active || processTable[i].completion_time == 0) continue;
        Process *p = &processTable[i];
        p->turnaround_time = p->completion_time - p->arrival_time;
        p->waiting_time = p->turnaround_time - p->burst_time;
        p->response_time = p->first_start - p->arrival_time;
    }
}

void printMetrics(void) {
    double aw = 0, at = 0, ar = 0;
    int n = activeCount();
    printf("\n================ SCHEDULING METRICS ================\n");
    printf("PID  NAME                 WAIT  TURNAROUND  RESPONSE\n");
    printf("----------------------------------------------------\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processTable[i].active) {
            Process *p = &processTable[i];
            printf("%-4d %-20s %-5d %-11d %-8d\n", p->pid, p->name,
                   p->waiting_time, p->turnaround_time, p->response_time);
            aw += p->waiting_time; at += p->turnaround_time; ar += p->response_time;
        }
    }
    if (n) printf("Average: waiting=%.2f, turnaround=%.2f, response=%.2f\n", aw/n, at/n, ar/n);
    printf("====================================================\n");
}

void runFor(Process *p, int units) {
    if (p->first_start == -1) p->first_start = currentTime;
    p->state = RUNNING;
    for (int i = 0; i < units && p->remaining_time > 0; i++) {
        p->remaining_time--;
        currentTime++;
    }
    if (p->remaining_time == 0) {
        p->state = TERMINATED;
        p->completion_time = currentTime;
        printf("Time %d: %-20s TERMINATED\n", currentTime, p->name);
    } else {
        p->state = READY;
    }
}

int chooseFCFS(void) {
    int s = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        Process *p = &processTable[i];
        if (p->active && p->state == READY && p->arrival_time <= currentTime) {
            if (s == -1 || p->arrival_time < processTable[s].arrival_time ||
                (p->arrival_time == processTable[s].arrival_time && p->pid < processTable[s].pid)) s = i;
        }
    }
    return s;
}

int chooseSJF(void) {
    int s = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        Process *p = &processTable[i];
        if (p->active && p->state == READY && p->arrival_time <= currentTime) {
            if (s == -1 || p->burst_time < processTable[s].burst_time ||
                (p->burst_time == processTable[s].burst_time && p->arrival_time < processTable[s].arrival_time)) s = i;
        }
    }
    return s;
}

int chooseSRTF(void) {
    int s = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        Process *p = &processTable[i];
        if (p->active && p->state == READY && p->arrival_time <= currentTime) {
            if (s == -1 || p->remaining_time < processTable[s].remaining_time ||
                (p->remaining_time == processTable[s].remaining_time && p->arrival_time < processTable[s].arrival_time)) s = i;
        }
    }
    return s;
}

int choosePriority(void) {
    int s = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        Process *p = &processTable[i];
        if (p->active && p->state == READY && p->arrival_time <= currentTime) {
            if (s == -1 || p->priority > processTable[s].priority ||
                (p->priority == processTable[s].priority && p->arrival_time < processTable[s].arrival_time)) s = i;
        }
    }
    return s;
}

void runNonPreemptive(const char *label, int (*chooser)(void)) {
    printf("\n===== %s =====\n", label);
    resetMetrics();
    while (!allFinished()) {
        int s = chooser();
        if (s == -1) { advanceToNextArrival(); continue; }
        Process *p = &processTable[s];
        printf("Time %d: %s (PID %d) selected\n", currentTime, p->name, p->pid);
        runFor(p, p->remaining_time);
    }
    calculateMetrics();
    printMetrics();
}

void runSRTF(void) {
    printf("\n===== SRTF (PREEMPTIVE SJF) =====\n");
    resetMetrics();
    while (!allFinished()) {
        int s = chooseSRTF();
        if (s == -1) { advanceToNextArrival(); continue; }
        Process *p = &processTable[s];
        printf("Time %d: %s (PID %d) selected for 1 unit\n", currentTime, p->name, p->pid);
        runFor(p, 1);
    }
    calculateMetrics();
    printMetrics();
}

void runRoundRobin(void) {
    printf("\n===== ROUND ROBIN (QUANTUM %d) =====\n", RR_QUANTUM);
    resetMetrics();
    while (!allFinished()) {
        int progressed = 0;
        for (int i = 0; i < MAX_PROCESSES; i++) {
            Process *p = &processTable[i];
            if (p->active && p->state == READY && p->arrival_time <= currentTime) {
                progressed = 1;
                printf("Time %d: %s (PID %d) gets quantum\n", currentTime, p->name, p->pid);
                runFor(p, RR_QUANTUM);
            }
        }
        if (!progressed) advanceToNextArrival();
    }
    calculateMetrics();
    printMetrics();
}

void schedulerMenu(void) {
    if (!activeCount()) {
        printf("No processes available.\n");
        return;
    }
    int choice;
    printf("\nScheduling Algorithms\n");
    printf("1. FCFS\n2. SJF\n3. SRTF\n4. Priority\n5. Round Robin\n6. Run All\n0. Back\n");
    printf("Enter choice: ");
    scanf("%d", &choice);
    switch (choice) {
        case 1: runNonPreemptive("FCFS", chooseFCFS); break;
        case 2: runNonPreemptive("SJF", chooseSJF); break;
        case 3: runSRTF(); break;
        case 4: runNonPreemptive("PRIORITY", choosePriority); break;
        case 5: runRoundRobin(); break;
        case 6:
            runNonPreemptive("FCFS", chooseFCFS);
            runNonPreemptive("SJF", chooseSJF);
            runSRTF();
            runNonPreemptive("PRIORITY", choosePriority);
            runRoundRobin();
            break;
        default: break;
    }
}

void memoryMenu(void) {
    int choice, blocks, pid;
    while (1) {
        printf("\nMemory Manager\n1. Allocate memory\n2. Free process memory\n3. Show memory\n0. Back\nChoice: ");
        scanf("%d", &choice);
        if (choice == 0) return;
        if (choice == 1) {
            printf("PID: "); scanf("%d", &pid);
            printf("Blocks required: "); scanf("%d", &blocks);
            int freeCount = 0;
            for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) if (!memory[i].used) freeCount++;
            if (blocks <= 0 || blocks > freeCount) { printf("Allocation failed. Free blocks: %d\n", freeCount); continue; }
            int done = 0;
            for (int i = 0; i < MAX_MEMORY_BLOCKS && done < blocks; i++) if (!memory[i].used) { memory[i].used = 1; memory[i].owner_pid = pid; done++; }
            printf("Allocated %d blocks to PID %d.\n", blocks, pid);
        } else if (choice == 2) {
            printf("PID: "); scanf("%d", &pid);
            int freed = 0;
            for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) if (memory[i].used && memory[i].owner_pid == pid) { memory[i].used = 0; memory[i].owner_pid = 0; freed++; }
            printf("Freed %d blocks.\n", freed);
        } else if (choice == 3) {
            printf("\nMemory blocks: ");
            for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) printf("[%s]", memory[i].used ? "USED" : "FREE");
            printf("\n");
        }
    }
}

void fileSystemMenu(void) {
    int choice;
    char name[64], content[MAX_FILE_CONTENT];
    while (1) {
        printf("\nMini File System\n1. Create file\n2. Write file\n3. Read file\n4. List files\n5. Delete file\n0. Back\nChoice: ");
        scanf("%d", &choice);
        getchar();
        if (choice == 0) return;
        if (choice == 1) {
            printf("File name: "); fgets(name, sizeof(name), stdin); name[strcspn(name, "\n")] = 0;
            int made = 0;
            for (int i = 0; i < MAX_FILES; i++) if (!fileTable[i].used) { fileTable[i].used = 1; strncpy(fileTable[i].name, name, 63); fileTable[i].content[0] = 0; fileTable[i].size = 0; made = 1; break; }
            printf(made ? "File created.\n" : "Directory full.\n");
        } else if (choice == 2) {
            printf("File name: "); fgets(name, sizeof(name), stdin); name[strcspn(name, "\n")] = 0;
            int found = -1; for (int i = 0; i < MAX_FILES; i++) if (fileTable[i].used && strcmp(fileTable[i].name, name) == 0) found = i;
            if (found < 0) { printf("File not found.\n"); continue; }
            printf("Content: "); fgets(content, sizeof(content), stdin); content[strcspn(content, "\n")] = 0;
            strncpy(fileTable[found].content, content, MAX_FILE_CONTENT - 1); fileTable[found].size = (int)strlen(fileTable[found].content);
            printf("File written.\n");
        } else if (choice == 3) {
            printf("File name: "); fgets(name, sizeof(name), stdin); name[strcspn(name, "\n")] = 0;
            int found = -1; for (int i = 0; i < MAX_FILES; i++) if (fileTable[i].used && strcmp(fileTable[i].name, name) == 0) found = i;
            if (found < 0) printf("File not found.\n"); else printf("%s: %s\n", fileTable[found].name, fileTable[found].content);
        } else if (choice == 4) {
            printf("\nFiles:\n"); for (int i = 0; i < MAX_FILES; i++) if (fileTable[i].used) printf("- %-20s %d bytes\n", fileTable[i].name, fileTable[i].size);
        } else if (choice == 5) {
            printf("File name: "); fgets(name, sizeof(name), stdin); name[strcspn(name, "\n")] = 0;
            int found = -1; for (int i = 0; i < MAX_FILES; i++) if (fileTable[i].used && strcmp(fileTable[i].name, name) == 0) found = i;
            if (found < 0) printf("File not found.\n"); else { fileTable[found].used = 0; printf("File deleted.\n"); }
        }
    }
}

void activityMonitor(void) {
    printf("\n================ ACTIVITY MONITOR ================\n");
    printf("Simulated CPU time: %d units\n", currentTime);
    printf("PID  NAME                 STATE       REMAINING\n");
    printf("-------------------------------------------------\n");
    for (int i = 0; i < MAX_PROCESSES; i++) if (processTable[i].active) printf("%-4d %-20s %-11s %-9d\n", processTable[i].pid, processTable[i].name, stateString(processTable[i].state), processTable[i].remaining_time);
}

void loadDemoProcesses(void) {
    resetProcesses();
    createProcess("Safari", 0);
    createProcess("Finder", 1);
    createProcess("Terminal", 2);
    createProcess("Music", 3);
}

int main(void) {
    loadDemoProcesses();
    int choice;
    printf("\n====================================================\n");
    printf("                 MINI macOS\n");
    printf("        Operating System Simulator\n");
    printf("====================================================\n");
    while (1) {
        printf("\n1. Process Manager\n2. CPU Scheduling\n3. Memory Manager\n4. File System\n5. Activity Monitor\n6. Load Demo Processes\n0. Shutdown\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) break;
        switch (choice) {
            case 1: displayProcessTable(); break;
            case 2: schedulerMenu(); break;
            case 3: memoryMenu(); break;
            case 4: fileSystemMenu(); break;
            case 5: activityMonitor(); break;
            case 6: loadDemoProcesses(); printf("Demo processes loaded.\n"); break;
            case 0: printf("Shutting down Mini macOS...\n"); return 0;
            default: printf("Invalid choice.\n");
        }
    }
    return 0;
}
