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

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MySynthAudioProcessorEditor)
};
