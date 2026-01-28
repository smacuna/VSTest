#pragma once

#include <JuceHeader.h>

class SynthSound : public juce::SynthesiserSound {
public:
  bool appliesToNote(int midiNoteNumber) override { return true; }
  bool appliesToChannel(int midiChannel) override { return true; }
};

class SynthVoice : public juce::SynthesiserVoice {
public:
  SynthVoice();

  bool canPlaySound(juce::SynthesiserSound *sound) override;

  void startNote(int midiNoteNumber, float velocity,
                 juce::SynthesiserSound *sound,
                 int currentPitchWheelPosition) override;
  void stopNote(float velocity, bool allowTailOff) override;
  void controllerMoved(int controllerNumber, int newControllerValue) override;
  void pitchWheelMoved(int newPitchWheelValue) override;
  void prepareToPlay(double sampleRate, int samplesPerBlock,
                     int outputChannels);
  void renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample,
                       int numSamples) override;

  // Methods to update parameters safely
  void updateParameters(float attack, float decay, float sustain, float release,
                        float oscAType, float oscBRange, float oscBLevel,
                        float oscBEnabled, float cutoff, float resonance,
                        float oscRange, float oscLevel, float oscEnabled,
                        float oscBType);

private:
  juce::dsp::Oscillator<float> oscillatorA;
  juce::dsp::Oscillator<float> oscillatorB;
  juce::dsp::Gain<float> gain;
  juce::ADSR adsr;
  juce::dsp::StateVariableTPTFilter<float> filter;
  juce::AudioBuffer<float> tempBuffer;
  juce::AudioBuffer<float> oscBBuffer;

  juce::dsp::ProcessSpec spec;

  // Waveform selection
  enum class OscType { Sine = 0, Saw, Square };
  OscType currentOscAType{OscType::Sine};
  OscType currentOscBType{OscType::Sine};

  // Oscillator Controls
  float frequencyMultiplierA{1.0f};
  float levelA{1.0f};
  bool isEnabledA{true};

  float frequencyMultiplierB{1.0f};
  float levelB{1.0f};
  bool isEnabledB{false};

  // Helpers
  void setOscillatorType(juce::dsp::Oscillator<float> &osc, OscType type);
};
