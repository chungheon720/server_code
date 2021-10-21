#ifndef NET_CONNECTION_H_INCLUDED
#define NET_CONNECTION_H_INCLUDED

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"

namespace olc
{
    namespace net
    {
        template<typename T>
        class connection : public std::enable_shared_from_this<connection<T>>
        {
        public:
            enum class owner
            {
                server,
                client
            };
            connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket,
                       tsqueue<owned_message<T>>& qIn)
                       : m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
            {
                m_nOwnerType = parent;
            }

            virtual ~connection()
            {}

            // This ID is used system wide - its how clients will understand other clients
			// exist across the whole system.
			uint32_t GetID() const
			{
				return id;
			}
        public:
            void ConnectToClient(uint32_t uid = 0)
            {
                if(m_nOwnerType == owner::server)
                {
                    if(m_socket.is_open())
                    {
                        id = uid;
                        ReadHeader();
                    }
                }
            }
            bool ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
            {
                if(m_nOwnerType == owner::client)
                {
                    asio::async_connect(m_socket, endpoints,
                        [this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
                        {
                           if(!ec)
                           {
                               ReadHeader();
                           }
                           else
                           {
                               std::cout << "Connect To Server Error: " << ec.message() << "\n";

                           }
                        });
                }
            }

            bool Disconnect()
            {
                if(IsConnected())
                    asio::post(m_asioContext, [this]() { m_socket.close(); });
            }

            bool IsConnected() const
            {
                return m_socket.is_open();
            }

        public:
            bool Send(const message<T>& msg)
            {
                asio::post(m_asioContext,
                    [this, msg]()
                    {
                        bool bWritingMessage = !m_qMessagesOut.empty();
                        m_qMessagesOut.push_back(msg);
                        if(!bWritingMessage)
                        {
                            WriteHeader();
                        }
                    });
            }

        private:
            //Async - Prime context ready to read message header
            void ReadHeader()
            {
                asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>),
                    [this](std::error_code ec, std::size_t length)
                    {
                       if(!ec)
                       {
                            if(m_msgTemporaryIn.header.size > 0)
                            {
                                m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                                ReadBody();
                            }
                            else
                            {
                                AddToIncomingMessageQueue();
                            }
                       }
                       else
                       {
                           std::cout << "[" << id << "] Read Header Failed\n";
                           //Checked by the server and removed in server.h
                           m_socket.close();
                       }
                    });

            }

             //Async - Prime context ready to read message body
            void ReadBody()
            {
                asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
                    [this](std::error_code ec, std::size_t length)
                    {
                       if(!ec)
                       {
                            AddToIncomingMessageQueue();
                       }
                       else
                       {
                           std::cout << "[" << id << "] Read Body Failed\n";
                           //Checked by the server and removed in server.h
                           m_socket.close();
                       }
                    });
            }

             //Async - Prime context ready to write message header
            void WriteHeader()
            {
                asio::write(m_socket, asio::buffer(&m_qMessagesOut.front(), sizeof(message_header<T>)),
                    [this](std::error_code ec, std::size_t length)
                    {
                       if(!ec)
                       {
                            if(m_qMessagesOut.front().body.size() > 0)
                            {
                                WriteBody();
                            }
                            else
                            {
                                m_qMessagesOut.pop_front();

                                if(!m_qMessagesOut.empty())
                                {
                                    WriteHeader();
                                }
                            }
                       }
                       else
                       {
                           std::cout << "[" << id << "] Write Header Failed\n";
                           m_socket.close();
                       }
                    });
            }

             //Async - Prime context ready to write message body
            void WriteBody()
            {
                asio::write(m_socket, asio::buffer(&m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
                    [this](std::error_code ec, std::size_t length)
                    {
                        if (!ec)
						{
							// Sending was successful, so we are done with the message
							// and remove it from the queue
							m_qMessagesOut.pop_front();

							// If the queue still has messages in it, then issue the task to
							// send the next messages' header.
							if (!m_qMessagesOut.empty())
							{
								WriteHeader();
							}
						}
						else
						{
							// Sending failed, see WriteHeader() equivalent for description :P
							std::cout << "[" << id << "] Write Body Fail.\n";
							m_socket.close();
						}

                    }
            }

            void AddToIncomingMessageQueue()
            {
                if(m_OwnerType == owner::server)
                    m_qMessagesIn.push_back({this->shared_from_this(), m_msgTemporaryIn});
                else
                    m_qMessagesIn.push_back({nullptr, m_msgTemporaryIn});


                //Register another task for the context to handle
                ReadHeader();
            }

        protected:
            //Each connection has a unique socket to a remote
            asio::ip::tcp::socket m_socket;

            //This context is shared among all asio connections
            asio::io_context& m_asioContext;

            //This queue holds all messages to be sent to the remote
            //site of this connection
            tsqueue<message<T>> m_qMessagesOut;

            //This queue holds all messages to be received from the remote
            //site of this connection. It is only a reference as the owner
            //would provide the queue
            tsqueue<owned_message<T>>& m_qMessagesIn;

            //The owner decides how some of the connection behaves
            owner m_nOwnerType = owner::server;

            uint32_t id = 0;

            //Temporarily hold the message that was read
            message<T> m_msgTemporaryIn;
        };
    }
}

#endif // NET_CONNECTION_H_INCLUDED
