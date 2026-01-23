#pragma once

#include "MyLookAndFeel.h"
#include "PluginProcessor.h"
#include <JuceHeader.h>

class MySynthAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
  MySynthAudioProcessorEditor(MySynthAudioProcessor &);
  ~MySynthAudioProcessorEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  MySynthAudioProcessor &audioProcessor;

  juce::Slider attackSlider;
  juce::Slider decaySlider;
  juce::Slider sustainSlider;
  juce::Slider releaseSlider;
  juce::ComboBox oscSelector;
  juce::Slider cutoffSlider;
  juce::Slider resSlider;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      attackAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      decayAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      sustainAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      releaseAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
      oscAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      cutoffAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      resAttachment;

  juce::ToggleButton chordModeToggle{"Chord Mode"};
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      chordModeAttachment;

  MyLookAndFeel myLookAndFeel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MySynthAudioProcessorEditor)
};
