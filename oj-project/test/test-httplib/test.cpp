#include <stdio.h>
#include "httplib.h"

void func(const httplib::Request& req, const httplib::Response& resq)
{
  printf("abc\n");
}

int main()
{
  httplib::Server srv;
  srv.Get("/abc", func);
  srv.listen("0.0.0.0", 18989);
  return 0;
}
