#include "svc.h"

static void Usage(void)
{
    std::cerr << "Usage: svc.exe [install|start|stop|delete]" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Bad number of arguments.\n";
        Usage();
        return 1;
    }

    const std::map<const std::string, int (*)(void)> actions{
        {"install", &ActionInstall},
        {"start", &ActionStart},
        {"stop", &ActionStop},
        {"delete", &ActionDelete}};

    try
    {
        int res = actions.at(argv[1])();
        if (res != 0)
        {
            std::cerr << "Error status: " << res << std::endl;
            return res;
        }
    }
    catch (std::exception &e)
    {
        (void)e;
        std::cerr << "Unknown action: " << argv[1] << std::endl;
        Usage();
        return 1;
    }

    std::cout << "Done!\n";

    return 0;
}