#ifndef _TOOLS_HPP_
#define _TOOLS_HPP_
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

#include <boost/algorithm/string.hpp>

using namespace std;

#define LOG(lev, msg) Log(lev, __FILE__, __LINE__, msg)
class TimeInit
{
  public:
    //写时间的时候，要用int64_t
    static int64_t GetTimeStampMs()
    {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      return tv.tv_sec + tv.tv_usec / 1000;
    }

    static string GetTimeStamp()
    {
      //获取当前系统时间
      time_t t;
      time(&t);
      struct tm *s = localtime(&t);

      // 返回时间
      return "[" + to_string(s->tm_year + 1900) + "-" + to_string(s->tm_mon + 1) + "-" + to_string(s->tm_mday) + " " + to_string(s->tm_hour) + ":" + to_string(s->tm_min) + ":" + to_string(s->tm_sec) + "]";
    }
};
class LogInfo
{
  public:
    static void WrietLogInfo(const string& logInfo)
    {
      int fd = open("./Log_File/log", O_CREAT | O_WRONLY | O_APPEND, 0666);
      if(fd < 0)
      {
        cout << -1 << endl;
      }
      write(fd, logInfo.c_str(), logInfo.size());
    }
};
enum LogLevel
{
  INFO=0,
  WARNING,
  ERROR,
  FATAL,
  DEBUG
};
//错误等级
const char* Level[] = {"INFO", "WARNING", "ERROR", "FATAL", "DEBUG"};

void Log(LogLevel lev, const char* file, int line, string& logmsg)
{
  string lev_info = Level[lev];

  logmsg += TimeInit::GetTimeStamp() + " " +  lev_info + " " + file + " " + to_string(line);
}

class FileInit
{
  public:
    static bool ReadFile(const string& filename, string& retcontent)
    {
      //从文件中读内容，然后返回给retcontent
      ifstream File(filename.c_str());
      if(!File.is_open())
      {
        string msg;    
        LOG(ERROR, msg);
        msg += " ReadFile of " + filename + " failed\n";
        LogInfo::WrietLogInfo(msg);
        //记录日志信息
        return false;
      }
      string line;
      while(getline(File, line))
      {
        retcontent += line + "\n";
      }
      File.close();
      return true;
    }

    static bool WriteFile(const string& filename, const string& code)
    {
      ofstream File(filename.c_str());
      if(!File.is_open())
      {
        string msg;    
        LOG(ERROR, msg);
        msg += " WrietFile of " + filename + " failed\n";
        LogInfo::WrietLogInfo(msg);  
        //记录日志信息
        return false;
      }

      File.write(code.c_str(), code.size());
      File.close();
      return true;
    }
};


class StringInit
{
  public:
    static void Split(const string& input, string split_char, vector<string>& output)
    {
      //output为输出，input为输入，is_any_of中以什么为分割符，token_compress_on 此时若连续有多个分隔符时，将这些符号压缩为，即为1个压缩
      boost::split(output, input, boost::is_any_of(split_char), boost::token_compress_on);
    }
};

class UrlInit
{
  public:
    static void WebContentDecodeCutting(const std::string& content, unordered_map<string, string>& content_kv)
    {
      //需要对网页内容进行切割
      //再对切割后的内容进行转码
      //先切割，后转码
      vector<string> kv_vec;
      StringInit::Split(content, "&", kv_vec);
      //可能有多个提交，比如测试代码，则需要用多个key-value形式
      for(const auto& e : kv_vec)
      {
        vector<string> sig_kv;
        StringInit::Split(e, "=", sig_kv);

        //网页提交的内容为，key--value形式，经过切割之后，必须会有两个内容
        if(sig_kv.size() != 2)
          continue;
        //对内容进行url解码，然后保存
        content_kv[sig_kv[0]] = UrlDecode(sig_kv[1]);
      }
    }

  private:

    static unsigned char ToHex(unsigned char x)   
    {   
      return  x > 9 ? x + 55 : x + 48;   
    }  

    static unsigned char FromHex(unsigned char x)   
    {   
      unsigned char y;  
      if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;  
      else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;  
      else if (x >= '0' && x <= '9') y = x - '0';  
      else assert(0);  
      return y;  
    }  

    static string UrlEncode(const std::string& str)  
    {  
      string strTemp = "";  
      size_t length = str.length();  
      for (size_t i = 0; i < length; i++)  
      {  
        if (isalnum((unsigned char)str[i]) ||   
            (str[i] == '-') ||  
            (str[i] == '_') ||   
            (str[i] == '.') ||   
            (str[i] == '~'))  
          strTemp += str[i];  
        else if (str[i] == ' ')  
          strTemp += "+";  
        else  
        {  
          strTemp += '%';  
          strTemp += ToHex((unsigned char)str[i] >> 4);  
          strTemp += ToHex((unsigned char)str[i] % 16);  
        }  
      }  
      return strTemp;  
    }  

    static string UrlDecode(const std::string& str)  
    {  
      string strTemp = "";  
      size_t length = str.length();  
      for (size_t i = 0; i < length; i++)  
      {  
        if (str[i] == '+') strTemp += ' ';  
        else if (str[i] == '%')  
        {  
          assert(i + 2 < length);  
          unsigned char high = FromHex((unsigned char)str[++i]);  
          unsigned char low = FromHex((unsigned char)str[++i]);  
          strTemp += high*16 + low;  
        }  
        else strTemp += str[i];  
      }  
      return strTemp;  
    }  
};


#endif
