#include <loader.h>

int SetChannels(FMOD::Studio::EventInstance *instance) {
    // Each track has a ChannelGroup. Inside a ChannelGroup are a number of other ChannelGroups
    FMOD::ChannelGroup *topGroup;
    FMOD::ChannelGroup *sub1, *sub2, *sub3, *sub4;
    FMOD::Channel *channel1, *channel2, *channel3, *channel4;

    instance->getChannelGroup(&topGroup);

    // Debug section

        int topNumGroups;
        topGroup->getNumGroups(&topNumGroups);

        printf("topNumGroups = %i", topNumGroups);

    // END

    topGroup->getGroup(0, &sub1);
    topGroup->getGroup(1, &sub2);
    topGroup->getGroup(2, &sub3);
    topGroup->getGroup(3, &sub4);

    // Channels role may vary from track to track
    // Lead channel
    sub1->getChannel(0, &channel1); // 0 is hardcoded, and may vary for each track.
    channel1->setVolume(1.0);

    // Drums channel
    sub2->getChannel(0, &channel2);  // 0 is hardcoded, and may vary for each track.
    channel2->setVolume(1.0);

    // Humming channel
    sub3->getChannel(0, &channel3);  // 0 is hardcoded, and may vary for each track.
    channel3->setVolume(0.0);  // MUTE!

    // ???
    //sub4->getChannel(0, &channel4);  // 0 is hardcoded, and may vary for each track.
    //channel4->setVolume(1.0);

    return 0;
}

int main(int argc, char ** argv)
{
    FMOD::Studio::System* system;
    ERRCHECK(FMOD::Studio::System::create(&system));
    FMOD::System* lowLevelSystem;
    ERRCHECK(system->getLowLevelSystem(&lowLevelSystem));

    unsigned int handle;

    ERRCHECK(lowLevelSystem->setPluginPath(PLUGINS_DIR.c_str())); // Mind no slash at the end
    ERRCHECK(lowLevelSystem->loadPlugin("libFModPlugins.dylib", &handle, 0)); // Name of your lib filename


    // Uncomment one of following lines if you want to save your sound to a .WAV file.

    //ERRCHECK(lowLevelSystem->setOutput(FMOD_OUTPUTTYPE_WAVWRITER_NRT)); // (NRT means not-real-time, so you wouldn't have to wait for the sound to play for it to save)
    //ERRCHECK(lowLevelSystem->setOutput(FMOD_OUTPUTTYPE_WAVWRITER));

    ERRCHECK(system->initialize(32, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0));

    std::vector<SoundDetails> soundDetails;
    std::vector<BankDetails> bankDetails;

    // Include .bank files that you need here. Master bank is required

    LoadBank(system, &soundDetails, &bankDetails, "MasterBank.strings.bank");
    LoadBank(system, &soundDetails, &bankDetails, "MasterBank.bank");
    //LoadBank(system, &soundDetails, &bankDetails, "AudenFMOD_Sounds.bank");
    //LoadBank(system, &soundDetails, &bankDetails, "AudenFMOD_OldSounds.bank");
    LoadBank(system, &soundDetails, &bankDetails, "AudenFMOD_Music.bank");
    //LoadBank(system, &soundDetails, &bankDetails, "AudenFMOD_Ambience.bank");

    // Load .fsb

    //LoadFSB(lowLevelSystem, &soundDetails, &bankDetails, "VO.fsb");

    std::sort(soundDetails.begin(), soundDetails.end(), SoundDetailsComparator);

    for (int i=0; i<soundDetails.size(); i++) {
        printf("[Event] %i: %-80s (from %-24s, length: %5i)\n", i, soundDetails[i].name.c_str(), bankDetails[soundDetails[i].bank_id].name.c_str(), soundDetails[i].length_ms);
    }

    std::string sound_string;
    unsigned long lastEventId = soundDetails.size() - 1;
    printf("[Main] Enter sound to play (nothing to quit, 0-%lu to play): ", lastEventId);
    std::cin >> sound_string;

    int sound_id = std::stoi(sound_string);

    const SoundDetails & sound = soundDetails[sound_id];

    std::cout << "[Main] Playing " << sound_id << " (" << sound.name << ")..." << std::endl;

    FMOD::Studio::EventInstance * instance;
    sound.sound->setMode(FMOD_2D);
    ERRCHECK(sound.event->createInstance(&instance));
    ERRCHECK(instance->start());

    system->flushCommands();

    SetChannels(instance);

    std::this_thread::sleep_for(std::chrono::milliseconds(sound.length_ms));

    ERRCHECK(system->release());

    return 0;
}
