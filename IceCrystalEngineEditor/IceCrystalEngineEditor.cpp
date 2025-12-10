#include <Ice/Core/Engine.h>

int main(int argc, char* argv[])
{
    Engine &engine = Engine::GetInstance();
    engine.Run();
    return 0;
}
