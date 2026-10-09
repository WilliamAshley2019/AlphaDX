#include "PluginProcessor.h"
#include "PluginEditor.h"

DX7AudioProcessorEditor::DX7AudioProcessorEditor(DX7AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel);

    // -------- Preset Selector --------
    presetSelector.onChange = [this] {
        audioProcessor.setCurrentProgram(presetSelector.getSelectedId() - 1);
        };

    for (int i = 0; i < audioProcessor.getNumPrograms(); ++i)
        presetSelector.addItem(audioProcessor.getProgramName(i), i + 1);
    presetSelector.setSelectedId(1);
    addAndMakeVisible(presetSelector);

    // -------- Section Frames --------
    auto setupGroup = [this](juce::GroupComponent& group, const juce::String& name) {
        group.setText(name);
        addAndMakeVisible(group);
        };

    setupGroup(envGroup, "ENVELOPE");
    setupGroup(modEnvGroup, "MOD ENVELOPE");
    setupGroup(oscGroup, "OSCILLATOR");
    setupGroup(lfoGroup, "LFO / PITCH");
    setupGroup(mixGroup, "MIX / CPU");

    // -------- Knobs --------
    addAndMakeVisible(attackKnob);
    addAndMakeVisible(decayKnob);
    addAndMakeVisible(releaseKnob);

    addAndMakeVisible(modInitKnob);
    addAndMakeVisible(modDecayKnob);
    addAndMakeVisible(modSusKnob);
    addAndMakeVisible(modRelKnob);
    addAndMakeVisible(modVelKnob);

    addAndMakeVisible(coarseKnob);
    addAndMakeVisible(fineKnob);
    addAndMakeVisible(waveKnob);

    addAndMakeVisible(octaveKnob);
    addAndMakeVisible(fineTuneKnob);
    addAndMakeVisible(vibratoKnob);
    addAndMakeVisible(lfoRateKnob);
    addAndMakeVisible(mwToVibKnob);

    addAndMakeVisible(modThruKnob);

    // -------- Toggles --------
    bypassModEnvToggle.setLookAndFeel(&customLookAndFeel);
    bypassLfoToggle.setLookAndFeel(&customLookAndFeel);
    bypassModThruToggle.setLookAndFeel(&customLookAndFeel);

    addAndMakeVisible(bypassModEnvToggle);
    addAndMakeVisible(bypassLfoToggle);
    addAndMakeVisible(bypassModThruToggle);

    // -------- APVTS Attachments --------
    auto& apvts = audioProcessor.getAPVTS();

    attAttack = std::make_unique<SliderAtt>(apvts, "attack", attackKnob.slider);
    attDecay = std::make_unique<SliderAtt>(apvts, "decay", decayKnob.slider);
    attRelease = std::make_unique<SliderAtt>(apvts, "release", releaseKnob.slider);

    attModInit = std::make_unique<SliderAtt>(apvts, "modInit", modInitKnob.slider);
    attModDecay = std::make_unique<SliderAtt>(apvts, "modDecay", modDecayKnob.slider);
    attModSus = std::make_unique<SliderAtt>(apvts, "modSus", modSusKnob.slider);
    attModRel = std::make_unique<SliderAtt>(apvts, "modRel", modRelKnob.slider);
    attModVel = std::make_unique<SliderAtt>(apvts, "modVel", modVelKnob.slider);

    attCoarse = std::make_unique<SliderAtt>(apvts, "coarse", coarseKnob.slider);
    attFine = std::make_unique<SliderAtt>(apvts, "fine", fineKnob.slider);
    attWave = std::make_unique<SliderAtt>(apvts, "waveform", waveKnob.slider);

    attOctave = std::make_unique<SliderAtt>(apvts, "octave", octaveKnob.slider);
    attFineTune = std::make_unique<SliderAtt>(apvts, "fineTune", fineTuneKnob.slider);
    attVibrato = std::make_unique<SliderAtt>(apvts, "vibrato", vibratoKnob.slider);
    attLfoRate = std::make_unique<SliderAtt>(apvts, "lfoRate", lfoRateKnob.slider);
    attMwToVib = std::make_unique<SliderAtt>(apvts, "mwToVib", mwToVibKnob.slider);

    attModThru = std::make_unique<SliderAtt>(apvts, "modThru", modThruKnob.slider);

    bypassModEnvAttachment = std::make_unique<ButtonAtt>(apvts, "bypassModEnv", bypassModEnvToggle);
    bypassLfoAttachment = std::make_unique<ButtonAtt>(apvts, "bypassLfo", bypassLfoToggle);
    bypassModThruAttachment = std::make_unique<ButtonAtt>(apvts, "bypassModThru", bypassModThruToggle);

    setSize(860, 480);
    resized();                    // <— force layout, in case setSize was a no-op

    startTimerHz(2);              // blink cursor at 2 Hz
}

DX7AudioProcessorEditor::~DX7AudioProcessorEditor()
{
    stopTimer();
    bypassModEnvToggle.setLookAndFeel(nullptr);
    bypassLfoToggle.setLookAndFeel(nullptr);
    bypassModThruToggle.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

//==============================================================================
// TIMER: cursor blink
//==============================================================================
void DX7AudioProcessorEditor::timerCallback()
{
    cursorOn = !cursorOn;
    repaint(0, 0, getWidth(), 48);   // only repaint the header
}

//==============================================================================
// PAINT: CRT glass, scanlines, header TUI chrome, status footer
//==============================================================================
void DX7AudioProcessorEditor::paint(juce::Graphics& g)
{
    using C = CustomCyberpunkLookAndFeel;

    // 1. Black glass
    g.fillAll(C::crtBlack);

    // 2. Faint green vignette (screen glow)
    {
        juce::ColourGradient glow(C::crtGreen.withAlpha(0.05f),
            getWidth() * 0.5f, getHeight() * 0.5f,
            juce::Colours::transparentBlack,
            getWidth() * 0.5f, getHeight() * 0.75f, true);
        g.setGradientFill(glow);
        g.fillAll();
    }

    // 3. Raster scanlines every 2px
    g.setColour(juce::Colours::black.withAlpha(0.35f));
    for (int y = 0; y < getHeight(); y += 2)
        g.drawHorizontalLine(y, 0.0f, (float)getWidth());

    // 4. Faint phosphor grid dots (character cell overlay)
    g.setColour(C::crtGreenDim.withAlpha(0.06f));
    for (int x = 0; x < getWidth(); x += 8)
        for (int y = 0; y < getHeight(); y += 8)
            g.fillRect(x, y, 1, 1);

    // =====================================================================
    // HEADER: TUI title bar ┌─[ PAGE 1 ]──...──[ PROGRAM 01 ]─┐
    // =====================================================================
    const int headerH = 34;
    {
        // black bar behind
        g.setColour(C::crtBlack);
        g.fillRect(0, 0, getWidth(), headerH);

        // Double rule under header
        g.setColour(C::crtBorder);
        g.fillRect(0, headerH - 2, getWidth(), 2);
        g.setColour(C::crtBorderDim);
        g.fillRect(0, headerH - 4, getWidth(), 1);

        // Title text (green), monospace bold, all caps
        g.setColour(C::crtGreen);
        g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        g.drawText("ALPHA-DX  FM  SYNTHESIZER  //  PAGE 1",
            14, 0, 460, headerH, juce::Justification::centredLeft);

        // Blinking block cursor right after title
        if (cursorOn)
        {
            g.setColour(C::crtAmber);
            g.fillRect(468, 9, 8, headerH - 20);
        }

        // Right side: status indicators
        g.setColour(C::crtAmber);
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));

        juce::String status;
        status << "PROGRAM " << juce::String(audioProcessor.getCurrentProgram() + 1).paddedLeft('0', 2)
            << "/" << juce::String(audioProcessor.getNumPrograms()).paddedLeft('0', 2)
            << "   [";
        status << (audioProcessor.getCurrentProgram() >= 0 ? "LOADED" : "EMPTY") << "]";

        g.drawText(status, 0, 0, getWidth() - 12, headerH, juce::Justification::centredRight);
    }

    // =====================================================================
    // FOOTER: status line, like a terminal prompt
    // =====================================================================
    const int footerH = 16;
    const int footerY = getHeight() - footerH;
    {
        g.setColour(C::crtBlack);
        g.fillRect(0, footerY, getWidth(), footerH);
        g.setColour(C::crtBorderDim);
        g.fillRect(0, footerY, getWidth(), 1);

        g.setColour(C::crtGreenDim);
        g.setFont(juce::FontOptions(10.0f, juce::Font::plain));
        g.drawText("READY  //  8 VOICES  //  FM ENGINE  //  (C) 1983 ALPHA SYSTEMS",
            12, footerY, getWidth() - 24, footerH, juce::Justification::centredLeft);

        // Right: a row of status "LED" squares
        int x = getWidth() - 12;
        for (int i = 0; i < 4; ++i)
        {
            x -= 10;
            g.setColour((i % 2 == 0) ? C::crtGreen : C::crtGreenDim);
            g.fillRect(x, footerY + 5, 6, 6);
        }
    }
}

//==============================================================================
// RESIZED
//==============================================================================
void DX7AudioProcessorEditor::resized()
{
    constexpr int PAD = 8;
    constexpr int TOP = 42;   // shifted down for new header
    constexpr int HDR = 20;
    constexpr int GAP = 6;
    constexpr int K_SZ = 62;
    constexpr int K_H = 80;
    constexpr int FOOTER = 16;

    const int usableH = getHeight() - FOOTER;

    presetSelector.setBounds(getWidth() - 250 - PAD, 6, 250, 22);

    // =====================================================================
    // ROW 1
    // =====================================================================
    int row1Y = TOP + PAD;
    int row1H = 195;

    int envW = 3 * (K_SZ + GAP) + 12;
    envGroup.setBounds(PAD, row1Y, envW, row1H);

    int envX = PAD + 6;
    int envControlY = row1Y + HDR + 12;
    attackKnob.setBounds(envX, envControlY, K_SZ, K_H);
    decayKnob.setBounds(envX + (K_SZ + GAP), envControlY, K_SZ, K_H);
    releaseKnob.setBounds(envX + 2 * (K_SZ + GAP), envControlY, K_SZ, K_H);

    int modEnvX = PAD + envW + GAP;
    int modEnvW = 5 * (K_SZ + GAP) + 12;
    modEnvGroup.setBounds(modEnvX, row1Y, modEnvW, row1H);

    int mControlX = modEnvX + 6;
    modInitKnob.setBounds(mControlX, envControlY, K_SZ, K_H);
    modDecayKnob.setBounds(mControlX + (K_SZ + GAP), envControlY, K_SZ, K_H);
    modSusKnob.setBounds(mControlX + 2 * (K_SZ + GAP), envControlY, K_SZ, K_H);
    modRelKnob.setBounds(mControlX + 3 * (K_SZ + GAP), envControlY, K_SZ, K_H);
    modVelKnob.setBounds(mControlX + 4 * (K_SZ + GAP), envControlY, K_SZ, K_H);

    int mixX = modEnvX + modEnvW + GAP;
    int mixW = getWidth() - mixX - PAD;
    mixGroup.setBounds(mixX, row1Y, mixW, row1H);

    int mixControlX = mixX + (mixW - K_SZ) / 2;
    modThruKnob.setBounds(mixControlX, row1Y + HDR + 4, K_SZ, K_H);

    int toggleY = row1Y + HDR + K_H + 12;
    int toggleW = mixW - 12;
    bypassModEnvToggle.setBounds(mixX + 6, toggleY, toggleW, 18);
    bypassLfoToggle.setBounds(mixX + 6, toggleY + 20, toggleW, 18);
    bypassModThruToggle.setBounds(mixX + 6, toggleY + 40, toggleW, 18);

    // =====================================================================
    // ROW 2
    // =====================================================================
    int row2Y = row1Y + row1H + GAP;
    int row2H = usableH - row2Y - PAD;
    int row2ControlY = row2Y + HDR + 20;

    int oscW = 3 * (K_SZ + GAP) + 12;
    oscGroup.setBounds(PAD, row2Y, oscW, row2H);

    int oscX = PAD + 6;
    coarseKnob.setBounds(oscX, row2ControlY, K_SZ, K_H);
    fineKnob.setBounds(oscX + (K_SZ + GAP), row2ControlY, K_SZ, K_H);
    waveKnob.setBounds(oscX + 2 * (K_SZ + GAP), row2ControlY, K_SZ, K_H);

    int lfoX = PAD + oscW + GAP;
    int lfoW = getWidth() - lfoX - PAD;
    lfoGroup.setBounds(lfoX, row2Y, lfoW, row2H);

    int lfoControlX = lfoX + 12;
    octaveKnob.setBounds(lfoControlX, row2ControlY, K_SZ, K_H);
    fineTuneKnob.setBounds(lfoControlX + (K_SZ + GAP), row2ControlY, K_SZ, K_H);
    vibratoKnob.setBounds(lfoControlX + 2 * (K_SZ + GAP), row2ControlY, K_SZ, K_H);
    lfoRateKnob.setBounds(lfoControlX + 3 * (K_SZ + GAP), row2ControlY, K_SZ, K_H);
    mwToVibKnob.setBounds(lfoControlX + 4 * (K_SZ + GAP), row2ControlY, K_SZ, K_H);
}