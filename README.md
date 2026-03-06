# 💻 AliOS 4.0 - Notebook Edition

AliOS 4 is a custom-built, 64-bit "Notebook" style operating system. It features a hardened security model, a multi-terminal interface (TTY), and a real-time status bar calibrated for high-precision timing.

## 🚀 Key Features

* **64-bit Long Mode:** Boots from 32-bit Multiboot 2 into a fully functional 64-bit environment with identity-mapped paging.
* **Hardened Security:** Integrated `lock_system_hardened` routine that stores failed login attempts in CMOS to prevent unauthorized access across reboots.
* **Multi-TTY Support:** Supports 10 independent virtual terminals (TTY0-TTY9) accessible via `Ctrl + Alt + F1-F10`.
* **Real-Time Status Bar:** A persistent 25th-row UI showing Date, Time (12h format), and active TTY ID, driven by CMOS data.
* **Dynamic Shell:** Features command tab-completion, system diagnostics (`neofetch`), and real-time memory tracking.

---

## 🏗️ Technical Architecture

### 1. Boot & Memory
* **Kernel Entry:** Written in Assembly (`boot.asm`), it builds a 4-level paging hierarchy: PML4 -> PDPT -> PDT.
* **Huge Pages:** Identity maps the first 10MB of RAM using 2MB "Huge Pages" to simplify the initial memory map.
* **Heap Manager:** A dynamic memory allocator starting at the 2MB mark (`0x200000`) to avoid clobbering kernel code.

### 2. Clock & Timing
* **PIT Frequency:** The Programmable Interval Timer is calibrated to **100Hz** (divisor 11931) for uptime and sleep functions.
* **Polling Loop:** The kernel uses a hardware polling loop (`timer_wait_tick`) to ensure the clock updates every 100 ticks without interrupting user input.

### 3. Video & I/O
* **VGA Driver:** Manages an 80x25 text buffer at `0xB8000` with custom "Notebook Yellow" styling (0x1E).
* **CMOS Integration:** Directly communicates with hardware ports `0x70/0x71` to retrieve real-time clock data and track security strikes.

---

## 🛠️ Built-in Shell Commands

| Command | Description |
| :--- | :--- |
| `help` | Lists all registered system commands. |
| `cls` | Clears the notebook screen. |
| `neofetch` | Displays CPU vendor, RAM usage, and OS mode. |
| `uptime` | Show how long AliOS has been running. |
| `free` | Check dynamic RAM usage (Total, Used, Free). |
| `timezone` | Adjusts the status bar clock offset in real-time. |
| `lock` | Manually triggers the hardened lock screen. |
| `test` | Verifies timer calibration with a 5s countdown. |
| `beep` | Plays a system alert sound through the PC Speaker. |
| `twins` | Shows my twins |

## 📁 Directory Structure
```text
.
├── src
│   ├── kernel.c           # Main OS Loop & Hardening Logic
│   ├── section1_cpu       # Boot, Timer, Speaker, & Heap
│   ├── section2_video     # VGA Driver & TTY Management
│   ├── section3_io        # CMOS & Keyboard Drivers
│   └── section4_shell     # Shell Logic & Command Library
└── linker.ld              # Kernel Memory Layout
```
## 🔒 Usage Policy
You are permitted to:
✅ Use this OS for personal purposes  
✅ Modify code for learning or improvement  

You are **NOT** allowed to:
❌ Rename or rebrand AliOS as your own project  
❌ Claim authorship of core components  
❌ Remove original copyright notices or credits  

⚠️ Violators will have their repos reported + exposed publicly across platforms. Git history does not lie.
