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
   
   # Start the client
   # To use the local server, please turn it on. Follow the steps above to start the server, and then use the local address 127.0.0.1 to access.
   ./build/client 127.0.0.1 www.google.com
   
   # Use the specified dns server request
   ./build/client 114.114.114.114 www.google.com
   ```

   

