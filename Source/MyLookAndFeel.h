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
  }

  void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPos, const float rotaryStartAngle,
                        const float rotaryEndAngle,
                        juce::Slider &slider) override {
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Fill
    g.setColour(juce::Colours::darkgrey);
    g.drawEllipse(rx, ry, rw, rw, 1.0f);

    juce::Path p;
    auto pointerLength = radius * 0.33f;
    auto pointerThickness = 2.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness,
                   pointerLength);
    p.applyTransform(
        juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    // Pointer
    g.setColour(juce::Colours::cyan);
    g.fillPath(p);

    // Arc
    juce::Path arc;
    arc.addCentredArc(centreX, centreY, radius * 0.8f, radius * 0.8f, 0.0f,
                      rotaryStartAngle, angle, true);
    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    g.strokePath(arc, juce::PathStrokeType(4.0f));
  }
};
