#include "util.hpp"

ns_shared_ptr<char> util::make_shared_buff(size_t size)
{
    return ns_shared_ptr<char>(new char[size] {0}, std::default_delete<char[]>());
}

ns_shared_ptr<char> util::make_shared_from_str(const std::string& str)
{
    auto ptr = make_shared_buff(str.size() + EXT_MEM);
    memset(ptr.get(), 0, str.size() + EXT_MEM);
    strcpy(ptr.get(), str.c_str());
    return ptr;
}

std::string util::uuid()
{
    // 这里是两个() ，因为这里是调用的 () 的运算符重载
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    return boost::uuids::to_string(a_uuid);
    //return "";
}

bool util::is_hex(char c, int& v)
{
    if (0x20 <= c && isdigit(c)) {
        v = c - '0';
        return true;
    }
    else if ('A' <= c && c <= 'F') {
        v = c - 'A' + 10;
        return true;
    }
    else if ('a' <= c && c <= 'f') {
        v = c - 'a' + 10;
        return true;
    }
    return false;
}

bool util::from_hex_to_i(const std::string& s, size_t i, size_t cnt, int& val)
{
    if (i >= s.size()) { return false; }

    val = 0;
    for (; cnt; i++, cnt--) {
        if (!s[i]) { return false; }
        int v = 0;
        if (is_hex(s[i], v)) {
            val = val * 16 + v;
        }
        else {
            return false;
        }
    }
    return true;
}

bool util::from_hex_to_i(const std::string& s, int& val)
{
    return from_hex_to_i(s, 0, s.size(), val);
}

size_t util::to_utf8(int code, char* buff)
{
    if (code < 0x0080) {
        buff[0] = (code & 0x7F);
        return 1;
    }
    else if (code < 0x0800) {
        buff[0] = (0xC0 | ((code >> 6) & 0x1F));
        buff[1] = (0x80 | (code & 0x3F));
        return 2;
    }
    else if (code < 0xD800) {
        buff[0] = (0xE0 | ((code >> 12) & 0xF));
        buff[1] = (0x80 | ((code >> 6) & 0x3F));
        buff[2] = (0x80 | (code & 0x3F));
        return 3;
    }
    else if (code < 0xE000) { // D800 - DFFF is invalid...
        return 0;
    }
    else if (code < 0x10000) {
        buff[0] = (0xE0 | ((code >> 12) & 0xF));
        buff[1] = (0x80 | ((code >> 6) & 0x3F));
        buff[2] = (0x80 | (code & 0x3F));
        return 3;
    }
    else if (code < 0x110000) {
        buff[0] = (0xF0 | ((code >> 18) & 0x7));
        buff[1] = (0x80 | ((code >> 12) & 0x3F));
        buff[2] = (0x80 | ((code >> 6) & 0x3F));
        buff[3] = (0x80 | (code & 0x3F));
        return 4;
    }

    // NOTREACHED
    return 0;
}

std::int64_t util::get_flie_size(const std::string& path)
{
    boost::system::error_code ec;
    auto s = boost::filesystem::file_size(path, ec);
    return s;
}

std::int64_t util::get_flie_size(std::fstream& file)
{
    auto now = file.tellg();
    file.seekg(0, std::ios::end);
    auto size = file.tellg();
    //恢复
    file.seekg(now, std::ios::beg);

    return size;
}

bool util::remove_flie(const std::string& path)
{
    boost::system::error_code ec;
    auto s = boost::filesystem::remove(path, ec);
    return s;
}

bool util::copy_flie(const std::string& src, const std::string& dst)
{
    boost::system::error_code ec;
    boost::filesystem::copy_file(src, dst, ec);
    return true;
}

std::string util::encode_url(const std::string& s)
{
    std::string result;

    for (auto i = 0; s[i]; i++)
    {
        switch (s[i]) {
        case ' ': result += "%20"; break;
        case '+': result += "%2B"; break;
        case '\r': result += "%0D"; break;
        case '\n': result += "%0A"; break;
        case '\'': result += "%27"; break;
        case ',': result += "%2C"; break;
        case ':': result += "%3A"; break;
        case ';': result += "%3B"; break;
        default:
            auto c = static_cast<uint8_t>(s[i]);
            if (c >= 0x80) {
                result += '%';
                char hex[4];
                size_t len = snprintf(hex, sizeof(hex) - 1, "%02X", c);
                //std::assert(len == 2);
                result.append(hex, len);
            }
            else {
                result += s[i];
            }
            break;
        }
    }

    return result;
}

std::string util::decode_url(const std::string& s)
{
    std::string result;

    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 1 < s.size()) {
            if (s[i + 1] == 'u') {
                int val = 0;
                if (from_hex_to_i(s, i + 2, 4, val)) {
                    // 4 digits Unicode codes
                    char buff[4];
                    size_t len = to_utf8(val, buff);
                    if (len > 0) { result.append(buff, len); }
                    i += 5; // 'u0000'
                }
                else {
                    result += s[i];
                }
            }
            else {
                int val = 0;
                if (from_hex_to_i(s, i + 1, 2, val)) {
                    // 2 digits hex codes
                    result += val;
                    i += 2; // '00'
                }
                else {
                    result += s[i];
                }
            }
        }
        else if (s[i] == '+') {
            result += ' ';
        }
        else {
            result += s[i];
        }
    }

    return result;
}

std::string util::lower_case(const std::string& s)
{
    std::string dst;
    for (auto ch_p : s)
    {
        if (ch_p >= 'A' && ch_p <= 'Z')
        {
            ch_p = 'a' + ch_p - 'A';
        }
        dst += ch_p;
    }
    return dst;
}

std::string util::upper_case(const std::string& s)
{
    std::string dst;
    for (auto ch_p : s)
    {
        if (ch_p >= 'a' && ch_p <= 'z')
        {
            ch_p = 'A' + ch_p - 'a';
        }
        dst += ch_p;
    }
    return dst;
}
