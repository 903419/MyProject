#ifndef _COMPILE_HPP_
#define _COMPILE_HPP_ 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <fcntl.h>


#include <iostream>
#include <string>
#include <atomic>

#include <json/json.h>

#include "tools.hpp"
using namespace std;
enum ErrorNo
{
  OK = 0,
  PRAM_ERROR,//参数错误
  INTERNAL_ERROR,//内部错误
  COMPILE_ERROR,//编译错误
  RUN_ERROR//运行错误
};

class Complier
{
public:
  //req：调用者的请求，包含多个key-value 
  //resp：出参，返回给调用者
  static void CompileAndRun(Json::Value& req, Json::Value& resp)
  {
    // 1.参数是否是错误的，Json串中code是否为空
    if(req["code"].empty())
    {
      resp["errorno"] = PRAM_ERROR;
      resp["reason"] = "parm erroe";
      return;
    }
    // 2.将代码写到文件中去
    string code = req["code"].asString();
    //文件命名时，加上了时间戳,区分不同的请求
    string file_nameheader = WriteTmpFile(code);
    if(file_nameheader.empty())
    {
      resp["errorno"] = INTERNAL_ERROR;
      resp["reason"] = "write to file failed";
      Clean(file_nameheader);
      return;
    }
    // 3.编译
    if(!Compile(file_nameheader))
    {
      resp["errono"] = COMPILE_ERROR;
      string reason;
      FileInit::ReadFile(CompileErrorPath(file_nameheader), reason);
      resp["reason"] = reason;
      Clean(file_nameheader);
      return;
    }

    // 4.运行
    int ret = Run(file_nameheader);
    if(ret != 0)
    {
      resp["errono"] = RUN_ERROR;
      resp["reason"] = "parm exit by reason" + to_string(ret);
      Clean(file_nameheader);
      return;
    }

    // 5.构造响应
    resp["errono"] = OK;
    resp["reason"] = "Compile and Run OK";
    string stdout_str;
    FileInit::ReadFile(StdoutPath(file_nameheader), stdout_str);
    resp["stdout"] = stdout_str;

    string stderr_str;
    FileInit::ReadFile(StderrPath(file_nameheader), stderr_str);
    resp["stderr"] = stderr_str;

    // 6.清空临时文件
    
    Clean(file_nameheader);
  }


private:

  static void Clean(const string& file_name)
  {
    unlink(SrcPath(file_name).c_str());
    unlink(ExePath(file_name).c_str());
    unlink(CompileErrorPath(file_name).c_str());
    unlink(StderrPath(file_name).c_str());
    unlink(StdoutPath(file_name).c_str());
  }

  static int Run(const string& file_name)
  {
    //创建子进程
    //父进程进行进程等待
    //子进程进行进程程序替换
    int pid = fork();
    if(pid > 0)
    {
      //获取子进程退出状态
      int status = 0;
      waitpid(pid, &status, 0);
      //退出码
      return status & 0x7f;
    }
    else if(pid == 0)
    {
      //限制子进程的运行时间
      alarm(2);//采用alarm信号

      //限制子进程的运行内存，若用户提交的代码有死递归，或者递归层数过多，则不满足oj的条件
      struct rlimit rlim;
      rlim.rlim_cur = 3000 * 1204;//3000k，软限制
      rlim.rlim_max = RLIM_INFINITY;//不限制最大运行内存大小
      //setrlimit(int resource, const struct rlimit *rlim);
      // 1. resource为限制什么资源  RLIMIT_AS：进程虚拟内存大小，RLIMIT_CORE：核心转储大小，RLIMIT_CPU：占用cpu时间限制，RLIMIT_DATA：进程最大的数据段
      // 2.struct rlimit *rlim 为软硬限制
      setrlimit(RLIMIT_AS, &rlim);

      //重定向，标准输出，标准错误
      int stdout_fd = open(StdoutPath(file_name).c_str(), O_CREAT | O_WRONLY, 0666);
      if(stdout_fd < 0)
      {
        return -1;
      }
      dup2(stdout_fd, 1);
      int stderr_fd = open(StderrPath(file_name).c_str(), O_CREAT | O_WRONLY, 0666);
      if(stderr_fd < 0)
      {
        return -2;
      }
      dup2(stderr_fd, 2);
      
      //程序替换
      execl(ExePath(file_name).c_str(), ExePath(file_name).c_str(), NULL);
      exit(0);
    }
    else 
    {
      return -3;
    }

    return 0;
  }

  static bool Compile(const string& file_name)
  {
    //创建子进程
    //父进程进行进程等待
    //子进程进行进程程序替换
    int pid = fork();
    if(pid > 0)
    {
      //父进程
      waitpid(pid, NULL, 0);
    }
    else if(pid == 0)
    {
      //child
      //进程程序替换
      //标准错误
      int fd = open(CompileErrorPath(file_name).c_str(), O_CREAT | O_WRONLY, 0666);
      if(fd < 0)
      {
        return false;
      }
      //标准错误的输出，重定向到fd文件描述中去
      dup2(fd, 2);
      execlp("g++", "g++", SrcPath(file_name).c_str(), "-o", ExePath(file_name).c_str(), "-std=c++11", "-D", "CompileOnline", NULL);
      //如果替换失败了，就让子进程退出了，如果替换成功了，不会走该逻辑
      exit(0);
    }
    else 
    {
      return false;
    }

    //判断是否产生了可执行程序，若产生了，则编译成功
    struct stat st;
    int ret = stat(SrcPath(file_name).c_str(), &st);
    if(ret < 0)
    {
      return false;
    }
    return true;
  }

  static string StdoutPath(const string& filename)
  {
    //标准输出
    return "./user_file/" + filename + "_Stdout"; 
  }

  //static string StdinPath(const string& filename)
  //{
  //  //标准输入
  //  return "./user_file/" + filename + "_Stdin"; 
  //}

  static string StderrPath(const string& filename)
  {
    //标准输入
    return "./user_file/" + filename + "_Stderr"; 
  }

  static string CompileErrorPath(const string& filename)
  {
    //编译错误文件
    return "./user_file/" + filename + "_complie";
  }

  static string ExePath(const string& filename)
  {
    //可执行程序
    return "./user_file/" + filename + "_exe"; 
  }

  static string SrcPath(const string& filename)
  {
    //源码文件
    return "./user_file/" + filename + ".cpp";
  }

  static string WriteTmpFile(const string& code)
  {
    // 文件名称，区分源码文件，以及生成的可执行程序文件
    static atomic_uint id(0);

    string tmp_filename = "tmp_" + to_string(TimeInit::GetTimeStampMs()) + "_" + to_string(id);
    //code写到文件当中去
    ++id;
    FileInit::WriteFile(SrcPath(tmp_filename), code);
    return tmp_filename;
  }
};


#endif 
