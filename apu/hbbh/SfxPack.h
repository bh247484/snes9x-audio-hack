#include <filesystem>
#include <unordered_map>
#include <string>
#include <vector>
#include "miniaudio.h"
#include "Sfx.h"

class SfxPack {
  private:
    std::unordered_map<int, Sfx*> sfxMap;

  public:
    SfxPack(std::string packName, ma_engine* engineRef, ma_node* output, ma_node_graph* graphRef);
    SfxPack() = delete;
    ~SfxPack();
    void PlaySfx(int sfxKey);
};
