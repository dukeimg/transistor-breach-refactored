//
// Created by Viktor Danilov on 11/11/2017.
//

#ifndef TRANSISTOR_BREACH_REFACTORED_LOADER_H
#define TRANSISTOR_BREACH_REFACTORED_LOADER_H

#endif //TRANSISTOR_BREACH_REFACTORED_LOADER_H

#include "fmod_studio.hpp"
#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <constance.h>

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

bool SoundDetailsComparator(const SoundDetails & x, const SoundDetails & y);
void LoadBank(FMOD::Studio::System * system, std::vector<SoundDetails> * soundDetails, std::vector<BankDetails> * bankDetails, const char * filename);
void LoadFSB(FMOD::System * system, std::vector<SoundDetails> * soundDetails, std::vector<BankDetails> * bankDetails, const char * filename);

void ERRCHECK(FMOD_RESULT result);