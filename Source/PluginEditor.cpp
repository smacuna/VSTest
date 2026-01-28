#include "PluginEditor.h"
#include "PluginProcessor.h"

MySynthAudioProcessorEditor::MySynthAudioProcessorEditor(
    MySynthAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setSize(540, 600);
  setLookAndFeel(&myLookAndFeel);

  startTimerHz(60); // Start repainting at 60fps for visualizer

  // Setup Toggle Helper
  auto setupToggleButton = [this](juce::TextButton &b) {
    b.setClickingTogglesState(true);
    b.setButtonText("");
    b.setColour(juce::TextButton::buttonOnColourId,
                juce::Colour::fromString("FFFF4500"));
    b.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    addAndMakeVisible(b);
  };

  // --- Helper for Oscillator UI Setup ---
  auto setupOscUI = [this, setupToggleButton](
                        OscillatorUI &ui, juce::String enabledId,
                        juce::String levelId, juce::String rangeId,
                        juce::String typeId) {
    // Enabled
    setupToggleButton(ui.enabledButton);
    ui.enabledAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            audioProcessor.apvts, enabledId, ui.enabledButton);

    // Level Slider
    ui.levelSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    ui.levelSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(ui.levelSlider);
    ui.levelAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, levelId, ui.levelSlider);

    // Range Buttons
    auto setupRangeButton = [this, rangeId](juce::TextButton &b, int index) {
      b.setButtonText(b.getButtonText()); // Keep text
      b.setClickingTogglesState(false);
      b.setColour(juce::TextButton::buttonColourId,
                  juce::Colour::fromString("FFced0ce"));
      b.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
      addAndMakeVisible(b);
      b.onClick = [this, rangeId, index]() {
        if (auto *p = audioProcessor.apvts.getParameter(rangeId))
          p->setValueNotifyingHost(p->convertTo0to1(index));
      };
    };
    setupRangeButton(ui.range16Button, 0);
    setupRangeButton(ui.range8Button, 1);
    setupRangeButton(ui.range4Button, 2);

    // Type Selector
    ui.typeSelector.addItem("Sine", 1);
    ui.typeSelector.addItem("Saw", 2);
    ui.typeSelector.addItem("Square", 3);
    addAndMakeVisible(ui.typeSelector);

    ui.typeAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, typeId, ui.typeSelector);
  };

  // Initialize Osc A
  setupOscUI(oscAUI, "oscEnabled", "oscLevel", "oscRange", "oscType");

  // Initialize Osc B
  setupOscUI(oscBUI, "oscBEnabled", "oscBLevel", "oscBRange", "oscBType");

  // Attack Slider
  attackSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  attackSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  addAndMakeVisible(attackSlider);

  attackAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "attack", attackSlider);

  // Decay Slider
  decaySlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  decaySlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  addAndMakeVisible(decaySlider);

  decayAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "decay", decaySlider);

  // Sustain Slider
  sustainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  sustainSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  addAndMakeVisible(sustainSlider);

  sustainAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "sustain", sustainSlider);

  // Release Slider
  releaseSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  releaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  addAndMakeVisible(releaseSlider);

  releaseAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "release", releaseSlider);

  // Arp Enabled
  setupToggleButton(arpEnabledButton);
  arpEnabledAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "arpEnabled", arpEnabledButton);

  // Arp Rate
  // Arp Rate
  arpRateSlider.setSliderStyle(
      juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
  arpRateSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  addAndMakeVisible(arpRateSlider);

  arpRateAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "arpRate", arpRateSlider);

  // Arp Seed
  arpSeedSlider.setSliderStyle(
      juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
  arpSeedSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  addAndMakeVisible(arpSeedSlider);

  arpSeedAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "arpSeed", arpSeedSlider);

  // Chord Mode Toggle
  setupToggleButton(chordModeToggle);
  chordModeAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "chordMode", chordModeToggle);

  // Filter Enabled
  setupToggleButton(filterEnabledButton);
  filterEnabledAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "filterEnabled", filterEnabledButton);

  // Cutoff Slider (FREQ)
  cutoffSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  cutoffSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  addAndMakeVisible(cutoffSlider);

  cutoffAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "cutoff", cutoffSlider);

  // Resonance Slider (RES)
  resSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  resSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  addAndMakeVisible(resSlider);

  resAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "resonance", resSlider);

  // Filter Env Slider (ENV)
  filterEnvSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  filterEnvSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  addAndMakeVisible(filterEnvSlider);

  filterEnvAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "filterEnv", filterEnvSlider);

  // Shift Slider Logic
  // Range: 24 (C0) to 127 (G8)
  rangeShiftSlider.setSliderStyle(
      juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
  rangeShiftSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  rangeShiftSlider.setRange(24.0, 127.0, 1.0);
  addAndMakeVisible(rangeShiftSlider);

  rangeShiftSlider.onValueChange = [this]() {
    // 1. Calculate current width
    int currentLow = 24;
    int currentHigh = 24;

    if (auto *p = audioProcessor.apvts.getParameter("lowNote"))
      currentLow = (int)p->convertFrom0to1(p->getValue());
    if (auto *p = audioProcessor.apvts.getParameter("highNote"))
      currentHigh = (int)p->convertFrom0to1(p->getValue());

    int width = currentHigh - currentLow;

    // 2. Get target Low from slider
    int newLow = (int)rangeShiftSlider.getValue();

    // 3. Calculate target High
    int newHigh = newLow + width;

    // 4. Constraint (Clamp High to 127)
    if (newHigh > 127) {
      newHigh = 127;
      newLow = newHigh - width; // Push back low
    }

    // 5. Update Params in Processor
    if (auto *p = audioProcessor.apvts.getParameter("lowNote"))
      p->setValueNotifyingHost(p->convertTo0to1((float)newLow));
    if (auto *p = audioProcessor.apvts.getParameter("highNote"))
      p->setValueNotifyingHost(p->convertTo0to1((float)newHigh));

    repaint();
  };

  // Modifiers
  auto setupModifier = [this](juce::TextButton &b) {
    b.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkgrey);
    b.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    b.setColour(juce::TextButton::buttonColourId,
                juce::Colour::fromString("FFced0ce"));
    b.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    b.setClickingTogglesState(false);
    addAndMakeVisible(b);
  };

  setupModifier(dimButton);
  setupModifier(minButton);
  setupModifier(majButton);
  setupModifier(sus2Button);

  setupModifier(sixthButton);
  setupModifier(min7Button);
  setupModifier(maj7Button);
  setupModifier(ninthButton);
}

MySynthAudioProcessorEditor::~MySynthAudioProcessorEditor() {
  setLookAndFeel(nullptr);
}

void MySynthAudioProcessorEditor::timerCallback() {
  repaint();

  // Sync Shift Slider with Low Note Parameter if not dragging
  if (!rangeShiftSlider.isMouseButtonDown()) {
    if (auto *p = audioProcessor.apvts.getParameter("lowNote")) {
      int currentLow = (int)p->convertFrom0to1(p->getValue());
      if ((int)rangeShiftSlider.getValue() != currentLow) {
        rangeShiftSlider.setValue((double)currentLow,
                                  juce::dontSendNotification);
      }
    }
  }

  auto setButtonState = [](juce::TextButton &b, bool active) {
    if (active) {
      b.setColour(juce::TextButton::buttonColourId,
                  juce::Colour::fromString("FFced0ce"));
      b.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    } else {
      b.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
      b.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }
  };

  auto updateRangeVisuals = [this, setButtonState](OscillatorUI &ui,
                                                   juce::String rangeId) {
    if (auto *param = audioProcessor.apvts.getParameter(rangeId)) {
      float val = param->convertFrom0to1(param->getValue());
      int index = static_cast<int>(std::round(val));

      setButtonState(ui.range16Button, index == 0);
      setButtonState(ui.range8Button, index == 1);
      setButtonState(ui.range4Button, index == 2);
    }
  };

  updateRangeVisuals(oscAUI, "oscRange");
  updateRangeVisuals(oscBUI, "oscBRange");

  // Update Modifier States
  dimButton.setToggleState(audioProcessor.isDimPressedVal(),
                           juce::dontSendNotification);
  minButton.setToggleState(audioProcessor.isMinPressedVal(),
                           juce::dontSendNotification);
  majButton.setToggleState(audioProcessor.isMajPressedVal(),
                           juce::dontSendNotification);
  sus2Button.setToggleState(audioProcessor.isSus2PressedVal(),
                            juce::dontSendNotification);

  sixthButton.setToggleState(audioProcessor.is6PressedVal(),
                             juce::dontSendNotification);
  min7Button.setToggleState(audioProcessor.isMin7PressedVal(),
                            juce::dontSendNotification);
  maj7Button.setToggleState(audioProcessor.isMaj7PressedVal(),
                            juce::dontSendNotification);
  ninthButton.setToggleState(audioProcessor.is9PressedVal(),
                             juce::dontSendNotification);

  // Arpeggiator Visualization Updates
  // 1. Read new notes from FIFO
  auto reader = audioProcessor.visualFifo.read(1);
  if (reader.blockSize1 > 0) {
    int bandIndex = audioProcessor.visualBuffer[(size_t)reader.startIndex1];

    VisualNote newNote;
    newNote.x = (float)getWidth(); // Start off-screen right
    newNote.bandIndex = bandIndex;
    activeVisualNotes.push_back(newNote);
  }

  // 2. Update Positions
  for (auto &note : activeVisualNotes) {
    note.x -= scrollSpeed;
  }

  // 3. Remove off-screen
  activeVisualNotes.erase(
      std::remove_if(activeVisualNotes.begin(), activeVisualNotes.end(),
                     [](const VisualNote &n) { return n.x + n.width < 0; }),
      activeVisualNotes.end());

  repaint();
}

void MySynthAudioProcessorEditor::paint(juce::Graphics &g) {
  const auto backgroundColor = juce::Colour::fromString("FF19191"
                                                        "9");
  const auto fontColor = juce::Colour::fromString("FFF4F6FC");
  const auto borderColor = juce::Colours::black;

  auto area = getLocalBounds();
  const auto pianoHeight = 60;
  const auto moduleHeight = (area.getHeight() - pianoHeight) / 4;
  const auto moduleWidth = area.getWidth() / 2;

  g.fillAll(backgroundColor);

  // Borders
  g.setColour(borderColor);

  auto oscilatorsArea = area.removeFromTop(moduleHeight);

  auto oscAArea = oscilatorsArea.removeFromLeft(moduleWidth);
  auto oscBArea = oscilatorsArea.removeFromLeft(moduleWidth);

  g.drawRect(oscAArea, 1.0f);
  g.drawRect(oscBArea, 1.0f);

  // Filter and envelope
  auto filterAndEnvelopeArea = area.removeFromTop(moduleHeight);

  auto filterArea = filterAndEnvelopeArea.removeFromLeft(moduleWidth);
  auto envelopeArea = filterAndEnvelopeArea.removeFromLeft(moduleWidth);

  g.drawRect(filterArea, 1.0f);
  g.drawRect(envelopeArea, 1.0f);

  // Piano
  auto pianoArea = area.removeFromTop(pianoHeight);

  // Exclude Shift Slider Area from Visualization
  pianoArea.removeFromRight(68);

  g.drawRect(pianoArea, 1.0f);

  // --- Draw Piano Keys ---
  // Start from C0 (Note 24) to G8 (Note 127)
  const int startNote = 24; // C0
  const int endNote = 127;
  int whiteKeyCount = 0;
  for (int i = startNote; i <= endNote; ++i) {
    int noteInOctave = i % 12;
    // White keys: 0, 2, 4, 5, 7, 9, 11
    if (noteInOctave == 0 || noteInOctave == 2 || noteInOctave == 4 ||
        noteInOctave == 5 || noteInOctave == 7 || noteInOctave == 9 ||
        noteInOctave == 11) {
      whiteKeyCount++;
    }
  }

  const float keyWidth = (float)pianoArea.getWidth() / (float)whiteKeyCount;
  const float blackKeyWidth = keyWidth * 0.6f;
  const float blackKeyHeight = (float)pianoArea.getHeight() * 0.6f;

  // Colors
  const auto activeWhite = juce::Colour::fromString("FFf4f6fc");
  const auto activeBlack = juce::Colours::black;
  const auto inactiveWhite = juce::Colours::grey;
  const auto inactiveBlack = juce::Colour::fromString("FF202020");

  // Get Ranges from Cached Parameters or APVTS
  // Note: We are in paint(), so accessing APVTS parameters is safe but maybe
  // strictly speaking we should use the cached values if we had them or valid
  // listeners. For Editor, reading parameter current value is fine.
  int lowNote = 0;
  int highNote = 127;

  if (auto *p = audioProcessor.apvts.getParameter("lowNote"))
    lowNote = (int)p->convertFrom0to1(p->getValue());
  if (auto *p = audioProcessor.apvts.getParameter("highNote"))
    highNote = (int)p->convertFrom0to1(p->getValue());

  // Pass 1: White Keys
  float currentX = (float)pianoArea.getX();
  for (int i = startNote; i <= endNote; ++i) {
    int noteInOctave = i % 12;
    bool isWhite =
        (noteInOctave == 0 || noteInOctave == 2 || noteInOctave == 4 ||
         noteInOctave == 5 || noteInOctave == 7 || noteInOctave == 9 ||
         noteInOctave == 11);

    if (isWhite) {
      juce::Colour color =
          (i >= lowNote && i <= highNote) ? activeWhite : inactiveWhite;
      g.setColour(color);
      g.fillRect(currentX, (float)pianoArea.getY(), keyWidth,
                 (float)pianoArea.getHeight());

      // Border
      g.setColour(juce::Colours::black);
      g.drawRect(currentX, (float)pianoArea.getY(), keyWidth,
                 (float)pianoArea.getHeight(), 1.0f);

      currentX += keyWidth;
    }
  }

  // Pass 2: Black Keys
  // Reset X
  currentX = (float)pianoArea.getX();
  for (int i = startNote; i <= endNote; ++i) {
    int noteInOctave = i % 12;
    bool isWhite =
        (noteInOctave == 0 || noteInOctave == 2 || noteInOctave == 4 ||
         noteInOctave == 5 || noteInOctave == 7 || noteInOctave == 9 ||
         noteInOctave == 11);

    if (isWhite) {
      currentX += keyWidth;
    } else {
      // It's a black key. It should be drawn centered on the boundary of the
      // PREVIOUS white key and this one? No, usually drawn ON TOP of the
      // boundary between the White Key BEFORE it and the White Key AFTER it?
      // Actually, black keys are 1, 3, 6, 8, 10.
      // 1 (C#) is between 0 (C) and 2 (D).
      // So if I am at loop index i=1. The white key C (0) has just been
      // processed. currentX is currently at the END of C. So the black key
      // center should be at currentX.

      juce::Colour color =
          (i >= lowNote && i <= highNote) ? activeBlack : inactiveBlack;
      g.setColour(color);

      float xPos = currentX - (blackKeyWidth / 2.0f);
      g.fillRect(xPos, (float)pianoArea.getY(), blackKeyWidth, blackKeyHeight);

      g.setColour(juce::Colours::white.withAlpha(0.3f)); // Subtle outline
      g.drawRect(xPos, (float)pianoArea.getY(), blackKeyWidth, blackKeyHeight,
                 1.0f);
    }
  }

  // Chords
  auto chordsArea = area.removeFromTop(moduleHeight);

  g.drawRect(chordsArea, 1.0f);

  // Arpeggiator
  auto arpeggiatorArea = area.removeFromTop(moduleHeight);

  g.drawRect(arpeggiatorArea, 1.0f);

  // Labels
  g.setColour(fontColor);
  g.setFont(15.0f);

  // Oscilators
  auto labelAArea = oscAArea.removeFromTop(40).reduced(5);
  g.drawFittedText("OSC A", labelAArea, juce::Justification::left, 1);

  auto labelBArea = oscBArea.removeFromTop(40).reduced(5);
  g.drawFittedText("OSC B", labelBArea, juce::Justification::left, 1);

  // Filter Labels
  // Layout: Enabled (50px) | Freq | Res | Env
  auto filterLabels = filterArea;
  filterLabels.removeFromLeft(50); // Skip Enabled button area

  const auto filterSliderWidth = filterLabels.getWidth() / 3;

  auto freqLabel = filterLabels.removeFromLeft(filterSliderWidth)
                       .removeFromTop(40)
                       .reduced(5);
  auto resLabel = filterLabels.removeFromLeft(filterSliderWidth)
                      .removeFromTop(40)
                      .reduced(5);
  auto envLabel = filterLabels.removeFromLeft(filterSliderWidth)
                      .removeFromTop(40)
                      .reduced(5);

  g.drawFittedText("Freq", freqLabel, juce::Justification::centred, 1);
  g.drawFittedText("Res", resLabel, juce::Justification::centred, 1);
  g.drawFittedText("Env", envLabel, juce::Justification::centred, 1);

  const auto sliderWidth = envelopeArea.getWidth() / 4;

  auto attackRect = envelopeArea.removeFromLeft(sliderWidth).reduced(5);
  auto decayRect = envelopeArea.removeFromLeft(sliderWidth).reduced(5);
  auto sustainRect = envelopeArea.removeFromLeft(sliderWidth).reduced(5);
  auto releaseRect = envelopeArea.removeFromLeft(sliderWidth).reduced(5);

  g.drawFittedText("A", attackRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("D", decayRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("S", sustainRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("R", releaseRect.removeFromTop(20),
                   juce::Justification::centred, 1);

  // Piano Area Labels (optional)
  // Maybe draw "Limit" labels near the sliders if needed, or just let sliders
  // handle it. The user didn't explicitly ask for labels in Piano Area, but
  // it's good practice. I'll skip for now to keep it clean.

  // Chords
  auto chordsButtonsArea = chordsArea.removeFromLeft(moduleWidth);
  auto chordsLabelArea = chordsButtonsArea.removeFromTop(40).reduced(5);
  g.drawFittedText("Chords", chordsLabelArea, juce::Justification::left, 1);

  auto chordsScreenArea = chordsArea.reduced(10);

  // Draw Screen Background
  g.setColour(juce::Colour::fromString("FF696D7D"));
  g.fillRoundedRectangle(chordsScreenArea.toFloat(), 10.0f);

  // Draw Chord Name
  g.setColour(juce::Colours::white);
  g.setFont(40.0f);
  g.drawFittedText(audioProcessor.getChordName(), chordsScreenArea,
                   juce::Justification::centred, 1);

  // Arpeggiator
  g.setFont(15.0f);

  auto arpeggiatorControlsArea = arpeggiatorArea.removeFromLeft(moduleWidth);
  auto arpeggiatorLabelArea =
      arpeggiatorControlsArea.removeFromTop(40).reduced(5);
  g.drawFittedText("Arpeggiator", arpeggiatorLabelArea,
                   juce::Justification::left, 1);

  auto arpeggiatorScreenArea = arpeggiatorArea.reduced(10);

  // Draw Screen Background
  g.setColour(juce::Colour::fromString("FF696D7D"));
  g.fillRoundedRectangle(arpeggiatorScreenArea.toFloat(), 10.0f);

  // Draw Visual Notes
  // Clip to screen area
  {
    juce::Graphics::ScopedSaveState saveState(g);
    g.reduceClipRegion(arpeggiatorScreenArea);

    g.setColour(fontColor);

    // Map bands 0-4 to Height
    const float bandHeight = arpeggiatorScreenArea.getHeight() / 5.0f;

    for (const auto &note : activeVisualNotes) {

      // Band 0 = Bottom.
      // y = bottom - (band + 1) * height
      float yVal =
          arpeggiatorScreenArea.getBottom() - (note.bandIndex + 1) * bandHeight;

      g.fillRect(note.x, yVal, note.width, bandHeight - 2.0f);
    }
  }
}

void MySynthAudioProcessorEditor::resized() {
  const auto bounds = getLocalBounds();
  const auto padding = 10;

  const auto pianoHeight = 60;
  const auto moduleHeight = (bounds.getHeight() - pianoHeight) / 4;
  const auto moduleWidth = bounds.getWidth() / 2;

  auto area = bounds;

  auto oscilatorsArea = area.removeFromTop(moduleHeight);
  auto oscAArea = oscilatorsArea.removeFromLeft(moduleWidth);
  auto oscBArea = oscilatorsArea.removeFromRight(moduleWidth);
  auto filterAndEnvelopeArea = area.removeFromTop(moduleHeight);
  auto filterArea = filterAndEnvelopeArea.removeFromLeft(moduleWidth);
  auto envelopeArea = filterAndEnvelopeArea.removeFromRight(moduleWidth);
  auto pianoArea = area.removeFromTop(pianoHeight);
  auto chordsArea = area.removeFromTop(moduleHeight);
  auto chordsButtonsArea = chordsArea.removeFromLeft(moduleWidth);
  auto arpeggiatorArea = area;
  auto arpeggiatorControlsArea = arpeggiatorArea.removeFromLeft(moduleWidth);

  // Helper for Osc Layout
  auto layoutOscUI = [](OscillatorUI &ui, juce::Rectangle<int> area) {
    auto oscControls = area.reduced(5);

    // Left: Enabled Toggle
    auto enableToggleArea = oscControls.removeFromLeft(55);
    ui.enabledButton.setBounds(
        enableToggleArea.removeFromRight(45).withSizeKeepingCentre(30, 30));

    // Right: Level Slider
    ui.levelSlider.setBounds(oscControls.removeFromRight(55));

    // Center
    auto centerArea = oscControls.reduced(5, 0);

    // Range Buttons
    auto rangeContainer =
        centerArea.removeFromTop(centerArea.getHeight() * 0.65);
    const int rangeBtnSize = 44;
    const int rangeBtnGap = 5;
    const int totalRangeWidth = (rangeBtnSize * 3) + (rangeBtnGap * 2);

    auto rangeGroup =
        rangeContainer.withSizeKeepingCentre(totalRangeWidth, rangeBtnSize);

    ui.range16Button.setBounds(rangeGroup.removeFromLeft(rangeBtnSize));
    rangeGroup.removeFromLeft(rangeBtnGap);
    ui.range8Button.setBounds(rangeGroup.removeFromLeft(rangeBtnSize));
    rangeGroup.removeFromLeft(rangeBtnGap);
    ui.range4Button.setBounds(rangeGroup.removeFromLeft(rangeBtnSize));

    // Bottom: Type Selector
    ui.typeSelector.setBounds(
        centerArea.removeFromTop(30).withSizeKeepingCentre(140, 30));
  };

  layoutOscUI(oscAUI, oscAArea);
  layoutOscUI(oscBUI, oscBArea);

  // Sliders area
  const auto sliderWidth = envelopeArea.getWidth() / 4;

  // Attack
  auto attackArea = envelopeArea.removeFromLeft(sliderWidth);
  attackArea.removeFromTop(20); // Label space
  attackSlider.setBounds(attackArea.reduced(padding));

  // Decay
  auto decayArea = envelopeArea.removeFromLeft(sliderWidth);
  decayArea.removeFromTop(20);
  decaySlider.setBounds(decayArea.reduced(padding));

  // Sustain
  auto sustainArea = envelopeArea.removeFromLeft(sliderWidth);
  sustainArea.removeFromTop(20);
  sustainSlider.setBounds(sustainArea.reduced(padding));

  // Release
  auto releaseArea = envelopeArea.removeFromLeft(sliderWidth);
  releaseArea.removeFromTop(20);
  releaseSlider.setBounds(releaseArea.reduced(padding));

  // Filter Area (Bottom)
  auto filterControls = filterArea;
  auto enabledArea = filterControls.removeFromLeft(60);

  filterEnabledButton.setBounds(
      enabledArea.removeFromRight(45).withSizeKeepingCentre(30, 30));

  const auto filterSliderWidth = filterControls.getWidth() / 3;

  // Cutoff
  auto cutoffArea = filterControls.removeFromLeft(filterSliderWidth);
  cutoffArea.removeFromTop(20); // Label
  cutoffSlider.setBounds(cutoffArea.reduced(padding));

  // Resonance
  auto resArea = filterControls.removeFromLeft(filterSliderWidth);
  resArea.removeFromTop(20);
  resSlider.setBounds(resArea.reduced(padding));

  // Env
  auto envArea = filterControls.removeFromLeft(filterSliderWidth);
  envArea.removeFromTop(20);
  filterEnvSlider.setBounds(envArea.reduced(padding));

  // Piano Area Limits
  // We have the rangeShiftSlider on the right (68 pixels width as requested)
  // And the piano visualization takes the rest.

  auto shiftControlArea = pianoArea.removeFromRight(68);
  rangeShiftSlider.setBounds(shiftControlArea.reduced(5));

  // The remaining pianoArea is for drawing keys
  // Store bounds for painting and interaction
  pianoAreaBounds = pianoArea;

  // Modifiers Area (Bottom)
  auto enableChordModeArea = chordsButtonsArea.removeFromLeft(60);
  chordModeToggle.setBounds(
      enableChordModeArea.removeFromRight(45).withSizeKeepingCentre(30, 30));
  auto modArea = chordsButtonsArea.reduced(10);
  // We use fixed width for
  // buttons to make them
  // look neat
  const int buttonWidth = 48;
  const int buttonSize = buttonWidth - 4; // Accounting for
                                          // reduced(2)
                                          // approximately
  const int buttonGap = 4;

  auto getSquareBounds = [&](juce::Rectangle<int> r) {
    return r.withSizeKeepingCentre(buttonSize, buttonSize);
  };

  const int totalHeight = buttonSize * 2 + buttonGap;
  auto centeredModArea =
      modArea.withSizeKeepingCentre(modArea.getWidth(), totalHeight);

  auto triadModRow = centeredModArea.removeFromTop(buttonSize);
  centeredModArea.removeFromTop(buttonGap);
  auto seventhModRow = centeredModArea;

  dimButton.setBounds(getSquareBounds(triadModRow.removeFromLeft(buttonWidth)));
  minButton.setBounds(getSquareBounds(triadModRow.removeFromLeft(buttonWidth)));
  majButton.setBounds(getSquareBounds(triadModRow.removeFromLeft(buttonWidth)));
  sus2Button.setBounds(
      getSquareBounds(triadModRow.removeFromLeft(buttonWidth)));

  sixthButton.setBounds(
      getSquareBounds(seventhModRow.removeFromLeft(buttonWidth)));
  min7Button.setBounds(
      getSquareBounds(seventhModRow.removeFromLeft(buttonWidth)));
  maj7Button.setBounds(
      getSquareBounds(seventhModRow.removeFromLeft(buttonWidth)));
  ninthButton.setBounds(
      getSquareBounds(seventhModRow.removeFromLeft(buttonWidth)));

  // Arpeggiator Area
  // (Bottom)
  auto enableArpArea = arpeggiatorControlsArea.removeFromLeft(60);
  arpEnabledButton.setBounds(
      enableArpArea.removeFromRight(45).withSizeKeepingCentre(30, 30));

  arpeggiatorControlsArea.removeFromRight(20);
  auto rateArea = arpeggiatorControlsArea.reduced(5);

  // Split remaining area for Rate and Seed
  auto rateControlArea = rateArea.removeFromLeft(rateArea.getWidth() / 2);
  auto seedControlArea = rateArea;

  arpRateSlider.setBounds(rateControlArea.withSizeKeepingCentre(60, 60));
  arpSeedSlider.setBounds(seedControlArea.withSizeKeepingCentre(60, 60));

  // Store Piano Area for Interaction
  pianoAreaBounds = pianoArea;
}

// --- Helper Functions for Piano Interaction ---

float MySynthAudioProcessorEditor::getXForNote(int note) const {
  if (pianoAreaBounds.isEmpty())
    return 0.0f;

  const int startNote = 24; // C0
  const int endNote = 127;

  // Recalculate keyWidth (same logic as paint)
  int whiteKeyCount = 0;
  for (int i = startNote; i <= endNote; ++i) {
    int noteInOctave = i % 12;
    if (noteInOctave == 0 || noteInOctave == 2 || noteInOctave == 4 ||
        noteInOctave == 5 || noteInOctave == 7 || noteInOctave == 9 ||
        noteInOctave == 11) {
      whiteKeyCount++;
    }
  }

  const float keyWidth =
      (float)pianoAreaBounds.getWidth() / (float)whiteKeyCount;

  float currentX = (float)pianoAreaBounds.getX();
  for (int i = startNote; i < note;
       ++i) { // Up to but not including active note
    int noteInOctave = i % 12;
    bool isWhite =
        (noteInOctave == 0 || noteInOctave == 2 || noteInOctave == 4 ||
         noteInOctave == 5 || noteInOctave == 7 || noteInOctave == 9 ||
         noteInOctave == 11);
    if (isWhite)
      currentX += keyWidth;
  }
  return currentX;
}

int MySynthAudioProcessorEditor::getNoteForX(float x) const {
  if (pianoAreaBounds.isEmpty())
    return 24;

  const int startNote = 24;
  const int endNote = 127;

  int bestNote = startNote;
  float minDiff = 100000.0f;

  // Let's do a linear search for the closest note boundary.
  for (int i = startNote; i <= endNote; ++i) {
    float noteX = getXForNote(i);

    float diff = std::abs(x - noteX);
    if (diff < minDiff) {
      minDiff = diff;
      bestNote = i;
    }
  }
  return bestNote;
}

void MySynthAudioProcessorEditor::mouseDown(const juce::MouseEvent &e) {
  if (pianoAreaBounds.contains(e.getPosition())) {
    // Get current parameter values
    int currentLow = 24;
    int currentHigh = 127;

    if (auto *p = audioProcessor.apvts.getParameter("lowNote"))
      currentLow = (int)p->convertFrom0to1(p->getValue());
    if (auto *p = audioProcessor.apvts.getParameter("highNote"))
      currentHigh = (int)p->convertFrom0to1(p->getValue());

    // Calculate positions
    float lowX = getXForNote(currentLow);

    // Check closest
    float distToLow = std::abs((float)e.x - lowX);

    // Safety check for 128
    float highRightEdgeX = (currentHigh < 127)
                               ? getXForNote(currentHigh + 1)
                               : (float)pianoAreaBounds.getRight();

    float distToHigh = std::abs((float)e.x - highRightEdgeX);

    const float grabThreshold = 20.0f;

    if (distToLow < grabThreshold && distToLow < distToHigh) {
      isDraggingLow = true;
    } else if (distToHigh < grabThreshold) {
      isDraggingHigh = true;
    }
  }
}

void MySynthAudioProcessorEditor::mouseDrag(const juce::MouseEvent &e) {
  if (isDraggingLow) {
    int newLow = getNoteForX((float)e.x);

    int currentHigh = 127;
    if (auto *p = audioProcessor.apvts.getParameter("highNote"))
      currentHigh = (int)p->convertFrom0to1(p->getValue());

    // Check collision with High Limit (gap < 12)
    if (newLow > currentHigh - 12) {
      // Push High Limit
      int requiredHigh = newLow + 12;

      // Global Max Constraint
      if (requiredHigh > 127) {
        requiredHigh = 127;
        newLow = requiredHigh - 12; // Clamp Low
      }

      // Update High Limit
      if (auto *p = audioProcessor.apvts.getParameter("highNote")) {
        p->setValueNotifyingHost(p->convertTo0to1((float)requiredHigh));
      }
    }

    // Global Min Constraint for Low
    if (newLow < 24)
      newLow = 24;

    if (auto *p = audioProcessor.apvts.getParameter("lowNote")) {
      p->setValueNotifyingHost(p->convertTo0to1((float)newLow));
    }
    repaint();
  } else if (isDraggingHigh) {
    int boundaryNote = getNoteForX((float)e.x);
    int newHigh = boundaryNote - 1;

    int currentLow = 24;
    if (auto *p = audioProcessor.apvts.getParameter("lowNote"))
      currentLow = (int)p->convertFrom0to1(p->getValue());

    // Check collision with Low Limit (gap < 12)
    if (newHigh < currentLow + 12) {
      // Push Low Limit
      int requiredLow = newHigh - 12;

      // Global Min Constraint
      if (requiredLow < 24) {
        requiredLow = 24;
        newHigh = requiredLow + 12; // Clamp High
      }

      // Update Low Limit
      if (auto *p = audioProcessor.apvts.getParameter("lowNote")) {
        p->setValueNotifyingHost(p->convertTo0to1((float)requiredLow));
      }
    }

    // Global Max Constraint for High
    if (newHigh > 127)
      newHigh = 127;

    if (auto *p = audioProcessor.apvts.getParameter("highNote")) {
      p->setValueNotifyingHost(p->convertTo0to1((float)newHigh));
    }
    repaint();
  }
}

void MySynthAudioProcessorEditor::mouseUp(const juce::MouseEvent &) {
  isDraggingLow = false;
  isDraggingHigh = false;
}
