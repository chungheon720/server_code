#pragma once
#include <net_common.h>

namespace olc
{
	namespace net
	{
		/*NOTE: Ensure that the size type dont change
			Byte ordering of the architecture are the same
			May require to code conversion
		*/

		//Message Header is sent at start of all messages. The template allows us
		// to use 'enum class' to ensure that all messages are valid at compile time
		template <typename T>
		struct message_header
		{
			T id();
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
		};

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
		friend message<T>& operator >> (message<T>& msg, const DataType& data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex");

			size_t i = msg.body.size() - sizeof(DataType);

			std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

			msg.body.resize(i);

			msg.header.size = msg.size();

			return msg;
		}
	}
}
