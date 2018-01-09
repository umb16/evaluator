#include "Interface.h"
#include "Evaluator.h"
#include "IControl.h"
#include "Controls.h"

#if SA_API
extern char *gINIPath;
#else

#ifdef OS_WIN
#include <windows.h>
#include <shlobj.h>
#include <sys/stat.h>
#else
#include "swell.h"
#endif

char *gINIPath = new char[200]; // path of ini file

void InitIniPath()
{
#ifdef OS_WIN
	if (SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, gINIPath) != S_OK)
	{
		DBGMSG("could not retrieve the user's application data directory!\n");

		//TODO error msg?
		return;
	}

	sprintf(gINIPath, "%s\\%s", gINIPath, BUNDLE_NAME); // Add the app name to the path

	struct stat st;
	if (stat(gINIPath, &st) == 0) // if directory exists
	{
		sprintf(gINIPath, "%s\\%s", gINIPath, "settings.ini"); // add file name to path
	}
#else
	const char *homeDir = getenv("HOME");
	sprintf(gINIPath, "%s/Library/Application Support/%s/", homeDir, BUNDLE_NAME);

	struct stat st;
	if (stat(gINIPath, &st) == 0) // if directory exists
	{
		sprintf(gINIPath, "%s%s", gINIPath, "settings.ini"); // add file name to path
	}
#endif
}
#endif

enum ELayout
{
	kEditorWidth = GUI_WIDTH,
	kEditorHeight = GUI_HEIGHT,
	kEditorMargin = 10,

	kPlugName_X = kEditorMargin,
	kPlugName_Y = 10,
	kPlugName_W = 100,
	kPlugName_H = 25,

	kVolumeLabel_X = kEditorMargin * 2,
	kVolumeLabel_Y = kPlugName_Y + kPlugName_H,
	kVolumeLabel_W = 30,
	kVolumeLabel_H = 15,

	kVolumeKnob_X = kVolumeLabel_X,
	kVolumeKnob_Y = kVolumeLabel_Y + kVolumeLabel_H,
	kVolumeKnob_W = kVolumeLabel_W,
	kVolumeKnob_H = kVolumeLabel_W,

	kBitDepthLabel_X = kVolumeLabel_X + kVolumeLabel_W + 15,
	kBitDepthLabel_Y = kVolumeLabel_Y,
	kBitDepthLabel_W = 30,
	kBitDepthLabel_H = kVolumeLabel_H,

	kBitDepth_X = kBitDepthLabel_X,
	kBitDepth_Y = kVolumeKnob_Y + 3,
	kBitDepth_W = kBitDepthLabel_W,
	kBitDepth_H = 20,

	// rect for label & buttons
	kRunModeLabel_X = kBitDepth_X + 45,
	kRunModeLabel_Y = kVolumeLabel_Y,
	kRunModeLabel_W = 110,
	kRunModeLabel_H = kVolumeLabel_H,

	// rect for label & buttons
	kRunMode_X = kBitDepth_X + 45,
	kRunMode_Y = kVolumeKnob_Y + 3,
	kRunMode_W = kRunModeLabel_W,
	kRunMode_H = 20,

	// rect for the tempo box in standalone
	kTempoLabel_X = kRunMode_X + kRunMode_W + 15,
	kTempoLabel_Y = kVolumeLabel_Y,
	kTempoLabel_W = 60,
	kTempoLabel_H = kVolumeLabel_H,

	kTempoBox_X = kTempoLabel_X,
	kTempoBox_Y = kVolumeKnob_Y + 3,
	kTempoBox_W = kTempoLabel_W,
	kTempoBox_H = 20,

	kVControl_W = 30,
	kVControl_H = 30,
	kVControl_S = kVControl_W + 10,
	// x coord of the *first* knob on the left
	kVControl_X = kEditorWidth - kEditorMargin * 2 - kVControl_S*(kVControl7 - kVControl0) - kVControl_W,
	kVControl_Y = kVolumeKnob_Y,

	kTResetLabel_X = kRunMode_X,
	kTResetLabel_Y = kVolumeKnob_Y + kVolumeKnob_H,
	kTResetLabel_H = 15,
	kTResetLabel_W = 45,

	kProgramLabel_X = kEditorMargin,
	kProgramLabel_Y = kTResetLabel_Y + kTResetLabel_H + 5,
	kProgramLabel_H = 20,
	kProgramLabel_W = 75,

	kProgramText_X = kEditorMargin,
	kProgramText_Y = kProgramLabel_Y + kProgramLabel_H,
	kProgramText_W = kEditorWidth - kEditorMargin * 2,
	kProgramText_H = 200,

	kCompilePrompt_W = 200,
	kCompilePrompt_H = 15,
	kCompilePrompt_X = kProgramText_X + kProgramText_W - kCompilePrompt_W,
	kCompilePrompt_Y = kProgramText_Y + kProgramText_H + 2,
	
	kTransport_X = kVControl_X + 40,
	kTransport_W = 40*3,
	kTransport_H = 25,
	kTransport_Y = kProgramText_Y - kTransport_H - 5,

	kConsoleTitle_X = kProgramText_X,
	kConsoleTitle_Y = kProgramText_Y + kProgramText_H + 10,
	kConsoleTitle_W = kProgramText_W,
	kConsoleTitle_H = 15,

	// the log window that shows the internal state of the expression
	kConsole_X = kProgramText_X,
	kConsole_Y = kConsoleTitle_Y + kConsoleTitle_H,
	kConsole_W = 375,
	kConsole_H = 140,

	kConsole_M = 5,   // margin
	kConsole_TH = 12,  // text height
	kConsole_TW = kConsole_W - kConsole_M * 2, // text width

	kWatchLabel_X = kConsoleTitle_X + kConsole_W + 10,
	kWatchLabel_Y = kConsoleTitle_Y,
	kWatchLabel_W = 25,
	kWatchLabel_H = 15,

	// rect for a watch text edit
	kWatchVar_X = kWatchLabel_X,
	kWatchVar_Y = kConsole_Y,
	kWatchVar_W = 50,
	kWatchVar_H = 12,
	kWatchVar_S = 2,

	// rect for the watch results
	kWatchVal_X = kWatchVar_X + kWatchVar_W + 5,
	kWatchVal_Y = kWatchVar_Y,
	kWatchVal_W = kEditorWidth - 10 - kWatchVal_X,
	kWatchVal_H = kWatchVar_H,

	kScopeTitle_X = kConsoleTitle_X,
	kScopeTitle_Y = kConsole_Y + kConsole_H + 10,
	kScopeTitle_W = 60,
	kScopeTitle_H = 15,

	kScopeWindowLabel_X = kScopeTitle_X + kScopeTitle_W,
	kScopeWindowLabel_Y = kScopeTitle_Y + 2,
	kScopeWindowLabel_W = 52,
	kScopeWindowLabel_H = 15,

	kScopeWindow_X = kScopeWindowLabel_X + kScopeWindowLabel_W,
	kScopeWindow_Y = kScopeWindowLabel_Y - 3,
	kScopeWindow_W = 15,
	kScopeWindow_H = 15,

	kScope_X = kScopeTitle_X,
	kScope_Y = kScopeTitle_Y + kScopeTitle_H,
	kScope_W = kProgramText_W,
	kScope_H = 125,

	kPresetPopup_X = 30,
	kPresetPopup_Y = kProgramText_Y,
	kPresetPopup_W = kEditorWidth - kPresetPopup_X * 2,
	kPresetPopup_H = kProgramText_H + kConsole_H + 15,

	kHelpButton_W = 15,
	kHelpButton_H = 15,
	kHelpButton_X = kEditorWidth - kEditorMargin - kHelpButton_W,
	kHelpButton_Y = kEditorMargin,

	// width of the "help" area, which we use to resize the window
	kHelpWidth = 350,

	kSyntaxLabel_X = kEditorWidth + kEditorMargin,
	kSyntaxLabel_Y = kPlugName_Y,
	kSyntaxLabel_H = kPlugName_H,
	kSyntaxLabel_W = kHelpWidth - kEditorMargin*2,

	kSyntax_X = kEditorWidth + kEditorMargin,
	kSyntax_Y = kSyntaxLabel_Y + kSyntaxLabel_H + 10,
	kSyntax_H = 375,
	kSyntax_W = kHelpWidth - kEditorMargin*2,
	
	kManualButton_X = kSyntax_X,
	kManualButton_Y = kSyntax_Y + kSyntax_H
};

// note: ICOLOR is ARGB
const IColor kBackgroundColor(255, 19, 44, 34);
const IColor kExprBackgroundColor(255, 32, 56, 59);
const IColor kTextColor(255, 218, 228, 226);
const IColor kGreenColor(255, 52, 209, 112);
// color of text set in the Text FG Color for expression text
const IColor kTextEditColor(kGreenColor);
const IColor kPlugNameColor(255, 62, 86, 89);
const IColor kConsoleBackgroundColor(255, 11, 26, 19);
const IColor kScopeBackgroundColor(255, 14, 21, 26);
const IColor kScopeLineColorLeft(255, 194, 218, 150);
const IColor kScopeLineColorRight(255, 60, 169, 198);

IText  kExpressionTextStyle(11,
	&kGreenColor,
	"Courier",
	IText::kStyleNormal,
	IText::kAlignNear,
	0, // orientation
	IText::kQualityDefault,
	&kExprBackgroundColor,
	&kTextEditColor
);

IText  kExprMsgTextStyle(11,
	&kTextColor,
	"Arial",
	IText::kStyleBold,
	IText::kAlignNear,
	0, // orientation
	IText::kQualityDefault);

IText  kConsoleTextStyle(11,
	&kGreenColor,
	"Courier",
	IText::kStyleNormal,
	IText::kAlignNear,
	0, // orientation
	IText::kQualityDefault);

IText  kWatchTextStyle(11,
	&kGreenColor,
	"Courier",
	IText::kStyleNormal,
	IText::kAlignNear,
	0, // orientation
	IText::kQualityDefault,
	&kExprBackgroundColor,
	&kGreenColor);

IText kLabelTextStyle(12,
	&kTextColor,
	"Arial",
	IText::kStyleBold,
	IText::kAlignCenter,
	0, // orientation
	IText::kQualityDefault);

IText kTitleTextStyle(16,
	&kTextColor,
	"Arial",
	IText::kStyleBold,
	IText::kAlignNear,
	0, // orientation
	IText::kQualityDefault);


static const char* kCompilePromptMessage =
#if defined(OS_OSX)
"Cmd+Enter to Compile Program";
#else
"Ctrl+Enter to Compile Program";
#endif

static const char* kLanguageSyntax =
"[]     audio input/output access\n"
"F      'frequency' unary operator\n"
"$      'sine' unary operator\n"
"#      'square' unary operator\n"
"T      'triangle' unary operator\n"
"R      'random' unary operator\n"
"V      V knob access operator\n"
"C      MIDI CC message access operator\n"
"@      memory access operator\n"
"!      logical NOT\n"
"~      bitwise NOT (complement)\n"
"*      multiplication\n"
"/      division\n"
"%      modulo (remainder)\n"
"+      unary plus and addition\n"
"-      unary minus and subtraction\n"
"<<     bitwise left shift (rhs % 64)\n"
">>     bitwise right shift (rhs % 64)\n"
"&      bitwise AND\n"
"^      bitwise XOR\n"
"|      bitwise OR\n"
"?:     ternary operator (non-branching)\n"
"a =    assign to a variable\n"
"@a =   assign to a memory address\n"
"[0] =  assign to left output\n"
"[1] =  assign to right output\n"
"[*] =  assign to all outputs\n";


Interface::Interface(Evaluator* plug, IGraphics* pGraphics)
	: mPlug(plug)
	, textEdit(nullptr)
	, programName(nullptr)
	, consoleTextControl(nullptr)
	, bitDepthControl(nullptr)
	, oscilloscope(nullptr)
	, transportButtons(nullptr)
	, timeResetLabel(nullptr)
	, timeResetToggle(nullptr)
{
	CreateControls(pGraphics);

#if !SA_API
	InitIniPath();
#endif
}

void Interface::CreateControls(IGraphics* pGraphics)
{
	pGraphics->AttachPanelBackground(&kBackgroundColor);

	pGraphics->AttachKeyCatcher(new MidiControl(mPlug));

	//--  Name of the plug and version
	{
		IText titleStyle(20, &kPlugNameColor, "Arial", IText::kStyleBold, IText::kAlignNear, 0, IText::kQualityDefault);
		pGraphics->AttachControl(new ITextControl(mPlug, MakeIRect(kPlugName), &titleStyle, "EVALUATOR" " " VST3_VER_STR));
	}

	//--- Text input for the expression ------
	{
		IRECT labelRect = MakeIRect(kProgramLabel);
		pGraphics->MeasureIText(&kTitleTextStyle, "PROGRAM:", &labelRect);
		pGraphics->AttachControl(new ITextControl(mPlug, labelRect, &kTitleTextStyle, "PROGRAM:"));

		const int labelWidth = labelRect.W() + 5;
		labelRect.L += labelWidth;
		labelRect.R += labelWidth;
		// some fudge to the top of the rect so this smaller text looks vertically centered on the bigger PROGRAM: label
		labelRect.T += 2;
		IText textStyle = kLabelTextStyle;
		textStyle.mAlign = IText::kAlignNear;
		programName = new ITextControl(mPlug, labelRect, &textStyle);
		pGraphics->AttachControl(programName);

		textStyle.mAlign = IText::kAlignFar;

		compilePrompt = new ITextControl(mPlug, MakeIRect(kCompilePrompt), &textStyle, kCompilePromptMessage);
		compilePrompt->Hide(true);
		pGraphics->AttachControl(compilePrompt);

		textEdit = new ITextEdit(mPlug, MakeIRect(kProgramText), kExpression, &kExpressionTextStyle, "", ETextEntryOptions(kTextEntryMultiline | kTextEntryEnterKeyInsertsCR));
		textEdit->SetNameDisplayControl(compilePrompt);

		pGraphics->AttachControl(textEdit);
	}

	//--- Label we show when the user is editing the program so they know they can commit the text with a keyboard stroke
	{
		
		
	}

	//-- "window" displaying internal state of the expression
	{
		pGraphics->AttachControl(new ITextControl(mPlug, MakeIRect(kConsoleTitle), &kTitleTextStyle, "AUTO"));
		IRECT LogRect = MakeIRect(kConsole);
		consoleTextControl = new ConsoleText(mPlug, LogRect, &kConsoleTextStyle, &kConsoleBackgroundColor, kConsole_M);
		pGraphics->AttachControl(consoleTextControl);
	}

	//-- watch window section
	{
		pGraphics->AttachControl(new ITextControl(mPlug, MakeIRect(kWatchLabel), &kTitleTextStyle, "WATCH"));

		IRECT watchVarRect = MakeIRect(kWatchVar);
		IRECT watchValRect = MakeIRect(kWatchVal);
		for (int i = 0; i < kWatchNum; ++i)
		{
			watches[i].var = new ITextEdit(mPlug, watchVarRect, kWatch + i, &kWatchTextStyle, "", kTextEntrySelectTextWhenFocused);
			watches[i].var->SetTextEntryLength(5);
			pGraphics->AttachControl(watches[i].var);
			watchVarRect.T += kWatchVar_H + kWatchVar_S;
			watchVarRect.B += kWatchVar_H + kWatchVar_S;
			
			watches[i].val = new ConsoleText(mPlug, watchValRect, &kConsoleTextStyle, &kConsoleBackgroundColor, 1);
			pGraphics->AttachControl(watches[i].val);
			watchValRect.T += kWatchVal_H + kWatchVar_S;
			watchValRect.B += kWatchVal_H + kWatchVar_S;
		}
	}

	// -- Oscilloscope display
	{
		pGraphics->AttachControl(new ITextControl(mPlug, MakeIRect(kScopeTitle), &kTitleTextStyle, "SCOPE"));
		oscilloscope = new Oscilloscope(mPlug, MakeIRect(kScope), &kScopeBackgroundColor, &kScopeLineColorLeft, &kScopeLineColorRight);
		pGraphics->AttachControl(oscilloscope);

		pGraphics->AttachControl(new ITextControl(mPlug, MakeIRect(kScopeWindowLabel), &kLabelTextStyle, "WINDOW"));

		IRECT updateRect = MakeIRect(kScopeWindow);
		IControl* updateControl = new KnobLineCoronaControl(mPlug, updateRect, kScopeWindow, &kGreenColor, &kGreenColor, 0.5);

		int w = updateRect.W() + 15;
		updateRect.L += w;
		updateRect.R += w;
		updateRect.T = kScopeWindowLabel_Y;
		IControl* caption = new ICaptionControl(mPlug, updateRect, kScopeWindow, &kLabelTextStyle);
		updateControl->SetValDisplayControl(caption);

		pGraphics->AttachControl(updateControl);
		pGraphics->AttachControl(caption);
	}

	//---Volume--------------------
	{
		//---Volume Label--------
		pGraphics->AttachControl(new ITextControl(mPlug, MakeIRect(kVolumeLabel), &kLabelTextStyle, "VOL"));

		//---Volume Knob-------
		IColor coronaColor(kGreenColor);
		pGraphics->AttachControl(new KnobLineCoronaControl(mPlug, MakeIRect(kVolumeKnob), kGain, &kGreenColor, &coronaColor, 1.5, 0, 14));
	}

	//---Bit Depth--------------
	{
		pGraphics->AttachControl(new ITextControl(mPlug, MakeIRect(kBitDepthLabel), &kLabelTextStyle, "BITS"));

		IText textStyle = kExpressionTextStyle;
		textStyle.mAlign = IText::kAlignCenter;
		IRECT backRect = MakeIRect(kBitDepth);
		IRECT textRect;
		pGraphics->MeasureIText(&textStyle, "000", &textRect);
		int HH = textRect.H() / 2;
		int HW = textRect.W() / 2;
		textRect.L = (int)backRect.MW() - HW;
		textRect.R = (int)backRect.MW() + HW;
		textRect.T = (int)backRect.MH() - HH;
		textRect.B = (int)backRect.MH() + HH;
		pGraphics->AttachControl(new TextBox(mPlug, backRect, kBitDepth, &textStyle, textRect));
	}

	// ---Run Mode------------------------------
	{
		pGraphics->AttachControl(new ITextControl(mPlug, MakeIRect(kRunModeLabel), &kLabelTextStyle, "RUN MODE"));
		IText textStyle = kExpressionTextStyle;
		textStyle.mAlign = IText::kAlignCenter;
		pGraphics->AttachControl(new EnumControl(mPlug, MakeIRect(kRunMode), kRunMode, &textStyle));
	}

	// ---MIDI note resets
	{
		IText textStyle = kLabelTextStyle;
		textStyle.mAlign = IText::kAlignNear;

		IRECT captionRect = MakeIRect(kTResetLabel);
		const char * label = mPlug->GetParam(kMidiNoteResetsTime)->GetNameForHost();
		pGraphics->MeasureIText(&textStyle, const_cast<char*>(label), &captionRect);

		timeResetLabel = new ITextControl(mPlug, captionRect, &textStyle, label);
		pGraphics->AttachControl(timeResetLabel);

		const int width = captionRect.W() + 5;
		captionRect.L += width;
		captionRect.R = captionRect.L + captionRect.H();

		timeResetToggle = new ToggleControl(mPlug, captionRect, kMidiNoteResetsTime, kExprBackgroundColor, kGreenColor);
		pGraphics->AttachControl(timeResetToggle);
	}

#if SA_API
	// tempo label and edit box
	{
		pGraphics->AttachControl(new ITextControl(mPlug, MakeIRect(kTempoLabel), &kLabelTextStyle, "BPM"));

		IText textStyle = kExpressionTextStyle;
		textStyle.mAlign = IText::kAlignCenter;
		IRECT backRect = MakeIRect(kTempoBox);
		IRECT textRect;
		pGraphics->MeasureIText(&textStyle, "000.00", &textRect);
		int HH = textRect.H() / 2;
		int HW = textRect.W() / 2;
		textRect.L = (int)backRect.MW() - HW;
		textRect.R = (int)backRect.MW() + HW;
		textRect.T = (int)backRect.MH() - HH;
		textRect.B = (int)backRect.MH() + HH;
		pGraphics->AttachControl(new TextBox(mPlug, backRect, kTempo, &textStyle, textRect));
	}

	// transport buttons
	{
		transportButtons = new TransportButtons(mPlug, MakeIRect(kTransport), kExprBackgroundColor, kGreenColor);
		pGraphics->AttachControl(transportButtons);
	}
#endif

	// ---V Control Knobs-------------------------
	{
		IRECT knobRect = MakeIRect(kVControl);
		IRECT labelRect = MakeIRect(kVolumeLabel);
		for (int paramIdx = kVControl0; paramIdx <= kVControl7; ++paramIdx)
		{
			labelRect.L = knobRect.L;
			labelRect.R = knobRect.R;

			ITextControl* label = new ITextControl(mPlug, labelRect, &kLabelTextStyle, mPlug->GetParam(paramIdx)->GetNameForHost());
			KnobLineCoronaControl* knob = new KnobLineCoronaControl(mPlug, knobRect, paramIdx, &kGreenColor, &kGreenColor, 1.5, 0, 14);

			pGraphics->AttachControl(label);
			pGraphics->AttachControl(knob);

			knobRect.L += kVControl_S;
			knobRect.R += kVControl_S;
		}
	}

	// --- Load/Save Buttons ---------------------------
	{
		IBitmap buttonBack = pGraphics->LoadIBitmap(BUTTON_BACK_ID, BUTTON_BACK_FN);
		int buttonX = kEditorWidth - kEditorMargin - buttonBack.W*2 - 10;
		const int buttonY = kProgramText_Y - buttonBack.H - 5;

		pGraphics->AttachControl(new SaveButton(mPlug, buttonX, buttonY, &buttonBack, &kLabelTextStyle, this));

		buttonX += buttonBack.W + 5;
		pGraphics->AttachControl(new LoadButton(mPlug, buttonX, buttonY, &buttonBack, &kLabelTextStyle, MakeIRect(kPresetPopup), &kConsoleTextStyle, this));		
	}

	// --- Syntax Reference Area -----------------------
	{
		IRECT backRect(kEditorWidth, 0, kEditorWidth + kHelpWidth, kEditorHeight);
		pGraphics->AttachControl(new IPanelControl(mPlug, backRect, &kConsoleBackgroundColor));
		
		IText textStyle = kExpressionTextStyle;
		textStyle.mAlign = IText::kAlignCenter;
		textStyle.mSize = 12;
		textStyle.mColor = kLabelTextStyle.mColor;
		pGraphics->AttachControl(new HelpButton(mPlug, MakeIRect(kHelpButton), &textStyle, this));
		
		IBitmap buttonBack = pGraphics->LoadIBitmap(BUTTON_BACK_ID, BUTTON_BACK_FN);
		textStyle = kLabelTextStyle;
		textStyle.mSize = 11;
		pGraphics->AttachControl(new ManualButton(mPlug, kManualButton_X, kManualButton_Y, &buttonBack, &textStyle, this));

		pGraphics->AttachControl(new ITextControl(mPlug, MakeIRect(kSyntaxLabel), &kLabelTextStyle, "LANGUAGE SYNTAX"));
		
		textStyle = kConsoleTextStyle;
#if defined(OS_OSX)
		textStyle.mSize += 2;
#endif
		pGraphics->AttachControl(new ITextControl(mPlug, MakeIRect(kSyntax), &textStyle, kLanguageSyntax));
	}
}


Interface::~Interface()
{
}

void Interface::SetDirty(int paramIdx, bool pushToPlug)
{
	switch (paramIdx)
	{
	case kBitDepth:
		if (bitDepthControl != nullptr)
		{
			bitDepthControl->SetDirty(pushToPlug);
		}
		break;

#if !SA_API
	case kRunMode:
		if (timeResetToggle != nullptr)
		{
			const bool toggleDisabled = mPlug->GetParam(kRunMode)->Int() == kRunModeProjectTime;
			timeResetLabel->Hide(toggleDisabled);
			timeResetToggle->Hide(toggleDisabled);
		}
#endif
	}
}

const char * Interface::GetProgramText() const
{
	return textEdit->GetText();
}

unsigned int Interface::GetProgramMemorySize() const
{
	return 1024 * 64;
}

void Interface::SetProgramName(const char* name)
{
	programName->SetTextFromPlug(const_cast<char*>(name));
}

const char * Interface::GetProgramName() const
{
	return programName->GetTextForPlug();
}

void Interface::SetProgramText(const char * programText)
{
	textEdit->TextFromTextEntry(programText);
}

void Interface::SetConsoleText(const char * consoleText)
{
	consoleTextControl->SetTextFromPlug(const_cast<char*>(consoleText));
}

void Interface::SetWatchValue(int idx, const char * watchText)
{
	if ( idx >= 0 && idx < kWatchNum )
	{
		watches[idx].val->SetTextFromPlug(const_cast<char*>(watchText));
	}
}

void Interface::UpdateOscilloscope(double left, double right)
{
	oscilloscope->AddSample(left, right);
}

int Interface::GetOscilloscopeWidth() const
{
	return oscilloscope->GetRECT()->W();
}

const char * Interface::GetWatch(int idx) const
{
	return watches[idx].var->GetText();
}

void Interface::SetWatch(int idx, const char * text)
{
	if (idx >= 0 && idx < kWatchNum)
	{
		watches[idx].var->SetTextFromPlug(text);
	}
}

void Interface::LoadPreset(int idx)
{
	mPlug->RestorePreset(idx);
	mPlug->InformHostOfProgramChange();
}

TransportState Interface::GetTransportState() const
{
	if (transportButtons != nullptr)
	{
		return transportButtons->GetTransportState();
	}
	
	return kTransportPlaying;
}

void Interface::ToggleHelp()
{
	if (mPlug->GetGUI()->Width() == kEditorWidth)
	{
		mPlug->GetGUI()->Resize(kEditorWidth + kHelpWidth, kEditorHeight);
	}
	else
	{
		mPlug->GetGUI()->Resize(kEditorWidth, kEditorHeight);
	}
}

bool Interface::GetSupportPath(WDL_String* outPath) const
{
#if defined(OS_WIN)
	outPath->SetLen(256);
	GetPrivateProfileString("install", "support path", NULL, outPath->Get(), 256, gINIPath);

	return true;
#else
	mPlug->GetGUI()->AppSupportPath(outPath, true);
	outPath->Append("/Evaluator");
	return true;
#endif
}
