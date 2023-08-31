# 42-mini_serv

### What's this about?
A small program in C realise for the exam-rank-06 of 42, it lets clients talk to each other.

The tests on this exam are a bit bizarre, and the traces are sometimes difficult to understand.  
I think the speed of the program is tested, so to pass this exam, I advise you to make the program as simple as possible, to use global variables, to avoid memory allocations and chained lists,...  

If you fail test 8 with a lot of bytes to read, I suggest you send the "client %d:" separately from the message line.  
I passed this test thanks to that, I don't really know why^^.

### Installation :
- Clone the repository.

### Usage:
- ```gcc -Wall -Wextra -Werror mini_serv.c```  
- ```./a.out <port>```
- ```nc localhost <port>``` (in another terminal to connect a client)
