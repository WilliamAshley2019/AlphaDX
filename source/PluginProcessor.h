#pragma once
#include <JuceHeader.h>

//==============================================================================
class DX7AudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    DX7AudioProcessor();
    ~DX7AudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter access
    std::atomic<float>* getAttackParam() { return attack; }
    std::atomic<float>* getDecayParam() { return decay; }
    std::atomic<float>* getReleaseParam() { return release; }
    std::atomic<float>* getCoarseParam() { return coarse; }
    std::atomic<float>* getFineParam() { return fine; }
    std::atomic<float>* getModInitParam() { return modInit; }
    std::atomic<float>* getModDecayParam() { return modDecay; }
    std::atomic<float>* getModSusParam() { return modSus; }
    std::atomic<float>* getModRelParam() { return modRel; }
    std::atomic<float>* getModVelParam() { return modVel; }
    std::atomic<float>* getVibratoParam() { return vibrato; }
    std::atomic<float>* getOctaveParam() { return octave; }
    std::atomic<float>* getFineTuneParam() { return fineTune; }
    std::atomic<float>* getWaveformParam() { return waveform; }
    std::atomic<float>* getModThruParam() { return modThru; }
    std::atomic<float>* getLfoRateParam() { return lfoRate; }
    std::atomic<float>* getMwToVibParam() { return mwToVib; }

    // CPU Saver bypass parameters
    std::atomic<float>* getBypassModEnvParam() { return bypassModEnv; }
    std::atomic<float>* getBypassLfoParam() { return bypassLfo; }
    std::atomic<float>* getBypassModThruParam() { return bypassModThru; }

    juce::AudioProcessorValueTreeState& getAPVTS() { return parameters; }
    void loadPreset(int index);

private:
    //==============================================================================
    struct VoiceState
    {
        float env = 0.0f, dmod = 0.0f, mod0 = 0.0f, mod1 = 0.0f;
        float menv = 0.0f, mlev = 0.0f, mdec = 0.0f;
        float car = 0.0f, dcar = 0.0f, cenv = 0.0f, catt = 0.0f, cdec = 0.0f;
        int note = 0;
        float tuning = 1.0f;
        bool active = false;
    };

    struct Preset
    {
        juce::String name;
        float params[17];
    };

    static constexpr int maxVoices = 8;
    static constexpr int numPresets = 32;
    static constexpr float silenceThreshold = 0.0003f;

    void updateParameters();
    void handleMidiEvent(const juce::MidiMessage& msg);
    void noteOn(int note, int velocity);
    void noteOff(int note);
    void renderVoice(int voiceIndex, float& output, bool skipModEnv, bool skipLfo, bool skipModThru);
    void fillPresets();

    std::array<VoiceState, maxVoices> voices;
    std::array<Preset, numPresets> presets;
    int currentPresetIndex = 0;

    float currentParams[17] = { 0 };
    float tune = 0.0f, ratio = 0.0f, depth = 0.0f, dept2 = 0.0f;
    float catt = 0.0f, cdec = 0.0f, crel = 0.0f, mdec = 0.0f, mrel = 0.0f;
    float rich = 0.0f, modmix = 0.0f, dlfo = 0.0f, vibratoAmount = 0.0f;
    float lfoPhase = 0.0f, lfoValue = 0.0f, modWheel = 0.0f;
    float volume = 0.0035f, pitchBend = 1.0f;
    bool modWheelToVibrato = false;
    int lfoCounter = 0, sustainPedal = 0;
    double sampleRate = 44100.0;

    std::atomic<float>* attack;
    std::atomic<float>* decay;
    std::atomic<float>* release;
    std::atomic<float>* coarse;
    std::atomic<float>* fine;
    std::atomic<float>* modInit;
    std::atomic<float>* modDecay;
    std::atomic<float>* modSus;
    std::atomic<float>* modRel;
    std::atomic<float>* modVel;
    std::atomic<float>* vibrato;
    std::atomic<float>* octave;
    std::atomic<float>* fineTune;
    std::atomic<float>* waveform;
    std::atomic<float>* modThru;
    std::atomic<float>* lfoRate;
    std::atomic<float>* mwToVib;

    std::atomic<float>* bypassModEnv;
    std::atomic<float>* bypassLfo;
    std::atomic<float>* bypassModThru;

    juce::AudioProcessorValueTreeState parameters;
    juce::CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DX7AudioProcessor)
};