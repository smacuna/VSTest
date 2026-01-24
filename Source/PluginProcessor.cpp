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
  chordModeParam = apvts.getRawParameterValue("chordMode");
  lowNoteParam = apvts.getRawParameterValue("lowNote");
  highNoteParam = apvts.getRawParameterValue("highNote");
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

  layout.add(std::make_unique<juce::AudioParameterBool>("chordMode",
                                                        "Chord Mode", true));

  layout.add(std::make_unique<juce::AudioParameterInt>("lowNote", "Low Note", 0,
                                                       127, 60));
  layout.add(std::make_unique<juce::AudioParameterInt>("highNote", "High Note",
                                                       0, 127, 72));

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
  int lowLimit = static_cast<int>(lowNoteParam->load());
  int highLimit = static_cast<int>(highNoteParam->load());

  // Propagate parameters to voices
  for (int i = 0; i < synthesiser.getNumVoices(); ++i) {
    if (auto voice = dynamic_cast<SynthVoice *>(synthesiser.getVoice(i))) {
      voice->updateParameters(currentAttack, currentDecay, currentSustain,
                              currentRelease, currentOscType, currentCutoff,
                              currentResonance);
    }
  }

  // 1. Process MIDI for Chord Mode
  juce::MidiBuffer processedMidi;

  bool isChordModeOn = *chordModeParam > 0.5f;

  // Mode Switch Logic: If switching from OFF to ON, kill existing notes (with
  // release)
  if (isChordModeOn && !wasChordModeOn) {
    for (int i = 1; i <= 16; ++i)
      synthesiser.allNotesOff(i, true);

    activeChordNotes.clear();
  }
  wasChordModeOn = isChordModeOn;

  if (isChordModeOn) {
    for (const auto metadata : midiMessages) {
      auto message = metadata.getMessage();
      const auto noteNumber = message.getNoteNumber();
      const auto velocity = message.getFloatVelocity();

      // ---- HANDLER FOR MODIFIERS (Octave 4: 60-71) ----
      if (noteNumber >= 60 && noteNumber <= 71) {
        bool isNoteOn = message.isNoteOn();

        // Triads
        if (noteNumber == 61)
          isDimPressed = isNoteOn; // C#4
        if (noteNumber == 63)
          isMinPressed = isNoteOn; // D#4
        if (noteNumber == 66)
          isMajPressed = isNoteOn; // F#4
        if (noteNumber == 68)
          isSus2Pressed = isNoteOn; // G#4

        // Extensions
        if (noteNumber == 60)
          is6Pressed = isNoteOn; // C4
        if (noteNumber == 62)
          isMin7Pressed = isNoteOn; // D4
        if (noteNumber == 65)
          isMaj7Pressed = isNoteOn; // F4
        if (noteNumber == 67)
          is9Pressed = isNoteOn; // G4

        // Consume modifier keys (don't play them)
        continue;
      }

      // ---- HANDLER FOR TRIGGER KEYS (Octave 5: 72-83) ----
      // ---- HANDLER FOR TRIGGER KEYS (Octave 5: 72-83) ----
      if (noteNumber >= 72 && noteNumber <= 83) {
        if (message.isNoteOn()) {
          // GLITCH FIX: Check if this trigger is already active.
          // If so, force NoteOffs for the OLD, stored notes before starting new
          // ones.
          if (activeChordNotes.count(noteNumber)) {
            auto oldNotes = activeChordNotes[noteNumber];

            // Send NoteOffs for Old Notes (Root is included in oldNotes now)
            for (int oldNote : oldNotes) {
              if (oldNote <= 127) {
                processedMidi.addEvent(
                    juce::MidiMessage::noteOff(message.getChannel(), oldNote,
                                               velocity),
                    metadata.samplePosition);
              }
            }
            activeChordNotes.erase(noteNumber);
          }

          auto intervals = getNoteIntervals();

          if (!intervals.empty()) {
            std::vector<int> currentChordNotes;

            // Add Root (Fitted)
            int fittedRoot = fitNoteToRange(noteNumber, lowLimit, highLimit);
            if (fittedRoot <= 127) {
              processedMidi.addEvent(
                  juce::MidiMessage::noteOn(message.getChannel(), fittedRoot,
                                            velocity),
                  metadata.samplePosition);
              currentChordNotes.push_back(fittedRoot);
            }

            // Add Intervals
            for (int interval : intervals) {
              auto newNote = noteNumber + interval;
              newNote = fitNoteToRange(newNote, lowLimit, highLimit);

              if (newNote <= 127) {
                processedMidi.addEvent(
                    juce::MidiMessage::noteOn(message.getChannel(), newNote,
                                              velocity),
                    metadata.samplePosition);
                currentChordNotes.push_back(newNote);
              }
            }
            activeChordNotes[noteNumber] = currentChordNotes;
            continue; // Handled
          }
        } else if (message.isNoteOff()) {
          // Check if this note was triggered as a chord
          if (activeChordNotes.count(noteNumber)) {
            auto oldNotes = activeChordNotes[noteNumber];

            // Add Intervals Off (using stored notes - includes Root)
            for (int oldNote : oldNotes) {
              if (oldNote <= 127) {
                processedMidi.addEvent(
                    juce::MidiMessage::noteOff(message.getChannel(), oldNote,
                                               velocity),
                    metadata.samplePosition);
              }
            }
            activeChordNotes.erase(noteNumber);
            continue; // Handled
          }
        }
      }

      // Pass through other notes (and Octave 3 if no modifiers active)
      processedMidi.addEvent(message, metadata.samplePosition);
    }

    midiMessages.swapWith(processedMidi);
  }
  // If not Chord Mode, just pass through (midiMessages stays as is)

  // Render Audio
  synthesiser.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

// Helper to calculate intervals based on current modifier state
std::vector<int> MySynthAudioProcessor::getNoteIntervals() {
  std::vector<int> intervals;

  // 1. Determine Triad (Lowest Note Priority)
  // Priority: Dim (C#2) > Min (D#2) > Maj (F#2) > Sus2 (G#2)

  if (isDimPressed) {
    intervals.push_back(3); // Minor Third
    intervals.push_back(6); // Diminished Fifth (Tritone)
  } else if (isMinPressed) {
    intervals.push_back(3); // Minor Third
    intervals.push_back(7); // Perfect Fifth
  } else if (isMajPressed) {
    intervals.push_back(4); // Major Third
    intervals.push_back(7); // Perfect Fifth
  } else if (isSus2Pressed) {
    intervals.push_back(2); // Major Second
    intervals.push_back(7); // Perfect Fifth
  }

  // 2. Determine Extensions (Cumulative with strict priority)
  // "C4: 6, D4: Min7, F4: Maj7, G4: 9"

  if (is6Pressed) {           // C4 - Priority 1
    intervals.push_back(9);   // Major 6th
  } else if (isMin7Pressed) { // D4 - Priority 2
    intervals.push_back(10);  // Minor 7th
  } else if (isMaj7Pressed) { // F4 - Priority 3
    intervals.push_back(11);  // Major 7th
  } else if (is9Pressed) {    // G4 - Priority 4
    intervals.push_back(10);  // Minor 7th
    intervals.push_back(14);  // Major 9th
  }

  return intervals;
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
// Helper to fit note within specific MIDI range
int MySynthAudioProcessor::fitNoteToRange(int note, int low, int high) {
  if (low >= high)
    return note; // Safety

  int candidate = note;

  // 1. Shift up until >= low (if needed)
  if (candidate < low) {
    while (candidate < low) {
      candidate += 12;
    }
  } else if (candidate > high) {
    while (candidate > high) {
      candidate -= 12;
    }
  }

  return candidate;
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new MySynthAudioProcessor();
}
