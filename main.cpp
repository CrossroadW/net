#include <arpa/inet.h>
#include "debug.hpp"
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <source_location>
#include <netdb.h>
auto checkError(auto res, std::source_location const &loc = std::source_location::current())
{
    if (res == -1) [[unlikely]]
    {
        throw std::system_error(errno, std::system_category(),
                                (std::string)loc.file_name() + ":" + std::to_string(loc.line()));
    }
    return res;
}
void foo(int fd)
{
    char buf[100];
    while (1)
    {
        memset(buf, 0, sizeof(buf));
        int sz = checkError(read(fd, buf, sizeof(buf)));
        if (sz == 0)
        {
            debug(), "client EOF";
            break;
        }
        debug(), std::string(buf, sz - 1);
        checkError(write(fd, buf, sz - 1));
    }
}
struct Connection
{
    // Connection(){}
    int sock_fd_;
    std::string ip_;
    uint16_t port_;
    sockaddr_in cli_addr_;
    auto readAll()
    {
        std::string all('\0', 100);
        char buf[128];
        while (int size = checkError(::read(sock_fd_, buf, sizeof(buf))))
        {
            all += std::string(buf, size);
        }

        return all;
    }
    auto read(int n)
    {
        std::string nread('\0',n);
        char buf[128];

        int has_read = 0;
        int need = std::min(128,n - has_read);
        
        while(has_read < n){
            int sz = checkError(::read(sock_fd_,buf,need));
            if(sz == 0)break;
            nread += std::string(buf,sz);
            has_read += sz;
            need = std::min(128,n - has_read);
        }
        if(has_read != n) throw std::runtime_error("read error");
        return nread;
    }
    auto readInt()
    {
        char buf[4];
        int val = -1;
        int size = checkError(::read(sock_fd_, buf, sizeof(buf)));
        if (size < 4)
            return -1;

        if (size == 4)
        {
            memcpy(&val, buf, sizeof(buf));
            return (int)ntohl(val);
        }
        throw std::runtime_error("错误返回");
    }
    ~Connection(){
        debug(),__PRETTY_FUNCTION__;
        close(sock_fd_);
    }
};
struct Server
{
    explicit Server(std::string ip, uint16_t port) : ip_(ip), port_(port),
                                                     sock_fd_(checkError(socket(AF_INET, SOCK_STREAM, 0)))
    {
        addr_in_.sin_family = AF_INET;
        addr_in_.sin_port = htons(port_);
        checkError(inet_pton(AF_INET, ip_.c_str(), &addr_in_.sin_addr));
    }
    void bind()
    {
        checkError(::bind(sock_fd_, (sockaddr *)&addr_in_, sizeof(addr_in_)));
    }
    void listen()
    {
        checkError(::listen(sock_fd_, 128));
    }
    auto accept()
    {
        sockaddr_in cli_in;
        uint32_t len_cli = sizeof(cli_in);
        int cli_fd = checkError(::accept(sock_fd_, (sockaddr *)&cli_in, &len_cli));
        char ip_str[16];
        debug(), "client ip: ", inet_ntop(AF_INET, &cli_in.sin_addr, ip_str, sizeof(ip_str));
        debug(), "client port: ", ntohs(cli_in.sin_port);
        Connection conn{.sock_fd_ = cli_fd,
                        .ip_ = inet_ntop(AF_INET, &cli_in.sin_addr, ip_str, sizeof(ip_str)),
                        .port_ = ntohs(cli_in.sin_port),
                        .cli_addr_ = cli_in};
        return conn;
    }
    ~Server(){
        debug(),__PRETTY_FUNCTION__;
        close(sock_fd_);
    }
    int sock_fd_;
    std::string ip_;
    uint16_t port_;
    sockaddr_in addr_in_;
};
int main(int argc, char const *argv[])
{
    // int fd = checkError(socket(AF_INET, SOCK_STREAM, 0));

    // sockaddr_in addr_in;
    // addr_in.sin_family = AF_INET;
    // addr_in.sin_port = htons(10666);
    // checkError(inet_pton(AF_INET, "127.0.0.1", &addr_in.sin_addr)); // s_add 也可以赋值 INADDR_ANY
    // checkError(bind(fd, (sockaddr *)&addr_in, sizeof(addr_in)));
    // checkError(listen(fd, 128));

    // sockaddr_in cli_in;
    // uint32_t len_cli = sizeof(cli_in);
    // int cli_fd = checkError(accept(fd, (sockaddr *)&cli_in, &len_cli));
    // char ip_str[16];
    // debug(), "client ip: ", inet_ntop(AF_INET, &cli_in.sin_addr, ip_str, sizeof(ip_str));
    // debug(), "client port: ", ntohs(cli_in.sin_port);
    // foo(cli_fd);

    // close(fd);
    // close(cli_fd);
    Server s("127.0.0.1", 12989);
    s.bind();
    s.listen();
    auto con = s.accept();
    int n = con.readInt();
    debug(),"read bytes cnt: ",n;
    debug(),con.read(n);
}
