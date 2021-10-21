#include <iostream>
#include <chrono>

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

std::vector<char> vBuffer(20 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket){
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
		[&](std::error_code ec, std::size_t length)
		{
			if(!ec){
				std::cout << "\nRead " << length << " bytes\n\n";
				
				for(int i = 0; i < length; i++){
					std::cout << vBuffer[i];
				}
				GrabSomeData(socket);
			}
		}
	);
		
}

int main()
{
    asio::error_code ec;

    asio::io_context context;
    
    asio::io_context::work idleWork(context);
    
    //runs the context nothing is left to do
    std::thread ctxThread = std::thread([&]() { context.run(); });

    asio::ip::address address = asio::ip::make_address("93.184.216.34" , ec);
    asio::ip::tcp::endpoint endpoint(address, 80);
    
    asio::ip::tcp::socket socket(context);

    socket.connect(endpoint, ec);

    if(!ec)
    {
        std::cout << "Connected!" << std::endl;
    }
    else
    {
        std::cout << "Failed to connect to address:\n" << ec.message() << std::endl; 
    }

    if(socket.is_open())
    {
    	GrabSomeData(socket);
    	        
        std::string sRequest = 
            "GET /index.html HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n";
        
        asio::const_buffers_1 inputBuffer = asio::buffer(sRequest.data(), sRequest.size());

        socket.write_some(inputBuffer, ec);
        

        std::this_thread::sleep_for( std::chrono::milliseconds(200));
        
        
        context.stop();
        if(ctxThread.joinable()) ctxThread.join();
        
//		socket.wait(socket.wait_read);
//
//        size_t bytes = socket.available();
//        std::cout << "Bytes Available: " << bytes << std::endl;
//        if(bytes > 0)
//        {
//            std::vector<char> vBuffer(bytes);
//            asio::mutable_buffers_1 outputBuffer = asio::buffer(vBuffer.data(), vBuffer.size());
//            socket.read_some(outputBuffer, ec);
//            
//            for(auto outputByte : vBuffer)
//                std::cout << outputByte;
//        }    
    }
    return 0;
}
