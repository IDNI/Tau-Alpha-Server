//
// Created by ak on 18/09/2019.
//

#include "TcpConnection.h"

boost::shared_ptr<TcpConnection>
TcpConnection::create(boost::asio::io_context &io_context, std::string uid, MessageOperations &messageOperations) {
    return boost::shared_ptr<TcpConnection>(new TcpConnection(io_context, uid, messageOperations));
}

TcpConnection::TcpConnection(boost::asio::io_context &io_context, std::string uid, MessageOperations &messageOperations)
        : io_context_(&io_context),
          socket_(io_context),
          uid_(uid),
          messageOperations_(messageOperations) {

}

bool TcpConnection::connect(tcp::resolver::results_type &tcpEndpoint) {
     #ifdef DEBUG_COUT
std::cout << __FILE__ << " " << __FUNCTION__ << " connecting to "
                  << tcpEndpoint->host_name()
                  << ":"
                  << tcpEndpoint->service_name()
                  << std::endl;
#endif

    boost::asio::connect(
            socket_,
            tcpEndpoint);
    return true;
}

bool TcpConnection::requestUnreadMessages(boost::system::error_code &ec) {
    #ifdef DEBUG_COUT
std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif
    json msg;
    msg["action"] = "Get messages";
    msg["recipientName"] = uid_;

    boost::asio::write(socket_, boost::asio::buffer(msg.dump() + "\n"), ec);
    if (ec) {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                  " " << ec << std::endl;
        return false;
    }
    return true;
}

std::tuple<json,size_t,size_t> TcpConnection::readHeader(boost::system::error_code &ec){
    json messageHeader;
    #ifdef DEBUG_COUT
std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif
    size_t messageHeaderSize = boost::asio::read_until(socket_, inbuf, '\n', ec);

    if (ec) {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                  " " << ec << std::endl;
    }

    #ifdef DEBUG_COUT
std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " "
              << "got HEADER size " << messageHeaderSize << std::endl;
#endif

    if (messageHeaderSize == 0) {
        #ifdef DEBUG_COUT
std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif
        return std::make_tuple(messageHeader, 0, 0);
    }

    std::string messageHeaderString(
            boost::asio::buffers_begin(inbuf.data()),
            boost::asio::buffers_begin(inbuf.data()) + messageHeaderSize);

    #ifdef DEBUG_COUT
    std::cout << messageHeaderString << std::endl;
std::cout << "----------------HEADER-----------------------" << std::endl;
#endif

    try {
        messageHeader = json::parse(messageHeaderString);
    } catch (std::exception &e) {
        std::cerr << __FILE__ << " : " << __FUNCTION__ << " : "
                  << std::dec << __LINE__ << " : " << e.what() << std::endl;
        return std::make_tuple(messageHeader, messageHeaderSize, 0);
    }
    //now we have the message size
    size_t messageSize = messageHeader["messageSize"].get<size_t>();

    inbuf.consume(messageHeaderSize);

    return std::make_tuple(messageHeader, messageHeaderSize, messageSize);
}

size_t TcpConnection::readMessage(boost::system::error_code &ec, size_t messageSize) {
    #ifdef DEBUG_COUT
std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif
    size_t bytes_transferred = boost::asio::read(socket_, inbuf, boost::asio::transfer_exactly(messageSize), ec);
    if (ec && ec != boost::asio::error::eof) {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                  " " << ec << std::endl;
    }

    #ifdef DEBUG_COUT
std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " "
              << "got message size " << bytes_transferred << std::endl;
#endif

    return bytes_transferred;
}

bool TcpConnection::getMessages() {
    #ifdef DEBUG_COUT
std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif
    boost::system::error_code ec;

    if(!requestUnreadMessages(ec)){
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                  " " << ec << std::endl;
        return false;
    }

    while (ec != boost::asio::error::eof) {
        auto [messageHeader, messageHeaderSize, messageSize] = readHeader(ec);

        if(!messageHeaderSize || !messageSize) {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
                      " " << ec << std::endl;
            break;
        }

        readMessage(ec, messageSize);

        #ifdef DEBUG_COUT
std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__
        << " " << "total transferred " << messageSize
        << " + " << messageHeaderSize <<
        " = " << messageHeaderSize + messageSize << std::endl;
#endif

        messageOperations_.save(messageHeader, inputStream, messageSize);

//        inbuf.consume(messageSize);
    }

    return true;
}

TcpConnection::~TcpConnection() {
    #ifdef DEBUG_COUT
std::cout << __FILE__ << " : " << __FUNCTION__ << " : "
              << std::dec << __LINE__ << " : " << "TcpConnection destroyed!" << std::endl;
#endif
}