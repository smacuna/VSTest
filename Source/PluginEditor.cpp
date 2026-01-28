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

  // Oscillator UI
  setupToggleButton(oscEnabledButton);
  oscEnabledAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "oscEnabled", oscEnabledButton);

  oscLevelSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
  oscLevelSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  addAndMakeVisible(oscLevelSlider);
  oscLevelAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "oscLevel", oscLevelSlider);

  // Range Buttons (Manual handling via onClick and timer)
  auto setupRangeButton = [this](juce::TextButton &b, int index) {
    b.setButtonText(b.getButtonText()); // Keep text
    b.setClickingTogglesState(false);
    b.setColour(juce::TextButton::buttonColourId,
                juce::Colour::fromString("FFced0ce"));
    b.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    addAndMakeVisible(b);
    b.onClick = [this, index]() {
      if (auto *p = audioProcessor.apvts.getParameter("oscRange"))
        p->setValueNotifyingHost(p->convertTo0to1(index));
    };
  };
  setupRangeButton(osc16Button, 0);
  setupRangeButton(osc8Button, 1);
  setupRangeButton(osc4Button, 2);

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

  oscSelector.addItem("Sine", 1);
  oscSelector.addItem("Saw", 2);
  oscSelector.addItem("Square", 3);
  addAndMakeVisible(oscSelector);

  oscAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
          audioProcessor.apvts, "oscType", oscSelector);

  // Arp Enabled
  setupToggleButton(arpEnabledButton);
  arpEnabledAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "arpEnabled", arpEnabledButton);

  // Arp Rate
  arpRateBox.addItem("1/2", 1);
  arpRateBox.addItem("1/4", 2);
  arpRateBox.addItem("1/8", 3);
  arpRateBox.addItem("1/16", 4);
  arpRateBox.addItem("1/32", 5);
  arpRateBox.addItem("1/64", 6);
  addAndMakeVisible(arpRateBox);
  arpRateAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
          audioProcessor.apvts, "arpRate", arpRateBox);

  // Chord Mode Toggle
  // Chord Mode Toggle
  setupToggleButton(chordModeToggle);
  chordModeAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "chordMode", chordModeToggle);

  // Cutoff Slider
  cutoffSlider.setSliderStyle(
      juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
  cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
  addAndMakeVisible(cutoffSlider);

  cutoffAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "cutoff", cutoffSlider);

  // Resonance Slider
  resSlider.setSliderStyle(
      juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
  resSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
  addAndMakeVisible(resSlider);

  resAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "resonance", resSlider);

  // Low Note Slider
  lowNoteSlider.setSliderStyle(
      juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
  lowNoteSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
  addAndMakeVisible(lowNoteSlider);

  lowNoteAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "lowNote", lowNoteSlider);

  // High Note Slider
  highNoteSlider.setSliderStyle(
      juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
  highNoteSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
  addAndMakeVisible(highNoteSlider);

  highNoteAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "highNote", highNoteSlider);

  // Modifiers
  // Modifiers
  auto setupModifier = [this](juce::TextButton &b) {
    b.setColour(juce::TextButton::buttonColourId,
                juce::Colour::fromString("FFced0ce"));
    b.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    b.setColour(juce::TextButton::buttonOnColourId,
                juce::Colour::fromString("FFced0ce"));
    b.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
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

  // Update Range Button States
  if (auto *param = audioProcessor.apvts.getParameter("oscRange")) {
    float val = param->convertFrom0to1(param->getValue());
    int index = static_cast<int>(std::round(val));

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

    setButtonState(osc16Button, index == 0);
    setButtonState(osc8Button, index == 1);
    setButtonState(osc4Button, index == 2);
  }

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
}

void MySynthAudioProcessorEditor::paint(juce::Graphics &g) {
  const auto backgroundColor = juce::Colour::fromString("FF191919");
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

  g.drawRect(pianoArea, 1.0f);

  // Chords
  auto chordsArea = area.removeFromTop(moduleHeight);

  g.drawRect(chordsArea, 1.0f);

  // Arpeggiator
  auto arpeggiatorArea = area.removeFromTop(moduleHeight);

  g.drawRect(arpeggiatorArea, 1.0f);

  // Palabras

  g.setColour(fontColor);
  g.setFont(15.0f);

  // Oscilators

  auto labelAArea = oscAArea.removeFromTop(40).reduced(5);
  g.drawFittedText("OSC A", labelAArea, juce::Justification::left, 1);

  auto labelBArea = oscBArea.removeFromTop(40).reduced(5);
  g.drawFittedText("OSC B", labelBArea, juce::Justification::left, 1);

  // Filter
  //   auto filterLabelArea = filterArea.removeFromTop(40);
  //   g.drawFittedText("Filter", filterLabelArea, juce::Justification::left,
  //   1);

  const auto filterSliderWidth = filterArea.getWidth() / 4;

  filterArea.removeFromLeft(filterSliderWidth);
  auto cutoffRect = filterArea.removeFromLeft(filterSliderWidth).reduced(5);
  auto resRect = filterArea.removeFromLeft(filterSliderWidth).reduced(5);
  auto envRect = filterArea.removeFromLeft(filterSliderWidth).reduced(5);

  g.drawFittedText("Cutoff", cutoffRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("Res", resRect.removeFromTop(20),
                   juce::Justification::centred, 1);
  g.drawFittedText("Env", envRect.removeFromTop(20),
                   juce::Justification::centred, 1);

  // Envelope
  //   auto envelopeLabelArea = envelopeArea.removeFromTop(40);
  //   g.drawFittedText("Envelope", envelopeLabelArea,
  //   juce::Justification::left, 1);

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

  // Chords
  auto chordsButtonsArea = chordsArea.removeFromLeft(moduleWidth);
  auto chordsLabelArea = chordsButtonsArea.removeFromTop(40).reduced(5);
  g.drawFittedText("Chords", chordsLabelArea, juce::Justification::left, 1);

  auto chordsScreenArea = chordsArea.reduced(10);

  // Draw Screen Background
  g.setColour(juce::Colour::fromString("FF696D7D").withAlpha(0.5f));
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
  g.setColour(juce::Colour::fromString("FF696D7D").withAlpha(0.5f));
  g.fillRoundedRectangle(arpeggiatorScreenArea.toFloat(), 10.0f);
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

  // Oscillator UI Layout
  auto oscControls = oscAArea.reduced(5);

  // Left: Enabled Toggle
  oscEnabledButton.setBounds(
      oscControls.removeFromLeft(40).withSizeKeepingCentre(30, 30));

  // Right: Level Slider
  oscLevelSlider.setBounds(oscControls.removeFromRight(30));

  // Center
  auto centerArea = oscControls.reduced(5, 0);

  // Layout for Range Buttons (centered, larger, spaced)
  auto rangeContainer = centerArea.removeFromTop(centerArea.getHeight() * 0.65);
  // Define button size and spacing
  const int rangeBtnSize = 44; // Increased size to 40
  const int rangeBtnGap = 5;
  const int totalRangeWidth = (rangeBtnSize * 3) + (rangeBtnGap * 2);

  // Center the group of buttons
  auto rangeGroup =
      rangeContainer.withSizeKeepingCentre(totalRangeWidth, rangeBtnSize);

  osc16Button.setBounds(rangeGroup.removeFromLeft(rangeBtnSize));
  rangeGroup.removeFromLeft(rangeBtnGap);
  osc8Button.setBounds(rangeGroup.removeFromLeft(rangeBtnSize));
  rangeGroup.removeFromLeft(rangeBtnGap);
  osc4Button.setBounds(rangeGroup.removeFromLeft(rangeBtnSize));

  // Bottom: Type Selector
  oscSelector.setBounds(
      centerArea.removeFromTop(30).withSizeKeepingCentre(140, 30));

  //   // Arp Controls
  //   arpEnabledButton.setBounds(topArea.removeFromRight(60).reduced(5, 5));
  //   arpRateBox.setBounds(topArea.removeFromRight(80).reduced(0, 5));

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
  const auto filterSliderWidth = filterArea.getWidth() / 4;

  // Cutoff
  auto cutoffArea = filterArea.removeFromLeft(filterSliderWidth);
  cutoffArea.removeFromTop(20);
  cutoffSlider.setBounds(cutoffArea.reduced(25));

  // Resonance
  auto resArea = filterArea.removeFromLeft(filterSliderWidth);
  resArea.removeFromTop(20);
  resSlider.setBounds(resArea.reduced(25));

  // Low Note
  auto lowArea = filterArea.removeFromLeft(filterSliderWidth);
  lowArea.removeFromTop(20);
  lowNoteSlider.setBounds(lowArea.reduced(25));

  // High Note
  auto highArea = filterArea.removeFromLeft(filterSliderWidth);
  highArea.removeFromTop(20);
  highNoteSlider.setBounds(highArea.reduced(25));

  // Modifiers Area (Bottom)
  auto enableChordModeArea = chordsButtonsArea.removeFromLeft(60);
  chordModeToggle.setBounds(
      enableChordModeArea.removeFromRight(45).withSizeKeepingCentre(30, 30));
  auto modArea = chordsButtonsArea.reduced(10);
  // We use fixed width for buttons to make them look neat
  const int buttonWidth = 48;
  const int buttonSize =
      buttonWidth - 4; // Accounting for reduced(2) approximately
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

  // Arpeggiator Area (Bottom)
  auto enableArpArea = arpeggiatorControlsArea.removeFromLeft(60);
  arpEnabledButton.setBounds(
      enableArpArea.removeFromRight(45).withSizeKeepingCentre(30, 30));
  auto rateArea = arpeggiatorControlsArea.reduced(10);
}
