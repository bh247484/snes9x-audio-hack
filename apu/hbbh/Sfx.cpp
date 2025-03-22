#include "Sfx.h"
#include <iostream>

Sfx::Sfx(){};

Sfx::Sfx(std::string filePath, std::string name, ma_engine* engineRef, ma_node *output, ma_node_graph* graphRef) {
  this->name = name;
  this->engineRef = engineRef;
  this->output = output;
  
  // Init sound.
  ma_result result = ma_sound_init_from_file(
    engineRef,
    filePath.c_str(),
    MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT,
    NULL,
    NULL,
    &sound
  );
  if (result != MA_SUCCESS) {
    std::cout << "Failed to intialize sound '" << name << ".'" << std::endl;
    std::cout << "Failed filePath '" << filePath << "'" << std::endl;
    return;
  }

  // Init FX.
  auto sampleRate = ma_engine_get_sample_rate(engineRef);
  auto numChannels = ma_engine_get_channels(engineRef);
  auto lpfNodeConfig = ma_lpf_node_config_init(numChannels, sampleRate, (double)(sampleRate / 90), 2);
  auto delayNodeConfig = ma_delay_node_config_init(numChannels, sampleRate, sampleRate * 0.5f, 0.5f);

  result = ma_lpf_node_init(graphRef, &lpfNodeConfig, NULL, &lpf);
  if (result != MA_SUCCESS) {
    std::cout << "Failed to intialize lpf node." << std::endl;
    return;
  }

  result = ma_delay_node_init(graphRef, &delayNodeConfig, NULL, &delay);
  if (result != MA_SUCCESS) {
    std::cout << "Failed to intialize delay node." << std::endl;
    return;
  }

  fxChain[0] = &lpf;
  fxChain[1] = &delay;

  for (int i = 0; i < fxChain.size(); i++) {
    // If last fx in chain connect to output.
    // Else connect it to the next fx in chain.
    if (i == fxChain.size() - 1) {
      result = ma_node_attach_output_bus(fxChain[i], 0, output, 0);
      if (result != MA_SUCCESS) {
        std::cout << "Failed to connect fx node index: " << i << std::endl;
        return;
      }
    } else {
      result = ma_node_attach_output_bus(fxChain[i], 0, fxChain[i + 1], 0);
      if (result != MA_SUCCESS) {
        std::cout << "Failed to connect fx node index: " << i << std::endl;
        return;
      }
    }
  }

  // // Connect sfx node to fx chain.
  ma_node_attach_output_bus(&sound, 0, fxChain[0], 0);
}

Sfx::~Sfx() {
  ma_sound_uninit(&sound);

  for (int i = 0; i < fxChain.size(); i++) {
    ma_node_uninit(&fxChain[i], NULL);
    delete fxChain[i];
  }
}

void Sfx::CleanupStaleElasticSounds() {
  for (auto &sndIndex : staleElasticSoundsQueue) {
    ma_sound_uninit(&elasticSounds[sndIndex]);
    elasticSounds.erase(elasticSounds.begin() + sndIndex);
  }
  staleElasticSoundsQueue.clear();
}

void Sfx::onElasticSoundEnd(int sndIndex) {
  staleElasticSoundsQueue.push_back(sndIndex);
}

struct CallbackData {
  Sfx* scope;
  int sndIndex;
};

ma_sound* Sfx::SpawnElasticSound() {
  std::cout << "Spawning elastic sound from Sfx: " << name << std::endl;
  ma_sound soundCopy;
  elasticSounds.push_back(soundCopy);
  int sndIndex = elasticSounds.size() - 1;
  ma_sound* soundRef = &elasticSounds[sndIndex];
  CallbackData* cbData = new CallbackData{ this, sndIndex };

  ma_result result = ma_sound_init_copy(
    engineRef,
    &sound,
    MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT,
    NULL,
    soundRef
  );

  if (result != MA_SUCCESS) {
    std::cout << "Failed to copy elastic sound: " << result << std::endl;
    std::cout << "Attempted copy from: " << name << std::endl;
  }

  // This from the miniaudio docs: "the callback is fired from the audio thread which means you cannot be uninitializing sound from the callback."
  // So we have to jump through some hoops here to dynamically allocate and then queue stale sounds for cleanup after they finish playing.
  ma_sound_set_end_callback(
    soundRef,
    [](void* pUserData, ma_sound* pSound) {
      printf("callback trig");
      if (pUserData) {
        CallbackData* data = static_cast<CallbackData*>(pUserData);
        data->scope->onElasticSoundEnd(data->sndIndex);

        delete data;
      }
    },
    cbData
  );

  // Attach to tip of FX chain.
  result = ma_node_attach_output_bus(soundRef, 0, fxChain[0], 0);

  // result = ma_node_attach_output_bus(soundRef, 0, output, 0);

  if (result != MA_SUCCESS) {
    std::cout << "Failed to attach elastic sound to fxChain: " << result << std::endl;
    std::cout << "Failed elastic sound attempted spawn from: " << name << std::endl;
  }

  return soundRef;
}

std::string Sfx::Play() {
  if (staleElasticSoundsQueue.size() > 0) {
    CleanupStaleElasticSounds();
  }

  if (ma_sound_is_playing(&sound)) {
    // Copy sound and assign it to an ElasticSound.
    ma_sound* soundRef = SpawnElasticSound();

    // Finally play the copied sound.
    ma_sound_start(soundRef);
    return name + "-elastic";
  } else {
    ma_sound_start(&sound);
    return name;
  }
}

std::string Sfx::GetName() {
  return name;
}
