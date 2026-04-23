#include <Geode/modify/AppDelegate.hpp>

using namespace geode::prelude;

float originalFPSValue = 60.f;

bool enabled = true;
bool reduceVolumeOnUnfocused = false;

float fpsReductionPercent = .1f;
float bgVolumeReductionPercent = .1f;
float sfxVolumeReductionPercent = .1f;

bool wasQualifiedForSDL = false;

class $modify(MyAppDelegate, AppDelegate) {
	void applicationWillResignActive() {
		if (!enabled) return AppDelegate::applicationWillResignActive();

		GameManager* gm = GameManager::get();
		#ifdef GEODE_IS_WINDOWS
		originalFPSValue = gm->m_customFPSTarget;
		#endif
		#ifdef GEODE_IS_MACOS
		Mod* sdl = Loader::get()->getLoadedMod("zmx.sdl");
		originalFPSValue = sdl->getSettingValue<int>("framerate-limit");
		#endif

		log::debug("enabled: {}", enabled);
		log::debug("originalFPSValue: {}", originalFPSValue);
		log::debug("fpsReductionPercent: {}", fpsReductionPercent);
		log::debug("result: {}", originalFPSValue * fpsReductionPercent);

		AppDelegate::applicationWillResignActive();

		#ifdef GEODE_IS_WINDOWS
		gm->m_customFPSTarget = originalFPSValue * fpsReductionPercent;
		log::debug("gm->m_customFPSTarget: {}", gm->m_customFPSTarget);
		#endif

		#ifdef GEODE_IS_MACOS
		if (sdl && sdl->getSettingValue<bool>("disable-vsync") && !sdl->getSettingValue<bool>("uncap-framerate")) {
			wasQualifiedForSDL = true;
			sdl->setSettingValue<int>("framerate-limit", originalFPSValue * fpsReductionPercent);
		}
		#endif

		if (reduceVolumeOnUnfocused) {
			log::debug("gm->m_bgVolume: {}", gm->m_bgVolume);
			log::debug("m_bgVolume result: {}", gm->m_bgVolume * bgVolumeReductionPercent);
			log::debug("gm->m_sfxVolume: {}", gm->m_sfxVolume);
			log::debug("m_sfxVolume result: {}", gm->m_sfxVolume * sfxVolumeReductionPercent);
			FMODAudioEngine::get()->setBackgroundMusicVolume(gm->m_bgVolume * bgVolumeReductionPercent);
			FMODAudioEngine::get()->setEffectsVolume(gm->m_sfxVolume * sfxVolumeReductionPercent);
		}
	}
	void applicationWillBecomeActive() {
		if (!enabled) return AppDelegate::applicationWillBecomeActive();
		GameManager* gm = GameManager::get();

		#ifdef GEODE_IS_WINDOWS
		gm->m_customFPSTarget = originalFPSValue;
		#endif

		#ifdef GEODE_IS_MACOS
		if (sdl && wasQualifiedForSDL) {
			sdl->setSettingValue<int>("framerate-limit", originalFPSValue);
			wasQualifiedForSDL = false;
		}
		#endif

		AppDelegate::applicationWillBecomeActive();
		if (reduceVolumeOnUnfocused) {
			FMODAudioEngine::get()->setBackgroundMusicVolume(gm->m_bgVolume);
			FMODAudioEngine::get()->setEffectsVolume(gm->m_sfxVolume);
		}
	}
};

$on_game(Exiting) {
	GameManager* gm = GameManager::get();
	gm->m_customFPSTarget = originalFPSValue;
	FMODAudioEngine::get()->setBackgroundMusicVolume(gm->m_bgVolume);
	FMODAudioEngine::get()->setEffectsVolume(gm->m_sfxVolume);
}

$on_game(ModsLoaded) {
	originalFPSValue = GameManager::get()->m_customFPSTarget;

	#ifdef GEODE_IS_WINDOWS
	enabled = Mod::get()->getSettingValue<bool>("enabled");
	#endif
	#ifdef GEODE_IS_MACOS
	enabled = Mod::get()->getSettingValue<bool>("enabled") && Loader::get()->isModLoaded("zmx.sdl");
	#endif
	listenForSettingChanges<bool>("enabled", [](const bool enabledNew) {
		enabled = enabledNew;
		#ifdef GEODE_IS_MACOS
		if (Loader::get()->isModLoaded("zmx.sdl")) return;
		MDPopup::create("Heads up!", "DynamicFPS <c-ff0000>***__REQUIRES__***</c> <cl>SDL Mod by qimiko</c> in order to work! Otherwise, this mod is effectively deadweight!\n\n<mod:zmx.sdl>", "I Understand")->show();
		#endif
	});

	reduceVolumeOnUnfocused = Mod::get()->getSettingValue<bool>("reduceVolumeOnUnfocused");
	listenForSettingChanges<bool>("reduceVolumeOnUnfocused", [](const bool reduceVolumeOnUnfocusedNew) {
		reduceVolumeOnUnfocused = reduceVolumeOnUnfocusedNew;
	});

	fpsReductionPercent = Mod::get()->getSettingValue<float>("fpsReductionPercent");
	listenForSettingChanges<float>("fpsReductionPercent", [](const float fpsReductionPercentNew) {
		fpsReductionPercent = std::clamp<float>(fpsReductionPercentNew, 5, 100) / 100.f;
	});

	bgVolumeReductionPercent = Mod::get()->getSettingValue<float>("bgVolumeReductionPercent");
	listenForSettingChanges<float>("bgVolumeReductionPercent", [](const float bgVolumeReductionPercentNew) {
		bgVolumeReductionPercent = std::clamp<float>(bgVolumeReductionPercentNew, 0, 100) / 100.f;
	});

	sfxVolumeReductionPercent = Mod::get()->getSettingValue<float>("sfxVolumeReductionPercent");
	listenForSettingChanges<float>("sfxVolumeReductionPercent", [](const float sfxVolumeReductionPercentNew) {
		sfxVolumeReductionPercent = std::clamp<float>(sfxVolumeReductionPercentNew, 0, 100) / 100.f;
	});
}