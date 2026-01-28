/*
  ==============================================================================

    MyLookAndFeel.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class MyLookAndFeel : public juce::LookAndFeel_V4 {
public:
  MyLookAndFeel() {
    setColour(juce::Slider::thumbColourId, juce::Colours::cyan);
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::cyan);
    setColour(juce::Slider::rotarySliderOutlineColourId,
              juce::Colours::darkgrey);
    setColour(juce::Slider::trackColourId, juce::Colours::darkgrey);
    setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    setColour(juce::ComboBox::textColourId,
              juce::Colour::fromString("FF191919"));
  }

  void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPos, const float rotaryStartAngle,
                        const float rotaryEndAngle,
                        juce::Slider &slider) override {
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto angle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Filled Circle (Background)
    g.setColour(juce::Colour::fromString("FFced0ce"));
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f,
                  radius * 2.0f);

    // Track (Background Ring) - Not strictly visible if same color, but kept
    // for structure or if alpha changed Actually, if we fill the circle, we
    // might not need the track arc if it's the same color. But let's keep it or
    // just rely on the fill. If the user wants "claro por dentro", filling the
    // circle achieves that.
    juce::Path trackArc;
    trackArc.addCentredArc(centreX, centreY, radius * 0.85f, radius * 0.85f,
                           0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour::fromString("FFced0ce"));
    g.strokePath(trackArc, juce::PathStrokeType(4.0f));

    // Progress Arc (Value)
    juce::Path progressArc;
    progressArc.addCentredArc(centreX, centreY, radius * 0.85f, radius * 0.85f,
                              0.0f, rotaryStartAngle, angle, true);
    g.setColour(juce::Colour::fromString("FF191919"));
    g.strokePath(progressArc, juce::PathStrokeType(4.0f));

    // // Pointer
    // juce::Path p;
    // auto pointerLength = radius * 0.33f;
    // auto pointerThickness = 2.0f;
    // p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness,
    //                pointerLength);
    // p.applyTransform(
    //     juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    // g.setColour(juce::Colour::fromString("FF191919"));
    // g.fillPath(p);
  }

  void drawButtonBackground(juce::Graphics &g, juce::Button &button,
                            const juce::Colour &backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override {
    auto cornerSize = 6.0f;
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

    auto baseColour =
        backgroundColour
            .withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f
                                                                    : 0.9f)
            .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
      baseColour =
          baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, cornerSize);
  }

  void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPos, float minSliderPos, float maxSliderPos,
                        const juce::Slider::SliderStyle style,
                        juce::Slider &slider) override {
    if (style == juce::Slider::LinearVertical) {
      auto trackWidth = 6.0f;
      auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width,
                                           (float)height);
      auto trackRect =
          bounds.withSizeKeepingCentre(trackWidth, bounds.getHeight());

      // Draw track background (darkery grey/black)
      g.setColour(juce::Colours::black);
      g.fillRoundedRectangle(trackRect, trackWidth * 0.5f);

      // Calculate fill height
      auto valueRatio = (float)((slider.getValue() - slider.getMinimum()) /
                                (slider.getMaximum() - slider.getMinimum()));

      auto barHeight = bounds.getHeight() * valueRatio;
      auto barRect = trackRect.removeFromBottom(barHeight);

      g.setColour(juce::Colour::fromString("FFced0ce"));
      g.fillRoundedRectangle(barRect, trackWidth * 0.5f);
    } else {
      // Fallback for other linear styles if any (none used yet)
      juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos,
                                             minSliderPos, maxSliderPos, style,
                                             slider);
    }
  }

  void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown,
                    int buttonX, int buttonY, int buttonW, int buttonH,
                    juce::ComboBox &box) override {
    auto cornerSize = 5.0f;
    juce::Rectangle<int> boxBounds(0, 0, width, height);

    g.setColour(juce::Colour::fromString("FFced0ce"));
    g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

    // Arrow
    juce::Path path;
    path.startNewSubPath((float)width - 30.0f, (float)height * 0.5f - 2.0f);
    path.lineTo((float)width - 25.0f, (float)height * 0.5f + 3.0f);
    path.lineTo((float)width - 20.0f, (float)height * 0.5f - 2.0f);

    g.setColour(juce::Colour::fromString("FF191919"));
    g.strokePath(path, juce::PathStrokeType(2.0f));
  }

  void positionComboBoxText(juce::ComboBox &box, juce::Label &label) override {
    label.setBounds(0, 0, box.getWidth() - 30, box.getHeight());
    label.setJustificationType(juce::Justification::centred);
  }
};
