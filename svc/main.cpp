#include <iostream>
#include <map>

void usage(void) {
  std::cerr << "Usage: svc.exe [install|start|stop|delete]" << std::endl;
}

int action_install(void) {
  std::cout << "Installing..." << std::endl;
  return 0;
}

int action_start(void) {
  std::cout << "Starting..." << std::endl;
  return 1;
}

int action_stop(void) {
  std::cout << "Stopping..." << std::endl;
  return 0;
}

int action_delete(void) {
  std::cout << "Deleting..." << std::endl;
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Bad number of arguments." << std::endl;
    usage();
    return 1;
  }

  const std::map<const std::string, int (*)(void)> actions{
      {"install", &action_install},
      {"start", &action_start},
      {"stop", &action_stop},
      {"delete", &action_delete}};

  try {
    int res = actions.at(argv[1])();
    if (res != 0) {
      std::cerr << "Error status: " << res << std::endl;
      return res;
    }
  } catch (std::exception &e) {
    (void)e;
    std::cerr << "Unknown action: " << argv[1] << std::endl;
    usage();
    return 1;
  }

  std::cout << "Done!" << std::endl;

  return 0;
}
