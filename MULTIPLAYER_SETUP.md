# Multiplayer Setup Guide

## What Changed

I've updated the game to support proper multiplayer functionality where all clients can see each other:

### Server Improvements:
1. **Player ID Assignment**: Server now sends back the assigned player ID to each connecting client
2. **Movement Handling**: Server processes player movement commands (LEFT, RIGHT, UP, DOWN)
3. **Shooting Handling**: Server handles shooting messages and creates bullets in the game state
4. **Player Leave**: Server properly handles player disconnections
5. **Broadcasting**: Server broadcasts the complete game state to all connected clients 30 times per second

### Client Improvements:
1. **Proper Initialization**: Client waits for server to assign player ID before allowing input
2. **Game State Sync**: Client properly deserializes game state updates from server
3. **Input to Server**: Client sends movement and shooting actions to server
4. **No Local Test Player**: Removed hardcoded test player, all players come from server

### Network Protocol:
- **Join**: Client sends player name, server assigns ID and creates player
- **Move**: Client sends movement keys, server updates player velocity
- **Shoot**: Client sends shooting action, server creates bullets
- **State Update**: Server sends complete game state (all players + bullets) to all clients

## Testing Instructions

### Your Friend (Server Host):
Your friend should run the server on their machine:
```bash
cd build
./server
```

The server will start on port 8080 and display when players connect.

### You (Client):
Connect to your friend's server:
```bash
cd build
./client 10.81.106.48
```

When prompted, enter your player name.

### Multiple Clients:
To test with multiple players, run the client on different devices:

**On another Mac/Linux:**
```bash
cd build
./client 10.81.106.48
```

**On Windows:**
```cmd
cd build
client.exe 10.81.106.48
```

## Game Controls
- **A/D or Left/Right Arrow**: Move left/right
- **W/S**: Move up/down
- **Mouse**: Aim direction
- **Left Click**: Shoot
- **ESC**: Exit game

## Expected Behavior
1. When you connect, you should see "Assigned player ID: X" in the terminal
2. Your player will appear at a random spawn position
3. When other players connect, you'll see them appear on your screen
4. All player movements and bullets are synchronized across all clients
5. The server broadcasts updates 30 times per second

## Troubleshooting

### "Failed to initialize networking"
- Make sure the server is running first
- Check firewall settings on the server machine
- Verify the IP address and port are correct

### "No players visible"
- Wait a few seconds for the first state update
- Check the server console to see if players are connected
- Make sure both client and server are using the updated build

### Players appear but don't move
- Check network connectivity
- Verify UDP port 8080 is not blocked
- Look at server console to see if it's receiving movement messages

## Network Requirements
- UDP port 8080 must be open on the server
- All clients must be able to reach the server IP
- Low latency connection recommended for smooth gameplay
