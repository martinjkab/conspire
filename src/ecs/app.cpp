#include "app.h"
#include <thread>

void App::run() {
  runSystems(STARTUP);

  try {
    while (running) {
      auto begin = std::chrono::high_resolution_clock::now();

      runSystems(UPDATE);
      runSystems(POST_UPDATE);

      auto end = std::chrono::high_resolution_clock::now();
      auto duration =
          std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
      auto sleep = std::chrono::milliseconds(17) - duration;

      if (sleep.count() > 0) {
        std::this_thread::sleep_for(sleep);
      }
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

void App::runSystems(Phase phase) {
  for (auto system : systems[phase]) {
    system();
  }
}

App& App::addPlugin(const Plugin& plugin) {
  plugin.onAdd(*this);
  return *this;
}
