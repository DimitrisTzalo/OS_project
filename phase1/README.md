# Multithreaded Storage Engine - MYY601 Lab 1 (Spring 2025)

## Overview
This project is the solution for **Lab 1** of the course **MYY601 - Operating Systems** at the University of Ioannina.  
The assignment required the implementation of **multithreaded** `add` and `get` operations in a **Kiwi storage engine** based on the **LSM-tree** structure.

## In order ro run follow these steps at kiwi-source folder:
1. make clean
2. make all
3. cd bench
4. for write for >1 threads: ./kiwi-bench multi_write #processes #threads
5. for read for >1 threads: ./kiwi-bench multi_read #processes #threads
6. for readwrite for >1 threads: ./kiwi-bench readwrite #processes #threads #write_percenage

- **Course**: MYY601 - Operating Systems
- **Instructor**: Mr. Stergios Anastasiadis
- **Semester Spring 2025** 
- ** University of Ioannina

## Contributors
- Vasiliki-Eleni Tsoumani
- Dimitrios Pagonis
- Dimitrios Tzalokostas

## Project Description
We enhanced the original Kiwi storage engine source code to support **multithreaded access** for the `add` and `get` operations using **Pthreads**.

The system allows:
- Multiple threads performing concurrent **add** operations.
- Multiple threads performing concurrent **get** operations.
- Simultaneous **add** and **get** operations.
- Safe and efficient synchronization between threads.
- Measurement and reporting of performance statistics (throughput and response time).

### Changes Made
The following files were modified:
- `kiwi.c`
- `bench.c`
- `db.c`
- `db.h`
- `utils.c`

### Main Features
- Implemented **mutex locks** and **reader-writer synchronization** for safe access to shared data.
- Added **performance measurement** capabilities.
- Extended the **benchmark tool** to support multithreaded scenarios (`multi_write`, `multi_read`, `readwrite` modes).
- Improved error handling and logging.

## Setup Instructions

### Environment
- **Host Machine**: Windows 11 Pro
- **Virtual Machine**: Debian Linux 12 ("Bookworm") via VMware
- **Specs**:
  - Intel Core i7-13700H (20 CPUs @ 2.4GHz)
  - 32GB RAM
  - 5.7GB RAM and 8 processors allocated to VM

### Prerequisites
- `gcc`, `gdb`, `make`, `pthreads` installed in VM.
- **Visual Studio Code** installed inside Debian VM for easier development (see installation steps inside Report).

### Compilation
```bash
cd kiwi-source
make all
```

### Execution Examples
```bash
# Single-threaded writes
./kiwi-bench write 10000

# Single-threaded reads
./kiwi-bench read 10000

# Multithreaded writes
./kiwi-bench multi_write 500000 4

# Multithreaded reads
./kiwi-bench multi_read 500000 4

# Combined multithreaded reads and writes
./kiwi-bench readwrite 500000 8 50
```

### Benchmark Commands
- `multi_write`: concurrent `add` operations
- `multi_read`: concurrent `get` operations
- `readwrite`: mix of concurrent `add` and `get` based on percentage

## Performance
Our experiments showed:
- Increasing threads improves **read throughput** significantly.
- **Write throughput** improvement is limited due to serialization requirements.
- Combined workloads (reads and writes) show balanced behavior depending on thread distribution.
  
## Documentation
Detailed technical explanations and code walkthroughs are available in [Report.pdf](./Report.pdf).

## Notes
- Only the modified source files are included in the final submission.
- Proper synchronization mechanisms (mutexes, condition variables) were added to avoid race conditions and deadlocks.
- All changes were heavily documented with in-code comments and report descriptions.



