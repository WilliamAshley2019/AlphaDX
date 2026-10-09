#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomCyberpunkLookAndFeel.h"
#include <memory>

//==============================================================================
// AlphaKnob — same modular widget, now styled as a TUI arc meter
//==============================================================================
class AlphaKnob : public juce::Component
{
public:
    juce::Slider slider;
    juce::Label  nameLabel;
    juce::Label  valueLabel;

    AlphaKnob(const juce::String& titleText, CustomCyberpunkLookAndFeel& laf)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setDoubleClickReturnValue(true, 0.5);
        slider.setLookAndFeel(&laf);
        addAndMakeVisible(slider);

        nameLabel.setText(titleText, juce::dontSendNotification);
        nameLabel.setJustificationType(juce::Justification::centred);
        nameLabel.setName("ParamName");
        nameLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
        addAndMakeVisible(nameLabel);

        valueLabel.setText("000", juce::dontSendNotification);
        valueLabel.setJustificationType(juce::Justification::centred);
        valueLabel.setName("ValueLabel");
        valueLabel.setFont(juce::FontOptions(10.0f));
        addAndMakeVisible(valueLabel);

        slider.onValueChange = [this] {
            // Convert 0..1 to a 0..255 HEX-style readout, super terminal-y
            int v = juce::jlimit(0, 255, (int)std::round(slider.getValue() * 255.0));
            valueLabel.setText(juce::String::toHexString(v).paddedLeft('0', 2).toUpperCase(),
                juce::dontSendNotification);
            };
        // Initialize the readout
        slider.onValueChange();
    }

    ~AlphaKnob() override
    {
        slider.setLookAndFeel(nullptr);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        nameLabel.setBounds(b.removeFromTop(13));
        valueLabel.setBounds(b.removeFromBottom(13));
        slider.setBounds(b);
    }
};

//==============================================================================
// Main Editor
//==============================================================================
class DX7AudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::Timer
{
public:
    explicit DX7AudioProcessorEditor(DX7AudioProcessor&);
    ~DX7AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override; // for blinking cursor

    DX7AudioProcessor& audioProcessor;
    CustomCyberpunkLookAndFeel customLookAndFeel;

    // Header / Preset Selection
    juce::ComboBox presetSelector;

    // Section Group Frames
    juce::GroupComponent envGroup;
    juce::GroupComponent modEnvGroup;
    juce::GroupComponent oscGroup;
    juce::GroupComponent lfoGroup;
    juce::GroupComponent mixGroup;

    // Envelope Knobs
    AlphaKnob attackKnob{ "ATK",  customLookAndFeel };
    AlphaKnob decayKnob{ "DEC",   customLookAndFeel };
    AlphaKnob releaseKnob{ "REL", customLookAndFeel };

    // Mod Envelope Knobs
    AlphaKnob modInitKnob{ "INI",  customLookAndFeel };
    AlphaKnob modDecayKnob{ "DEC",  customLookAndFeel };
    AlphaKnob modSusKnob{ "SUS",  customLookAndFeel };
    AlphaKnob modRelKnob{ "REL",  customLookAndFeel };
    AlphaKnob modVelKnob{ "VEL",  customLookAndFeel };

    // Oscillator Knobs
    AlphaKnob coarseKnob{ "CRS", customLookAndFeel };
    AlphaKnob fineKnob{ "FIN", customLookAndFeel };
    AlphaKnob waveKnob{ "WAV", customLookAndFeel };

    // LFO / Pitch Knobs
    AlphaKnob octaveKnob{ "OCT", customLookAndFeel };
    AlphaKnob fineTuneKnob{ "TUN", customLookAndFeel };
    AlphaKnob vibratoKnob{ "VIB", customLookAndFeel };
    AlphaKnob lfoRateKnob{ "LFO", customLookAndFeel };
    AlphaKnob mwToVibKnob{ "M>V", customLookAndFeel };

    // Mix / Mod Thru Knob
    AlphaKnob modThruKnob{ "THR", customLookAndFeel };

    // CPU Saver Toggles
    juce::ToggleButton bypassModEnvToggle{ "MOD ENV" };
    juce::ToggleButton bypassLfoToggle{ "LFO" };
    juce::ToggleButton bypassModThruToggle{ "MOD THRU" };

    // APVTS Attachments
    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAtt> attAttack, attDecay, attRelease;
    std::unique_ptr<SliderAtt> attModInit, attModDecay, attModSus, attModRel, attModVel;
    std::unique_ptr<SliderAtt> attCoarse, attFine, attWave;
    std::unique_ptr<SliderAtt> attOctave, attFineTune, attVibrato, attLfoRate, attMwToVib;
    std::unique_ptr<SliderAtt> attModThru;

    std::unique_ptr<ButtonAtt> bypassModEnvAttachment;
    std::unique_ptr<ButtonAtt> bypassLfoAttachment;
    std::unique_ptr<ButtonAtt> bypassModThruAttachment;

    // Blink state
    bool cursorOn = true;
    int  blinkCounter = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DX7AudioProcessorEditor)
};