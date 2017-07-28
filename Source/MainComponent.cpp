#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <regex>

class MainContentComponent  : public Component,
                              private ComboBox::Listener,
                              private MidiInputCallback,
						      public Button::Listener	
{
public:
    MainContentComponent()
      : lastInputIndex (0),
        startTime (Time::getMillisecondCounterHiRes() * 0.001)
    {
        setOpaque (true);

        addAndMakeVisible (midiInputListLabel);
        midiInputListLabel.setText ("MIDI Input:", dontSendNotification);
        midiInputListLabel.attachToComponent (&midiInputList, true);

        addAndMakeVisible (midiInputList);
        midiInputList.setTextWhenNoChoicesAvailable ("No MIDI Inputs Enabled");
        const StringArray midiInputs (MidiInput::getDevices());
        midiInputList.addItemList (midiInputs, 1);
        midiInputList.addListener (this);

        // find the first enabled device and use that by default
        for (int i = 0; i < midiInputs.size(); ++i)
        {
            if (deviceManager.isMidiInputEnabled (midiInputs[i]))
            {
                setMidiInput (i);
                break;
            }
        }

        // if no enabled devices were found just use the first one in the list
        if (midiInputList.getSelectedId() == 0)
            setMidiInput (0);

		// OSC editors
		addAndMakeVisible(oscTopicEditorLabel);
		oscTopicEditorLabel.setText("OSC topic to publish on:", dontSendNotification);
		oscTopicEditorLabel.attachToComponent(&oscTopicEditor, true);
		addAndMakeVisible(oscTopicEditor);
		oscTopicEditor.setText("/specktr/{channel}/{message_type}");
		oscTopicEditor.moveCaretToEnd();
		oscTopicEditor.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
		oscTopicEditor.setColour(TextEditor::outlineColourId, Colour(0x1c000000));
		oscTopicEditor.setColour(TextEditor::shadowColourId, Colour(0x16000000));

		addAndMakeVisible(oscHostEditorLabel);
		oscHostEditorLabel.setText("OSC host address:", dontSendNotification);
		oscHostEditorLabel.attachToComponent(&oscHostEditor, true);
		addAndMakeVisible(oscHostEditor);
		oscHostEditor.setText("127.0.0.1");
		oscHostEditor.moveCaretToEnd();
		oscHostEditor.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
		oscHostEditor.setColour(TextEditor::outlineColourId, Colour(0x1c000000));
		oscHostEditor.setColour(TextEditor::shadowColourId, Colour(0x16000000));

		addAndMakeVisible(oscPortEditorLabel);
		oscPortEditorLabel.setText("OSC host port:", dontSendNotification);
		addAndMakeVisible(oscPortEditor);
		oscPortEditorLabel.attachToComponent(&oscPortEditor, true);
		oscPortEditor.setText("6006");
		oscPortEditor.moveCaretToEnd();
		oscPortEditor.setInputRestrictions(6, "0123456789");
		oscPortEditor.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
		oscPortEditor.setColour(TextEditor::outlineColourId, Colour(0x1c000000));
		oscPortEditor.setColour(TextEditor::shadowColourId, Colour(0x16000000));

		addAndMakeVisible(oscStartButton);
		oscStartButton.setButtonText("Start OSC");
		oscStartButton.addListener(this);

		addAndMakeVisible(oscStopButton);
		oscStopButton.setButtonText("Stop OSC");
		oscStopButton.addListener(this);


		// message box
        addAndMakeVisible (midiMessagesBox);
        midiMessagesBox.setMultiLine (true);
        midiMessagesBox.setReturnKeyStartsNewLine (true);
        midiMessagesBox.setReadOnly (true);
        midiMessagesBox.setScrollbarsShown (true);
        midiMessagesBox.setCaretVisible (false);
        midiMessagesBox.setPopupMenuEnabled (true);
        midiMessagesBox.setColour (TextEditor::backgroundColourId, Colour (0x32ffffff));
        midiMessagesBox.setColour (TextEditor::outlineColourId, Colour (0x1c000000));
        midiMessagesBox.setColour (TextEditor::shadowColourId, Colour (0x16000000));

		addAndMakeVisible(errorLog);
		errorLog.setReadOnly(true);
		midiMessagesBox.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
		midiMessagesBox.setColour(TextEditor::outlineColourId, Colour(0x1cff0000));

        setSize (600, 400);
    }

    ~MainContentComponent()
    {
        deviceManager.removeMidiInputCallback (MidiInput::getDevices()[midiInputList.getSelectedItemIndex()], this);
        midiInputList.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds());
        midiInputList.setBounds (area.removeFromTop (36).removeFromRight (getWidth() - 150).reduced (8));

		oscTopicEditor.setBounds(area.removeFromTop(36).removeFromRight(getWidth() - 150).reduced(4));
		oscHostEditor.setBounds(area.removeFromTop(36).removeFromRight(getWidth() - 150).reduced(4));
		oscPortEditor.setBounds(area.removeFromTop(36).removeFromRight(getWidth() - 150).reduced(4));

		oscStartButton.setSize(150, 24);
		oscStartButton.setTopLeftPosition(8, 36*4+4);

		oscStopButton.setSize(150, 24);
		oscStopButton.setTopLeftPosition(getWidth() - 150 - 8, 36*4+4);

		area.removeFromTop(30);

        midiMessagesBox.setBounds (area.reduced(8));
    }

	void buttonClicked(Button* button) override
	{
		if (button == &oscStartButton)
		{
			if (!(oscClient.connect(oscHostEditor.getText(), atoi(oscPortEditor.getText().toStdString().c_str())))) {
				showErrorMessage("Error: could not connect to " + oscHostEditor.getText() + " on port " + oscPortEditor.getText());
			}
		}
		else if (button == &oscStopButton)
		{
			oscClient.disconnect();
		}
	}

private:
	static String getMidiMessageType(const MidiMessage& m)
	{
		if (m.isNoteOn())           return "note_on";
		if (m.isNoteOff())          return "note_off";
		if (m.isProgramChange())    return "program_change";
		if (m.isPitchWheel())       return "pitch_bend";
		if (m.isAftertouch())       return "aftertouch";
		if (m.isChannelPressure())  return "channel_pressure";
		if (m.isAllNotesOff())      return "all_notes_off";
		if (m.isAllSoundOff())      return "all_sound_off";
		if (m.isController())		return "control_change";
		if (m.isMetaEvent())        return "Meta event";
		return "unknown";
	}

	std::string getOscTopic(const MidiMessage& m)
	{
		std::string topic = oscTopicEditor.getText().toStdString();
		int channel = m.getChannel();
		std::string messageType = getMidiMessageType(m).toStdString();

		topic = std::regex_replace(topic, std::regex("\\{channel\\}"), std::to_string(channel));
		topic = std::regex_replace(topic, std::regex("\\{message_type\\}"), messageType);

		return topic;
	}

	void sendOsc(const MidiMessage& m)
	{
		// build topic
		std::string topic = getOscTopic(m);
		std::string messageType = getMidiMessageType(m).toStdString();

		OSCMessage res = OSCMessage(OSCAddressPattern(topic.c_str()));

		if (messageType == "note_on")          { res.addInt32(m.getNoteNumber()); 
		                                         res.addInt32(m.getVelocity()); }
		if (messageType == "note_off")         { res.addInt32(m.getNoteNumber());
		                                         res.addInt32(m.getVelocity()); }
		if (messageType == "program_change")   { res.addInt32(m.getProgramChangeNumber());}
		if (messageType == "pitch_bend")      { res.addInt32(m.getPitchWheelValue()); }
		if (messageType == "aftertouch")       { res.addInt32(m.getAfterTouchValue()); }
		if (messageType == "channel_pressure") { res.addInt32(m.getChannelPressureValue()); }
		if (messageType == "control_change")   { res.addInt32(m.getControllerNumber());
		                                         res.addInt32(m.getControllerValue()); }

		oscClient.send(res);
	}
	
    static String getMidiMessageDescription (const MidiMessage& m)
    {
        if (m.isNoteOn())           return "Note on "  + MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3);
        if (m.isNoteOff())          return "Note off " + MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3);
        if (m.isProgramChange())    return "Program change " + String (m.getProgramChangeNumber());
        if (m.isPitchWheel())       return "Pitch wheel " + String (m.getPitchWheelValue());
        if (m.isAftertouch())       return "After touch " + MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3) +  ": " + String (m.getAfterTouchValue());
        if (m.isChannelPressure())  return "Channel pressure " + String (m.getChannelPressureValue());
        if (m.isAllNotesOff())      return "All notes off";
        if (m.isAllSoundOff())      return "All sound off";
        if (m.isMetaEvent())        return "Meta event";

        if (m.isController())
        {
            String name (MidiMessage::getControllerName (m.getControllerNumber()));

            if (name.isEmpty())
                name = "[" + String (m.getControllerNumber()) + "]";

            return "Controller " + name + ": " + String (m.getControllerValue());
        }

        return String::toHexString (m.getRawData(), m.getRawDataSize());
    }

    void logMessage (const String& m)
    {
        midiMessagesBox.moveCaretToEnd();
        midiMessagesBox.insertTextAtCaret (m + newLine);
    }

    /** Starts listening to a MIDI input device, enabling it if necessary. */
    void setMidiInput (int index)
    {
        const StringArray list (MidiInput::getDevices());

        deviceManager.removeMidiInputCallback (list[lastInputIndex], this);

        const String newInput (list[index]);

        if (! deviceManager.isMidiInputEnabled (newInput))
            deviceManager.setMidiInputEnabled (newInput, true);

        deviceManager.addMidiInputCallback (newInput, this);
        midiInputList.setSelectedId (index + 1, dontSendNotification);

        lastInputIndex = index;
    }

    void comboBoxChanged (ComboBox* box) override
    {
        if (box == &midiInputList)
            setMidiInput (midiInputList.getSelectedItemIndex());
    }

    // These methods handle callbacks from the midi device
    void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) override
    {
        keyboardState.processNextMidiEvent (message);
        postMessageToList (message, source->getName());
    }

    // This is used to dispach an incoming message to the message thread
    class IncomingMessageCallback   : public CallbackMessage
    {
    public:
        IncomingMessageCallback (MainContentComponent* o, const MidiMessage& m, const String& s)
           : owner (o), message (m), source (s)
        {}

        void messageCallback() override
        {
            if (owner != nullptr)
                owner->addMessageToList (message, source);
        }

        Component::SafePointer<MainContentComponent> owner;
        MidiMessage message;
        String source;
    };

    void postMessageToList (const MidiMessage& message, const String& source)
    {
        (new IncomingMessageCallback (this, message, source))->post();
    }

    void addMessageToList (const MidiMessage& message, const String& source)
    {
        const double time = message.getTimeStamp() - startTime;

        const int hours = ((int) (time / 3600.0)) % 24;
        const int minutes = ((int) (time / 60.0)) % 60;
        const int seconds = ((int) time) % 60;
        const int millis = ((int) (time * 1000.0)) % 1000;

        const String timecode (String::formatted ("%02d:%02d:%02d.%03d",
                                                  hours,
                                                  minutes,
                                                  seconds,
                                                  millis));

        const String description (getMidiMessageDescription (message));

        const String midiMessageString (timecode + "  -  " + description + " (" + source + ")");
        logMessage (midiMessageString);
		sendOsc(message);
    }

	void showErrorMessage(const String& messageText)
	{
		AlertWindow::showMessageBoxAsync(
			AlertWindow::WarningIcon,
			"Connection error",
			messageText,
			"OK");
	}

    //==============================================================================
    AudioDeviceManager deviceManager;
    ComboBox midiInputList;
    Label midiInputListLabel;
    int lastInputIndex;

    MidiKeyboardState keyboardState;

	TextEditor oscTopicEditor;
	Label oscTopicEditorLabel;
	TextEditor oscHostEditor;
	Label oscHostEditorLabel;
	TextEditor oscPortEditor;
	Label oscPortEditorLabel;

	TextButton oscStartButton;
	TextButton oscStopButton;

    TextEditor midiMessagesBox;

	TextEditor errorLog;
    double startTime;

	OSCSender oscClient;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent);
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
