#include <iostream>
#include "Orchestrator.h"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// Change SFX pack here.
const std::string SFX_PACK_NAME = "metroid";

// Constructor
Orchestrator::Orchestrator() {
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.");
    }

    channels   = ma_engine_get_channels(&engine);
    sampleRate = ma_engine_get_sample_rate(&engine);

    graph = ma_engine_get_node_graph(&engine);
    output = ma_engine_get_endpoint(&engine);

    sfxPack = new SfxPack(SFX_PACK_NAME, engine, output);

    // result = ma_sound_init_from_file(&engine, "/Users/bh/Documents/game-audio/snes9x/apu/hbbh/wavs/metroid/blip.wav", MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT, NULL, NULL, &testSound);
    // if (result != MA_SUCCESS) {
    //     printf("Failed to initialize testSound.");
    // }

    // result = ma_node_attach_output_bus(&testSound, 0, output, 0);
    // if (result != MA_SUCCESS) {
    //     // Failed to attach node.
    // }

    // ma_sound_start(&testSound);
}

// Destructor
Orchestrator::~Orchestrator() {
    delete sfxPack;
    ma_engine_uninit(&engine);
}

// Method to forward an event
void Orchestrator::ForwardEvent(int snd_queue) {
    auto entry = sfxPack->sfxMap.find(snd_queue);
    if (entry != sfxPack->sfxMap.end()) {
         ma_sound_start(&entry->second);
    }
    
    std::cout << "Snd Queue: " << snd_queue << std::endl;
}
