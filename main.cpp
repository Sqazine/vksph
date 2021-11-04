
#include <memory>
#include "App.h"
#undef main
int main(int argc,char** argv)
{
    auto app=std::make_unique<App>("vksph",1024,768);
    app->Run();
}