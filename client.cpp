#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "debug.hpp"
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <source_location>
auto checkError(auto res, std::source_location const &loc = std::source_location::current())
{
    if (res == -1) [[unlikely]]
    {
        throw std::system_error(errno, std::system_category(),
                                (std::string)loc.file_name() + ":" + std::to_string(loc.line()));
    }
    return res;
}
/**
 * return 域名对应ip,domain也可以直接是ip
 *  sockaddr_in addr_in;
    std::string ip = "176.246.37.101";
    debug(), ipv4_get(&addr_in.sin_addr, ip).c_str();
 */
std::string ipv4_get(in_addr *addr_in, std::string domain)
{
    if (checkError(inet_pton(AF_INET, domain.data(), addr_in)) == 0)
    {
        hostent *hent = gethostbyname2(domain.data(), AF_INET);
        memcpy(addr_in, hent->h_addr_list[0], sizeof(addr_in));
    }
    std::string buf(16, '0');
    inet_ntop(AF_INET, addr_in, buf.data(), buf.size());
    return buf;
}
void writeInt(int fd,int val){
    char buf[4];
    val = (int)htonl(val);
    memcpy(buf,&val,sizeof(val));
    int sz = checkError(write(fd,buf,sizeof(buf)));
    if(sz != 4)
    std::runtime_error("write error");
}
int main()
{
    int fd = checkError(socket(AF_INET, SOCK_STREAM, 0));
    sockaddr_in ser_addr;
    ser_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &ser_addr.sin_addr);
    ser_addr.sin_port = htons(12989);
    checkError(connect(fd, (sockaddr *)&ser_addr, sizeof(ser_addr)));

    char buf[100];
    int n;
    while (1)
    {
        n = 0;
        memset(buf, 0, sizeof(buf));
        while ((buf[n++] = getchar()) != '\n');
        writeInt(fd,n-1);
        checkError(write(fd, buf, n-1));
        // memset(buf, 0, sizeof(buf));
        // int sz = checkError(read(fd, buf, sizeof(buf)));
        // debug(), "read: ", std::string(buf, sz);
    }
    close(fd);
   
}