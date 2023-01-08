# RC Pair Programming Task (Server)

I implemented the Database Server task in C using POSIX TCP sockets and standard lib utilities.


I'm still pretty new to C, so I chose to perform a task I understand pretty well (building a simple web server) in a context I'm less familiar with to learn more about the next level "down stack" from where I typically work.  I tested this out on linux using cURL and chrome to verify that the requirements have been met.

The "get" and "set" data operations are isolated to a separate file, so migrating them to the filesystem in the interview should be manageable.


While I'm pretty sure the code is free of plain data corruption issues, there are a few quirks I noted in comments, mostly related to setup/teardown of the socket object.

I really enjoyed working on this task and look forward to returning to it in the pairing session.

