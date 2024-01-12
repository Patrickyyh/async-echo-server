#  Asio and proactive pattern
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




# 1. version:1 issue
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

# 2. version:2 update
   1. we extend the life-time of the Session.
   2. We would like to change the echo server into a bidirectional server.
   3. Need to implement a MsgNode which stores the data.
   4. Since we have to make sure that the data/message we send from the previous send operation is completed, we need to put message/data node into a queue
   5. Since we are not sure which thread call the callback function, hence we need to add a lock on the queue to handle the race condition.
   6. Modify the logic of the `handle_read` and `handle_write`.
   7. issue still not resolved:
      1. data/message fragmentatio or Packet sticking might happend inside the server



# 3. version:2.1 update: resolve the TCP packet sticking issue.
   1. Packet Sticking
   Packet sticking, often encountered in stream-based transport protocols like TCP, occurs when multiple messages sent consecutively by the sender are not adequately delimited, making it challenging for the receiver to distinguish message boundaries. Common strategies to handle this include:
      1. Length-Prefixed Messaging: Prefixing each message with a field indicating its length, allowing the receiver to determine the exact length of each message.
      2. Delimiter-Based Framing: Using special characters or strings as message delimiters, ensuring these delimiters do not occur within the messages themselves.
      3. Fixed-Length Messages: Employing messages of a consistent, fixed length, which is suitable for formats where message lengths are predictable and uniform.
   2. use TLV format msg Data
      1. `message.id` , `message.length`, `message content`
      2. we use 2 bytes to store the length of the message content
      3. So the total bytes of the message is length_of_message + 2 bytes
   3. Create two share_ptrs to store the recv_msg_node and _recv_head_node
      1. `_recv_head_node` store the head-info which is the length of the bytes
      2. `_recv_msg_node` store the content of the msg
   4. Test the server
      1. Let the frequency of message sending from the client > frequency of acceptence of the message from the server
      2. We seperate the read and write operation of the client by putting them into the different thread.


# 4. version:2.1 big-endian and little-endian convertion.
   1. Convert the host byte order into the network byte order and send it.
   2. When received, convert the network byte order into the host byte order.
   3. Define the maximum number of the data packet that the send_queue could handle.

# 5. version:3 Reconstrure Business Logic layer
   1. Adding JSON serialization using jsoncpp open source library
   2. Add logical layer (Business layer)into the server.
      1. Why we seperate the asio layer and business logic layer
         - suppose that, If we need to do some Blocking I/O operation, such as access the database,
         this operation will take up the resources when we issue them inside the asio layer. As we know the asio use single thread to do the epoll, which indicates that it will not coninute working on next callback function until the previous one is done. This will effect the performance of the asio layer.
         Hence, we need to create another thread inside the logich layer to call the callback function.
   3. logic layer is the sigleton thread.
   4. logic layer is the consumer and the the asio layer is the producer
      - so We need to use mutex and condition variable together to handle the race condition.
   4. Re-design the message Node
      1. Contains message id, message length, and message content
      2. Separate the Receiv message node and send message node.

# 6. version:3 Enhance the server's performance by concurrency
In our previous designs, we used ASIO in a single-threaded mode. To enhance the efficiency of concurrent network I/O processing, we have now developed a multi-threaded mode of ASIO usage
