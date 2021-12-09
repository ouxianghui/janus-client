#pragma once

#include <string>
#include <exception>
#if !defined(__ANDROID__)
#include <codecvt>
#endif
#include <locale>

template<typename T>
struct stringable {
private:
	stringable();
};

template<>
struct stringable<std::exception>
{
	std::string operator()(const std::exception& e) { return e.what(); }
    std::exception convert_back(const std::string& s) { return std::runtime_error(s); }
};

template<>
struct stringable<std::string>
{
	std::string operator()(const std::string& s) { return s; }
    std::string operator()(const char* s) {if(nullptr == s) return ""; else return s;}
	std::string convert_back(const std::string& s) {return s; }
};

//unicode <-> utf8 translator
template<>
struct stringable<std::wstring>
{
    std::string operator()(const std::wstring& value)
    {
#if defined(__ANDROID__)
        throw std::runtime_error("not implemented for ndk");
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t>> convt;
        return convt.to_bytes(value);
#endif
    }
    
    std::wstring convert_back(const std::string& value)
    {
#if defined(__ANDROID__)
        throw std::runtime_error("not implemented for ndk");
#else
        try {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> convt;
            return convt.from_bytes(value);
        }
        catch (std::range_error&)
        {
            std::wstring wtext;
            for (auto& c : value)
                wtext += wchar_t(c);
            return wtext;
        }
#endif
    }
};

