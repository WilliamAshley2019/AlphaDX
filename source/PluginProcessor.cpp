#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DX7AudioProcessor::DX7AudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, juce::Identifier("DX7FM"),
        {
            std::make_unique<juce::AudioParameterFloat>("attack", "Attack", 0.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("decay", "Decay", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("release", "Release", 0.0f, 1.0f, 0.3f),
            std::make_unique<juce::AudioParameterFloat>("coarse", "Coarse", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("fine", "Fine", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("modInit", "Mod Init", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("modDecay", "Mod Decay", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("modSus", "Mod Sus", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("modRel", "Mod Rel", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("modVel", "Mod Vel", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("vibrato", "Vibrato", 0.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("octave", "Octave", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("fineTune", "Fine Tune", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("waveform", "Waveform", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("modThru", "Mod Thru", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("lfoRate", "LFO Rate", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("mwToVib", "MW->Vib", 0.0f, 1.0f, 0.0f),
            // CPU Saver Toggles
            std::make_unique<juce::AudioParameterBool>("bypassModEnv", "Bypass Mod Env", false),
            std::make_unique<juce::AudioParameterBool>("bypassLfo", "Bypass LFO", false),
            std::make_unique<juce::AudioParameterBool>("bypassModThru", "Bypass Mod Thru", false)
        })
{
    attack = parameters.getRawParameterValue("attack");
    decay = parameters.getRawParameterValue("decay");
    release = parameters.getRawParameterValue("release");
    coarse = parameters.getRawParameterValue("coarse");
    fine = parameters.getRawParameterValue("fine");
    modInit = parameters.getRawParameterValue("modInit");
    modDecay = parameters.getRawParameterValue("modDecay");
    modSus = parameters.getRawParameterValue("modSus");
    modRel = parameters.getRawParameterValue("modRel");
    modVel = parameters.getRawParameterValue("modVel");
    vibrato = parameters.getRawParameterValue("vibrato");
    octave = parameters.getRawParameterValue("octave");
    fineTune = parameters.getRawParameterValue("fineTune");
    waveform = parameters.getRawParameterValue("waveform");
    modThru = parameters.getRawParameterValue("modThru");
    lfoRate = parameters.getRawParameterValue("lfoRate");
    mwToVib = parameters.getRawParameterValue("mwToVib");

    bypassModEnv = parameters.getRawParameterValue("bypassModEnv");
    bypassLfo = parameters.getRawParameterValue("bypassLfo");
    bypassModThru = parameters.getRawParameterValue("bypassModThru");

    fillPresets();
    loadPreset(0);
}

DX7AudioProcessor::~DX7AudioProcessor() {}

//==============================================================================
void DX7AudioProcessor::fillPresets()
{
    struct PresetData { const char* name; float p[17]; };
    PresetData presetData[] = {
        { "Bright E.Piano", {0.000f, 0.650f, 0.441f, 0.842f, 0.329f, 0.230f, 0.800f, 0.050f, 0.800f, 0.900f, 0.000f, 0.500f, 0.500f, 0.447f, 0.000f, 0.414f, 0.0f} },
        { "Jazz E.Piano", {0.000f, 0.500f, 0.100f, 0.671f, 0.000f, 0.441f, 0.336f, 0.243f, 0.800f, 0.500f, 0.000f, 0.500f, 0.500f, 0.178f, 0.000f, 0.500f, 0.0f} },
        { "E.Piano Pad", {0.000f, 0.700f, 0.400f, 0.230f, 0.184f, 0.270f, 0.474f, 0.224f, 0.800f, 0.974f, 0.250f, 0.500f, 0.500f, 0.428f, 0.836f, 0.500f, 0.0f} },
        { "Fuzzy E.Piano", {0.000f, 0.700f, 0.400f, 0.320f, 0.217f, 0.599f, 0.670f, 0.309f, 0.800f, 0.500f, 0.263f, 0.507f, 0.500f, 0.276f, 0.638f, 0.526f, 0.0f} },
        { "Soft Chimes", {0.400f, 0.600f, 0.650f, 0.760f, 0.000f, 0.390f, 0.250f, 0.160f, 0.900f, 0.500f, 0.362f, 0.500f, 0.500f, 0.401f, 0.296f, 0.493f, 0.0f} },
        { "Harpsichord", {0.000f, 0.342f, 0.000f, 0.280f, 0.000f, 0.880f, 0.100f, 0.408f, 0.740f, 0.000f, 0.000f, 0.600f, 0.500f, 0.842f, 0.651f, 0.500f, 0.0f} },
        { "Funk Clav", {0.000f, 0.400f, 0.100f, 0.360f, 0.000f, 0.875f, 0.160f, 0.592f, 0.800f, 0.500f, 0.000f, 0.500f, 0.500f, 0.303f, 0.868f, 0.500f, 0.0f} },
        { "Sitar", {0.000f, 0.500f, 0.704f, 0.230f, 0.000f, 0.151f, 0.750f, 0.493f, 0.770f, 0.500f, 0.000f, 0.400f, 0.500f, 0.421f, 0.632f, 0.500f, 0.0f} },
        { "Chiff Organ", {0.600f, 0.990f, 0.400f, 0.320f, 0.283f, 0.570f, 0.300f, 0.050f, 0.240f, 0.500f, 0.138f, 0.500f, 0.500f, 0.283f, 0.822f, 0.500f, 0.0f} },
        { "Tinkle", {0.000f, 0.500f, 0.650f, 0.368f, 0.651f, 0.395f, 0.550f, 0.257f, 0.900f, 0.500f, 0.300f, 0.800f, 0.500f, 0.000f, 0.414f, 0.500f, 0.0f} },
        { "Space Pad", {0.000f, 0.700f, 0.520f, 0.230f, 0.197f, 0.520f, 0.720f, 0.280f, 0.730f, 0.500f, 0.250f, 0.500f, 0.500f, 0.336f, 0.428f, 0.500f, 0.0f} },
        { "Koto", {0.000f, 0.240f, 0.000f, 0.390f, 0.000f, 0.880f, 0.100f, 0.600f, 0.740f, 0.500f, 0.000f, 0.500f, 0.500f, 0.526f, 0.480f, 0.500f, 0.0f} },
        { "Harp", {0.000f, 0.500f, 0.700f, 0.160f, 0.000f, 0.158f, 0.349f, 0.000f, 0.280f, 0.900f, 0.000f, 0.618f, 0.500f, 0.401f, 0.000f, 0.500f, 0.0f} },
        { "Jazz Guitar", {0.000f, 0.500f, 0.100f, 0.390f, 0.000f, 0.490f, 0.250f, 0.250f, 0.800f, 0.500f, 0.000f, 0.500f, 0.500f, 0.263f, 0.145f, 0.500f, 0.0f} },
        { "Steel Drum", {0.000f, 0.300f, 0.507f, 0.480f, 0.730f, 0.000f, 0.100f, 0.303f, 0.730f, 1.000f, 0.000f, 0.600f, 0.500f, 0.579f, 0.000f, 0.500f, 0.0f} },
        { "Log Drum", {0.000f, 0.300f, 0.500f, 0.320f, 0.000f, 0.467f, 0.079f, 0.158f, 0.500f, 0.500f, 0.000f, 0.400f, 0.500f, 0.151f, 0.020f, 0.500f, 0.0f} },
        { "Trumpet", {0.000f, 0.990f, 0.100f, 0.230f, 0.000f, 0.000f, 0.200f, 0.450f, 0.800f, 0.000f, 0.112f, 0.600f, 0.500f, 0.711f, 0.000f, 0.401f, 0.0f} },
        { "Horn", {0.280f, 0.990f, 0.280f, 0.230f, 0.000f, 0.180f, 0.400f, 0.300f, 0.800f, 0.500f, 0.000f, 0.400f, 0.500f, 0.217f, 0.480f, 0.500f, 0.0f} },
        { "Reed 1", {0.220f, 0.990f, 0.250f, 0.170f, 0.000f, 0.240f, 0.310f, 0.257f, 0.900f, 0.757f, 0.000f, 0.500f, 0.500f, 0.697f, 0.803f, 0.500f, 0.0f} },
        { "Reed 2", {0.220f, 0.990f, 0.250f, 0.450f, 0.070f, 0.240f, 0.310f, 0.360f, 0.900f, 0.500f, 0.211f, 0.500f, 0.500f, 0.184f, 0.000f, 0.414f, 0.0f} },
        { "Violin", {0.697f, 0.990f, 0.421f, 0.230f, 0.138f, 0.750f, 0.390f, 0.513f, 0.800f, 0.316f, 0.467f, 0.678f, 0.500f, 0.743f, 0.757f, 0.487f, 0.0f} },
        { "Chunky Bass", {0.000f, 0.400f, 0.000f, 0.280f, 0.125f, 0.474f, 0.250f, 0.100f, 0.500f, 0.500f, 0.000f, 0.400f, 0.500f, 0.579f, 0.592f, 0.500f, 0.0f} },
        { "E.Bass", {0.230f, 0.500f, 0.100f, 0.395f, 0.000f, 0.388f, 0.092f, 0.250f, 0.150f, 0.500f, 0.200f, 0.200f, 0.500f, 0.178f, 0.822f, 0.500f, 0.0f} },
        { "Clunk Bass", {0.000f, 0.600f, 0.400f, 0.230f, 0.000f, 0.450f, 0.320f, 0.050f, 0.900f, 0.500f, 0.000f, 0.200f, 0.500f, 0.520f, 0.105f, 0.500f, 0.0f} },
        { "Thick Bass", {0.000f, 0.600f, 0.400f, 0.170f, 0.145f, 0.290f, 0.350f, 0.100f, 0.900f, 0.500f, 0.000f, 0.400f, 0.500f, 0.441f, 0.309f, 0.500f, 0.0f} },
        { "Sine Bass", {0.000f, 0.600f, 0.490f, 0.170f, 0.151f, 0.099f, 0.400f, 0.000f, 0.900f, 0.500f, 0.000f, 0.400f, 0.500f, 0.118f, 0.013f, 0.500f, 0.0f} },
        { "Square Bass", {0.000f, 0.600f, 0.100f, 0.320f, 0.000f, 0.350f, 0.670f, 0.100f, 0.150f, 0.500f, 0.000f, 0.200f, 0.500f, 0.303f, 0.730f, 0.500f, 0.0f} },
        { "Upright Bass 1", {0.300f, 0.500f, 0.400f, 0.280f, 0.000f, 0.180f, 0.540f, 0.000f, 0.700f, 0.500f, 0.000f, 0.400f, 0.500f, 0.296f, 0.033f, 0.500f, 0.0f} },
        { "Upright Bass 2", {0.300f, 0.500f, 0.400f, 0.360f, 0.000f, 0.461f, 0.070f, 0.070f, 0.700f, 0.500f, 0.000f, 0.400f, 0.500f, 0.546f, 0.467f, 0.500f, 0.0f} },
        { "Harmonics", {0.000f, 0.500f, 0.500f, 0.280f, 0.000f, 0.330f, 0.200f, 0.000f, 0.700f, 0.500f, 0.000f, 0.500f, 0.500f, 0.151f, 0.079f, 0.500f, 0.0f} },
        { "Scratch", {0.000f, 0.500f, 0.000f, 0.000f, 0.240f, 0.580f, 0.630f, 0.000f, 0.000f, 0.500f, 0.000f, 0.600f, 0.500f, 0.816f, 0.243f, 0.500f, 0.0f} },
        { "Syn Tom", {0.000f, 0.355f, 0.350f, 0.000f, 0.105f, 0.000f, 0.000f, 0.200f, 0.500f, 0.500f, 0.000f, 0.645f, 0.500f, 1.000f, 0.296f, 0.500f, 0.0f} }
    };

    for (int i = 0; i < numPresets && i < 32; ++i)
    {
        presets[i].name = presetData[i].name;
        for (int j = 0; j < 17; ++j)
            presets[i].params[j] = presetData[i].p[j];
    }
}

void DX7AudioProcessor::loadPreset(int index)
{
    if (index >= 0 && index < numPresets)
    {
        currentPresetIndex = index;

        // CRITICAL FIX: Push preset values to the APVTS so the UI and Audio Engine stay in sync
        parameters.getParameter("attack")->setValueNotifyingHost(presets[index].params[0]);
        parameters.getParameter("decay")->setValueNotifyingHost(presets[index].params[1]);
        parameters.getParameter("release")->setValueNotifyingHost(presets[index].params[2]);
        parameters.getParameter("coarse")->setValueNotifyingHost(presets[index].params[3]);
        parameters.getParameter("fine")->setValueNotifyingHost(presets[index].params[4]);
        parameters.getParameter("modInit")->setValueNotifyingHost(presets[index].params[5]);
        parameters.getParameter("modDecay")->setValueNotifyingHost(presets[index].params[6]);
        parameters.getParameter("modSus")->setValueNotifyingHost(presets[index].params[7]);
        parameters.getParameter("modRel")->setValueNotifyingHost(presets[index].params[8]);
        parameters.getParameter("modVel")->setValueNotifyingHost(presets[index].params[9]);
        parameters.getParameter("vibrato")->setValueNotifyingHost(presets[index].params[10]);
        parameters.getParameter("octave")->setValueNotifyingHost(presets[index].params[11]);
        parameters.getParameter("fineTune")->setValueNotifyingHost(presets[index].params[12]);
        parameters.getParameter("waveform")->setValueNotifyingHost(presets[index].params[13]);
        parameters.getParameter("modThru")->setValueNotifyingHost(presets[index].params[14]);
        parameters.getParameter("lfoRate")->setValueNotifyingHost(presets[index].params[15]);
        parameters.getParameter("mwToVib")->setValueNotifyingHost(presets[index].params[16]);
    }
}

void DX7AudioProcessor::updateParameters()
{
    const float ifs = 1.0f / static_cast<float>(sampleRate);

    float octaveShift = std::floor(currentParams[11] * 6.9f) - 2.0f;
    tune = 8.175798915644f * ifs * std::pow(2.0f, octaveShift);

    float rati = currentParams[3];
    rati = std::floor(40.1f * rati * rati);
    float ratf;
    if (currentParams[4] < 0.5f)
        ratf = 0.2f * currentParams[4] * currentParams[4];
    else
    {
        int idx = static_cast<int>(8.9f * currentParams[4]);
        switch (idx)
        {
        case 4: ratf = 0.25f; break;
        case 5: ratf = 0.33333333f; break;
        case 6: ratf = 0.50f; break;
        case 7: ratf = 0.66666667f; break;
        default: ratf = 0.75f;
        }
    }
    ratio = 1.570796326795f * (rati + ratf);

    depth = 0.0002f * currentParams[5] * currentParams[5];
    dept2 = 0.0002f * currentParams[7] * currentParams[7];
    catt = 1.0f - std::exp(-ifs * std::exp(8.0f - 8.0f * currentParams[0]));
    cdec = (currentParams[1] > 0.98f) ? 1.0f : std::exp(-ifs * std::exp(5.0f - 8.0f * currentParams[1]));
    crel = std::exp(-ifs * std::exp(5.0f - 5.0f * currentParams[2]));
    mdec = 1.0f - std::exp(-ifs * std::exp(6.0f - 7.0f * currentParams[6]));
    mrel = 1.0f - std::exp(-ifs * std::exp(5.0f - 8.0f * currentParams[8]));

    rich = 0.50f - 3.0f * currentParams[13] * currentParams[13];
    modmix = 0.25f * currentParams[14] * currentParams[14];
    dlfo = 628.3f * ifs * 25.0f * currentParams[15] * currentParams[15];
    vibratoAmount = 0.001f * currentParams[10] * currentParams[10];
    modWheelToVibrato = currentParams[16] >= 0.5f;
}

//==============================================================================
const juce::String DX7AudioProcessor::getName() const { return JucePlugin_Name; }
bool DX7AudioProcessor::acceptsMidi() const { return true; }
bool DX7AudioProcessor::producesMidi() const { return false; }
double DX7AudioProcessor::getTailLengthSeconds() const { return 0.0; }
int DX7AudioProcessor::getNumPrograms() { return numPresets; }
int DX7AudioProcessor::getCurrentProgram() { return currentPresetIndex; }
void DX7AudioProcessor::setCurrentProgram(int index) { loadPreset(index); }
const juce::String DX7AudioProcessor::getProgramName(int index) { return (index >= 0 && index < numPresets) ? presets[index].name : juce::String(); }
void DX7AudioProcessor::changeProgramName(int index, const juce::String& newName) { if (index >= 0 && index < numPresets) presets[index].name = newName; }

//==============================================================================
void DX7AudioProcessor::prepareToPlay(double sampleRate_, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    sampleRate = sampleRate_;
    updateParameters();
    for (auto& voice : voices) {
        voice.active = false;
        voice.env = voice.car = voice.dcar = voice.mod0 = voice.mod1 = 0.0f;
    }
    lfoPhase = 0.0f;
    lfoCounter = 0;
}

void DX7AudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DX7AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts); return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) return false;
#endif
    return true;
#endif
}
#endif

void DX7AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ScopedLock sl(lock);

    currentParams[0] = attack->load();   currentParams[1] = decay->load();
    currentParams[2] = release->load();  currentParams[3] = coarse->load();
    currentParams[4] = fine->load();     currentParams[5] = modInit->load();
    currentParams[6] = modDecay->load(); currentParams[7] = modSus->load();
    currentParams[8] = modRel->load();   currentParams[9] = modVel->load();
    currentParams[10] = vibrato->load(); currentParams[11] = octave->load();
    currentParams[12] = fineTune->load(); currentParams[13] = waveform->load();
    currentParams[14] = modThru->load(); currentParams[15] = lfoRate->load();
    currentParams[16] = mwToVib->load();
    updateParameters();

    // Read CPU saver states
    bool skipModEnv = bypassModEnv->load() >= 0.5f;
    bool skipLfo = bypassLfo->load() >= 0.5f;
    bool skipModThru = bypassModThru->load() >= 0.5f;

    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);
    int numSamples = buffer.getNumSamples();

    for (const auto metadata : midiMessages) handleMidiEvent(metadata.getMessage());
    buffer.clear();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        if (--lfoCounter <= 0) {
            lfoPhase += dlfo;
            if (lfoPhase > 6.2831853f) lfoPhase -= 6.2831853f;
            lfoValue = std::sin(lfoPhase);
            lfoCounter = 100;
        }

        float output = 0.0f;
        for (int voice = 0; voice < maxVoices; ++voice)
            if (voices[voice].active || voices[voice].env > silenceThreshold)
                renderVoice(voice, output, skipModEnv, skipLfo, skipModThru);

        leftChannel[sample] = output;
        rightChannel[sample] = output;
    }
}

void DX7AudioProcessor::renderVoice(int voiceIndex, float& output, bool skipModEnv, bool skipLfo, bool skipModThru)
{
    auto& v = voices[voiceIndex];
    if (!v.active && v.env < silenceThreshold) return;

    v.env *= v.cdec;
    v.cenv += v.catt * (v.env - v.cenv);

    float x = v.dmod * v.mod0 - v.mod1;
    v.mod1 = v.mod0;
    v.mod0 = x;

    // CPU Saver: Skip Mod Envelope calculations if bypassed
    if (!skipModEnv) {
        v.menv += v.mdec * (v.mlev - v.menv);
    }
    else {
        v.menv = 0.0f;
    }

    // CPU Saver: Skip LFO calculations if bypassed
    float lfoMod = 0.0f;
    if (!skipLfo) {
        lfoMod = lfoValue * vibratoAmount;
        if (modWheelToVibrato) lfoMod *= (modWheel + 1.0f);
    }

    v.dcar = tune * pitchBend * v.tuning;
    float phase = v.car + v.dcar + x * v.menv + lfoMod;

    while (phase > 1.0f) phase -= 2.0f;
    while (phase < -1.0f) phase += 2.0f;
    v.car = phase;

    float sine = phase + phase * phase * phase * (rich * phase * phase - 1.0f - rich);

    // CPU Saver: Skip Mod Thru mixing if bypassed
    float mix = sine;
    if (!skipModThru) {
        mix += modmix * v.mod1;
    }

    output += v.cenv * mix;

    if (v.env < silenceThreshold && !v.active) {
        v.env = v.cenv = v.menv = v.mlev = 0.0f;
    }
}

//==============================================================================
bool DX7AudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* DX7AudioProcessor::createEditor() { return new DX7AudioProcessorEditor(*this); }

//==============================================================================
void DX7AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DX7AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
void DX7AudioProcessor::handleMidiEvent(const juce::MidiMessage& msg)
{
    if (msg.isNoteOn()) noteOn(msg.getNoteNumber(), msg.getVelocity());
    else if (msg.isNoteOff()) noteOff(msg.getNoteNumber());
    else if (msg.isController())
    {
        int cc = msg.getControllerNumber();
        int value = msg.getControllerValue();
        switch (cc)
        {
        case 1: modWheel = 0.00000005f * value * value; break;
        case 7: volume = 0.00000035f * value * value; break;
        case 64:
            sustainPedal = (value >= 64) ? 1 : 0;
            if (sustainPedal == 0)
                for (auto& voice : voices) if (voice.note == 128) noteOff(voice.note);
            break;
        case 123: case 120:
            for (auto& voice : voices) { voice.active = false; voice.env = 0.0f; }
            sustainPedal = 0;
            break;
        }
    }
    else if (msg.isProgramChange()) loadPreset(msg.getProgramChangeNumber());
    else if (msg.isPitchWheel())
    {
        float bend = (float)msg.getPitchWheelValue() - 8192.0f;
        pitchBend = (bend > 0.0f) ? 1.0f + 0.000014951f * bend : 1.0f + 0.000013318f * bend;
    }
}

void DX7AudioProcessor::noteOn(int note, int velocity)
{
    if (velocity == 0) { noteOff(note); return; }

    int targetVoice = 0;
    float minEnv = std::numeric_limits<float>::max();
    for (int i = 0; i < maxVoices; ++i) {
        if (!voices[i].active) { targetVoice = i; break; }
        if (voices[i].env < minEnv) { minEnv = voices[i].env; targetVoice = i; }
    }

    auto& v = voices[targetVoice];
    float fineShift = currentParams[12] + currentParams[12] - 1.0f;
    v.tuning = std::exp(0.05776226505f * (note + fineShift));
    v.note = note;
    v.active = true;

    v.car = 0.0f;
    v.dcar = tune * pitchBend * v.tuning;

    float keyTrack = std::min(v.tuning, 50.0f);
    float velScale = (64.0f + currentParams[9] * (velocity - 64));
    float amp = keyTrack * velScale;
    v.menv = depth * amp;
    v.mlev = dept2 * amp;
    v.mdec = mdec;
    v.dmod = ratio * v.dcar;
    v.mod0 = 0.0f;
    v.mod1 = std::sin(v.dmod);
    v.dmod = 2.0f * std::cos(v.dmod);

    v.env = (1.5f - currentParams[13]) * volume * (velocity + 10);
    v.catt = catt;
    v.cenv = 0.0f;
    v.cdec = cdec;
}

void DX7AudioProcessor::noteOff(int note)
{
    for (auto& v : voices) {
        if (v.note == note) {
            if (sustainPedal == 0) {
                v.cdec = crel; v.env = v.cenv; v.catt = 1.0f; v.mlev = 0.0f; v.mdec = mrel;
            }
            else {
                v.note = 128;
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DX7AudioProcessor(); }