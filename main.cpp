
#include <memory>
#include "App.h"
#undef main
int main(int argc,char** argv)
{
    WindowCreateInfo info={};
    info.title="vksph";
    info.width=1024;
    info.height=768;
    info.resizeable=true;
    auto app=std::make_unique<App>(info);
    app->Run();
}