#include <array>
#include <vector>
#include <string>
#include "miniaudio.h"

class Sfx {
  private:
    std::string name;
    ma_engine* engineRef;
    ma_node* output;
    ma_sound sound;
    ma_lpf_node lpf;
    ma_delay_node delay;
    std::array<ma_node*, 2> fxChain = { nullptr, nullptr };
    std::vector<ma_sound> elasticSounds; // Dynamically allocated and self cleaning (elastic) sounds.
    std::vector<int> staleElasticSoundsQueue; // List of indices for sounds that are ready to be cleaned up.
    void CleanupStaleElasticSounds();
    ma_sound* SpawnElasticSound();
    void onElasticSoundEnd(int sndIndex);
  public:
    Sfx(std::string filePath, std::string name, ma_engine* engineRef, ma_node *output, ma_node_graph* graphRef);
    Sfx();
    ~Sfx();
    std::string Play();
    std::string GetName();
};