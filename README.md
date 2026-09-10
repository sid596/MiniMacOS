# Mini macOS

## Operating Systems Working Model

Mini macOS is an educational operating system simulator developed in C and inspired by the architecture and functionality of macOS.

The purpose of this project is to demonstrate fundamental Operating Systems concepts through a working software model rather than only presenting theoretical information.

> **Note:** Mini macOS is an educational simulation. It is not a replacement for macOS and does not implement or modify the actual macOS/XNU kernel.

---

## Features

### Process Management
- Process creation and termination
- Process Control Block (PCB)
- Process IDs (PID)
- Process states:
  - NEW
  - READY
  - RUNNING
  - WAITING
  - TERMINATED
- Process arrival time
- CPU burst time
- Remaining execution time
- Process priorities

### CPU Scheduling

The simulator implements several classical CPU scheduling algorithms:

1. **First Come First Served (FCFS)**
2. **Shortest Job First (SJF)**
3. **Shortest Remaining Time First (SRTF)**
4. **Priority Scheduling**
5. **Round Robin**

The simulator also demonstrates:

- Preemptive scheduling
- Non-preemptive scheduling
- Time quantum
- Context switching
- Waiting time
- Turnaround time
- Response time
- Average scheduling metrics

### Memory Management

The memory manager provides a simplified simulation of memory allocation.

It demonstrates:

- Memory blocks
- Allocation
- Deallocation
- Process ownership
- Available and used memory

### File System

A simplified in-memory file system is included to demonstrate:

- File creation
- File writing
- File reading
- File listing
- File deletion

### Activity Monitor

The Activity Monitor provides a view of currently simulated processes and their resource-related information, inspired by the purpose of macOS Activity Monitor.

---

## Project Structure

```text
MiniMacOS/
│
├── src/
│   └── main.c
│
├── Makefile
├── README.md
└── Mini_macOS_Assignment_Report.docx