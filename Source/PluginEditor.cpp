#include "PluginEditor.h"
#include "PluginProcessor.h"

MySynthAudioProcessorEditor::MySynthAudioProcessorEditor(
    MySynthAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setSize(600, 600);
  setLookAndFeel(&myLookAndFeel);

  startTimerHz(60); // Start repainting at 60fps for visualizer

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

  // Low Note Slider
  lowNoteSlider.setSliderStyle(
      juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
  lowNoteSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
  addAndMakeVisible(lowNoteSlider);

  lowNoteAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "lowNote", lowNoteSlider);

  // High Note Slider
  highNoteSlider.setSliderStyle(
      juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
  highNoteSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
  addAndMakeVisible(highNoteSlider);

  highNoteAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "highNote", highNoteSlider);
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

  // Filter Area (Bottom)
  // We now have 4 sliders in the bottom row: Cutoff, Res, Low, High
  auto filterArea = area.removeFromTop(200);
  const auto filterSliderWidth = filterArea.getWidth() / 4;

  auto cutoffRect = filterArea.removeFromLeft(filterSliderWidth);
  auto resRect = filterArea.removeFromLeft(filterSliderWidth);
  auto lowRect = filterArea.removeFromLeft(filterSliderWidth);
  auto highRect = filterArea.removeFromLeft(filterSliderWidth);

  g.drawFittedText("Cutoff", cutoffRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("Resonance", resRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("Low Limit", lowRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("High Limit", highRect.removeFromTop(20),
                   juce::Justification::centred, 1);

  // Draw Modifier Status
  g.setColour(juce::Colours::yellow);
  g.setFont(20.0f);
  auto modArea = area.removeFromBottom(40).reduced(10, 0);

  juce::String statusText = "";

  if (audioProcessor.isDimPressedVal())
    statusText += "Diminished ";
  else if (audioProcessor.isMinPressedVal())
    statusText += "Minor ";
  else if (audioProcessor.isMajPressedVal())
    statusText += "Major ";
  else if (audioProcessor.isSus2PressedVal())
    statusText += "Sus2 ";

  if (audioProcessor.is6PressedVal())
    statusText += "6 ";
  else if (audioProcessor.isMin7PressedVal())
    statusText += "m7 ";
  else if (audioProcessor.isMaj7PressedVal())
    statusText += "M7 ";
  else if (audioProcessor.is9PressedVal())
    statusText += "9";

  if (statusText.isEmpty())
    statusText = "No Modifiers";

  g.drawFittedText(statusText, modArea, juce::Justification::centred, 1);
}

void MySynthAudioProcessorEditor::timerCallback() { repaint(); }

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
  topArea.removeFromLeft(80); // Skip label area

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
  // We now have 4 sliders in the bottom row: Cutoff, Res, Low, High
  auto filterArea = area.removeFromTop(200);
  const auto filterSliderWidth = filterArea.getWidth() / 4;

  // Cutoff
  auto cutoffArea = filterArea.removeFromLeft(filterSliderWidth);
  cutoffArea.removeFromTop(20);
  cutoffSlider.setBounds(cutoffArea.reduced(padding));

  // Resonance
  auto resArea = filterArea.removeFromLeft(filterSliderWidth);
  resArea.removeFromTop(20);
  resSlider.setBounds(resArea.reduced(padding));

  // Low Note
  auto lowArea = filterArea.removeFromLeft(filterSliderWidth);
  lowArea.removeFromTop(20);
  lowNoteSlider.setBounds(lowArea.reduced(padding));

  // High Note
  auto highArea = filterArea.removeFromLeft(filterSliderWidth);
  highArea.removeFromTop(20);
  highNoteSlider.setBounds(highArea.reduced(padding));
}
