<div align="center">

# ft_irc

**A fully functional IRC server built from scratch in C++98**

![Language](https://img.shields.io/badge/language-C%2B%2B98-blue?style=flat-square)
![School](https://img.shields.io/badge/École-42-black?style=flat-square)
![Status](https://img.shields.io/badge/status-finished-success?style=flat-square)

<img src="https://go-skill-icons.vercel.app/api/icons?i=cpp,linux,git" />

</div>

---

## About

`ft_irc` is a fully RFC-compliant IRC server implemented in C++98, built as part of the École 42 curriculum. The server handles multiple simultaneous clients over TCP/IP using non-blocking I/O with `poll()`, and supports real-time communication through channels, private messages, and operator commands.

A custom IRC bot is also included as a bonus feature.

---

## Architecture

```
ft_irc/
├── Server.cpp / Server.hpp     # Core server logic, poll loop, client management
├── Client.cpp / Client.hpp     # Client state and authentication
├── Channel.cpp / Channel.hpp   # Channel management, modes, operators
├── Bot.cpp / Bot.hpp           # Bonus: IRC bot
├── safeSend.cpp / safeSend.hpp # Safe send wrapper
└── main.cpp                    # Entry point
```

---

## Features

- **Non-blocking I/O** — handles multiple clients simultaneously via `poll()`
- **Client authentication** — PASS, NICK, USER handshake
- **Channels** — JOIN, PART, TOPIC, MODE, KICK, INVITE
- **Operator commands** — MODE +o/+i/+t/+k/+l
- **Private messaging** — PRIVMSG, NOTICE
- **IRC Bot** — bonus bot responding to commands
- **Compatible with real IRC clients** (irssi, weechat, netcat)

---

## Usage

```bash
make
./ircserv <port> <password>
```

Then connect with any IRC client:

```bash
# With netcat (basic test)
nc localhost 6667

# With irssi
irssi -c localhost -p 6667 -w <password>
```

---

## Authors

| Login | GitHub |
|-------|--------|
| ejonsery | [@EloanJ](https://github.com/EloanJ) |
| vduarte  | [@ValentinD17](https://github.com/ValentinD17) |

---

<div align="center">
  <sub>École 42 Mulhouse — ft_irc</sub>
</div>
