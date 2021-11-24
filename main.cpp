
#include <memory>
#include "App.h"
#undef main
int main(int argc,char** argv)
{
    
	WindowCreateInfo windowInfo{};
	windowInfo.title = "vksph";
	windowInfo.extent = glm::vec2(1024, 768);
    auto app=std::make_unique<App>(windowInfo);
    app->Run();
    return 0;
}