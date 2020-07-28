# Network Programing

**Server-client:**

1. Compile the source code:

   ```bash
   make -f ./makefile
   ```

   

2. Run server and client:

   ```bash
   # Start the server
   ./build/server
   
   # Start the client, There is two ways
   # After startup, enter the server address, port and user name as prompted
   ./build/client
   
   # When starting, specify the server address, port and user name
   ./build/client 127.0.0.1 5001 username
   ```

   

