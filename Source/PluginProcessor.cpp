#include "PluginProcessor.h"
#include "ChordNameUtils.h"
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
  cutoffParam = apvts.getRawParameterValue("cutoff");
  resonanceParam = apvts.getRawParameterValue("resonance");
  filterEnvParam = apvts.getRawParameterValue("filterEnv");
  filterEnabledParam = apvts.getRawParameterValue("filterEnabled");
  chordModeParam = apvts.getRawParameterValue("chordMode");
  retriggerParam = apvts.getRawParameterValue("retriggerMode");
  lowNoteParam = apvts.getRawParameterValue("lowNote");
  highNoteParam = apvts.getRawParameterValue("highNote");
  arpEnabledParam = apvts.getRawParameterValue("arpEnabled");
  arpRateParam = apvts.getRawParameterValue("arpRate");
  oscRangeParam = apvts.getRawParameterValue("oscRange");
  oscLevelParam = apvts.getRawParameterValue("oscLevel");
  oscEnabledParam = apvts.getRawParameterValue("oscEnabled");

  oscBTypeParam = apvts.getRawParameterValue("oscBType");
  oscBRangeParam = apvts.getRawParameterValue("oscBRange");
  oscBLevelParam = apvts.getRawParameterValue("oscBLevel");
  oscBEnabledParam = apvts.getRawParameterValue("oscBEnabled");

  apvts.addParameterListener("lowNote", this);
  apvts.addParameterListener("highNote", this);
  apvts.addParameterListener("arpSeed", this);

  generateArpPattern();
}

MySynthAudioProcessor::~MySynthAudioProcessor() {
  apvts.removeParameterListener("lowNote", this);
  apvts.removeParameterListener("highNote", this);
  apvts.removeParameterListener("arpSeed", this);
}

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
  auto cutoffRange = juce::NormalisableRange<float>(20.0f, 20000.0f);
  cutoffRange.setSkewForCentre(640.0f);

  layout.add(std::make_unique<juce::AudioParameterFloat>("cutoff", "Cutoff",
                                                         cutoffRange, 6730.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "resonance", "Resonance", 1.0f, 10.0f, 1.0f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "filterEnv", "Filter Env", 0.0f, 1.0f, 0.0f));

  layout.add(std::make_unique<juce::AudioParameterBool>(
      "filterEnabled", "Filter Enabled", true));

  juce::StringArray oscChoices;
  oscChoices.add("Sine");
  oscChoices.add("Saw");
  oscChoices.add("Square");

  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "oscType", "Oscillator Type", oscChoices, 2));

  juce::StringArray rangeChoices;
  rangeChoices.add("16");
  rangeChoices.add("8");
  rangeChoices.add("4");

  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "oscRange", "Oscillator Range", rangeChoices, 0)); // Default 16'

  layout.add(std::make_unique<juce::AudioParameterFloat>("oscLevel", "Level",
                                                         0.0f, 1.0f, 1.0f));

  layout.add(std::make_unique<juce::AudioParameterBool>(
      "oscEnabled", "Oscillator Enabled", true));

  // --- Osc B ---
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "oscBType", "Oscillator B Type", oscChoices, 1));

  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "oscBRange", "Oscillator B Range", rangeChoices, 1)); // Default 8'

  layout.add(std::make_unique<juce::AudioParameterFloat>("oscBLevel", "Level B",
                                                         0.0f, 1.0f, 1.0f));

  layout.add(std::make_unique<juce::AudioParameterBool>(
      "oscBEnabled", "Oscillator B Enabled", true));

  layout.add(std::make_unique<juce::AudioParameterBool>("chordMode",
                                                        "Chord Mode", true));

  layout.add(std::make_unique<juce::AudioParameterBool>("retriggerMode",
                                                        "Retrigger", false));

  layout.add(std::make_unique<juce::AudioParameterInt>(
      juce::ParameterID("lowNote", 1), "Low Limit", 24, 127, 48,
      juce::AudioParameterIntAttributes().withStringFromValueFunction(
          [](int value, int) {
            return juce::MidiMessage::getMidiNoteName(value, true, true, 3);
          }))); // Default C3

  layout.add(std::make_unique<juce::AudioParameterInt>(
      juce::ParameterID("highNote", 1), "High Limit", 24, 127, 84,
      juce::AudioParameterIntAttributes().withStringFromValueFunction(
          [](int value, int) {
            return juce::MidiMessage::getMidiNoteName(value, true, true, 3);
          }))); // Default C6

  layout.add(std::make_unique<juce::AudioParameterBool>("arpEnabled",
                                                        "Arpeggiator", false));

  juce::StringArray rateChoices;
  rateChoices.add("1/2");
  rateChoices.add("1/4");
  rateChoices.add("1/8");
  rateChoices.add("1/16");
  rateChoices.add("1/32");
  rateChoices.add("1/64");

  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "arpRate", "Arp Rate", rateChoices, 2)); // Default 1/8

  layout.add(std::make_unique<juce::AudioParameterInt>("arpSeed", "Arp Seed", 0,
                                                       10000, 12345));

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
  float currentOscRange = oscRangeParam->load();
  float currentOscLevel = oscLevelParam->load();
  float currentOscEnabled = oscEnabledParam->load();

  float currentOscBType = oscBTypeParam->load();
  float currentOscBRange = oscBRangeParam->load();
  float currentOscBLevel = oscBLevelParam->load();
  float currentOscBEnabled = oscBEnabledParam->load();

  // 1. Process MIDI for Chord Mode
  juce::MidiBuffer processedMidi;

  bool isChordModeOn = *chordModeParam > 0.5f;

  // Detect Range Changes
  int currentLow = static_cast<int>(lowNoteParam->load());
  int currentHigh = static_cast<int>(highNoteParam->load());

  bool rangeChanged = false;
  if (lastLowLimit != -1 &&
      (currentLow != lastLowLimit || currentHigh != lastHighLimit)) {
    rangeChanged = true;
  }
  lastLowLimit = currentLow;
  lastHighLimit = currentHigh;

  if (rangeChanged && !heldTriggerNotes.empty()) {
    // Re-evaluate the held note with new range (Smart Update)
    // IMPORTANT: Write to processedMidi, NOT midiMessages, to bypass the
    // input loop (modifier detection)

    bool shouldRetrigger = *retriggerParam > 0.5f;
    bool useSmartUpdate = !shouldRetrigger;

    playChord(heldTriggerNotes.back(), 1.0f, 0, processedMidi, useSmartUpdate);
  }

  // Propagate parameters to voices
  for (int i = 0; i < synthesiser.getNumVoices(); ++i) {
    if (auto voice = dynamic_cast<SynthVoice *>(synthesiser.getVoice(i))) {
      voice->updateParameters(
          currentAttack, currentDecay, currentSustain, currentRelease,
          currentOscType, currentOscBRange, currentOscBLevel,
          currentOscBEnabled, currentCutoff, currentResonance, currentOscRange,
          currentOscLevel, currentOscEnabled, currentOscBType);
    }
  }

  // Mode Switch Logic: If switching from OFF to ON, kill existing notes (with
  // release)
  if (isChordModeOn && !wasChordModeOn) {
    for (int i = 1; i <= 16; ++i)
      synthesiser.allNotesOff(i, true);

    sendVisualNoteEvent(-1, false); // All Notes Off logic
    activeChordNotes.clear();
    lastTriggeredNote = -1;
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
        bool modifierChanged = false;

        // Triads
        if (noteNumber == 61 && isDimPressed != isNoteOn) {
          isDimPressed = isNoteOn;
          modifierChanged = true;
        }
        if (noteNumber == 63 && isMinPressed != isNoteOn) {
          isMinPressed = isNoteOn;
          modifierChanged = true;
        }
        if (noteNumber == 66 && isMajPressed != isNoteOn) {
          isMajPressed = isNoteOn;
          modifierChanged = true;
        }
        if (noteNumber == 68 && isSus2Pressed != isNoteOn) {
          isSus2Pressed = isNoteOn;
          modifierChanged = true;
        }

        // Extensions
        if (noteNumber == 60 && is6Pressed != isNoteOn) {
          is6Pressed = isNoteOn;
          modifierChanged = true;
        }
        if (noteNumber == 62 && isMin7Pressed != isNoteOn) {
          isMin7Pressed = isNoteOn;
          modifierChanged = true;
        }
        if (noteNumber == 65 && isMaj7Pressed != isNoteOn) {
          isMaj7Pressed = isNoteOn;
          modifierChanged = true;
        }
        if (noteNumber == 67 && is9Pressed != isNoteOn) {
          is9Pressed = isNoteOn;
          modifierChanged = true;
        }

        // If a modifier changed and we have a chord playing, re-trigger it
        if (modifierChanged && !heldTriggerNotes.empty()) {
          // Kill current chord
          for (auto const &[rootNote, playedNotes] : activeChordNotes) {
            for (int noteToOff : playedNotes) {
              if (noteToOff <= 127) {
                processedMidi.addEvent(
                    juce::MidiMessage::noteOff(message.getChannel(), noteToOff,
                                               0.0f), // Force off
                    metadata.samplePosition);
                sendVisualNoteEvent(noteToOff, false);
              }
            }
          }
          activeChordNotes.clear();

          // Re-trigger last held note with new modifiers
          int lastNote = heldTriggerNotes.back();
          // Use processedMidi to skip re-processing this note as a modifier
          playChord(lastNote, 1.0f, metadata.samplePosition, processedMidi);
        }

        // Consume modifier keys (don't play them)
        continue;
      }

      // ---- HANDLER FOR TRIGGER KEYS (Octave 5: 72-83) ----
      if (noteNumber >= 72 && noteNumber <= 83) {
        if (message.isNoteOn()) {
          // 1. Manage Held Notes (Last Note Priority)
          heldTriggerNotes.erase(std::remove(heldTriggerNotes.begin(),
                                             heldTriggerNotes.end(),
                                             noteNumber),
                                 heldTriggerNotes.end());
          heldTriggerNotes.push_back(noteNumber);

          // 2. Kill ANY currently sounding chord (Monophonic behavior)
          for (auto const &[rootNote, playedNotes] : activeChordNotes) {
            for (int noteToOff : playedNotes) {
              if (noteToOff <= 127) {
                processedMidi.addEvent(
                    juce::MidiMessage::noteOff(message.getChannel(), noteToOff,
                                               0.0f),
                    metadata.samplePosition);
                sendVisualNoteEvent(noteToOff, false);
              }
            }
          }
          activeChordNotes.clear();

          // 3. Trigger the NEW note (Last pressed)
          playChord(noteNumber, velocity, metadata.samplePosition,
                    processedMidi);

          continue; // Handled
        } else if (message.isNoteOff()) {
          // 1. Remove from held list
          heldTriggerNotes.erase(std::remove(heldTriggerNotes.begin(),
                                             heldTriggerNotes.end(),
                                             noteNumber),
                                 heldTriggerNotes.end());

          // 2. If the released note is the one currently sounding...
          if (activeChordNotes.count(noteNumber)) {
            auto oldNotes = activeChordNotes[noteNumber];
            for (int oldNote : oldNotes) {
              if (oldNote <= 127) {
                processedMidi.addEvent(
                    juce::MidiMessage::noteOff(message.getChannel(), oldNote,
                                               velocity),
                    metadata.samplePosition);
                sendVisualNoteEvent(oldNote, false);
              }
            }
            activeChordNotes.erase(noteNumber);

            // 3. Retrigger the specific previous note if available
            if (!heldTriggerNotes.empty()) {
              int noteToRetrigger = heldTriggerNotes.back();
              playChord(noteToRetrigger, 1.0f, metadata.samplePosition,
                        processedMidi);
            } else {
              lastTriggeredNote = -1;
            }

            continue; // Handled
          }
        }
      }

      // Pass through other notes
      processedMidi.addEvent(message, metadata.samplePosition);
      if (message.isNoteOn())
        sendVisualNoteEvent(message.getNoteNumber(), true);
      else if (message.isNoteOff())
        sendVisualNoteEvent(message.getNoteNumber(), false);
    }

    midiMessages.swapWith(processedMidi);
  } else {
    // Pass-through mode: Scan for visual events
    for (const auto metadata : midiMessages) {
      auto msg = metadata.getMessage();
      if (msg.isNoteOn())
        sendVisualNoteEvent(msg.getNoteNumber(), true);
      else if (msg.isNoteOff())
        sendVisualNoteEvent(msg.getNoteNumber(), false);
    }
  }

  // 2. Process Arpeggiator
  processArpeggiator(midiMessages, buffer.getNumSamples());

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

void MySynthAudioProcessor::parameterChanged(const juce::String &parameterID,
                                             float newValue) {
  if (parameterID == "lowNote") {
    // If Low Note moves up, ensure High Note is at least 12 semitones above
    auto *highParam =
        dynamic_cast<juce::AudioParameterInt *>(apvts.getParameter("highNote"));
    int currentLow = static_cast<int>(newValue);
    int currentHigh = highParam->get();

    if (currentHigh < currentLow + 12) {
      // Push High Note up
      highParam->beginChangeGesture();
      highParam->setValueNotifyingHost(
          highParam->convertTo0to1(currentLow + 12));
      highParam->endChangeGesture();
    }
  } else if (parameterID == "highNote") {
    // If High Note moves down, ensure Low Note is at least 12 semitones
    // below
    auto *lowParam =
        dynamic_cast<juce::AudioParameterInt *>(apvts.getParameter("lowNote"));
    int currentHigh = static_cast<int>(newValue);
    int currentLow = lowParam->get();

    if (currentLow > currentHigh - 12) {
      // Push Low Note down
      lowParam->beginChangeGesture();
      lowParam->setValueNotifyingHost(
          lowParam->convertTo0to1(currentHigh - 12));
      lowParam->endChangeGesture();
    }
  } else if (parameterID == "arpSeed") {
    generateArpPattern();
  }
}

// Creation function
// Helper to fit note within specific MIDI range
int MySynthAudioProcessor::fitNoteToRange(int note, int low, int high) {
  if (low >= high)
    return note; // Safety

  int candidate = note;

  // Simple "while" to shift via octaves (12 semitones)
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

// Helper for display
juce::String MySynthAudioProcessor::getChordName() {
  if (lastTriggeredNote < 0)
    return "";

  return ChordNameUtils::getChordName(
      lastTriggeredNote, isDimPressedVal(), isMinPressedVal(),
      isMajPressedVal(), isSus2PressedVal(), is6PressedVal(),
      isMin7PressedVal(), isMaj7PressedVal(), is9PressedVal());
}

void MySynthAudioProcessor::processArpeggiator(juce::MidiBuffer &midiMessages,
                                               int numSamples) {
  bool isArpOn = *arpEnabledParam > 0.5f;

  // Simple clean up if Arp was just turned off
  if (!isArpOn) {
    if (currentArpNote != -1) {
      midiMessages.addEvent(juce::MidiMessage::noteOff(1, currentArpNote, 0.0f),
                            0);
      sendVisualNoteEvent(currentArpNote, false);
      currentArpNote = -1;
    }
    return;
  }

  // If no chord is active, stop arp
  if (activeChordNotes.empty()) {
    if (currentArpNote != -1) {
      midiMessages.addEvent(juce::MidiMessage::noteOff(1, currentArpNote, 0.0f),
                            0);
      sendVisualNoteEvent(currentArpNote, false);
      currentArpNote = -1;
    }
    arpSequenceStep = 0; // Reset sequence when no chord played
    return;
  }

  // Calculate Rate
  double bpm = 120.0;
  if (auto *ph = getPlayHead()) {
    if (auto position = ph->getPosition()) {
      if (position->getBpm().hasValue())
        bpm = *position->getBpm();
    }
  }

  int rateIndex = static_cast<int>(*arpRateParam);
  // validation
  if (rateIndex < 0)
    rateIndex = 0;
  if (rateIndex > 5)
    rateIndex = 5;

  // "1/2", "1/4", "1/8", "1/16", "1/32", "1/64"
  double denominator = std::pow(2.0, rateIndex + 1); // 2^1=2, 2^2=4, ...
  double beatsPerSec = bpm / 60.0;
  double samplesPerSec = getSampleRate();
  double samplesPerBeat = samplesPerSec / beatsPerSec;
  double samplesPerStep = samplesPerBeat * (4.0 / denominator);

  // Collect all valid notes from active chords
  std::vector<int> pool;
  for (auto const &[root, notes] : activeChordNotes) {
    for (int n : notes) {
      if (n >= 0 && n <= 127)
        pool.push_back(n);
    }
  }
  std::sort(pool.begin(), pool.end());

  if (pool.empty())
    return;

  // Run Arp Logic
  // Phase goes from 0 to samplesPerStep
  // In this block, we advance phase by numSamples.
  // If phase crosses threshold, trigger.

  double samplesRemainingInBlock = numSamples;
  int currentSampleOffset = 0;

  while (samplesRemainingInBlock > 0) {
    double samplesUntilNextTrigger = samplesPerStep - arpPhase;

    if (samplesUntilNextTrigger <= samplesRemainingInBlock) {
      // Trigger happens in this block
      int triggerOffset = currentSampleOffset + (int)samplesUntilNextTrigger;

      // 1. Note Off Previous
      if (currentArpNote != -1) {
        midiMessages.addEvent(
            juce::MidiMessage::noteOff(1, currentArpNote, 0.0f), triggerOffset);
        sendVisualNoteEvent(currentArpNote, false);
      }

      // 2. Pick New Note (Deterministic based on Seed/Pattern)
      int rawRandom = arpPattern[arpSequenceStep % arpPattern.size()];
      int randIndex = std::abs(rawRandom) % (int)pool.size();
      currentArpNote = pool[randIndex];

      arpSequenceStep++;

      // Visualization: Send band index (rank % 5) to Editor
      auto writer = visualFifo.write(1);
      if (writer.blockSize1 > 0)
        visualBuffer[(size_t)writer.startIndex1] = randIndex % 5;

      // 3. Note On New
      midiMessages.addEvent(juce::MidiMessage::noteOn(1, currentArpNote, 1.0f),
                            triggerOffset);
      sendVisualNoteEvent(currentArpNote, true);

      // Reset Phase
      arpPhase = 0;
      // Technically we should subtract the utilized samples
      double utilized = samplesUntilNextTrigger;
      currentSampleOffset += (int)utilized;
      samplesRemainingInBlock -= utilized;
    } else {
      // No trigger in rest of block
      arpPhase += samplesRemainingInBlock;
      samplesRemainingInBlock = 0;
    }
  }
}

void MySynthAudioProcessor::generateArpPattern() {
  int seedVal = 12345;
  if (auto *p = apvts.getParameter("arpSeed")) {
    // AudioParameterInt::get() returns int
    if (auto *intP = dynamic_cast<juce::AudioParameterInt *>(p)) {
      seedVal = intP->get();
    }
  }

  juce::Random rng(seedVal);
  arpPattern.resize(1024); // Large enough cycle
  for (int &val : arpPattern) {
    val = rng.nextInt();
  }
}

// Helper to trigger a chord
void MySynthAudioProcessor::playChord(int triggerNote, float velocity,
                                      int sampleOffset,
                                      juce::MidiBuffer &midiMessages,
                                      bool isSmartUpdate) {
  bool isArpOn = *arpEnabledParam > 0.5f;

  int lowLimit = static_cast<int>(lowNoteParam->load());
  int highLimit = static_cast<int>(highNoteParam->load());

  auto intervals = getNoteIntervals();
  std::vector<int> targetChordNotes;

  // Helper lambda to add all instances of a pitch class within range
  // Constrained to NEVER generate notes below the triggerNote to prevent Root
  // replacement
  auto addNotesInRange = [&](int baseNote) {
    int pitchClass = baseNote % 12;

    int candidate = lowLimit;
    int candidatePitchClass = candidate % 12;

    int diff = pitchClass - candidatePitchClass;
    if (diff < 0)
      diff += 12;

    candidate += diff;

    while (candidate <= highLimit) {
      if (candidate <= 127) {
        bool exists = false;

        for (int n : targetChordNotes) {
          if (n == candidate) {
            exists = true;
            break;
          }
        }
        if (!exists)
          targetChordNotes.push_back(candidate);
      }
      candidate += 12;
    }
  };

  // 1. Calculate Target Notes (Filling Strategy)

  // A. Fill Range with Root Octaves
  addNotesInRange(triggerNote);

  // B. Fill Range with Intervals
  if (!intervals.empty()) {
    for (int interval : intervals) {
      addNotesInRange(triggerNote + interval);
    }
  }

  // 2. Diffing or Direct Play
  std::vector<int> &currentNotes = activeChordNotes[triggerNote];

  if (isSmartUpdate) {
    // A. Stop notes that are in current but NOT in target
    for (int oldNote : currentNotes) {
      bool stillPlaying = false;
      for (int newNote : targetChordNotes) {
        if (newNote == oldNote) {
          stillPlaying = true;
          break;
        }
      }
      if (!stillPlaying) {
        midiMessages.addEvent(juce::MidiMessage::noteOff(1, oldNote, 0.0f),
                              sampleOffset);
        sendVisualNoteEvent(oldNote, false);
      }
    }

    // B. Start notes that are in target but NOT in current
    for (int newNote : targetChordNotes) {
      bool alreadyPlaying = false;
      for (int oldNote : currentNotes) {
        if (oldNote == newNote) {
          alreadyPlaying = true;
          break;
        }
      }
      if (!alreadyPlaying && !isArpOn) {
        midiMessages.addEvent(juce::MidiMessage::noteOn(1, newNote, velocity),
                              sampleOffset);
        sendVisualNoteEvent(newNote, true);
      }
    }

  } else {
    // Retrigger Behavior: Kill old notes, play new ones
    // 1. Stop ALL currently active notes for this trigger
    for (int oldNote : currentNotes) {
      midiMessages.addEvent(juce::MidiMessage::noteOff(1, oldNote, 0.0f),
                            sampleOffset);
      sendVisualNoteEvent(oldNote, false);
    }

    // 2. Play ALL target notes
    if (!isArpOn) {
      for (int note : targetChordNotes) {
        midiMessages.addEvent(juce::MidiMessage::noteOn(1, note, velocity),
                              sampleOffset);
        sendVisualNoteEvent(note, true);
      }
    }
  }

  // 3. Update State
  activeChordNotes[triggerNote] = targetChordNotes;
  lastTriggeredNote = triggerNote;
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new MySynthAudioProcessor();
}

void MySynthAudioProcessor::sendVisualNoteEvent(int note, bool on) {
  auto writer = noteFifo.write(1);
  if (writer.blockSize1 > 0) {
    noteEventBuffer[(size_t)writer.startIndex1] = {note, on};
  }
}
