using namespace geode::prelude;

bool theShowIsGo = false;

bool unfocusedForFirstTime = false;

bool isUnfocused = false;

bool windowsOnlyVSyncValue = false;

float originalFPSValue = 60.f;
int trueOriginalFPSFromSDL = 10;

float musicVolumeOriginal = 1.f;
float sfxVolumeOriginal = 1.f;

bool enabled = true;
bool reduceVolumeOnUnfocused = false;

float fpsReductionPercent = .1f;
float bgVolumeReductionPercent = .1f;
float sfxVolumeReductionPercent = .1f;

bool wasQualifiedForSDL = false;

bool stopCheckingForFocus = false;

void performUnfocus() {
	if (stopCheckingForFocus || isUnfocused) return;
	if (!enabled || !theShowIsGo) return;
	isUnfocused = true;

	GameManager* gm = GameManager::get();
	#ifdef GEODE_IS_WINDOWS
	originalFPSValue = gm->getGameVariable("0116") ? gm->m_customFPSTarget : 60.f;
	#endif
	#ifdef GEODE_IS_MACOS
	Mod* sdl = Loader::get()->getLoadedMod("zmx.sdl");
	trueOriginalFPSFromSDL = sdl->getSettingValue<int>("framerate-limit");
	originalFPSValue = std::clamp<int>(trueOriginalFPSFromSDL, 10, 120);
	#endif

	log::debug("enabled: {}", enabled);
	log::debug("originalFPSValue: {}", originalFPSValue);
	log::debug("fpsReductionPercent: {}", fpsReductionPercent);
	log::debug("{} * {} = {}", originalFPSValue, fpsReductionPercent, originalFPSValue * fpsReductionPercent);

	#ifdef GEODE_IS_WINDOWS
	gm->m_customFPSTarget = originalFPSValue * fpsReductionPercent;
	gm->m_vsyncEnabled = false;
	CCApplication::sharedApplication()->toggleVerticalSync(false);
	CCDirector::get()->setAnimationInterval(1.f / std::clamp<float>((gm->getGameVariable("0116") ? gm->m_customFPSTarget : 60.f), 10.f, originalFPSValue));
	#endif
	#ifdef GEODE_IS_MACOS
	if (sdl && sdl->getSettingValue<bool>("disable-vsync") && !sdl->getSettingValue<bool>("uncap-framerate")) {
		wasQualifiedForSDL = true;
		sdl->setSettingValue<int>("framerate-limit", std::clamp<int>(originalFPSValue * fpsReductionPercent, 10, trueOriginalFPSFromSDL));
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
	if (stopCheckingForFocus || !isUnfocused) return;
	if (!enabled || !theShowIsGo) return;
	isUnfocused = false;

	#ifdef GEODE_IS_WINDOWS
	GameManager* gm = GameManager::get();
	gm->m_customFPSTarget = originalFPSValue;
	gm->m_vsyncEnabled = gm->getGameVariable("0030");
	CCApplication::sharedApplication()->toggleVerticalSync(gm->getGameVariable("0030"));
	CCDirector::get()->setAnimationInterval(1.f / (gm->getGameVariable("0116") ? gm->m_customFPSTarget : 60.f));
	#endif
	#ifdef GEODE_IS_MACOS
	Mod* sdl = Loader::get()->getLoadedMod("zmx.sdl");
	if (sdl && wasQualifiedForSDL) {
		sdl->setSettingValue<int>("framerate-limit", trueOriginalFPSFromSDL);
		wasQualifiedForSDL = false;
	}
	#endif

	if (reduceVolumeOnUnfocused) {
		FMODAudioEngine::get()->setBackgroundMusicVolume(musicVolumeOriginal);
		FMODAudioEngine::get()->setEffectsVolume(sfxVolumeOriginal);
	}
}

void restoreValues() {
	if (!isUnfocused) return;
	GameManager* gm = GameManager::get();
	#ifdef GEODE_IS_WINDOWS
	gm->m_customFPSTarget = originalFPSValue;
	gm->m_vsyncEnabled = gm->getGameVariable("0030");
	CCApplication::sharedApplication()->toggleVerticalSync(gm->getGameVariable("0030"));
	CCDirector::get()->setAnimationInterval(1.f / (gm->getGameVariable("0116") ? gm->m_customFPSTarget : 60.f));
	#endif
	#ifdef GEODE_IS_MACOS
	if (Mod* sdl = Loader::get()->getLoadedMod("zmx.sdl"); sdl) {
		sdl->setSettingValue<int>("framerate-limit", trueOriginalFPSFromSDL);
	}
	#endif
	FMODAudioEngine::get()->setBackgroundMusicVolume(musicVolumeOriginal);
	FMODAudioEngine::get()->setEffectsVolume(sfxVolumeOriginal);
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
bool isFocused();
class DynamicFPSObserverForWindows final : public CCNode {
public:
	void checkFocusScheduler(float) { checkFocus(); }
	static void checkFocus() {
		if (isFocused()) performRefocus();
		else performUnfocus();
	}
};

$on_game(Loaded) {
	GameManager::get()->schedule(schedule_selector(DynamicFPSObserverForWindows::checkFocusScheduler), .125f);
}
/*
#include <Geode/modify/AppDelegate.hpp>
class $modify(MyAppDelegate, AppDelegate) {
	void applicationWillResignActive() {
		AppDelegate::applicationWillResignActive();
		if (!unfocusedForFirstTime) unfocusedForFirstTime = true;
		theShowIsGo = true;
		performUnfocus();
	}
	void applicationWillBecomeActive() {
		AppDelegate::applicationWillBecomeActive();
		if (!unfocusedForFirstTime) return;
		theShowIsGo = true;
		performRefocus();
	}
};
*/
#endif

// fix a bug where the wrong values would be saved to FMOD and zmx.sdl (only when confirming exit from popup, apparently)
#include <Geode/modify/MenuLayer.hpp>
class $modify(MyMenuLayer, MenuLayer) {
	void endGame() {
		restoreValues();
		stopCheckingForFocus = true;
		MenuLayer::endGame();
	}
};

$on_game(Exiting) {
	restoreValues();
	stopCheckingForFocus = true;
}

$on_game(ModsLoaded) {
	#ifdef GEODE_IS_WINDOWS
	theShowIsGo = true;
	GameManager* gm = GameManager::get();
	enabled = Mod::get()->getSettingValue<bool>("enabled");
	originalFPSValue = gm->getGameVariable("0116") ? gm->m_customFPSTarget : 60.f;
	// according to local crackhead omgrod and more trustworthy lead dev cvolton,
	// platform-specific dependencies are not enforced properly when downloading from the index.
	// seeing as SDL Mod was designed to be macOS-exclusive from Day One and has no real reason
	// to be ported over to Windows (and nothing is loaded anyway), automatic uninstall instead.
	// "mm for as much as i don't really like the idea i suppose you could try" -qimiko/zmx/chloe
	if (Loader::get()->isModInstalled("zmx.sdl") && Loader::get()->getInstalledMod("zmx.sdl")) {
		Loader::get()->getInstalledMod("zmx.sdl")->uninstall();
	}
	#endif
	#ifdef GEODE_IS_MACOS
	enabled = Mod::get()->getSettingValue<bool>("enabled") && Loader::get()->isModLoaded("zmx.sdl");
	if (Mod* sdl = Loader::get()->getLoadedMod("zmx.sdl"); sdl) {
		trueOriginalFPSFromSDL = sdl->getSettingValue<int>("framerate-limit");
		originalFPSValue = std::clamp<int>(trueOriginalFPSFromSDL, 10, 120);
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

	fpsReductionPercent = std::clamp<float>(Mod::get()->getSettingValue<float>("fpsReductionPercent"), .001f, 100.f) / 100.f;
	listenForSettingChanges<float>("fpsReductionPercent", [](const float fpsReductionPercentNew) {
		fpsReductionPercent = std::clamp<float>(fpsReductionPercentNew, .001f, 100.f) / 100.f;
	});

	bgVolumeReductionPercent = std::clamp<float>(Mod::get()->getSettingValue<float>("bgVolumeReductionPercent"), 0.f, 100.f) / 100.f;
	listenForSettingChanges<float>("bgVolumeReductionPercent", [](const float bgVolumeReductionPercentNew) {
		bgVolumeReductionPercent = std::clamp<float>(bgVolumeReductionPercentNew, 0.f, 100.f) / 100.f;
	});

	sfxVolumeReductionPercent = std::clamp<float>(Mod::get()->getSettingValue<float>("sfxVolumeReductionPercent"), 0.f, 100.f) / 100.f;
	listenForSettingChanges<float>("sfxVolumeReductionPercent", [](const float sfxVolumeReductionPercentNew) {
		sfxVolumeReductionPercent = std::clamp<float>(sfxVolumeReductionPercentNew, 0.f, 100.f) / 100.f;
	});
}
