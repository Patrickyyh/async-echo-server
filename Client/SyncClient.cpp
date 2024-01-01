#include <boost/asio.hpp>
#include <iostream>
#include <thread>

// const int MAX_LENGTH = 2048;
#define HEADER_LENGTH 2
#define MAX_LENGTH 2048

using namespace boost;
using namespace std;
using namespace boost::asio::ip;
using boost::asio::ip::tcp;

int main()
{
   try
   {
      boost::asio::io_context ioc;

      // create the endpoint
      // ip addr and socket of the server
      tcp::endpoint remote_ep(address::from_string("127.0.0.1"), 10086);

      // create a socket
      tcp::socket new_socket(ioc);
      system::error_code error = boost::asio::error::host_not_found;
      new_socket.connect(remote_ep, error);

      if (error)
      {
         cout << "connection failed, the code : " << error.value() << "error message is : "
              << error.message() << endl;

         return 0;
      }

      // This thread is responsible for sending message to the server.
      thread send_thread([&new_socket]
                         {
         for(;;){

             this_thread::sleep_for(std::chrono::milliseconds(2));
             const char * request_message = "Hellow world !";
             // return the lenght of the c-string
             size_t request_length = strlen(request_message);
             char send_data[MAX_LENGTH] = {0};
             memcpy(send_data , &request_length , HEADER_LENGTH);
             memcpy(send_data + HEADER_LENGTH , request_message , request_length);
             boost::asio::write(new_socket ,asio::buffer(send_data , HEADER_LENGTH + request_length));


         } });

      // This thread is responsible for sending message to the client.
      thread recv_thread([&new_socket]
                         {
         for (;;)
         {
            this_thread::sleep_for(std::chrono::milliseconds(2));
            cout << "begin to receive... " << endl;
            char reply_head[HEADER_LENGTH];
            size_t reply_length = boost::asio::read(new_socket, boost::asio::buffer(reply_head, HEADER_LENGTH));
            short msg_length = 0;
            memcpy(&msg_length, reply_head, HEADER_LENGTH);

            // Get the msg
            char msg[MAX_LENGTH] = {0};
            size_t message_length = boost::asio::read(new_socket, boost::asio::buffer(msg, msg_length));
            std::cout << "Reply is: ";
            std::cout.write(msg, msg_length);
            std::cout << "\n";

            std::cout << "Reply len is " << msg_length;
            std::cout << "\n";

           } });

      send_thread.join();
      recv_thread.join();
      // Enter the message
      // std::cout << "Enter message: ";
      // char request_buf[MAX_LENGTH];
      // std::cin.getline(request_buf, MAX_LENGTH);

      // // return the lenght of the c-string
      // size_t request_length = strlen(request_buf);
      // char send_data[MAX_LENGTH] = {0};
      // memcpy(send_data, &request_length, HEADER_LENGTH);
      // memcpy(send_data + HEADER_LENGTH, request_buf, request_length);
      // boost::asio::write(new_socket, asio::buffer(send_data, HEADER_LENGTH + request_length));

      // // Get the header
      // char reply_head[HEADER_LENGTH];
      // size_t reply_length = boost::asio::read(new_socket, boost::asio::buffer(reply_head, HEADER_LENGTH));
      // short msg_length = 0;
      // memcpy(&msg_length, reply_head, HEADER_LENGTH);

      // // Get the msg
      // char msg[MAX_LENGTH] = {0};
      // size_t message_length = boost::asio::read(new_socket, boost::asio::buffer(msg, msg_length));
      // std::cout << "Reply is: ";
      // std::cout.write(msg, msg_length);
      // std::cout << "\n";

      // std::cout << "Reply len is " << msg_length;
      // std::cout << "\n";
   }
   catch (const std::exception &e)
   {
      std::cerr << e.what() << '\n';
   }
}
