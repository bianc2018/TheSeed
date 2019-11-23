#include "message.h"

size_t msg::Message::get_body_len()
{
    return body_.get_size();
}

util::BufferCache& msg::Message::get_body()
{
    return body_;
}

std::string msg::Message::get_head_value(const std::string& key, const std::string& notfond)
{
    auto p = header_.find(key);
    if (header_.end() == p)
        return notfond;
    return p->second;
}


bool msg::Message::set_head_value_not_over(const std::string& key, const std::string& value)
{
    if ("" == get_head_value(key))
    {
        //没有对应的
        return set_head_value(key, value);
    }

    return false;
}

bool msg::Message::set_head_value(const std::string& key, const std::string& value)
{
    header_[key] = value;
    return true;
}

std::string msg::Message::get_head()
{
    //增加字段
    set_head_value_not_over(BODY_LEN, std::to_string(body_.get_size()));
    //set_head_value_not_over(KEEP, "false");

    std::string head;
    for (auto& h : header_)
    {

        head += h.first + HEAD_SPLIT + h.second + CRLF;

    }
    head += CRLF;
    return head;
}

bool msg::Message::set_method(const std::string& method)
{
    return set_head_value(METHOD, method);
}

std::string msg::Message::get_method(const std::string& not_find)
{
    return get_head_value(METHOD, not_find);
}

bool msg::Message::set_cmd(const std::string& cmd)
{
    return set_head_value(CMD, cmd);
}

std::string msg::Message::get_cmd(const std::string& not_find)
{
    return get_head_value(CMD, not_find);
}

bool msg::Message::set_token(const std::string& token)
{
    return set_head_value(TOKEN, token);
}

std::string msg::Message::get_token(const std::string& not_find)
{
    return get_head_value(TOKEN, not_find);
}

bool msg::Message::set_src_node(const std::string& ip, int port)
{
    set_head_value(SRC_IP, ip);
    set_head_value(SRC_PORT, std::to_string(port));
    return true;
}

std::pair<std::string, int> msg::Message::get_src_node(const std::pair<std::string, int>& not_find)
{
    std::pair<std::string, int> p;
    p.first = get_head_value(SRC_IP, not_find.first);
    auto port = get_head_value(SRC_PORT, "");
    if ("" == port)
    {
        p.second = not_find.second;
    }
    else
    {
        p.second = std::stod(port);
    }
    return p;
}

bool msg::Message::set_dst_node(const std::string& ip, int port)
{
    set_head_value(DST_IP, ip);
    set_head_value(DST_PORT, std::to_string(port));
    return true;
}

std::pair<std::string, int> msg::Message::get_dst_node(const std::pair<std::string, int>& not_find)
{
    std::pair<std::string, int> p;
    p.first = get_head_value(DST_IP, not_find.first);
    auto port = get_head_value(DST_PORT, "");
    if ("" == port)
    {
        p.second = not_find.second;
    }
    else
    {
        p.second = std::stod(port);
    }
    return p;
}

bool msg::Message::set_close(bool flag)
{
    if (flag)
    {
        set_head_value(Close, "true");
    }
    else
    {
        set_head_value(Close, "false");
    }
    return true;
}

bool msg::Message::get_close(const bool& not_find)
{
    auto p = get_head_value(Close, "");
    if ("" == p)
        return not_find;

    if ("true" == p)
    {
        return true;
    }
    else if ("false" == p)
    {
        return false;
    }

    return not_find;
}

bool msg::Message::set_body_from_file(const std::string& file_path)
{
    return body_.set_file_content(file_path);
}

bool msg::Message::save_body_as_file(const std::string& file_path)
{
    return  body_.save_content_as_file(file_path);;
}

bool msg::Message::write_body(const char* data, size_t len)
{
    return body_.write(data,len);
}

std::int64_t msg::Message::read_body(char* buff, size_t len)
{
    return body_.read_content(buff,len);
}
