#include "SynthVoice.h"

SynthVoice::SynthVoice() {
  // Initialize Oscillator with a default function (Sine)
  setOscillatorType(OscType::Sine);
}

bool SynthVoice::canPlaySound(juce::SynthesiserSound *sound) {
  return dynamic_cast<juce::SynthesiserSound *>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity,
                           juce::SynthesiserSound *sound,
                           int currentPitchWheelPosition) {
  juce::ignoreUnused(sound, currentPitchWheelPosition);

  oscillator.setFrequency(
      juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
  adsr.noteOn();
}

void SynthVoice::stopNote(float velocity, bool allowTailOff) {
  juce::ignoreUnused(velocity, allowTailOff);
  adsr.noteOff();

  if (!allowTailOff || !adsr.isActive())
    clearCurrentNote();
}

void SynthVoice::controllerMoved(int controllerNumber, int newControllerValue) {
  juce::ignoreUnused(controllerNumber, newControllerValue);
}

void SynthVoice::pitchWheelMoved(int newPitchWheelValue) {
  juce::ignoreUnused(newPitchWheelValue);
}

void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock,
                               int outputChannels) {
  spec.sampleRate = sampleRate;
  spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
  spec.numChannels = (juce::uint32)outputChannels;

  tempBuffer.setSize(outputChannels, samplesPerBlock);

  oscillator.prepare(spec);
  gain.prepare(spec);
  filter.prepare(spec);
  filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

  adsr.setSampleRate(sampleRate);

  gain.setGainLinear(0.3f); // Master volume for safety

  // Initial ADSR Config
  juce::ADSR::Parameters adsrParams;
  adsrParams.attack = 0.1f;
  adsrParams.decay = 0.1f;
  adsrParams.sustain = 1.0f;
  adsrParams.release = 0.4f;
  adsr.setParameters(adsrParams);
}

void SynthVoice::updateParameters(float attack, float decay, float sustain,
                                  float release, float oscType, float cutoff,
                                  float resonance) {
  // Update ADSR
  auto adsrParams = adsr.getParameters();
  adsrParams.attack = attack;
  adsrParams.decay = decay;
  adsrParams.sustain = sustain;
  adsrParams.release = release;
  adsr.setParameters(adsrParams);

  // Update Filter
  filter.setCutoffFrequency(cutoff);
  filter.setResonance(resonance);

  // Update Oscillator Type safely
  int typeIndex = static_cast<int>(oscType);
  if (typeIndex >= 0 && typeIndex <= 2) {
    if (static_cast<OscType>(typeIndex) != currentOscType) {
      setOscillatorType(static_cast<OscType>(typeIndex));
    }
  }
}

void SynthVoice::setOscillatorType(OscType type) {
  currentOscType = type;

  switch (currentOscType) {
  case OscType::Sine:
    oscillator.initialise([](float x) { return std::sin(x); });
    break;
  case OscType::Saw:
    // Naive Sawtooth: x is in [-pi, pi], so x/pi is [-1, 1]
    oscillator.initialise(
        [](float x) { return x / juce::MathConstants<float>::pi; });
    break;
  case OscType::Square:
    // Naive Square
    oscillator.initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; });
    break;
  }
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float> &outputBuffer,
                                 int startSample, int numSamples) {
  if (!isVoiceActive())
    return;

  // 1. Prepare temp buffer
  tempBuffer.clear();

  // Process the whole temp buffer from index 0, but only for numSamples
  juce::dsp::AudioBlock<float> audioBlock(tempBuffer);
  auto contextBlock = audioBlock.getSubBlock(0, (size_t)numSamples);

  // DSP Process Context on Temp Buffer
  juce::dsp::ProcessContextReplacing<float> context(contextBlock);

  // Process Chain
  oscillator.process(context);

  // Apply Filter
  filter.process(context);

  // Apply ADSR to the temp buffer
  adsr.applyEnvelopeToBuffer(tempBuffer, 0, numSamples);

  // Apply Master Gain
  gain.process(context);

  // 2. Mix temp buffer into main output buffer
  for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel) {
    outputBuffer.addFrom(channel, startSample, tempBuffer, channel, 0,
                         numSamples);
  }

  // Check if ADSR finished
  if (!adsr.isActive()) {
    clearCurrentNote();
  }
}
