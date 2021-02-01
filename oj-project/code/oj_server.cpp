#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <json/json.h>
#include "httplib.h"
#include "OjModel.hpp"
#include "tools.hpp"
#include "ojview.hpp"
#include "compile.hpp"
using namespace std;

int main()
{
  using namespace httplib;
  OjModel model;
  Server svr;//初始化svr对象
  //获取整个题目列表
  svr.Get("/all_questions", [&model](const Request& req, Response& resp){
      vector<Question> questions;
      model.GetAllQuestions(questions);//获取题目列表
      //网页
      string html;
      //网页填充，渲染
      OjView::DrawAllQuestions(questions, html);

      resp.set_content(html, "text/html");
      });

  //获取单个题目内容
  //这里因为是每个题目都是一个资源，则访问每一题目时，需要按照其题目的id，访问该题目的资源，使用正则匹配，*表示匹配0个或者多个，\d表示数字
  svr.Get(R"(/question/(\d*))", [&model](const Request& req, Response& resp){
      Question ques;
      model.GetOneQuestion(req.matches[1].str(), ques);

      string html;
      OjView::DrawOneQuestion(ques, html);
      resp.set_content(html, "text/html");
      });

  //获取用户提交的代码，编译运行
  svr.Post(R"(/compile/(\d*))", [&model](const Request& req, Response& resp){
      //获取题目信息
      Question ques;
      model.GetOneQuestion(req.matches[1].str(), ques);
      //post提交方法，是经过网页encode的，如果要正常获取浏览器提交的内容，则需要进行decode
      unordered_map<string, string> content_kv;

      //网页内容解码
      UrlInit::WebContentDecodeCutting(req.body, content_kv);
      //用户提交的代码
      string user_code = content_kv["code"];
      //构造json对象，将代码交给编译模块运行
      Json::Value req_json;
      //将用户提交的代码，与后台执行的测试用例保存到一块
      req_json["code"] = user_code + ques.tail_cpp;
      //req_json["stdin"] = "";
      //cout << req_json["code"].asString() << endl;
      
      Json::Value resp_json;
      Complier::CompileAndRun(req_json, resp_json);

      //string stdout = resp["stdout"].asString();
      //cout << stdout << endl;
      //cout << resp["stdout"].asString() << endl;
      //cout << resp["reason"].asString() << endl;
      string html;
      OjView::DrawCaseResult(resp_json, html);

      resp.set_content(html, "test/html");
      });

  string msg;
  LOG(INFO, msg);
  msg += " listen_port : 9999\n";
  LogInfo::WrietLogInfo(msg);
  svr.listen("0.0.0.0", 9999);
  return 0;
}
