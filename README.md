# Online Chatroom

## How to run

1. Download the zip file.
2. Unzip the `Online_Chatroom.zip` folder.
3. Go to `Online_Chatroom\MySoln\x64\Debug` file.
4. Double click `Server.exe` to run the `Server` programme.
5. Double click `Client.exe` to run the `Client` programme.

## File Explanation

1. Server
   - `Server.h`: include all declaration of structs and subfunctions in `Server.cpp`.
   - `Server.cpp`: `main` function of `Server` and Definition of the structs and subfunctions in `Server.h`.
2. Client
   - `Client.h`: include all declaration of struct and subfunctions in `Client.cpp`.
   - `Client.cpp`: `main` function of `Client` and Definition of the structs and subfunctions in `Client.h`.

## Command Structure in Client

1. Create a new group: `new Username1 Username2 ...`
2. Add user to group: `add GroupId Username`
3. Delete user from group: `delete GroupId Username`
4. Send message to all users: `all Message`
5. Send message to a user: `user Username Message`
6. Send message to a group: `group groupId Message`

**Note: All other input command structure will be considered as invalid input.**
