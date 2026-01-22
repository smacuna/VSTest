#include "PluginProcessor.h"
#include "PluginEditor.h"

MySynthAudioProcessor::MySynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
{
  // Add voices
  for (int i = 0; i < 8; ++i)
    synthesiser.addVoice(new SynthVoice());

  // Add a sound (required for the synthesiser to work)
  synthesiser.addSound(new SynthSound());

  // Cache parameters
  attackParam = apvts.getRawParameterValue("attack");
  releaseParam = apvts.getRawParameterValue("release");
  oscTypeParam = apvts.getRawParameterValue("oscType");
  decayParam = apvts.getRawParameterValue("decay");
  sustainParam = apvts.getRawParameterValue("sustain");
  cutoffParam = apvts.getRawParameterValue("cutoff");
  resonanceParam = apvts.getRawParameterValue("resonance");
}

MySynthAudioProcessor::~MySynthAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
MySynthAudioProcessor::createParameterLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout layout;

  layout.add(std::make_unique<juce::AudioParameterFloat>("attack", "Attack",
                                                         0.01f, 1.0f, 0.1f));
  layout.add(std::make_unique<juce::AudioParameterFloat>("release", "Release",
                                                         0.1f, 3.0f, 0.4f));

  // ADSR: Decay & Sustain
  layout.add(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", 0.1f,
                                                         3.0f, 0.5f));
  layout.add(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain",
                                                         0.0f, 1.0f, 1.0f));

  // Filter: Cutoff & Resonance
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "cutoff", "Cutoff",
      juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.5f), 1000.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "resonance", "Resonance", 1.0f, 10.0f, 1.0f));

  juce::StringArray oscChoices;
  oscChoices.add("Sine");
  oscChoices.add("Saw");
  oscChoices.add("Square");

  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "oscType", "Oscillator Type", oscChoices, 0));

  return layout;
}

const juce::String MySynthAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool MySynthAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool MySynthAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool MySynthAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double MySynthAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int MySynthAudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int MySynthAudioProcessor::getCurrentProgram() { return 0; }

void MySynthAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String MySynthAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void MySynthAudioProcessor::changeProgramName(int index,
                                              const juce::String &newName) {
  juce::ignoreUnused(index, newName);
}

void MySynthAudioProcessor::prepareToPlay(double sampleRate,
                                          int samplesPerBlock) {
  synthesiser.setCurrentPlaybackSampleRate(sampleRate);

  for (int i = 0; i < synthesiser.getNumVoices(); ++i) {
    if (auto voice = dynamic_cast<SynthVoice *>(synthesiser.getVoice(i))) {
      voice->prepareToPlay(sampleRate, samplesPerBlock,
                           getTotalNumOutputChannels());
    }
  }
}

void MySynthAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MySynthAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void MySynthAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                         juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // Update parameters
  float currentAttack = attackParam->load();
  float currentRelease = releaseParam->load();
  float currentDecay = decayParam->load();
  float currentSustain = sustainParam->load();
  float currentOscType = oscTypeParam->load();
  float currentCutoff = cutoffParam->load();
  float currentResonance = resonanceParam->load();

  // Propagate parameters to voices
  for (int i = 0; i < synthesiser.getNumVoices(); ++i) {
    if (auto voice = dynamic_cast<SynthVoice *>(synthesiser.getVoice(i))) {
      voice->updateParameters(currentAttack, currentDecay, currentSustain,
                              currentRelease, currentOscType, currentCutoff,
                              currentResonance);
    }
  }

  // Render Audio
  synthesiser.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

bool MySynthAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor *MySynthAudioProcessor::createEditor() {
  return new MySynthAudioProcessorEditor(*this);
}

void MySynthAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
  auto state = apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void MySynthAudioProcessor::setStateInformation(const void *data,
                                                int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));

  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(apvts.state.getType()))
      apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// Creation function
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new MySynthAudioProcessor();
}
