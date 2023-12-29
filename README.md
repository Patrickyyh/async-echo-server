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
1. delete the session is not the safe
