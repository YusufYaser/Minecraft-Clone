#include "SoundEngine.h"
#include "../Logging.h"

SoundEngine::SoundEngine() {
	ma_engine_config config = ma_engine_config_init();

	engine = new ma_engine();
	if (ma_engine_init(&config, engine) != MA_SUCCESS) {
		return;
	}

	ma_device* dev = ma_engine_get_device(engine);
	print("Using sound device:", dev->playback.name);

	t = std::thread([this]() {
		soundTick();
		});

	m_loaded = true;
}

SoundEngine::~SoundEngine() {
	if (!m_loaded) return;
	unloading.store(true);

	ma_engine_uninit(engine);
	delete engine;
	engine = nullptr;

	for (auto& [type, esound] : sounds) {
		for (int i = 0; i < esound->loadedCount; i++) {
			for (int j = 0; j < SAME_SOUND_COUNT; j++) {
				//ma_sound_uninit(esound->sounds[i][j]);
			}
		}
	}

	queue.clear();

	print("Waiting for sound engine thread");
	t.join();

	sounds.clear();
}

const char* SoundEngine::getSoundDeviceName() {
	if (!m_loaded) return "not loaded";
	ma_device* dev = ma_engine_get_device(engine);
	return dev->playback.name;
}

void SoundEngine::loadSounds() {
	if (!m_loaded) return;
	// button
	Sound button{};
	button.type = SOUND_TYPE::BUTTON;
	button.variations = 1;
	button.fileNames[0] = "assets/sounds/button.wav";
	registerSound(button);
}

void SoundEngine::registerSound(Sound& sound) {
	if (!m_loaded) return;
	ESound* esound = new ESound();
	esound->soundData = sound;

	ma_result res;
	int c = 0;
	for (int i = 0; i < sound.variations; i++) {
		bool loaded = false;
		for (int j = 0; j < SAME_SOUND_COUNT; j++) {
			ma_sound* msound = new ma_sound();
			res = ma_sound_init_from_file(engine, sound.fileNames[i], MA_SOUND_FLAG_ASYNC, NULL, NULL, msound);
			if (res == MA_SUCCESS) {
				esound->sounds[c][j] = msound;
				loaded = true;
			} else {
				error("Failed to load sound:", sound.fileNames[i]);
				break;
			}
		}
		if (loaded != 0) c++;
	}
	if (c == 0) {
		error("No sounds loaded for", (int)sound.type);
		delete esound;
		return;
	}

	esound->loadedCount = c;

	sounds[sound.type] = esound;
}

void SoundEngine::playSound(SOUND_TYPE soundType) {
	if (!m_loaded) return;
	queueMux.lock();
	queue.push_back(soundType);
	queueMux.unlock();
}

void SoundEngine::soundTick() {
	if (!m_loaded) return;
	while (!unloading.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		if (queue.size() == 0) continue;
		queueMux.lock();
		SOUND_TYPE soundType = queue.front();
		queue.erase(queue.begin());
		queueMux.unlock();

		auto it = sounds.find(soundType);
		if (it == sounds.end()) {
			warn("Failed to play sound:", (int)soundType);
			continue;
		}

		ESound* esound = it->second;

		bool played = false;
		int variation = rand() % esound->loadedCount;
		for (int i = 0; i < SAME_SOUND_COUNT; i++) {
			if (ma_sound_is_playing(esound->sounds[variation][i])) continue;
			ma_sound_start(esound->sounds[variation][i]);
			played = true;
			break;
		}
	}
}
