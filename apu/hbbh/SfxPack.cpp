#include <iostream>
#include <mach-o/dyld.h>
#include <limits.h>
#include "SfxPack.h"
#include <filesystem>

inline const std::unordered_map<std::string, std::unordered_map<int, std::string>> packs_map = {
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

SfxPack::SfxPack( std::string packName, ma_engine engine, ma_node *output ) {
  std::cout << "Init SfxPack: " << packName << std::endl;

  for (const auto& pair : packs_map.at(packName)) {
    std::cout << "Loading sfx: " << pair.first << ", " << pair.second << std::endl;
    ma_sound sound;
    sfxMap[pair.first] = sound;
    std::string file_path = getExecutablePath() + "/../Resources/wavs/" + packName + "/" + pair.second;
    ma_result result = ma_sound_init_from_file(&engine, file_path.c_str(), MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT, NULL, NULL, &sfxMap[pair.first]);
    if (result != MA_SUCCESS) {
      printf("Failed to initialize sound in pack.");
    }

    result = ma_node_attach_output_bus(&sfxMap[pair.first], 0, output, 0);
    if (result != MA_SUCCESS) {
        printf("Failed to attach sfx sound node to output.");
    }

    // Debug sound loading.
    // ma_sound_start(&sfxMap[pair.first]);
  }
}

SfxPack::~SfxPack() {
  for (auto& pair : sfxMap) {
    ma_sound_uninit(&pair.second);
  }
}