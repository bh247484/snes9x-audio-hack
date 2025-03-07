#include <filesystem>
#include <unordered_map>
#include <string>
#include <vector>
#include "miniaudio.h"

class SfxPack {
  std::vector<ma_sound> sounds;
  public:
  SfxPack(std::string packName, ma_engine engine, ma_node *output);
  SfxPack() = delete;
  ~SfxPack();
  std::unordered_map<int, ma_sound> sfxMap;
};
