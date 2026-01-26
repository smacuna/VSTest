#pragma once

#include <JuceHeader.h>

namespace ChordNameUtils {

inline juce::String getNoteName(int noteNumber) {
  const char *noteNames[] = {"C",  "C#", "D",  "D#", "E",  "F",
                             "F#", "G",  "G#", "A",  "A#", "B"};
  return noteNames[noteNumber % 12];
}

inline juce::String getChordName(int rootNote, bool isDim, bool isMin,
                                 bool isMaj, bool isSus2, bool is6, bool isMin7,
                                 bool isMaj7, bool is9) {
  if (rootNote < 0)
    return "Waiting...";

  juce::String name = getNoteName(rootNote);

  // Quality
  bool majorTriad = false;
  bool minorTriad = false;

  if (isDim) {
    name += "dim";
    minorTriad = true; // functionally minor-ish base
  } else if (isMin) {
    name += "m";
    minorTriad = true;
  } else if (isMaj) {
    // name += ""; // Default
    majorTriad = true;
  } else if (isSus2) {
    name += "sus2";
  } else {
    // Default to Major if nothing pressed? Or unknown?
    // Assuming default is Major based on previous logic if available,
    // but check Processor: "Priority: Dim > Min > Maj > Sus2"
    // If none pressed, intervals is empty -> no sound.
    // So if none, it's just the root note?
    if (!isDim && !isMin && !isMaj && !isSus2) {
      // name += "?"; // Or just Root
    }
  }

  // Extensions
  // Logic from Processor:
  // 6 (C4) > Min7 (D4) > Maj7 (F4) > 9 (G4)

  if (is6) {
    name += "6";
  } else if (isMin7) {
    if (majorTriad)
      name += "7"; // Dominant 7
    else
      name += "7"; // m7 or dim7 (technically m7b5 if dim)
  } else if (isMaj7) {
    if (minorTriad)
      name += "Maj7"; // mM7
    else
      name += "Maj7"; // CMaj7
  } else if (is9) {
    // 9 implies 7 usually. Processor adds Minor 7th and Major 9th.
    if (majorTriad)
      name += "9"; // C9 (Dominant 9)
    else
      name += "9"; // Cm9
  }

  return name;
}

} // namespace ChordNameUtils
