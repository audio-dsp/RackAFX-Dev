#ifndef __synthparamlimits__
#define __synthparamlimits__


//-----------------------------------------------------------------------------
// MultiFilter Parameters
//-----------------------------------------------------------------------------
enum {     	
	MIDI_PITCHBEND = 2048,
	MIDI_MODWHEEL,
	MIDI_VOLUME_CC7,
	MIDI_PAN_CC10,
	MIDI_EXPRESSION_CC11,
	MIDI_SUSTAIN_PEDAL,
	MIDI_CHANNEL_PRESSURE,
	MIDI_ALL_NOTES_OFF,
	PLUGIN_SIDE_BYPASS,

	NUMBER_OF_SYNTH_PARAMETERS // alsways last
};

#define MIN_FILTER_FC 80.0
#define MAX_FILTER_FC 18000.0
#define DEFAULT_FILTER_FC 10000.0
#define FILTER_RAW_MAP 0.5
#define FILTER_COOKED_MAP 1800.0

// --- define the HI, LO and DEFAULT values for our controls
// for all EG Int EXCEPT LFO->PAN Int
#define MIN_BIPOLAR -1.0
#define MAX_BIPOLAR 1.0
#define DEFAULT_BIPOLAR 0.0

#define MIN_UNIPOLAR 0.0
#define MAX_UNIPOLAR 1.0
#define DEFAULT_UNIPOLAR 0.0
#define DEFAULT_UNIPOLAR_HALF 0.5

// NOTE these are +/- units, so +/-1 to +/-12 semis
#define MIN_PITCHBEND_RANGE 1
#define MAX_PITCHBEND_RANGE 12
#define DEFAULT_PITCHBEND_RANGE 1	

// on/off switches
#define MIN_ONOFF_SWITCH 0
#define MAX_ONOFF_SWITCH 1
#define DEFAULT_ONOFF_SWITCH 0	

#define MIN_MIDI_VALUE 0
#define MAX_MIDI_VALUE 127
#define DEFAULT_MIDI_VALUE 64

#define DEFAULT_LEGATO_MODE 0
#define DEFAULT_RESET_TO_ZERO 0
#define DEFAULT_FILTER_KEYTRACK 0
#define DEFAULT_VELOCITY_TO_ATTACK 0
#define DEFAULT_NOTE_TO_DECAY 0
#define DEFAULT_MIDI_PITCHBEND 0
#define DEFAULT_MIDI_MODWHEEL 0
#define DEFAULT_MIDI_VOLUME 127
#define DEFAULT_MIDI_PAN 64
#define DEFAULT_MIDI_EXPRESSION 0

#endif	// __synthparamlimits__

