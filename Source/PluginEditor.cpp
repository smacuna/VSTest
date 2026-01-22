#include "PluginEditor.h"
#include "PluginProcessor.h"

MySynthAudioProcessorEditor::MySynthAudioProcessorEditor(
    MySynthAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setSize(400, 300);
}

MySynthAudioProcessorEditor::~MySynthAudioProcessorEditor() {}

void MySynthAudioProcessorEditor::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::black);
  g.setColour(juce::Colours::white);
  g.setFont(15.0f);
  g.drawFittedText("MySynth - Generic Editor", getLocalBounds(),
                   juce::Justification::centred, 1);
}

void MySynthAudioProcessorEditor::resized() {}
