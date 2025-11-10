# IRC Chat Program

A simple **IRC-style chat application** Michal and I developed as part of the **42 Vienna curriculum**.

---

## 🚀 Project Overview
This project simulates a basic **Internet Relay Chat (IRC)** system, including:
- Multi-client communication
- Real-time messaging
- Command parsing (e.g., /nick, /join, /msg)
- Server-client architecture in **C/C++**

It was developed to **practice low-level networking** and **multi-threaded programming** concepts.

---

## 🛠️ Features
- TCP/UDP socket communication
- Handling multiple clients concurrently
- Custom commands for user interaction
- Error handling for robustness
- Optional: Logging chat history

---

## 🧑‍💻 Tech Stack
- **Languages:** C, C++
- **Networking:** Sockets, select/poll for multiplexing
- **Concurrency:** Threads & synchronization primitives
- **OS:** Unix/Linux

---

## 💡 Usage
1. Compile the server and client:
./irc_server <PORT> , ./irc_client <SERVER_IP> <PORT>
/nick <nickname>
/join <channel>
/msg <user> <message>

```bash
make
