# 1 Asio and proactive pattern
1. Your program initiates the connect operation by calling the I/O object:

socket.async_connect(server_endpoint, your_completion_handler);
where your_completion_handler is a function or function object with the signature:
void your_completion_handler(const boost::system::error_code& ec);
The exact signature required depends on the asynchronous operation being performed. The reference documentation indicates the appropriate form for each operation.

2. The I/O object forwards the request to the I/O execution context.
3. The I/O execution context signals to the operating system that it should start an asynchronous connect.
4. The operating system indicates that the connect operation has completed by placing the result on a queue, ready to be picked up by the I/O execution context.
5. When using an io_context as the I/O execution context, your program must make a call to io_context::run() (or to one of the similar io_context member functions) in order for the result to be retrieved. A call to io_context::run() blocks while there are unfinished asynchronous operations, so you would typically call it as soon as you have started your first asynchronous operation.
6. While inside the call to io_context::run(), the I/O execution context dequeues the result of the operation, translates it into an error_code, and then passes it to your completion handler.

This is a simplified picture of how Boost.Asio operates. You will want to delve further into the documentation if your needs are more advanced, such as extending Boost.Asio to perform other types of asynchronous operations.
![image](https://github.com/Patrickyyh/async-echo-server/assets/34131663/7aba5061-bf14-4989-8a50-001bccd3f363)

![image](https://github.com/Patrickyyh/async-echo-server/assets/34131663/d940112c-1634-4d74-8235-53b27e6b0394)




# 1 version:1 issue
1. delete the session is not a safe operation.
   1. For example, when the server read the data from the client, handle_read callback function will be invoked
   2. At this moment, the client disconnet, so when the async_write is called, its callback function `handle_write` will be invoked
      and find that the client is disconnected. Hence, an error will occurred and the session will be deleted with `delete this` operation.
   3. When the client is disconnected, the `handle_read` callback will also be invoked as well, hence now the the delete operation will be called twice
      which will lead to the memory issue.
   4. **Reason**: we use the same socket to handle the `async_write` an `async_read` operation.
   5. **Possible solution**
      1. make use of the share_pointer to extend the life-time of the Session instance.
2. Other Limitation:
   1. This is just an echo server
   2. write and read are not fully separated(same thread and same socket)

# 2 version:2 update
   1. we extend the life-time of the Session.
   2. We would like to change the echo server into a bidirectional server.
   3. Need to implement a MsgNode which stores the data.
   4. Since we have to make sure that the data/message we send from the previous send operation is completed, we need to put message/data node into a queue
   5. Since we are not sure which thread call the callback function, hence we need to add a lock on the queue to handle the race condition.
   6. Modify the logic of the `handle_read` and `handle_write`.
   7. issue still not resolved:
      1. data/message fragmentation might happend inside the server


# 3. version:2.1 update
   1. use TLV format msg Data
      1. `message.id` , `message.length`, `message content`
      2. we use 2 bytes to store the length of the message content
      3. So the total bytes of the message is length_of_message + 2 bytes
   2. Create two share_ptrs to store the recv_msg_node and _recv_head_node
      1. `_recv_head_node` store the head-info which is the length of the bytes




