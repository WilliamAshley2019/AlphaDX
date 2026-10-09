#pragma once
#include <JuceHeader.h>

class CustomCyberpunkLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomCyberpunkLookAndFeel();
    ~CustomCyberpunkLookAndFeel() override;

    // ================================================================
    // PALETTE: P1 Phosphor (green) + P3 Phosphor (amber) terminal
    // ================================================================
    static inline const juce::Colour crtBlack = juce::Colour::fromRGB(0, 0, 0);
    static inline const juce::Colour crtGlass = juce::Colour::fromRGB(6, 10, 6);
    static inline const juce::Colour crtPanel = juce::Colour::fromRGB(4, 12, 4);
    static inline const juce::Colour crtBorderDim = juce::Colour::fromRGB(18, 60, 24);
    static inline const juce::Colour crtBorder = juce::Colour::fromRGB(40, 120, 50);
    static inline const juce::Colour crtGreen = juce::Colour::fromRGB(51, 255, 102);  // P1
    static inline const juce::Colour crtGreenDim = juce::Colour::fromRGB(20, 120, 40);
    static inline const juce::Colour crtAmber = juce::Colour::fromRGB(255, 176, 0);    // P3
    static inline const juce::Colour crtAmberDim = juce::Colour::fromRGB(120, 80, 0);
    static inline const juce::Colour crtWhite = juce::Colour::fromRGB(200, 255, 200);

    // Legacy aliases so existing code compiles unchanged
    static inline const juce::Colour cyberBlack = crtBlack;
    static inline const juce::Colour cyberDark = crtPanel;
    static inline const juce::Colour cyberPanel = crtPanel;
    static inline const juce::Colour cyberBorder = crtBorder;
    static inline const juce::Colour cyberWhite = crtWhite;
    static inline const juce::Colour cyberPhosphor = crtGreen;
    static inline const juce::Colour cyberPhosphorDim = crtGreenDim;

    // LookAndFeel overrides
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& slider) override;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawGroupComponentOutline(juce::Graphics& g, int width, int height,
        const juce::String& text, const juce::Justification& position,
        juce::GroupComponent& group) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
        int buttonX, int buttonY, int buttonW, int buttonH,
        juce::ComboBox& comboBox) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
        bool isSeparator, bool isActive, bool isHighlighted,
        bool isTicked, bool hasSubMenu, const juce::String& text,
        const juce::String& shortcutKeyText, const juce::Drawable* icon,
        const juce::Colour* textColourToUse) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    // Helper for drawing the CRT bloom
    static void drawGlowLine(juce::Graphics& g, juce::Line<float> line, juce::Colour c, float thickness);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomCyberpunkLookAndFeel)
};