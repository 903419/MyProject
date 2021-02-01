#ifndef _OJVIEW_HPP_
#define _OJVIEW_HPP_
//该hpp是用来对网页进行渲染的，网页前端，与用户进行交互

#include <iostream>
#include <string>
#include <vector>

#include <json/json.h> 
#include <ctemplate/template.h>
#include "OjModel.hpp"

class OjView
{
  public:
    static void DrawAllQuestions(const vector<Question>& questions, string& html)
    {
      //创建大字典
      ctemplate::TemplateDictionary  dict("all_questions");

      //遍历vector，将试题信息，进行填充
      for(const auto& ques : questions)
      {
        //生成子字典，即每一个题目的字典
        ctemplate::TemplateDictionary* sub_dict = dict.AddSectionDictionary("question"); 
        //给每一个子字典，放入题目的信息,set_value接口
        sub_dict->SetValue("id", ques.id);
        sub_dict->SetValue("id", ques.id);
        sub_dict->SetValue("title", ques.title);
        sub_dict->SetValue("star", ques.star);
      }
      //网页填充
      //第二个参数的含义为：当扩展模板时，如何处理空格
      //DO_NOT_STRIP：模板保持不变
      //STRIP_BLANK_LINES：删掉空行
      //STRIP_WHITESPACE：删掉空行，并且删除每一行 行首行尾的空格
      ctemplate::Template* gtl = ctemplate::Template::GetTemplate("./template/all_questions.html", ctemplate::DO_NOT_STRIP);

      //网页渲染
      gtl->Expand(&html, &dict);
    }

    //单个题目的信息
    static void DrawOneQuestion(const Question& question, string& html)
    {
      ctemplate::TemplateDictionary  dict("question");
      dict.SetValue("id", question.id);
      dict.SetValue("title", question.title);
      dict.SetValue("star", question.star);
      dict.SetValue("desc", question.desc);
      dict.SetValue("id", question.id);
      dict.SetValue("code", question.header_cpp);
      ctemplate::Template* gtl = ctemplate::Template::GetTemplate("./template/question.html", ctemplate::DO_NOT_STRIP); 
      gtl->Expand(&html, &dict);
    }

    //对用户提交的代码，返回运行结果
    static void DrawCaseResult(const Json::Value& res_json, string& html)
    {
      ctemplate::TemplateDictionary  dict("result");
      dict.SetValue("errno", res_json["errno"].asString());
      dict.SetValue("compile_resul", res_json["stdout"].asString());
      dict.SetValue("reason", res_json["reason"].asString());
      ctemplate::Template* gtl = ctemplate::Template::GetTemplate("./template/case_result.html", ctemplate::DO_NOT_STRIP);     
      gtl->Expand(&html, &dict);   
    }
};

#endif
