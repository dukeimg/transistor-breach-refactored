//
// Created by Viktor Danilov on 11/11/2017.
//

#include <loader.h>

void ERRCHECK(FMOD_RESULT result) {
    if (result != 0) {
        printf("[ERROR] Error at line %i in \"%s\". FMOD_RESULT: %i\n", __LINE__, __FILE__, result);
    }
}

bool SoundDetailsComparator(const SoundDetails & x, const SoundDetails & y) {
    return x.name.compare(y.name) < 0;
}

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

void LoadFSB(FMOD::System * system, std::vector<SoundDetails> * soundDetails, std::vector<BankDetails> * bankDetails, const char * filename) {
    printf("[LoadVO] Loading \"%s\".\n", filename);

    BankDetails bank;
    bank.name = std::string(filename);
    bank.filename = std::string(SOUND_DIR.c_str()) + std::string(filename);

    bankDetails->push_back(bank);

    FMOD::Sound * sound;
    ERRCHECK(system->createSound(bank.filename.c_str(), 584, NULL, &sound));

    RecurseSounds(system, soundDetails, bankDetails, bankDetails->size() - 1, sound);
}