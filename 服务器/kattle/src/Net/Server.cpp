#include"Server.h"
#include"Session.h"
TcpServer::TcpServer(short port)
    : port(port),
    Tcpserver_ip_port(asio::ip::address_v4::any(), port),
    ios_(std::make_shared<asio::io_context>()),
    accept(std::make_shared<asio::ip::tcp::acceptor>(*ios_, Tcpserver_ip_port)), session_num(4),running(false),index(0)
{

}
TcpServer::~TcpServer() {
}
void TcpServer::run() {
    accept.run();
    for (int i = 0; i < session_num; i++) {
        session_arr.push_back(std::make_shared<Session>());
        if(readback)
        session_arr[i]->setReadBack(readback);
        if(writeback)
        session_arr[i]->setWriteBack(writeback);
        if(closeBack){
            session_arr[i]->setCloseBack(closeBack);
        }
    }
    running = true;
    while (running) {
    auto soc  = accept.pop();
    auto& sess = session_arr[index];
    auto mine = std::make_shared<boost::asio::ip::tcp::socket>(*sess->GetLoop()); 
    mine->assign(asio::ip::tcp::v4(), soc->release()); 
    auto conn = std::make_shared<Socket>(mine, sess->GetLoop()); 
    sess->addSocket(conn); 
    index = (index + 1) % session_num;
}
}