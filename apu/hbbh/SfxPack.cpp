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

SfxPack::SfxPack( std::string packName, ma_engine engineRef, ma_node *outputNode ) {
  std::cout << "Init SfxPack: " << packName << std::endl;
  engine = engineRef;
  output = outputNode;

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

void SfxPack::onSoundEnd(int sfxKey, int sndIndex) {
  staleElasticSoundsQueue[sfxKey].push_back(sndIndex);
}

void SfxPack::CleanupStaleElasticSounds(int sfxKey) {
  for (auto &sndIndex : staleElasticSoundsQueue[sfxKey]) {
    ma_sound_uninit(&elasticSounds[sfxKey][sndIndex]);
    elasticSounds[sfxKey].erase(elasticSounds[sfxKey].begin() + sndIndex);
  }
  staleElasticSoundsQueue[sfxKey].clear();
}

ma_sound* SfxPack::SpawnElasticSound(int sfxKey, ma_sound* originalSnd) {
  ma_sound soundCopy;
  elasticSounds[sfxKey].push_back(soundCopy);
  int sndIndex = elasticSounds[sfxKey].size() - 1;
  ma_sound* soundRef = &elasticSounds[sfxKey][sndIndex];
  CallbackData* cbData = new CallbackData{ this, sfxKey, sndIndex };

  ma_result result = ma_sound_init_copy(
    &engine,
    originalSnd,
    MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT,
    NULL,
    soundRef
  );

  if (result != MA_SUCCESS) {
    std::cout << "Failed to copy elastic sound: " << result << std::endl;
  }

  // This from the miniaudio docs: "the callback is fired from the audio thread which means you cannot be uninitializing sound from the callback."
  // So we have to jump through some hoops here to dynamically allocate and then queue stale sounds for cleanup after they finish playing.
  ma_sound_set_end_callback(
    soundRef,
    [](void* pUserData, ma_sound* pSound) {
      printf("callback trig");
      if (pUserData) {
        CallbackData* data = static_cast<CallbackData*>(pUserData);
        data->scope->onSoundEnd(data->sfxKey, data->sndIndex);

        delete data;
      }
    },
    cbData
  );

  result = ma_node_attach_output_bus(soundRef, 0, output, 0);

  if (result != MA_SUCCESS) {
    std::cout << "Failed to attach elastic sound to output bus: " << result << std::endl;
  }

  return soundRef;
}

void SfxPack::PlaySound(int sfxKey) {
  auto entry = sfxMap.find(sfxKey);
  if (entry != sfxMap.end()) {
    // Cleanup stale sounds if they exist for this sfxKey.
    if (staleElasticSoundsQueue.find(sfxKey) != staleElasticSoundsQueue.end() && staleElasticSoundsQueue[sfxKey].size() > 0) {
      CleanupStaleElasticSounds(sfxKey);
    }

    auto sound = &entry->second;
    if (ma_sound_is_playing(sound)) {
        // Copy sound and assign it to an ElasticSound.
        ma_sound* soundRef = SpawnElasticSound(sfxKey, sound);

        // Finally play the copied sound.
        ma_sound_start(soundRef);
    } else {
        ma_sound_start(sound);
    }
  }
}
