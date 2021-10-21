#pragma once

#ifndef NET_MESSAGE_H_INCLUDED
#define NET_MESSAGE_H_INCLUDED

#include "net_common.h"

namespace olc
{
	namespace net
	{
		/*NOTE: Ensure that the size type dont change
			Byte ordering of the architecture are the same
			May require to code conversion
		*/

        /*
        Example
        olc::net::message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::FireBullet;

        int a = 1;
        bool b = true;
        float c = 3.14;

        struct
        {
            float x;
            float y;
        } d[5];

        a = 99;
        b = false;
        c = 31.1f;

        msg >> d >> c >> b >> a;
        */

		//Message Header is sent at start of all messages. The template allows us
		// to use 'enum class' to ensure that all messages are valid at compile time
		template <typename T>
		struct message_header
		{
			T id{};
			uint32_t size = 0;
		};

		template <typename T>
		struct message
		{
			message_header<T> header{};
			std::vector<int8_t> body;

			size_t size() const
			{
				return sizeof(message_header<T>) + body.size();
			}

			//Override std::cout compatibility - produces friendly description of message
            friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
            {
                os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
                return os;
            }

            //Pushes any POD-like data into the message buffer
            template <typename DataType>
            friend message<T>& operator << (message<T>& msg, const DataType& data)
            {
                static_assert(std::is_standard_layout<DataType>::value, "Data is too complex");

                size_t i = msg.body.size();

                //Resizing causes some overhead, as we are resizing every message
                msg.body.resize(msg.body.size() + sizeof(DataType));

                std::memcpy(msg.body.data() + i , &data, sizeof(DataType));

                msg.header.size = msg.size();

                return msg;
            }

            template <typename DataType>
            friend message<T>& operator >> (message<T>& msg, DataType& data)
            {
                static_assert(std::is_standard_layout<DataType>::value, "Data is too complex");

                size_t i = msg.body.size() - sizeof(DataType);

                std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

                msg.body.resize(i);

                msg.header.size = msg.size();

                return msg;
            }
		};

        // An "owned" message is identical to a regular message, but it is associated with
		// a connection. On a server, the owner would be the client that sent the message,
		// on a client the owner would be the server.

		// Forward declare the connection
		template <typename T>
		class connection;

		template <typename T>
		struct owned_message
		{
			std::shared_ptr<connection<T>> remote = nullptr;
			message<T> msg;

			// Again, a friendly string maker
			friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg)
			{
				os << msg.msg;
				return os;
			}
		};

	}
}

#endif // NET_MESSAGE_H_INCLUDED
