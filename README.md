# 💬 ft_irc

Internet Relay Chat server implementation compatible with standard IRC clients.

## 🚀 Usage

```bash
./ircserv <port> <password>
```

## 📋 Features

### Server Operations
- TCP/IP Socket Programming
- Non-blocking I/O with poll()
- Password Authentication
- Multiple Client Management
- Nickname Management
- Channel Operations

### Supported Commands
- `PASS` - Authentication
- `NICK` - Set/change nickname
- `USER` - Set username
- `JOIN` - Join channel
- `PRIVMSG` - Send messages
- `NOTICE` - Send notices
- `KICK` - Remove user from channel
- `INVITE` - Invite user to channel
- `TOPIC` - Set/view channel topic
- `MODE` - Change channel/user modes
- `QUIT` - Disconnect from server
- `PING/PONG` - Connection check

### Channel Modes
- `i` (invite-only)
- `t` (topic protection)
- `k` (key/password)
- `o` (operator)
- `l` (user limit)

## 🛠️ Installation

```bash
git clone https://github.com/fsantos23/ft_irc.git
cd ft_irc
make
```

## 🧪 Testing

```bash
# Start server
./ircserv 6667 password

# Connect with client
nc -c localhost -p 6667
```

## ⚠️ Error Handling

- Connection failures
- Authentication errors
- Invalid commands
- Permission issues
- Nickname conflicts
- Channel operation errors

## ⭐ Show your support

Give a ⭐️ if this project helped you!
