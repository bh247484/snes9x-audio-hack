#include <filesystem>
#include <unordered_map>
#include <string>
#include <vector>
#include "miniaudio.h"

class SfxPack {
  private:
    // Dynamically allocated and self cleaning (elastic) sounds organized per sfxKey.
    std::unordered_map<int, std::vector<ma_sound>> elasticSounds;
    // List of indices for sounds that are ready to be cleaned up, organized per sfxKey.
    std::unordered_map<int, std::vector<int>> staleElasticSoundsQueue;
    ma_engine engine;
    ma_node* output;
    std::unordered_map<int, ma_sound> sfxMap;
    void CleanupStaleElasticSounds(int sfxKey);
    ma_sound* SpawnElasticSound(int sfxKey, ma_sound* originalSnd);
    void onSoundEnd(int sfxKey, int sndIndex);

  public:
    SfxPack(std::string packName, ma_engine engineRef, ma_node *output);
    SfxPack() = delete;
    ~SfxPack();
    void PlaySound(int sfxKey);
};
