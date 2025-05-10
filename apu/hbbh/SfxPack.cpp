#include <iostream>
#include <mach-o/dyld.h>
#include <limits.h>
#include "SfxPack.h"
#include <filesystem>

inline const std::unordered_map<std::string, std::unordered_map<int, std::string>> packsMap = {
    {
      "metroid", {
        { 3, "blip.wav" },
        // { 33, "sound.wav" },
      }
    },
    // {
    //   "zelda", {
    //     { 1, "blip.wav" },
    //     { 33, "sound.wav" },
    //   }
    // },
};

struct CallbackData {
  SfxPack* scope;
  int sfxKey;
  int sndIndex;
};

std::string getExecutablePath() {
  char path[1024];
  uint32_t size = sizeof(path);
  
  // Call _NSGetExecutablePath to get the executable path
  if (_NSGetExecutablePath(path, &size) == 0) {
    std::filesystem::path execPath(path);
    return execPath.parent_path().string();
  } else {
      return "";
  }
}

SfxPack::SfxPack( std::string packName, ma_engine* engineRef, ma_node* outputNode, ma_node_graph* graphRef ) {
  std::cout << "Init SfxPack: " << packName << std::endl;
  // engine = engineRef;
  // output = outputNode;

  for (const auto& pair : packsMap.at(packName)) {
    std::cout << "Loading sfx: " << pair.first << ", " << pair.second << std::endl;
    std::string filePath = getExecutablePath() + "/../Resources/wavs/" + packName + "/" + pair.second;
    sfxMap[pair.first] = new Sfx(filePath, pair.second, engineRef, outputNode, graphRef);

    // Debug sound loading.
    // ma_sound_start(sfxMap[pair.first]->Play());
  }
}

SfxPack::~SfxPack() {
  for (auto& pair : sfxMap) {
    delete pair.second;
  }

  sfxMap.clear();
}

void SfxPack::PlaySfx(int sfxKey) {
  auto entry = sfxMap.find(sfxKey);
  if (entry != sfxMap.end()) {
    std::string sfxName = entry->second->Play();
    // Debug
    std::cout << "Playing Sfx: " << sfxName << std::endl;
  }
}
