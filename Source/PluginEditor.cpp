#include "PluginEditor.h"
#include "PluginProcessor.h"

MySynthAudioProcessorEditor::MySynthAudioProcessorEditor(
    MySynthAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setSize(400, 300);

  // Attack Slider
  attackSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
  addAndMakeVisible(attackSlider);

  attackAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "attack", attackSlider);

  // Release Slider
  releaseSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
  addAndMakeVisible(releaseSlider);

  releaseAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "release", releaseSlider);

  // Oscillator Selector
  oscSelector.addItem("Sine", 1);
  oscSelector.addItem("Saw", 2);
  oscSelector.addItem("Square", 3);
  addAndMakeVisible(oscSelector);

  oscAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
          audioProcessor.apvts, "oscType", oscSelector);
}

MySynthAudioProcessorEditor::~MySynthAudioProcessorEditor() {}

void MySynthAudioProcessorEditor::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::black);

  g.setColour(juce::Colours::white);
  g.setFont(15.0f);

  auto area = getLocalBounds().reduced(10);
  auto topArea = area.removeFromTop(40);
  auto labelArea = topArea.removeFromLeft(80);

  g.drawFittedText("Oscillator", labelArea, juce::Justification::centredRight,
                   1);

  auto sliderArea = area.removeFromTop(200);
  auto attackRect = sliderArea.removeFromLeft(100);
  auto releaseRect = sliderArea.removeFromLeft(100);

  g.drawFittedText("Attack", attackRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("Release", releaseRect.removeFromTop(20),
                   juce::Justification::centred, 1);
}

void MySynthAudioProcessorEditor::resized() {
  const auto bounds = getLocalBounds().reduced(10);
  const auto padding = 10;
  const auto sliderWidth = 100;
  const auto sliderHeight = 200;

  auto area = bounds;

  // Oscillator selector at the top
  auto topArea = area.removeFromTop(40);
  topArea.removeFromLeft(80); // Skip label area
  oscSelector.setBounds(topArea.reduced(0, 5));

  // Sliders below
  auto sliderArea = area.removeFromTop(sliderHeight);

  // Attack
  auto attackArea = sliderArea.removeFromLeft(sliderWidth);
  attackArea.removeFromTop(20); // Space for label
  attackSlider.setBounds(attackArea.reduced(padding));

  // Release
  auto releaseArea = sliderArea.removeFromLeft(sliderWidth);
  releaseArea.removeFromTop(20); // Space for label
  releaseSlider.setBounds(releaseArea.reduced(padding));
}
