#pragma once
#include<boost/asio.hpp>
#include<memory>
class Socket;
using Server_Position = boost::asio::ip::tcp::endpoint;
using EventLoop = std::shared_ptr<boost::asio::io_context>;
using SmartSocket = std::shared_ptr<boost::asio::ip::tcp::socket>;
using TcpSmartSocket = std::shared_ptr<Socket>;