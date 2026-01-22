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

  // Métodos para actualizar parámetros sin bloquear
  void updateParameters(float attack, float decay, float sustain, float release,
                        float oscType, float cutoff, float resonance);

private:
  juce::dsp::Oscillator<float> oscillator;
  juce::dsp::Gain<float> gain;
  juce::ADSR adsr;
  juce::dsp::StateVariableTPTFilter<float> filter;
  juce::AudioBuffer<float> tempBuffer;

  juce::dsp::ProcessSpec spec;

  // Waveform selection
  enum class OscType { Sine = 0, Saw, Square };
  OscType currentOscType{OscType::Sine};

  // Buffers internos para DSP si fueran necesarios (evitar alocación en
  // process) En este caso simple, dsp::Oscillator escribe directamente si
  // usamos AudioBlock correctamente
};
