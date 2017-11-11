#include "fmod_studio.hpp"
#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <iostream>
#include <sstream>

char app_dir_buffer[PATH_MAX];
std::string APP_DIR = getcwd(app_dir_buffer, sizeof(app_dir_buffer));
std::string SOUND_DIR = APP_DIR + "/assets/sounds/";
std::string PLUGINS_DIR = APP_DIR + "/assets/plugins";


class ParameterDetails {
public:
    std::string name;
};

class SoundDetails {
public:
    int length_ms;
    std::string name;
    std::vector<ParameterDetails> parameters;
    int user_property_count;
    int bank_id;
    int event_id;

    FMOD::Studio::EventDescription * event;
    FMOD::Sound * sound;
};

class BankDetails {
public:
    std::string name;
    std::string filename;
    FMOD::Studio::Bank * bank;
};

void ERRCHECK_fn(FMOD_RESULT result, const char *file, int line) {
    if (result != 0) {
        printf("[ERROR] Error at line %i in \"%s\". FMOD_RESULT: %i\n", line, file, result);
    }
}
#define ERRCHECK(_result) ERRCHECK_fn(_result, __FILE__, __LINE__)

void RecurseSounds(FMOD::System * system, std::vector<SoundDetails> * soundDetails, std::vector<BankDetails> * bankDetails, int bank_id, FMOD::Sound * sound) {
    SoundDetails soundDetail;
    soundDetail.event = NULL;

    sound->getLength((unsigned int *)&(soundDetail.length_ms), FMOD_TIMEUNIT_MS);

    char name[8192];
    memset(name, 0, sizeof(name));
    ERRCHECK(sound->getName(name, sizeof(name)));
    soundDetail.name = std::string(name);

    soundDetail.user_property_count = 0;
    soundDetail.bank_id = bank_id;
    soundDetail.event_id = 0;
    soundDetail.sound = sound;
    soundDetails->push_back(soundDetail);

    int numSubSounds = 0;
    ERRCHECK(sound->getNumSubSounds(&numSubSounds));

    for (int i=0; i<numSubSounds; i++) {
        FMOD::Sound * subSound;
        ERRCHECK(sound->getSubSound(i, &subSound));
        RecurseSounds(system, soundDetails, bankDetails, bank_id, subSound);
    }

}

void LoadBank(FMOD::Studio::System * system, std::vector<SoundDetails> * soundDetails, std::vector<BankDetails> * bankDetails, const char * filename) {
    printf("[LoadBank] Loading \"%s\".\n", filename);

    BankDetails bank;
    bank.name = std::string(filename);
    bank.filename = std::string(SOUND_DIR.c_str()) + std::string(filename);
    ERRCHECK(system->loadBankFile(bank.filename.c_str(), 0, &bank.bank));

    int event_count = -1;
    ERRCHECK(bank.bank->getEventCount(&event_count));
    printf("[LoadBank] Bank has %i events.\n", event_count);

    int bus_count = -1;
    ERRCHECK(bank.bank->getBusCount(&bus_count));
    printf("[LoadBank] Bank has %i busses.\n", bus_count);

    int string_count = -1;
    ERRCHECK(bank.bank->getStringCount(&string_count));
    printf("[LoadBank] Bank has %i strings.\n", string_count);

    int vca_count = -1;
    ERRCHECK(bank.bank->getVCACount(&vca_count));
    printf("[LoadBank] Bank has %i \"VCA\"s.\n", vca_count);

    bankDetails->push_back(bank);

    FMOD::Studio::EventDescription ** events = new FMOD::Studio::EventDescription*[event_count];
    int event_list_count = -1;
    ERRCHECK(bank.bank->getEventList(events, event_count, &event_list_count));
    printf("[LoadBank] Reading %i event descriptors.\n", event_list_count);

    //std::vector<std::string> names;
    for (int i=0; i<event_count; i++) {
        SoundDetails sound;
        sound.sound = NULL;
        sound.event = events[i];

        char path[8192];
        memset(path, 0, sizeof(path));
        int out_len;
        ERRCHECK(events[i]->getPath(path, sizeof(path), &out_len));
        sound.name = std::string(path);
        ERRCHECK(events[i]->getLength(&sound.length_ms));
        ERRCHECK(events[i]->getUserPropertyCount(&sound.user_property_count));

        int param_count;
        ERRCHECK(events[i]->getParameterCount(&param_count));

        for (int j=0; j<param_count; j++) {
            FMOD_STUDIO_PARAMETER_DESCRIPTION param;
            ERRCHECK(events[i]->getParameterByIndex(j, &param));

            ParameterDetails param_details;
            param_details.name = std::string(param.name);
            sound.parameters.push_back(param_details);
        }

        sound.bank_id = bankDetails->size() - 1;
        sound.event_id = i;
        soundDetails->push_back(sound);
    }
}

void LoadVO(FMOD::System * system, std::vector<SoundDetails> * soundDetails, std::vector<BankDetails> * bankDetails, const char * filename) {
    printf("[LoadVO] Loading \"%s\".\n", filename);

    BankDetails bank;
    bank.name = std::string(filename);
    bank.filename = std::string(SOUND_DIR.c_str()) + std::string(filename);

    bankDetails->push_back(bank);

    FMOD::Sound * sound;
    ERRCHECK(system->createSound(bank.filename.c_str(), 584, NULL, &sound));

    RecurseSounds(system, soundDetails, bankDetails, bankDetails->size() - 1, sound);
}

bool SoundDetailsComparator(const SoundDetails & x, const SoundDetails & y) {
    return x.name.compare(y.name) < 0;
}

bool channelFinished = false;

FMOD_RESULT F_CALLBACK channelEndCallback(FMOD_CHANNELCONTROL *chanControl, FMOD_CHANNELCONTROL_TYPE controlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *commandData1, void *commandData2)
{
    if (callbackType == FMOD_CHANNELCONTROL_CALLBACK_END) {
        channelFinished = true;
    }

    return FMOD_OK;
}

// Split from https://stackoverflow.com/questions/236129, thanks to Evan Teran.
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

int main(int argc, char ** argv)
{
    printf("SOUND_DIR: %s \n", SOUND_DIR.c_str());
    printf("PLUGINS_DIR: %s \n", PLUGINS_DIR.c_str());

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

    //LoadVO(lowLevelSystem, &soundDetails, &bankDetails, "VO.fsb");

    std::sort(soundDetails.begin(), soundDetails.end(), SoundDetailsComparator);

    for (int i=0; i<soundDetails.size(); i++) {
        printf("[Event] %i: %-80s (from %-24s, length: %5i)\n", i, soundDetails[i].name.c_str(), bankDetails[soundDetails[i].bank_id].name.c_str(), soundDetails[i].length_ms);
    }

    std::string sound_string;
    std::cout << "[Main] Enter sound to play (nothing to quit, 0-5 to play 0-5 inclusive): ";
    std::cin >> sound_string;

    std::vector<std::string> numbers = split(sound_string, '-');

    int sound_id = atoi(numbers[0].c_str());

    const SoundDetails & sound = soundDetails[sound_id];

    std::cout << "[Main] Playing " << sound_id << " (" << sound.name << ")..." << std::endl;

    FMOD::Studio::EventInstance * instance;
    ERRCHECK(sound.event->createInstance(&instance));


    ERRCHECK(instance->start());

    FMOD_3D_ATTRIBUTES attributes = { { 0 } };
    attributes.forward.z = 1.0f;
    attributes.up.y = 1.0f;
    ERRCHECK( system->setListenerAttributes(0, &attributes) );

    attributes.position.z = 0.0f;
    ERRCHECK( instance->set3DAttributes(&attributes) );

    FMOD_STUDIO_PLAYBACK_STATE state;

    bool a = true; // Just for debugging
    while (true) {
        // The following configuration will play event /Music/MainTheme_MC as idle (with no drums and humming)

        system->update();
        // Each track has a ChannelGroup. Inside a ChannelGroup are a number of other ChannelGroups..
        FMOD::ChannelGroup *topGroup;
        int topNumChannels;  // Only good for debug purposes, e.g. printing the number of channels.
        instance->getChannelGroup(&topGroup);
        topGroup->getNumGroups(&topNumChannels);

        if (a == true) {
            std::cout << "topNumChannels = " << topNumChannels << std::endl;
        }

        // In Transistor's audio, the track's sub-ChannelGroups are the ones that have the actual audio channels.
        FMOD::ChannelGroup *sub1, *sub2;
        int numChannels;
        int numGroups;

        // Get the ChannelGroup that we want to manipulate (e.g. mute)
        topGroup->getGroup(0, &sub1);  // may vary for each track.
        topGroup->getGroup(2, &sub2);

        // Get some info about the Sub-ChannelGroup
        sub1->getNumChannels(&numChannels);
        sub1->getNumGroups(&numGroups);

        if (a == true) {
            std::cout << "numChannels = " << numChannels << std::endl;
            std::cout << "numGroups = " << numGroups << std::endl;
            a = false;
        }

        // The audio channels don't load in instantly, so we want to wait until they exist before doing stuff to them.
        if (numChannels > 0) {
            // Get the audio channel that we want to manuplate.
            FMOD::Channel *channel1, *channel2;
            sub1->getChannel(0, &channel1);  // 0 is hardcoded, and may vary for each track.
            channel1->setVolume(0.0);  // MUTE!

            sub2->getChannel(0, &channel2);  // 0 is hardcoded, and may vary for each track.
            channel2->setVolume(0.0);  // MUTE!
        }

        usleep(1000); // Important. FMOD_OUTPUTTYPE_WAVWRITER_NRT will play with echoes if reduce usleep()
    }

    ERRCHECK(system->release());

    return 0;
}
