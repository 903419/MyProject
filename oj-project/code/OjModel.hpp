#ifndef _OJMODEL_HPP_
#define _OJMODEL_HPP_

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <vector>

#include "tools.hpp"
using namespace std;

struct Question
{
  string id;//题目id
  string title;//题目名称
  string star;//题目难度
  string _path;//题目路径
  string desc;//题目描述
  string header_cpp;//题目预定义的头部
  string tail_cpp;//题目尾，包含测试用例
};


class OjModel
{
  public:
    OjModel()
    {
      Load("./oj_data/oj_config.cfg");
    }
    ~OjModel(){}
    //从文件中获取题目信息
    bool Load(const string& filename)
    {
      //打开filename文件夹,该文件夹中存放的是题目的路径，难度
      ifstream ListofTopicsFile(filename.c_str());
      if(!ListofTopicsFile.is_open())
      {
        //如果文件没有被打开，需要保存该错误日志
        string msg;
        LOG(ERROR, msg);
        msg += " config file open failed\n";
        LogInfo::WrietLogInfo(msg);
        return false;
      }

      //文件打开成功，保留该文件中每一行的信息
      //对于每一行的信息，分别取得其中题号，题目名称，题目难易程度，题目路径
      string line;//每一行信息
      while(getline(ListofTopicsFile, line))
      {
        vector<string> vs;//每一行有多条信息，分别存储
        StringInit::Split(line, "\t", vs);
        Question ques;
        ques.id = vs[0];
        ques.title = vs[1];
        ques.star = vs[2];
        ques._path = vs[3];

        //分别将，题目描述，题目头，题目尾读到
        FileInit::ReadFile(ques._path + "/desc.txt", ques.desc);
        FileInit::ReadFile(ques._path + "/header.cpp", ques.header_cpp); 
        FileInit::ReadFile(ques._path + "/tail.cpp", ques.tail_cpp); 
        //保存该问题
        _ques_map[ques.id] = ques;
      }
      return true;
    }
    
    //获取题目列表，将题目列表放入到vector中
    bool GetAllQuestions(vector<Question>& questions)
    {
      //遍历无序map，将试题信息，返回给调用者
      for(const auto& om : _ques_map)
      {
        questions.push_back(om.second);
      }
      //因为unorederedmap是无序，所以需要对题目列表进行排序
      sort(questions.begin(), questions.end(), [](const Question& a, const Question& b){
          return stoi(a.id) < stoi(b.id);
          });

      return true;
    }

    //获取单个题目信息，并做题
    bool GetOneQuestion(const string& id, Question& question)
    {
      //因为是用户点击进来寻找题目信息，则题不可能存在不在的情况
      question = _ques_map[id];
      return true;
    }
  private:
    unordered_map<string, Question> _ques_map;
};



#endif
