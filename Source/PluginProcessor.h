#pragma once

#include "SynthVoice.h"
#include <JuceHeader.h>

class MySynthAudioProcessor : public juce::AudioProcessor {
public:
  MySynthAudioProcessor();
  ~MySynthAudioProcessor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  // Parameter Layout
  static juce::AudioProcessorValueTreeState::ParameterLayout
  createParameterLayout();
  juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters",
                                           createParameterLayout()};

private:
  juce::Synthesiser synthesiser;

  // Cached pointers for fast access in processBlock
  std::atomic<float> *attackParam = nullptr;
  std::atomic<float> *releaseParam = nullptr;
  std::atomic<float> *oscTypeParam = nullptr;
  std::atomic<float> *decayParam = nullptr;
  std::atomic<float> *sustainParam = nullptr;
  std::atomic<float> *cutoffParam = nullptr;
  std::atomic<float> *resonanceParam = nullptr;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MySynthAudioProcessor)
};
