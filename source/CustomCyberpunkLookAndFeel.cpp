#include "CustomCyberpunkLookAndFeel.h"

CustomCyberpunkLookAndFeel::CustomCyberpunkLookAndFeel()
{
    setColour(juce::ComboBox::backgroundColourId, crtBlack);
    setColour(juce::ComboBox::outlineColourId, crtBorder);
    setColour(juce::ComboBox::textColourId, crtAmber);
    setColour(juce::ComboBox::arrowColourId, crtAmber);
    setColour(juce::PopupMenu::backgroundColourId, crtBlack);
    setColour(juce::PopupMenu::textColourId, crtAmber);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, crtAmberDim);
    setColour(juce::PopupMenu::highlightedTextColourId, crtBlack);
    setColour(juce::Label::textColourId, crtGreen);
    setColour(juce::Slider::textBoxTextColourId, crtGreen);
    setColour(juce::Slider::textBoxOutlineColourId, crtBorderDim);
    setColour(juce::ToggleButton::textColourId, crtGreen);
}

CustomCyberpunkLookAndFeel::~CustomCyberpunkLookAndFeel() {}

// ============================================================================
// Bloom helper: draw a thick transparent line under a bright thin line
// ============================================================================
void CustomCyberpunkLookAndFeel::drawGlowLine(juce::Graphics& g, juce::Line<float> line,
    juce::Colour c, float thickness)
{
    g.setColour(c.withAlpha(0.18f));
    g.drawLine(line, thickness * 3.0f);
    g.setColour(c.withAlpha(0.35f));
    g.drawLine(line, thickness * 1.8f);
    g.setColour(c);
    g.drawLine(line, thickness);
}

// ============================================================================
// ROTARY SLIDER — rendered as a terminal ARC METER (TUI style)
// ============================================================================
void CustomCyberpunkLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(3.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto centre = bounds.getCentre();
    auto cx = centre.x;
    auto cy = centre.y;

    // 1. Outer bezel: hard rectangle, no roundness
    g.setColour(crtBlack);
    g.fillRect(bounds);
    g.setColour(crtBorderDim);
    g.drawRect(bounds, 1.0f);

    // 2. Background arc (dim phosphor)
    juce::Path bgArc;
    bgArc.addCentredArc(cx, cy, radius - 4.0f, radius - 4.0f, 0.0f,
        rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(crtGreenDim.withAlpha(0.35f));
    g.strokePath(bgArc, juce::PathStrokeType(2.0f));

    // 3. Value arc with phosphor bloom
    if (sliderPos > 0.001f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(cx, cy, radius - 4.0f, radius - 4.0f, 0.0f,
            rotaryStartAngle, toAngle, true);

        // Bloom layers
        g.setColour(crtGreen.withAlpha(0.10f));
        g.strokePath(valueArc, juce::PathStrokeType(8.0f));
        g.setColour(crtGreen.withAlpha(0.25f));
        g.strokePath(valueArc, juce::PathStrokeType(5.0f));
        g.setColour(crtGreen);
        g.strokePath(valueArc, juce::PathStrokeType(2.0f));
    }

    // 4. Tick marks around the dial (0-10, like a 70s VU meter)
    g.setColour(crtAmberDim);
    for (int i = 0; i <= 10; ++i)
    {
        float a = rotaryStartAngle + (i / 10.0f) * (rotaryEndAngle - rotaryStartAngle);
        float r1 = radius - 1.0f;
        float r2 = radius - (i % 5 == 0 ? 6.0f : 3.0f);
        g.drawLine(cx + std::sin(a) * r1, cy - std::cos(a) * r1,
            cx + std::sin(a) * r2, cy - std::cos(a) * r2, 1.0f);
    }

    // 5. Needle (straight hard line, no curve)
    auto needleLen = radius - 8.0f;
    juce::Line<float> needle(
        cx, cy,
        cx + std::sin(toAngle) * needleLen,
        cy - std::cos(toAngle) * needleLen);

    // Amber needle with green bloom, super 70s
    drawGlowLine(g, needle, crtAmber, 1.5f);

    // 6. Center pivot: square, not circle (TUI aesthetic)
    g.setColour(crtAmber);
    g.fillRect(cx - 2.0f, cy - 2.0f, 4.0f, 4.0f);
    g.setColour(crtBlack);
    g.fillRect(cx - 1.0f, cy - 1.0f, 2.0f, 2.0f);
}

// ============================================================================
// LINEAR SLIDER — terminal BAR GRAPH style (used if we switch later)
// ============================================================================
void CustomCyberpunkLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float minSliderPos, float maxSliderPos,
    juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal && style != juce::Slider::LinearVertical)
    {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height,
            sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height);
    g.setColour(crtBlack);
    g.fillRect(bounds);
    g.setColour(crtBorderDim);
    g.drawRect(bounds, 1.0f);

    // Character cells: 8px wide blocks, like a terminal bar graph
    const int cellSize = 6;
    if (style == juce::Slider::LinearHorizontal)
    {
        int cells = width / cellSize;
        int lit = juce::jlimit(0, cells, (int)(sliderPos * cells / width));
        for (int i = 0; i < cells; ++i)
        {
            juce::Rectangle<float> cell((float)(x + i * cellSize + 1), (float)y + 2,
                (float)(cellSize - 2), (float)height - 4);
            if (i < lit)
            {
                g.setColour(crtGreen.withAlpha(0.2f));
                g.fillRect(cell.expanded(1.0f));
                g.setColour(crtGreen);
                g.fillRect(cell);
            }
            else
            {
                g.setColour(crtGreenDim.withAlpha(0.25f));
                g.fillRect(cell);
            }
        }
    }
    else // vertical
    {
        int cells = height / cellSize;
        int lit = juce::jlimit(0, cells, (int)(sliderPos * cells / height));
        for (int i = 0; i < cells; ++i)
        {
            juce::Rectangle<float> cell((float)x + 2, (float)(y + height - (i + 1) * cellSize + 1),
                (float)width - 4, (float)(cellSize - 2));
            if (i < lit)
            {
                g.setColour(crtGreen.withAlpha(0.2f));
                g.fillRect(cell.expanded(1.0f));
                g.setColour(crtGreen);
                g.fillRect(cell);
            }
            else
            {
                g.setColour(crtGreenDim.withAlpha(0.25f));
                g.fillRect(cell);
            }
        }
    }
}

// ============================================================================
// TOGGLE BUTTON — TUI checkbox: [X] or [ ] with amber label
// ============================================================================
void CustomCyberpunkLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    bool isOn = button.getToggleState();

    // Reserve 30px for the checkbox frame
    auto boxArea = bounds.removeFromLeft(30.0f).reduced(0.0f, 3.0f);

    // Box outline
    g.setColour(crtBorder);
    g.drawRect(boxArea, 1.0f);

    if (isOn)
    {
        // Bright X inside box
        g.setColour(crtGreen.withAlpha(0.25f));
        g.drawLine(boxArea.getX() + 4, boxArea.getY() + 4,
            boxArea.getRight() - 4, boxArea.getBottom() - 4, 3.0f);
        g.setColour(crtGreen);
        g.drawLine(boxArea.getX() + 4, boxArea.getY() + 4,
            boxArea.getRight() - 4, boxArea.getBottom() - 4, 1.5f);
        g.drawLine(boxArea.getRight() - 4, boxArea.getY() + 4,
            boxArea.getX() + 4, boxArea.getBottom() - 4, 1.5f);
    }
    else
    {
        // Dim dot placeholder
        g.setColour(crtGreenDim);
        g.drawLine(boxArea.getX() + 4, boxArea.getCentreY(),
            boxArea.getRight() - 4, boxArea.getCentreY(), 1.0f);
    }

    // Label
    auto textArea = bounds.reduced(6.0f, 0.0f);
    juce::Colour textCol = isOn ? crtGreen : crtAmber.withAlpha(0.65f);
    if (shouldDrawButtonAsHighlighted) textCol = textCol.brighter(0.3f);

    g.setColour(textCol);
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText(button.getButtonText().toUpperCase(), textArea,
        juce::Justification::centredLeft);
}

// ============================================================================
// GROUP COMPONENT — TUI box-drawing frame: ┌─[ TITLE ]────┐
// ============================================================================
void CustomCyberpunkLookAndFeel::drawGroupComponentOutline(juce::Graphics& g, int width, int height,
    const juce::String& text, const juce::Justification& position,
    juce::GroupComponent& group)
{
    juce::ignoreUnused(position, group);

    auto b = juce::Rectangle<float>(0.5f, 0.5f, (float)width - 1.0f, (float)height - 1.0f);

    // Fill: nearly black with faint green tint
    g.setColour(crtPanel);
    g.fillRect(b);

    // Hard 1px outer border
    g.setColour(crtBorder);
    g.drawRect(b, 1.0f);

    // Double-line inner border accent (2px inset)
    g.setColour(crtBorderDim);
    g.drawRect(b.reduced(2.0f), 1.0f);

    if (text.isNotEmpty())
    {
        // Title strip across the top
        juce::String upper = text.toUpperCase();
        juce::Font titleFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setFont(titleFont);

        // Measure text
        int textW = (int)std::ceil(titleFont.getStringWidthFloat(upper));
        int stripH = 16;
        int stripY = 5;

        // Black out a strip behind the title so it looks like a TUI label
        juce::Rectangle<int> strip(8, stripY - 1, textW + 10, stripH + 2);
        g.setColour(crtBlack);
        g.fillRect(strip);

        // Draw ─[ TITLE ]─ style framing
        g.setColour(crtGreen);
        g.setFont(titleFont);
        g.drawText("[ " + upper + " ]", strip, juce::Justification::centred);

        // Amber corner brackets (accent)
        g.setColour(crtAmber);
        auto tb = b.toNearestInt();
        const int cl = 6;
        // Top-left bracket
        g.drawLine((float)tb.getX() + 3, (float)tb.getY() + 3, (float)tb.getX() + 3 + cl, (float)tb.getY() + 3, 1.5f);
        g.drawLine((float)tb.getX() + 3, (float)tb.getY() + 3, (float)tb.getX() + 3, (float)tb.getY() + 3 + cl, 1.5f);
        // Top-right
        g.drawLine((float)tb.getRight() - 3, (float)tb.getY() + 3, (float)tb.getRight() - 3 - cl, (float)tb.getY() + 3, 1.5f);
        g.drawLine((float)tb.getRight() - 3, (float)tb.getY() + 3, (float)tb.getRight() - 3, (float)tb.getY() + 3 + cl, 1.5f);
        // Bottom-left
        g.drawLine((float)tb.getX() + 3, (float)tb.getBottom() - 3, (float)tb.getX() + 3 + cl, (float)tb.getBottom() - 3, 1.5f);
        g.drawLine((float)tb.getX() + 3, (float)tb.getBottom() - 3, (float)tb.getX() + 3, (float)tb.getBottom() - 3 - cl, 1.5f);
        // Bottom-right
        g.drawLine((float)tb.getRight() - 3, (float)tb.getBottom() - 3, (float)tb.getRight() - 3 - cl, (float)tb.getBottom() - 3, 1.5f);
        g.drawLine((float)tb.getRight() - 3, (float)tb.getBottom() - 3, (float)tb.getRight() - 3, (float)tb.getBottom() - 3 - cl, 1.5f);
    }
}

// ============================================================================
// COMBO BOX — terminal input field with amber text, green block cursor
// ============================================================================
void CustomCyberpunkLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
    int buttonX, int buttonY, int buttonW, int buttonH,
    juce::ComboBox& comboBox)
{
    juce::ignoreUnused(buttonX, buttonY, buttonW, buttonH);

    auto b = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height);
    g.setColour(crtBlack);
    g.fillRect(b);

    // Border: brighter when focused/pressed
    g.setColour(isButtonDown ? crtAmber : crtBorder);
    g.drawRect(b, 1.0f);
    g.setColour(crtBorderDim);
    g.drawRect(b.reduced(2.0f), 1.0f);

    // Down-arrow: classic ASCII `v` shape (two lines)
    float ax = (float)width - 16.0f;
    float ay = (float)height * 0.5f;
    g.setColour(crtAmber);
    g.drawLine(ax - 4, ay - 2, ax, ay + 2, 1.5f);
    g.drawLine(ax, ay + 2, ax + 4, ay - 2, 1.5f);

    // Text prefix prompt ">" like a terminal
    g.setColour(crtGreen);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText(">", 6, 0, 12, height, juce::Justification::centredLeft);

    // Combo's own text is drawn by the LookAndFeel's Label child, but
    // we set the label's font via the parent's label color above.
    juce::ignoreUnused(comboBox);
}

// ============================================================================
// POPUP MENU
// ============================================================================
void CustomCyberpunkLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.fillAll(crtBlack);
    g.setColour(crtBorder);
    g.drawRect(0, 0, width, height, 1);
    g.setColour(crtBorderDim);
    g.drawRect(2, 2, width - 4, height - 4, 1);
}

void CustomCyberpunkLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
    bool isSeparator, bool isActive, bool isHighlighted,
    bool isTicked, bool hasSubMenu, const juce::String& text,
    const juce::String& shortcutKeyText, const juce::Drawable* icon,
    const juce::Colour* textColourToUse)
{
    juce::ignoreUnused(shortcutKeyText, icon, textColourToUse);

    if (isSeparator)
    {
        g.setColour(crtBorderDim);
        g.fillRect(area.reduced(4, 0).withHeight(1));
        return;
    }

    if (isHighlighted && isActive)
    {
        g.setColour(crtAmberDim);
        g.fillRect(area);
        g.setColour(crtAmber);
        g.drawRect(area, 1.0f);
    }

    juce::Colour col = (isHighlighted && isActive) ? crtBlack : crtAmber;
    if (!isActive) col = crtAmberDim;

    // Tick mark (current program) — a bright `*` block
    if (isTicked)
    {
        g.setColour(crtGreen);
        g.fillRect(area.getX() + 4, area.getCentreY() - 3, 6, 6);
    }

    g.setColour(col);
    g.setFont(juce::FontOptions(12.0f));
    g.drawText(text, area.reduced(16, 0), juce::Justification::centredLeft);
}

// ============================================================================
// LABEL — monospace caps, green param names, amber value readouts
// ============================================================================
void CustomCyberpunkLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;

        if (label.getName() == "ParamName")
        {
            g.setColour(crtGreen.withMultipliedAlpha(alpha));
            g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
            g.drawFittedText(label.getText().toUpperCase(),
                label.getLocalBounds(),
                juce::Justification::centred, 1);
        }
        else if (label.getName() == "ValueLabel")
        {
            // Amber readout, right-aligned like a terminal numeric display
            g.setColour(crtAmber.withMultipliedAlpha(alpha));
            g.setFont(juce::FontOptions(10.0f, juce::Font::plain));
            g.drawFittedText(label.getText(),
                label.getLocalBounds(),
                juce::Justification::centred, 1);
        }
        else
        {
            g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
            g.setFont(label.getFont());
            g.drawFittedText(label.getText(), label.getLocalBounds(),
                label.getJustificationType(), 1);
        }
    }
}