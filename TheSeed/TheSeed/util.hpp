
#ifndef  NS_UTIL_HPP_
#define  NS_UTIL_HPP_


#ifndef __UNIX
//#include <WinSock2.h>
//#pragma comment(lib, "Ws2_32.lib")
//#pragma warning(disable:4996)
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <memory>

#include <boost/filesystem.hpp>

//动态数组指针
#define ns_shared_ptr std::shared_ptr
#define ns_make_shared std::make_shared
#define ns_bind std::bind
#define ns_func std::function
#define ns_weak_ptr std::weak_ptr
#define SHARED_BUFF_PTR(size) ns_shared_ptr<char>(new char[size], std::default_delete<char[]>())
#define SHARED_ANY_PTR(type,size) ns_shared_ptr<type>(new type[size], std::default_delete<type[]>())


namespace util
{

    //拓展的区域
    const size_t EXT_MEM(5);

    ns_shared_ptr<char> make_shared_buff(size_t size);

    template<typename T>
    ns_shared_ptr<T> make_shared_array(size_t size)
    {
        return ns_shared_ptr<T>(new T[size], std::default_delete<T[]>());
    }

    ns_shared_ptr<char> make_shared_from_str(const std::string& str);
    
    std::string uuid();

    bool is_hex(char c, int& v);

    bool from_hex_to_i(const std::string& s, size_t i, size_t cnt,
        int& val);
    
    bool from_hex_to_i(const std::string& s, int& val);
    
    size_t to_utf8(int code, char* buff);


    //获取文件大小
    std::int64_t get_flie_size(const std::string& path);
    /*
    fsWrite.seekp(0, fsWrite.end);
size_t dstFileSize = fsWrite.tellp();
    */
    //获取文件大小
    std::int64_t get_flie_size(std::fstream& file);

    //删除文件
    bool remove_flie(const std::string& path);

    //删除文件
    bool copy_flie(const std::string& src, const std::string& dst);

    std::string encode_url(const std::string& s);

    std::string decode_url(const std::string& s);

    //转换成小写
    std::string lower_case(const std::string& s);

    //转换成大写
    std::string upper_case(const std::string& s);
}

#endif
