/*
	数据缓存
*/
#ifndef NS_CACHE_HPP_
#define NS_CACHE_HPP_
#include <string>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>

#include "util.hpp"
#include "logging.hpp"

namespace util
{
    enum CACHE_CONTENT_TYPE
    {
        //直接缓存在内存中
        CACHE_STRING = 0,
        //直接缓存在硬盘文件中
        CACHE_FILE = 1,
        //直接缓存在缓存文件中
        CACHE_TEMP = 2,
        //无请求体
        CACHE_NONE = 3,
    };

    class BufferCache
    {
    public:
        BufferCache() :
            MAX_BODY_SIZE(5 * 1024), type_(CACHE_NONE), \
            w_ptr_(0), r_ptr_(0), range_(0, 0)
        {}
        ~BufferCache()
        {
            clear();
        }
        std::string read_content(std::int64_t npos, size_t len)
        {
            if (0 == len)
                return "";

            auto buff = util::make_shared_array<char>(len);

            auto size = read_content(npos, buff.get(), len);

            if (size > 0)
                return "";

            return std::string(buff.get(), size);
        }
        std::string read_content(size_t len)
        {
            if (0 == len)
                return "";

            auto buff = util::make_shared_array<char>(len);

            auto size = read_content(buff.get(), len);

            if (size > 0)
                return "";

            return std::string(buff.get(), size);
        }
        std::int64_t read_content(std::int64_t npos, char* buff, size_t len)
        {
            if (npos < 0)
                return -1;

            r_ptr_ = npos;

            return read_content(buff, len);
        }
        std::int64_t read_content(char* buff, size_t len)
        {
            if (0 == len)
                return 0;

            if (r_ptr_ >= get_size())
                return 0;

            std::int64_t readlen = 0;
            if (CACHE_STRING == type_)
            {
                if (body_info_.size() < r_ptr_ + len)
                    readlen = body_info_.size();
                else
                    readlen = len;

                memcpy(buff, body_info_.c_str() + r_ptr_, readlen);
            }
            else if (CACHE_FILE == type_)
            {
                //读取起点
                auto ptr = r_ptr_ + range_.first;
                //剩余可读数据长度
                auto datalen = range_.second - ptr + 1;
                if (datalen <= 0)
                {
                    readlen = 0;
                }
                else
                {
                    if (datalen < len)
                        readlen = datalen;//读完
                    else
                        readlen = len;//读要求长度

                    cache_file_.seekg(ptr, std::ios::beg);
                    cache_file_.read(buff, readlen);
                    readlen = cache_file_.gcount();
                }
            }
            else if (CACHE_TEMP == type_)
            {
                cache_file_.seekg(r_ptr_, std::ios::beg);
                cache_file_.read(buff, len);
                readlen = cache_file_.gcount();
            }

            //读指针前移
            if (readlen > 0)
                r_ptr_ += readlen;
            return readlen;
        }

        bool write(std::int64_t npos, const std::string& data)
        {
            return write(npos, data.c_str(), data.size());
        }
        bool write(const std::string& data)//app
        {
            return write(data.c_str(), data.size());
        }
        bool write(std::int64_t npos, const char* data, size_t len)
        {
            if (npos < 0)
                return false;

            w_ptr_ = npos;

            return write(data, len);
        }
        bool write(const char* data, size_t len)//app
        {
            //不可写空数据 和 磁盘文件
            if (0 == len || CACHE_FILE == type_)
                return false;

            if (CACHE_STRING == type_)
            {
                body_info_ = body_info_.substr(0, w_ptr_) + \
                    std::string(data, len) + body_info_.substr(w_ptr_);

                if (MAX_BODY_SIZE <= body_info_.size())
                    return create_tmp();
                //return true;
            }
            else if (CACHE_TEMP == type_)
            {
                if (!cache_file_)
                    return false;
                cache_file_.seekp(w_ptr_, std::ios::beg);
                cache_file_.write(data, len);
                
                //std::system()
            }
            else if (CACHE_NONE == type_)
            {
                type_ = CACHE_STRING;
                body_info_ = std::string(data, len);
                if (MAX_BODY_SIZE <= body_info_.size())
                    return create_tmp();
                //return true;
            }

            w_ptr_ += len;
            return true;
        }

        bool set_file_content(const std::string& path)
        {
            //cache_file_
            clear();

            cache_file_.open(path, std::ios::binary | std::ios::in);
            if (!cache_file_)
            {
                LOG_ERR << "body_info_ not is a file:" << path;
                return false;
            }

            type_ = CACHE_FILE;
            body_info_ = path;
            set_range(-1, -1);
            return true;
        }

        bool save_content_as_file(const std::string& path)
        {
            if (CACHE_NONE == type_)
                return false;

            cache_file_.sync();

            if (CACHE_TEMP == type_ || CACHE_FILE == type_)
                return util::copy_flie(body_info_, path);
            else if (CACHE_STRING == type_)
            {
                //打开文件
                std::fstream file(path, \
                    std::ios_base::out | std::ios_base::app | std::ios_base::binary);//
                if (!file)
                {
                    LOG_ERR << "this path not is a file:" << path;
                    return false;
                }

                if ("" != body_info_)
                    file.write(body_info_.c_str(), body_info_.size());
                file.close();
                return true;
            }
            return false;
        }

        bool set_range(std::int64_t b, std::int64_t e)//[]
        {
            if (CACHE_FILE == type_)
            {
                std::pair<std::int64_t, std::int64_t> tmp;
                if (b < 0)
                {
                    tmp.first = 0;
                }
                else
                {
                    tmp.first = b;
                }
                if (e < 0)
                {
                    tmp.second = util::get_flie_size(body_info_);
                }
                else
                {
                    tmp.second = e;
                }

                if (tmp.first > tmp.second)
                    return false;
                range_ = tmp;
                return true;
            }
            return false;
        }
        std::int64_t get_size()
        {
            if (CACHE_NONE == type_)
                return 0;
            else if (CACHE_STRING == type_)
                return body_info_.size();
            else if (CACHE_TEMP == type_ || CACHE_FILE == type_)
                return util::get_flie_size(cache_file_);

            return 0;
        }

        CACHE_CONTENT_TYPE get_content_type()
        {
            return type_;
        }

        bool set_other(const std::string& key, const std::string& value)
        {
            other_map_[key] = value;
        }
        bool remove_other(const std::string& key)
        {
            other_map_.erase(key);
        }
        std::string get_other(const std::string& key, const std::string& notfind = "")
        {
            auto f = other_map_.find(key);
            if (other_map_.end() == f)
                return notfind;
            return f->second;
        }

        bool clear()
        {
            if (CACHE_FILE == type_)
            {
                cache_file_.close();
            }
            else if (CACHE_TEMP == type_)
            {
                cache_file_.close();
                body_info_ = "";
                type_ = CACHE_NONE;
                w_ptr_ = 0;
                r_ptr_ = 0;
                LOG_DBG << "remove temp body_info_:" << body_info_;
                return util::remove_flie(body_info_);
            }

            body_info_ = "";
            type_ = CACHE_NONE;
            w_ptr_ = 0;
            r_ptr_ = 0;
            return true;
        }
    public:
        //name 内存块名
        std::string name_;
    private:
        //创建缓存文件
        bool create_tmp()
        {
            auto tmp_filen_name = "./" + util::uuid() + ".http_temp";

            //缓存
            std::string buff;
            if (CACHE_STRING == type_)
            {
                buff = body_info_;
            }

            //清空
            clear();

            //打开文件
            cache_file_.open(tmp_filen_name, \
                std::ios_base::out | std::ios_base::app | std::ios_base::binary);//
            if (!cache_file_)
            {
                LOG_ERR << "body_info_ not is a file:" << tmp_filen_name;
                return false;
            }

            if ("" != buff)
                cache_file_.write(buff.c_str(), buff.size());
            w_ptr_ = buff.size();
            type_ = CACHE_TEMP;
            body_info_ = tmp_filen_name;
            return true;
        }

    private:
        CACHE_CONTENT_TYPE type_;
        std::map<std::string, std::string> other_map_;

        std::string body_info_;
        std::fstream cache_file_;

        //范围从文件读取 表示在文件中的范围
        std::pair<std::int64_t, std::int64_t> range_;

        //读写指针
        std::int64_t w_ptr_, r_ptr_;

        const std::int64_t MAX_BODY_SIZE;

    };

}

#endif