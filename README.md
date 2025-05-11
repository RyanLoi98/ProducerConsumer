# 🎈 Producer-Consumer Problem Simulation

**CPSC 457 Principles of Operating Systems | Fall 2023 | University of Calgary**

### 👨‍💻 Author

Ryan Loi

---

## 📌 Overview

This assignment explores synchronization and concurrency through the **classic Producer-Consumer problem**, adapted to a real-world-inspired simulation at the **Calgary Carnival**. The implementation focuses on correctly using **semaphores**, **threads**, and **bounded buffers** to avoid race conditions and simulate a coordinated, multithreaded system.

The project is divided into two parts:

---

## 🧩 Part 1: Single Producer, Single Consumer

In this simulation:

* 🎈 **Balloon Bob** (producer) makes **balloon animals** (`Balloon Monkey`, `Balloon Dog`) and places them into a **single cart** (buffer).
* 🧍‍♂️ **Customers** (consumer) arrive in an infinite loop and pick up balloon animals from the cart.
* 🕹️ The program includes **user interaction**, allowing the user to observe or interact with the simulation.
* 🔒 The implementation ensures correct synchronization using **mutexes** and **semaphores**.

### ✅ Features:

* Single buffer (bounded)
* One producer process
* One consumer process
* Mutual exclusion enforced
* Option to display buffer state

---

## 🧩 Part 2: Multiple Producers, Multiple Consumers

In this extended simulation:

* 🎈 **Balloon Bob** and **Helium Harry** act as **two producer threads**.

  * Balloon Bob makes **Balloon Monkey** and **Balloon Dog**
  * Helium Harry makes **Balloon Hut** and **Balloon Tower**
* Each producer uses their **own buffer (cart)**
* 🧍‍♂️ Three types of **consumer threads**:

  1. Wants any **animal balloon**
  2. Wants any **house balloon**
  3. Wants **both** types
* 🕓 The simulation runs for **45 seconds**, demonstrating continuous production/consumption without deadlocks or race conditions.

### ✅ Features:

* Two bounded buffers
* Two producer threads
* Three consumer threads
* Each thread sleeps for a random interval to simulate realism
* Synchronization via binary and counting semaphores
* Proper thread lifecycle handling (`pthread_create`, `pthread_join`)
* Real-time buffer state printing

---

## 🛠 Implementation Details

* **Language**: C / C++
* **Threads**: POSIX Threads (pthreads)
* **Synchronization**: Binary semaphore (`mutex`) + Counting semaphores (`empty`, `full`)
* **Random Delays**: Simulated using `sleep(rand % N)` to mimic real-time production and consumption
* **Critical Sections**: Managed with mutex to ensure mutual exclusion
* **Buffer**: Circular queue (FIFO) with `in` and `out` pointers
* **Execution Time**: Part 2 runs automatically for 45 seconds

---

## 🔧 Compilation

To compile both parts:

```bash
# Part 1
gcc -o part1 producer_consumer_1.c -lpthread

# Part 2
gcc -o part2 producer_consumer_2.c -lpthread
```

---

## ▶️ Running the Programs

### Part 1 – Interactive Producer-Consumer

```bash
./part1
```

Now you can simulate making balloons and buying balloons while checking up on the stock (buffer), the slot, and mutex lock, and here a screen shot of it running:

![Simulation Screenshot](https://i.imgur.com/tTOWurL.png)



### Part 2 – Multi-threaded Simulation

```bash
./part2
```
Part 2 runs automatically and the output displays which producer or consumer performed an action, which item was produced/consumed, and the current buffer state, slot, and mutex lock.

Here's a screenshot of it running:

![Simulation Screenshot](https://imgur.com/OdKx2v1.png)


---

## 🧠 Concepts Practiced

* Process vs. Thread distinction
* Thread lifecycle management
* Mutual exclusion
* Bounded buffer implementation
* Real-world inspired synchronization problem
* Proper use of sleep/random delays to simulate real time
* Debugging concurrent access issues (data races, starvation, deadlock)