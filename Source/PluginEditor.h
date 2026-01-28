#pragma once

#include "MyLookAndFeel.h"
#include "PluginProcessor.h"
#include <JuceHeader.h>

class MySynthAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::Timer {
public:
  MySynthAudioProcessorEditor(MySynthAudioProcessor &);
  ~MySynthAudioProcessorEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;
  void timerCallback() override;

private:
  MySynthAudioProcessor &audioProcessor;

  juce::Slider attackSlider;
  juce::Slider decaySlider;
  juce::Slider sustainSlider;
  juce::Slider releaseSlider;

  juce::Slider cutoffSlider;
  juce::Slider resSlider;
  juce::Slider lowNoteSlider;
  juce::Slider highNoteSlider;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      attackAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      decayAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      sustainAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      releaseAttachment;

  struct OscillatorUI {
    juce::TextButton enabledButton{"On"};
    juce::TextButton range16Button{"16"};
    juce::TextButton range8Button{"8"};
    juce::TextButton range4Button{"4"};
    juce::Slider levelSlider;
    juce::ComboBox typeSelector;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        enabledAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        levelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        typeAttachment;
  };

  OscillatorUI oscAUI;
  OscillatorUI oscBUI;

  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      cutoffAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      resAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      lowNoteAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      highNoteAttachment;

  juce::TextButton chordModeToggle{"Chord Mode"};
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      chordModeAttachment;

  // Modifier Buttons (Indicators)
  juce::TextButton dimButton{"Dim"};
  juce::TextButton minButton{"Min"};
  juce::TextButton majButton{"Maj"};
  juce::TextButton sus2Button{"Sus2"};

  juce::TextButton sixthButton{"6"};
  juce::TextButton min7Button{"m7"};
  juce::TextButton maj7Button{"M7"};
  juce::TextButton ninthButton{"9"};

  // Arpeggiator UI
  juce::TextButton arpEnabledButton{"Arp"};
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      arpEnabledAttachment;

  juce::ComboBox arpRateBox;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
      arpRateAttachment;
  juce::Label arpRateLabel;

  MyLookAndFeel myLookAndFeel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MySynthAudioProcessorEditor)
};
