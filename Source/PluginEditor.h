#pragma once

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
  juce::Slider releaseSlider;
  juce::ComboBox oscSelector;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      attackAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      releaseAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
      oscAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MySynthAudioProcessorEditor)
};
