#include <Geode/modify/AppDelegate.hpp>

using namespace geode::prelude;

bool theShowIsGo = false; 

bool isUnfocused = false;

float originalFPSValue = 60.f;

float musicVolumeOriginal = 1.f;
float sfxVolumeOriginal = 1.f;

bool enabled = true;
bool reduceVolumeOnUnfocused = false;

float fpsReductionPercent = .1f;
float bgVolumeReductionPercent = .1f;
float sfxVolumeReductionPercent = .1f;

bool wasQualifiedForSDL = false;

void performUnfocus() {
	if (isUnfocused) return;
	if (!enabled || !theShowIsGo) return;
	isUnfocused = true;

	GameManager* gm = GameManager::get();
	#ifdef GEODE_IS_WINDOWS
	originalFPSValue = gm->m_customFPSTarget;
	#endif
	#ifdef GEODE_IS_MACOS
	Mod* sdl = Loader::get()->getLoadedMod("zmx.sdl");
	originalFPSValue = std::clamp<int>(sdl->getSettingValue<int>("framerate-limit"), 10, 120);
	#endif

	log::debug("enabled: {}", enabled);
	log::debug("originalFPSValue: {}", originalFPSValue);
	log::debug("fpsReductionPercent: {}", fpsReductionPercent);
	log::debug("{} * {} = {}", originalFPSValue, fpsReductionPercent, originalFPSValue * fpsReductionPercent);

	#ifdef GEODE_IS_WINDOWS
	gm->m_customFPSTarget = originalFPSValue * fpsReductionPercent;
	#endif

	#ifdef GEODE_IS_MACOS
	if (sdl && sdl->getSettingValue<bool>("disable-vsync") && !sdl->getSettingValue<bool>("uncap-framerate")) {
		wasQualifiedForSDL = true;
		sdl->setSettingValue<int>("framerate-limit", originalFPSValue * fpsReductionPercent);
	}
	#endif

	if (reduceVolumeOnUnfocused) {
		log::debug("gm->m_bgVolume: {}", gm->m_bgVolume);
		log::debug("{} * {}: {}", gm->m_bgVolume, bgVolumeReductionPercent, gm->m_bgVolume * bgVolumeReductionPercent);
		log::debug("gm->m_sfxVolume: {}", gm->m_sfxVolume);
		log::debug("{} * {}: {}", gm->m_sfxVolume, sfxVolumeReductionPercent, gm->m_sfxVolume * sfxVolumeReductionPercent);
		FMODAudioEngine* fmod = FMODAudioEngine::get();
		musicVolumeOriginal = fmod->m_musicVolume;
		sfxVolumeOriginal = fmod->m_sfxVolume;
		fmod->setBackgroundMusicVolume(fmod->m_musicVolume * bgVolumeReductionPercent);
		fmod->setEffectsVolume(fmod->m_sfxVolume * sfxVolumeReductionPercent);
	}
}

void performRefocus() {
	if (!isUnfocused) return;
	if (!enabled || !theShowIsGo) return;
	isUnfocused = false;

	GameManager* gm = GameManager::get();

	#ifdef GEODE_IS_WINDOWS
	gm->m_customFPSTarget = originalFPSValue;
	#endif

	#ifdef GEODE_IS_MACOS
	Mod* sdl = Loader::get()->getLoadedMod("zmx.sdl");
	if (sdl && wasQualifiedForSDL) {
		sdl->setSettingValue<int>("framerate-limit", originalFPSValue);
		wasQualifiedForSDL = false;
	}
	#endif

	if (reduceVolumeOnUnfocused) {
		FMODAudioEngine::get()->setBackgroundMusicVolume(musicVolumeOriginal);
		FMODAudioEngine::get()->setEffectsVolume(sfxVolumeOriginal);
	}
}

#ifdef GEODE_IS_WINDOWS
class DynamicFPSObserverForWindows final : public CCNode {
public:
	void checkFocusScheduler(float) { checkFocus(); }
	static void checkFocus() {
		if (GetActiveWindow() == WindowFromDC(wglGetCurrentDC())) performRefocus();
		else performUnfocus();
	}
};

$on_game(Loaded) {
	GameManager::get()->schedule(schedule_selector(DynamicFPSObserverForWindows::checkFocusScheduler), .125f);
}
#endif

#ifdef GEODE_IS_MACOS
class $modify(MyAppDelegate, AppDelegate) {
	void applicationWillResignActive() {
		AppDelegate::applicationWillResignActive();
		performUnfocus();
	}
	void applicationWillBecomeActive() {
		AppDelegate::applicationWillBecomeActive();
		performRefocus();
	}
};
#endif

$on_game(Exiting) {
	if (!isUnfocused) return;
	GameManager* gm = GameManager::get();
	gm->m_customFPSTarget = originalFPSValue;
	FMODAudioEngine::get()->setBackgroundMusicVolume(musicVolumeOriginal);
	FMODAudioEngine::get()->setEffectsVolume(sfxVolumeOriginal);
}

$on_game(ModsLoaded) {
	theShowIsGo = true;

	#ifdef GEODE_IS_WINDOWS
	enabled = Mod::get()->getSettingValue<bool>("enabled");
	originalFPSValue = GameManager::get()->m_customFPSTarget;
	#endif
	#ifdef GEODE_IS_MACOS
	enabled = Mod::get()->getSettingValue<bool>("enabled") && Loader::get()->isModLoaded("zmx.sdl");
	if (Mod* sdl = Loader::get()->getLoadedMod("zmx.sdl"); sdl) {
		originalFPSValue = std::clamp<int>(sdl->getSettingValue<int>("framerate-limit"), 10, 120);
	}
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
		fpsReductionPercent = std::clamp<float>(fpsReductionPercentNew, 0.1f, 100.f) / 100.f;
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