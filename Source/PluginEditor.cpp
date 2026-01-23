#include "PluginEditor.h"
#include "PluginProcessor.h"

MySynthAudioProcessorEditor::MySynthAudioProcessorEditor(
    MySynthAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setSize(600, 450);
  setLookAndFeel(&myLookAndFeel);

  // Attack Slider
  attackSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 25);
  addAndMakeVisible(attackSlider);

  attackAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "attack", attackSlider);

  // Decay Slider
  decaySlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 25);
  addAndMakeVisible(decaySlider);

  decayAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "decay", decaySlider);

  // Sustain Slider
  sustainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  sustainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 25);
  addAndMakeVisible(sustainSlider);

  sustainAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "sustain", sustainSlider);

  // Release Slider
  releaseSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 25);
  addAndMakeVisible(releaseSlider);

  releaseAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "release", releaseSlider);

  // Oscillator Selector
  oscSelector.addItem("Sine", 1);
  oscSelector.addItem("Saw", 2);
  oscSelector.addItem("Square", 3);
  addAndMakeVisible(oscSelector);

  std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
      audioProcessor.apvts, "oscType", oscSelector);

  // Chord Mode Toggle
  addAndMakeVisible(chordModeToggle);
  chordModeAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "chordMode", chordModeToggle);

  // Cutoff Slider
  cutoffSlider.setSliderStyle(
      juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
  cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
  addAndMakeVisible(cutoffSlider);

  cutoffAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "cutoff", cutoffSlider);

  // Resonance Slider
  resSlider.setSliderStyle(
      juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
  resSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
  addAndMakeVisible(resSlider);

  resAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "resonance", resSlider);
}

MySynthAudioProcessorEditor::~MySynthAudioProcessorEditor() {
  setLookAndFeel(nullptr);
}

void MySynthAudioProcessorEditor::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::black);

  g.setColour(juce::Colours::white);
  g.setFont(15.0f);

  auto area = getLocalBounds().reduced(10);
  auto topArea = area.removeFromTop(40);
  auto labelArea = topArea.removeFromLeft(80);

  g.drawFittedText("Oscillator", labelArea, juce::Justification::centred, 1);

  auto sliderArea = area.removeFromTop(200);
  const auto sliderWidth = sliderArea.getWidth() / 4;

  auto attackRect = sliderArea.removeFromLeft(sliderWidth);
  auto decayRect = sliderArea.removeFromLeft(sliderWidth);
  auto sustainRect = sliderArea.removeFromLeft(sliderWidth);
  auto releaseRect = sliderArea.removeFromLeft(sliderWidth);

  g.drawFittedText("Attack", attackRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("Decay", decayRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("Sustain", sustainRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("Release", releaseRect.removeFromTop(20),
                   juce::Justification::centred, 1);

  auto filterArea = area;
  const auto filterSliderWidth = filterArea.getWidth() / 2;
  auto cutoffRect = filterArea.removeFromLeft(filterSliderWidth);
  auto resRect = filterArea.removeFromLeft(filterSliderWidth);

  g.drawFittedText("Cutoff", cutoffRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("Resonance", resRect.removeFromTop(20),
                   juce::Justification::centred, 1);
}

void MySynthAudioProcessorEditor::resized() {
  const auto bounds = getLocalBounds().reduced(10);
  const auto padding = 10;
  // 4 ADSR sliders + 2 Filter sliders = 6 sliders.
  // Layout: One row for ADSR, one row for Filter? Or all in one row?
  // Let's do:
  // Top: Osc Selector
  // Mid: ADSR (Attack, Decay, Sustain, Release)
  // Bottom: Filter (Cutoff, Resonance)

  auto area = bounds;

  // Oscillator selector at the top
  auto topArea = area.removeFromTop(40);
  auto labelArea = topArea.removeFromLeft(80); // Skip label area

  // Split top area for Osc and Toggle
  oscSelector.setBounds(topArea.removeFromLeft(100).reduced(0, 5));

  // Toggle Button
  chordModeToggle.setBounds(topArea.removeFromLeft(100).reduced(0, 5));

  // Sliders area
  auto sliderArea = area.removeFromTop(200);
  const auto sliderWidth = sliderArea.getWidth() / 4;

  // Attack
  auto attackArea = sliderArea.removeFromLeft(sliderWidth);
  attackArea.removeFromTop(20); // Label space
  attackSlider.setBounds(attackArea.reduced(padding));

  // Decay
  auto decayArea = sliderArea.removeFromLeft(sliderWidth);
  decayArea.removeFromTop(20);
  decaySlider.setBounds(decayArea.reduced(padding));

  // Sustain
  auto sustainArea = sliderArea.removeFromLeft(sliderWidth);
  sustainArea.removeFromTop(20);
  sustainSlider.setBounds(sustainArea.reduced(padding));

  // Release
  auto releaseArea = sliderArea.removeFromLeft(sliderWidth);
  releaseArea.removeFromTop(20);
  releaseSlider.setBounds(releaseArea.reduced(padding));

  // Filter Area (Bottom)
  auto filterArea = area; // Remaining space
  const auto filterSliderWidth = filterArea.getWidth() / 2;

  // Cutoff
  auto cutoffArea = filterArea.removeFromLeft(filterSliderWidth);
  cutoffArea.removeFromTop(20);
  cutoffSlider.setBounds(cutoffArea.reduced(padding));

  // Resonance
  auto resArea = filterArea.removeFromLeft(filterSliderWidth);
  resArea.removeFromTop(20);
  resSlider.setBounds(resArea.reduced(padding));
}
