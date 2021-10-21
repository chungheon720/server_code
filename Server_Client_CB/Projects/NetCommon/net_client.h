#ifndef NET_CLIENT_H_INCLUDED
#define NET_CLIENT_H_INCLUDED

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"

namespace olc
{
    namespace net
    {
        //Incharge of setting up asio and connection
        //Access point for server
        template<typename T>
        class client_interface
        {
        public:
            client_interface() : m_socket(m_context)
            {
                //Initialize the socket with io context
            }
            virtual ~client_interface()
            {
                //If client is destroyed, always try and disconnect from server
                Disconnect();
            }
        public:
            //Connect to server with hostname/ip-address and port
            bool Connect(const std::string& host, const uint16_t port)
            {
                try
                {

                    //Resolve hostname/ip-address into tangible physical address
                    asio::ip::tcp::resolver resolver(m_context);
                    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

                    //Create connection
                    m_connection = std::make_unique<connection<T>>(
                        connection<T>::owner::client,
                        m_context,
                        asio::ip::tcp::socket(m_context), m_qMessageIn);



                    m_connection->ConnectToServer(endpoints);

                    //Start Context Thread
                    ctxThread = std::thread([this](){ m_context.run();});
                }
                catch (std::exception& e)
                {
                    std::cerr << "Client exception: " << e.what() << "\n";
                    return false;
                }
                return true;
            }

            //Disconnect from server
            void Disconnect()
            {
                //Check if connection is connected, ...
                if(IsConnected())
                {
                    //... disconnect connection
                    m_connection->Disconnect();
                }

                //Stop the context
                m_context.stop();

                //And the context thread
                if(ctxThread.joinable())
                    ctxThread.join();

                //Release pointer
                m_connection.release();
            }

            //Get connection status
            bool IsConnected()
            {
                if(m_connection)
                    return m_connection->IsConnected();
                else
                    return false;
            }

            tsqueue<owned_message<T>>&  Incoming()
            {
                return m_qMessageIn;
            }
        protected:
            //Handles the asio, data transfer
            asio::io_context m_context;
            //... requires a thread to loop and keep running
            std::thread ctxThread;
            //Hardware socket connected to the server
            asio::ip::tcp::socket m_socket;
            //Connection object that handles the data transfer
            std::unique_ptr<connection<T>> m_connection;


        private:
            tsqueue<owned_message<T>> m_qMessageIn;

        };
    }
}

#endif // NET_CLIENT_H_INCLUDED
