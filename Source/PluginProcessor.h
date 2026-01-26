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

  // Public getters for modifier state (for visualizer)
  bool isDimPressedVal() const { return isDimPressed; }
  bool isMinPressedVal() const { return isMinPressed; }
  bool isMajPressedVal() const { return isMajPressed; }
  bool isSus2PressedVal() const { return isSus2Pressed; }

  bool is6PressedVal() const { return is6Pressed; }
  bool isMin7PressedVal() const { return isMin7Pressed; }
  bool isMaj7PressedVal() const { return isMaj7Pressed; }

  bool is9PressedVal() const { return is9Pressed; }

  juce::String getChordName();

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
  std::atomic<float> *chordModeParam = nullptr;
  std::atomic<float> *lowNoteParam = nullptr;
  std::atomic<float> *highNoteParam = nullptr;

  // Helper to fit note within specific MIDI range (inversions)
  int fitNoteToRange(int note, int low, int high);

  // Chord Mode internal state
  // Triad Modifiers
  std::atomic<bool> isDimPressed{false};  // C#2
  std::atomic<bool> isMinPressed{false};  // D#2
  std::atomic<bool> isMajPressed{false};  // F#2
  std::atomic<bool> isSus2Pressed{false}; // G#2

  // Extension Modifiers
  std::atomic<bool> is6Pressed{false};    // C4
  std::atomic<bool> isMin7Pressed{false}; // D4
  std::atomic<bool> isMaj7Pressed{false}; // F4
  std::atomic<bool> is9Pressed{false};    // G4

  // Track previous mode state for transition handling
  bool wasChordModeOn = true; // Default to true to match default parameter

  // Last Triggered Note (Root) for Display
  std::atomic<int> lastTriggeredNote{-1};

  // Helper to calculate intervals based on current state
  std::vector<int> getNoteIntervals();

  // Track active chord notes (absolute MIDI numbers) to ensure correct NoteOffs
  // Key: Trigger Note Number, Value: Vector of actual played MIDI notes
  std::map<int, std::vector<int>> activeChordNotes;

  // Track physically held trigger keys to handle Last-Note Priority
  std::vector<int> heldTriggerNotes;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MySynthAudioProcessor)
};
