#include "SynthVoice.h"

SynthVoice::SynthVoice() {
  // Initialize Oscillators with a default function (Sine)
  setOscillatorType(oscillatorA, OscType::Sine);
  setOscillatorType(oscillatorB, OscType::Sine);
}

bool SynthVoice::canPlaySound(juce::SynthesiserSound *sound) {
  return dynamic_cast<juce::SynthesiserSound *>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity,
                           juce::SynthesiserSound *sound,
                           int currentPitchWheelPosition) {
  juce::ignoreUnused(sound, currentPitchWheelPosition);

  auto hz = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);

  oscillatorA.setFrequency(hz * frequencyMultiplierA, true);
  oscillatorB.setFrequency(hz * frequencyMultiplierB, true);
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
  oscBBuffer.setSize(outputChannels, samplesPerBlock); // Aux buffer

  oscillatorA.prepare(spec);
  oscillatorB.prepare(spec);
  gain.prepare(spec); // Master gain (optional if we scale individually)
  filter.prepare(spec);
  filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

  adsr.setSampleRate(sampleRate);

  gain.setGainLinear(0.3f); // We'll update this in updateParameters anyway

  // Initial ADSR Config
  juce::ADSR::Parameters adsrParams;
  adsrParams.attack = 0.1f;
  adsrParams.decay = 0.1f;
  adsrParams.sustain = 1.0f;
  adsrParams.release = 0.4f;
  adsr.setParameters(adsrParams);
}

void SynthVoice::updateParameters(float attack, float decay, float sustain,
                                  float release, float oscAType,
                                  float oscBRange, float oscBLevel,
                                  float oscBEnabled, float cutoff,
                                  float resonance, float oscRange,
                                  float oscLevel, float oscEnabled,
                                  float oscBType) {
  // --- Oscillator A ---
  isEnabledA = oscEnabled > 0.5f;
  levelA = oscLevel;

  int rangeIndexA = static_cast<int>(oscRange);
  if (rangeIndexA == 0)
    frequencyMultiplierA = 0.5f; // 16'
  else if (rangeIndexA == 1)
    frequencyMultiplierA = 1.0f; // 8'
  else if (rangeIndexA == 2)
    frequencyMultiplierA = 2.0f; // 4'

  int typeIndexA = static_cast<int>(oscAType);
  if (typeIndexA >= 0 && typeIndexA <= 2) {
    if (static_cast<OscType>(typeIndexA) != currentOscAType) {
      currentOscAType = static_cast<OscType>(typeIndexA);
      setOscillatorType(oscillatorA, currentOscAType);
    }
  }

  // --- Oscillator B ---
  isEnabledB = oscBEnabled > 0.5f;
  levelB = oscBLevel;

  int rangeIndexB = static_cast<int>(oscBRange);
  if (rangeIndexB == 0)
    frequencyMultiplierB = 0.5f; // 16'
  else if (rangeIndexB == 1)
    frequencyMultiplierB = 1.0f; // 8'
  else if (rangeIndexB == 2)
    frequencyMultiplierB = 2.0f; // 4'

  int typeIndexB = static_cast<int>(oscBType);
  if (typeIndexB >= 0 && typeIndexB <= 2) {
    if (static_cast<OscType>(typeIndexB) != currentOscBType) {
      currentOscBType = static_cast<OscType>(typeIndexB);
      setOscillatorType(oscillatorB, currentOscBType);
    }
  }

  // --- Global ---
  // Update Master Gain to 1.0 generally, and control individual gains manually
  gain.setGainLinear(0.3f); // Global output scaling

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
}

void SynthVoice::setOscillatorType(juce::dsp::Oscillator<float> &osc,
                                   OscType type) {
  switch (type) {
  case OscType::Sine:
    osc.initialise([](float x) { return std::sin(x); });
    break;
  case OscType::Saw:
    // Naive Sawtooth
    osc.initialise([](float x) { return x / juce::MathConstants<float>::pi; });
    break;
  case OscType::Square:
    // Naive Square
    osc.initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; });
    break;
  }
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float> &outputBuffer,
                                 int startSample, int numSamples) {
  if (!isVoiceActive())
    return;

  // 1. Clear buffers to prepare for writing
  tempBuffer.clear();
  oscBBuffer.clear();

  // Create blocks for DSP
  juce::dsp::AudioBlock<float> blockA(tempBuffer);
  auto contextBlockA = blockA.getSubBlock(0, (size_t)numSamples);
  juce::dsp::ProcessContextReplacing<float> contextA(contextBlockA);

  juce::dsp::AudioBlock<float> blockB(oscBBuffer);
  auto contextBlockB = blockB.getSubBlock(0, (size_t)numSamples);
  juce::dsp::ProcessContextReplacing<float> contextB(contextBlockB);

  // 2. Process Oscillators
  if (isEnabledA) {
    oscillatorA.process(contextA);
    contextBlockA.multiplyBy(levelA);
  } else {
    contextBlockA.clear();
  }

  if (isEnabledB) {
    oscillatorB.process(contextB);
    contextBlockB.multiplyBy(levelB);
  } else {
    contextBlockB.clear();
  }

  // 3. Mix B into A (Result in tempBuffer)
  // We can just add valid samples from oscBBuffer to tempBuffer
  if (isEnabledB) {
    // Add oscBBuffer to tempBuffer for all channels
    for (size_t ch = 0; ch < spec.numChannels; ++ch) {
      tempBuffer.addFrom((int)ch, 0, oscBBuffer, (int)ch, 0, numSamples);
    }
  }

  // 4. Apply Filter (on the mixed signal in tempBuffer)
  filter.process(contextA);

  // 5. Apply ADSR
  adsr.applyEnvelopeToBuffer(tempBuffer, 0, numSamples);

  // 6. Apply Master Gain
  gain.process(contextA);

  // 7. Copy to Output
  for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel) {
    outputBuffer.addFrom(channel, startSample, tempBuffer, channel, 0,
                         numSamples);
  }

  // Check if ADSR finished
  if (!adsr.isActive()) {
    clearCurrentNote();
  }
}
