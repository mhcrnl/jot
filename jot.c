//
//Copyright (C) 2013 Jon. Easterbrook
//This programme, it's associated scripts and documentation is free software.
//It is distributed in case someone might find it useful,
//it can be modified and redistributed without restriction.
//No warrenty is given or implied for whatever purpose.
//

#define _GNU_SOURCE
#define GNU_SOURCE
#if defined(CHROME)
#define LINUX
#define NOWIDECHAR
#endif

#if defined(VC)
#define _UNICODE
#define NoRegEx
#define nolstat
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <malloc.h>
#include <windows.h> 
#include <tchar.h>
#include <strsafe.h>
#include <locale.h>
#include <direct.h>
#include <io.h>
#include <WinBase.h>
#include "..\wine\libgw32c-0.4\include\glibc\search.h"
//#include "unistd.h"
//Dreadful tack. This is defined in unistd.h and not in winsock.h, but winsock.h conflicts with unistd.h ... I give up.
#define W_OK 2
 
#elif defined(LINUX)
#if defined(CHROME)
#include "/home/chronos/user/toolchain/usr/include/ncursesw/curses.h"
#include "/home/chronos/user/toolchain/usr/include//wchar.h"
#else
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Shell.h>
#endif
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <dirent.h>
#include <search.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>
#include <ctype.h>
#include <setjmp.h>
#if defined(CYGWIN)
#include <ncurses6/curses.h>
#else
//#include <ncurses/curses.h>
#include <curses.h>
#endif
 
#elif defined(ANDROID)
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <wchar.h>
#include <ctype.h>
#include <locale.h>
#endif 

#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
 
#ifndef NoRegEx
#include <regex.h>
#endif
 
#if defined(VC)
#define KBD_NAME "CON"
#define NULL_STREAM "NUL"
 
#define RubOut     0x08
#define Shift_key  0x10
#define Ctrl_key   0x11
#define Alt_key    0x12
 
//Take a look at WinCon.h to make sense of this.
#define Normal_Text        ( FOREGROUND_INTENSITY | FOREGROUND_GREEN  | FOREGROUND_BLUE  | FOREGROUND_RED )
#define Selected_Substring ( FOREGROUND_INTENSITY | FOREGROUND_RED )
#define Current_Chr        ( FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_GREEN  | BACKGROUND_BLUE  | BACKGROUND_RED  )
#define Reverse_Video      (                                                                 BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED )
 
#elif defined(LINUX)
#define KBD_NAME "/dev/tty"
#define NULL_STREAM "/dev/null"
#define RubOut 0x7F
 
#define Normal_Text A_NORMAL            //0 in the debugger.
#define Selected_Substring A_UNDERLINE  //262144 in the debugger.
//#define Current_Chr A_REVERSE           //131072 in debugger.
#define Current_Chr A_STANDOUT          //
#define Reverse_Video A_REVERSE         //  -- ditto --

#elif defined(ANDROID)
#define KBD_NAME "/dev/tty"
#define NULL_STREAM "/dev/null"
#define RubOut 0x7F
 
#define Normal_Text        999
#define Selected_Substring 999
#define Current_Chr        999
#define Reverse_Video      999
#endif
  
//GetBuffer() CreateOrDelete arg.
#define AlwaysNew 0      //If there was a preexisting buffer GetBuffer will check for edit locks then clear out the old buffer.
#define OptionallyNew 1  //If there is a preexisting buffer GetBuffer will return it as is.
#define NeverNew 2       //If there is no preexisting buffer GetBuffer will return NULL.
 
//Command-mode bits:
#define Screen_ToggleScreen      0x00000001      //Reverts to command mode temporarily for each new hotkey.
#define Screen_ScreenMode        0x00000002      //Currently in screen mode.
#define Screen_OverTypeMode      0x00000004      //Typing into screen overwrites preexisting text.

#ifndef VERSION_STRING 
#define VERSION_STRING "Test version"
#endif

#define TRUE 1
#define FALSE 0
#define StringMaxChr 1024         //The largest string that can be accomodated by most static and local strins.
#define BucketSize 1000           //Size of temporary buffer used by ReadUnbufferedStreamRec and ReadNewRecord.
#define StackSize 100             //The default stack maximum.
#define MaxAutoTabStops 1000      //The maximum no. of tabstops that may be automatically assigned.
#define ScreenTableSize 100       //The maximum screen height (no. of lines).
#define EscapeSequenceLength 8    //Maximum no. of characters allowed in an escape sequence.
#define ASCII_Escape 0x1B  
#define Control_D 'D'-64  
#define Control_U 'U'-64  
#define UNTRANSLATED_CHR 1        //Returned by TransEscapeSequence() when a control character is not recognized
                                  //behaviour now is to assume it's unicode and enter it into the command string.
#define CTRL_C_INTERRUPT 2        //Returned by TransEscapeSequence() when a Ctrl+C interrupt is received.
  
#define FirstRecord_Flag 0X1      //This record is the first received from the input stream.
#define LastRecord_Flag  0X2      //This record was followed by an EOF from the input stream.

//Trace-vector settings (g_TraceMode)
#define Trace_AllCommands  0x0001 //Traces all commands.
#define Trace_CommandLines 0x0002 //Traces at start of new command line.
#define Trace_Failures     0x0004 //Traces command following a failing command.
#define Trace_Macros       0x0008 //Traces on entry to macros.
#define Trace_Scripts      0x0010 //Traces on entry to command-file scripts.
#define Trace_Int          0x0020 //Traces Ctrl+C interrupts.
#define Trace_Backtrace    0x0800 //Backtraces to ? buffer.
#define Trace_Stack        0x1000 //Dumps stack at each trace event.
#define Trace_Print        0x2000 //Prints current line at each trace event.
#define Trace_Source       0x4000 //Displays line of source code at each trace event.
#define Trace_Break        0x8000 //Break before command triggering a trace event.
  
#define PathDelimChr '/'          //Path-element delimiter.
#define ExtnDelimChr '.'          //file-name extention delimiter.

//Tag type codes.
#define TextType       0x01       //This tag is a text string - Tag->Attr points to the string.
#define TargetType     0x02       //The text is readonly at this point the Tag->Attr points to a HtabEntry.
#define ColourType     0x03       //The text is to be displayed in the forground/background - Tag->Attr points to the text string.
#define FontType       0x04       //When output to a relevant file type, the text is to be written in the font specified in the FontTag - Tag->Attr points to FontTag.
  
//Values for AdjustMode in FreeRec() - determines where any adjustable target points end up.
#define DeleteNotAdjust 0         //As it's name suggests - the TargetTag gets deleted.
#define AdjustForwards 1          //Any adjustable target tag gets shifted to the next record except for last record.
#define AdjustBack 1              //Any adjustable target tag gets shifted to the previous record except for first record.
  
struct Tag {                      //A member of a records tag chain - attributes (readonly, for/background colour and font attributes).
  struct Tag * next;              //The next member in the ordered list of tags on this Rec.
  int StartPoint;                 //The first (or only, in the case of target-types) character that this tag applies to.
  int EndPoint;                   //The end point for colour, text or font tags.
  void * Attr;                    //Points to attribute descriptor, where applicible - a target point of a hash-table, a Font or a text string.
  unsigned char Type;             //The tag type see above for type codes.
  };
  
//ColourTags are read by JotUpdateWindow - this structure optimizes screen updates.
struct ColourTag {                //This tag defines the forground, background and selected-substring colours.
  short Foreground;               //The foreground colour.
  short Background;               //The background colour.
  char TagName[12];               //The name of the colour pair
  int ColourPair;                 //In linux, the curses colour-pair id, in windows a colour pair.
  struct ColourTag * next;        //The next colour tag in the chain.
  };
  
//If ever FontTags get properly implemented the actual structure may be more like the ColourTags - e.g. no EndPoint but a strt tag and a stop tag.
struct FontTag {                  //This tag defines font name and size for the tagged text.
  char * FontName;                //The name of the font.
  int FontSize;                   //The font size (Pt).
  };
  
////More than one TextTag is allowed to ba active, hence their validity must be controlled with with an explicit EndPoint
//struct TextTag {                  //This tag defines a text string and the limit of it's validity.
//  char * Text;                    //The text string.
//  int EndPoint;                   //The last character for which the text is valid. 
//  };
  
struct HtabEntry {                //Contains a full description of current entry - for use with hashtables.
  char * HashKey;                 //The hashtable key associated with this entry.
  struct Rec * TargetRec;         //Focus-point record, if NULL the entry is deemed to have been deleted.
  int CurrentChr;                 //Focus-point character no.
  int LineNumber;                 //Focus-point line no.
  struct HtabEntry * next;        //Points to next member in the chain.
  char BufferKey;                 //Focus-point buffer.
  char HashBufKey;                //The hash-table buffer key.
  };

//Values for DisplayFlag in Rec struct.
#define Redraw_None 0             //This line is accurately represented on the screen already.
#define Remove_Cursor 1           //This line previously contained the cursor/current substring and these characters need redrawing.
#define Redraw_Line 2             //This line has been redefined and needs completely rewriting.

struct Rec {                      //A record belonging to a Buf's record ring.
  struct Rec * prev;              //The previous Rec in this buffer's record chain - n.b. the chain is circular, FirstRec->prev points to the last Rec in the chain.
  struct Rec * next;              //The next Rec in this buffer's record chain.
  int length;                     //The maximum allowable length for this record (the original malloc allocation).
  struct Tag * TagChain;          //Chain of Tag elements in text order.
  char * text;                    //The text string for this record.
  char DisplayFlag;               //Indicates to JotUpdateWindow that this line needs redrawing.
  };

//Edit permissions (for the EditLock field):
//#define Unrestricted 0            //Like it says - no restrictions.
#define ReadOnly 1                //No changes to this buffer allowed.
#define WriteIfChanged 2          //If changed then ensure these are written before exiting the editor session.

//Modification status bits (for the UnchangedStatus field) - the UnchangedStatus byte is set to 0X00 every time there is a change to the buffer. 
#define SameSinceIO       0x01    //Set to 1 when originally read (or defined by %Q, %E etc.) set again by a write.
#define SameSinceIndexed  0x02    //Set to 1 After a hash-table entry is made.
#define SameSinceCompiled 0x04    //Set to 1 When macro is compiled.
#define SameFlag1         0x08    //Set by user %q[<key>]=UserFlag1 [0|1]
 
//FileType tag values, any positive values indicate binary-mode record length.
#define ASCII 0
    
//Buf->HashtableMode has these possible states:
#define NoHashtable 0             //No hashtable in this buffer.
#define ProtectEntries 1          //May change or delete any text except the hash-table target strings.
#define AdjustEntries 2           //If a hash-table target is removed, the entry is silently adjusted to point to somewhere nearby.
#define DeleteEntries 3           //If a hash-table target is removed, the entry is silently removed from the hashtable.
#define DestroyHashtables 4       //If a hash-table target is removed, the whole hashtable is silently destroyed.

struct Buf {
  struct Rec *FirstRec;           //The first Rec in the buffer.
  struct Rec *CurrentRec;         //The Rec that contains the current character.
  int CurrentChr;                 //The current character offset.
  int SubstringLength;            //The length of the currently-selected substring.
  int LineNumber;                 //The line number of the current Rec in this buffer.
  int OldLastLineNumber;          //The line number of the last line in the open window (i.e. the window that follows the current buffer).
  struct Buf *NextBuf;            //The next buffer in the g_BufferChain.
  int * TabStops;                 //Tabstop array - TabStops[0] = no. of tabstops.
  int LeftOffset;                 //View left offset when displaying long lines, normally 0, any nagative value means auto LeftOffset.
  struct Com * Predecessor;       //The buffer was called as a macro by Com (or NULL).
  char * PathName;                //The original pathName.
  char * Header;                  //Header to be displayed above window.
  struct ColourTag * FirstColTag; //A list of colour-tag objects associated with this buffer.
  struct HtabEntry * FirstEntry;  //Chain of Entry structs.
  struct hsearch_data * htab;     //Hash table associated with this buffer.
#if defined(VC)
  int CodePage;                   //Code page to be used for this buffer.
#endif
  char AbstractWholeRecordFlag;   //When inserting this text  (i.e. Here command) into some other buffer, the inserted text goes at the start of the current word.
  char UnchangedStatus;           //The buffer has not been changed since it was read/indexed.
  char EditLock;                  //The requested edit permission for buffer. One of Unrestricted (the default), WriteIfChanged and ReadOnly.
  char HashtableMode;             //Defines behaviour when targets of the hashtable are to be deleted (see above for values).
  char NewPathName;               //When TRUE, the PathName has been changed - triggers an update in the display.
  char NoUnicode;                 //When TRUE, turns off unicode support.
  char FileType;                  //File type identified from FileType or defined explicitly to ReadUnbufferedChan()
  char AutoTabStops;              //Assign tabstops automatically, when TRUE.
  char BufferKey;                 //The key associated with this buffer.
  };

//Types of arguments in an Arg struct:
#define NumericArg 1
#define StringArg 2
#define DeferredStringArg 3
#define BlockArg 4

struct Arg {                      //One element in argument list, next = NULL at end of list.
  long long    intArg;            //Numeric Arg
  void *       pointer;           //Points to string/data-struct arg or VOID.
  struct Arg * next;              //Points to next arg in chain.
  char         type;              //Must be one of these: NumericArg, StringArg, DeferredStringArg or BlockArg.
  };

struct Com {                      //Stores one command and pointer to argument list. 
  int          CommandKey;        //Identifies command type.
  struct Arg * ArgList;           //Points to first arg in list.
  struct Com * NextCommand;       //Points to next command in sequence.
  struct Buf * CommandBuf;        //Points to buffer holding the source.
  struct Rec * CommandRec;        //Points to record with command line.
  int          CommandChrNo;      //Chr. no. of command in line.
  int          ComChrs;           //No. of characters in command source, including arguments.
  int          CommandLineNo;     //Line no. of command in macro.
  };

struct Seq {                      //Sequence of instructions with possible ELSE sequence. 
  struct Com * FirstCommand;      //First in a sequence of commands.
  struct Seq * ElseSequence;      //Sequence to be obayed in event of failure.
  struct Seq * NextSequence;      //next sequence to be obayed.
  };

struct Block {                    //Allows implementation of repeated block.
  struct Seq * BlockSequence;     //The sequence associated with this block.
  int          Repeats;           //Number of times to repeat this sequence.
  };

struct BacktraceFrame {           //These are chained to provide a simple backtrace diagnostic.
  struct BacktraceFrame * prev;   //Points back to predecessor frame.
  struct Com * ParentCommand;     //The command triggering the context change.
  struct CommandFile *EditorInput;//The active command file  where appropriate.
  char Type;                      //See below for valid type codes.
  };
#define ScriptFrame 1             //The command is from a %r=<scriptPath>
#define MacroFrame 2              //The command is from a '<key> macro call.
#define CallFrame 3               //The command is from a %h'=call <functionName>
#define ConsoleCommand 4          //The command was read from the console or an -asConsole script.
  
static struct BacktraceFrame * s_BacktraceFrame = NULL; //Points to first member of backtrace chain.

#define StackVoidFrame 0
#define StackIntFrame 1
#define StackFloatFrame 2
#define StackBufFrame 3

struct intFrame {                   //One frame in the stack - (long long) is assumed to be the buggest of (long long), (double) and (struct Buf *).
  long long Value;                  //The int value.
  char type;                        //One of StackIntFrame, StackFloatFrame or StackBufFrame - always StackIntFrame for intFrame.
  };
struct floatFrame {                 //One frame in the stack - used only to cast the value field as (double).
  double fValue;                    //The floating-point value object - same size as long long.
  char type;                        //Always StackFloatFrame
  };
struct bufFrame {                   //One frame in the stack - used only to cast the value field as (struct Buf *).
  struct Buf * BufPtr;              //Points to buffer - normally 4 bytes ... but no one's making any promises.
  char Padding[sizeof(long long)-sizeof(struct Buf * )];
  char type;                        //Always StackBufFrame
  };

struct Window {
  struct Window * NextWindow;     //The next window on the screen (in %w order).
  int OldLastLineNumber;          //The line number of the last record displayed in this window.
  int LastSubstringStart;         //The start point of the selected substring last displayed in this window.
  int LastSubstringLength;        //The length of the selected substring last displayed in this window.
  char WindowHeight;              //The number of lines displayed in this window.
  short int DisplayName;          //When true, indicates that JotUpdateWindow should follow the window with the buffer details in reverse video.
  short int TextHeight;           //No. of lines actually displayed, normally TextHeight==WindowHeight except when displaying very small buffers.
  char WindowKey;                 //Key for the buffer associated with this window (if '\0' then JotUpdateWindow takes the g_CurrentBuf).
  char LastKey;                   //Key for the last buffer displayed in buffer.
  };

struct CommandFile {
  FILE *FileHandle;               //The filehandle from open( ...).
  struct CommandFile *ReturnFile; //The CommandFile that initiated this command file, at the bottom of the chain is g_EditorInput.
  struct Buf *CommandBuf;         //The command buffer associated with this command file.
  struct Buf *OldCommandBuf;      //The command buf that launched this  command file.
  int LineNo;                     //Counts the line numbers for error reporting.
  char *FileName;                 //The pathName of the command file.
  struct CommandFile * Pred;      //The next command file in the chain or NULL;
  char asConsole;                 //From the -asConsole qualifier.
  };
 
struct PathElemDesc {
  char * Start;
  int Length; };

struct BucketBlock {              //Used to buffer reads from unbuffered streams.
  int NextRecStartChr;            //The start of the next record in this bucket.
  int BufEndChr;                  //The last byte in the bucket.
  struct BucketBlock *Next;       //Next bucket in chain.
  char Bucket[BucketSize];        //The read bucket.
  };

#if defined(VC)
static int s_DefaultColourPair = Normal_Text;
static int s_CodePage = 65001;    //That's UTF-8 if you need to know.
DWORD fdwMode, fdwOldMode; 
HANDLE hStdout, hStdin;            //The handles used by low-level I/O routines read() and write() to access stdin/stdout.
CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
WORD wOldColorAttrs; 
HANDLE OpenWindowsCommand(char *);
BOOL Ctrl_C_Interrupt(DWORD);
struct Buf * ReadBuffer(FILE *, int, HANDLE, char *, char, struct Buf *, int);
 
#else
static int s_DefaultColourPair;
static char * s_Locale;
void Ctrl_C_Interrupt(int);
struct Buf * ReadBuffer(FILE *, int, int, char *, char, struct Buf *, int);
#endif

#if defined(VC)
int ReadBucket(HANDLE , char * );
#else
int ReadBucket(int , char * );
#endif
#if defined(VC)
int ReadUnbufferedStreamRec(struct Buf * , HANDLE , int);
#else
int ReadUnbufferedStreamRec(struct Buf * , int , int);
#endif

void TextViaJournal(struct Buf * , char * , char * );
void FreeTarget(struct Buf * , struct HtabEntry *);
void FreeTag(struct Rec * , struct Tag * );
int ReadNewRecord(struct Buf *, FILE *, int);
int MoveDown(struct Buf *, int, char *);
int Backtrace(struct Buf * );
int Run_Sequence(struct Seq * );
int Run_Block(struct Block * );
int JotGetPerceivedChr(struct Buf * );
int DoublePopen(struct Buf * , char * );
int GetChrNo(struct Buf *);
char CheckNextCommand(struct Buf * , char * );
struct Block * JOT_Block(struct Block *, struct Buf *, int);
void StartEdit(char * , struct Buf * );
void ErrorExit();
void ExpandEnv(char *);
void DestroyHtab(struct Buf *);
void AddFormattedRecord(struct Buf * , char *String, ...);
int JotGetCh(FILE * );

struct Buf * QueryKey(char * , struct Buf * );
void AddTag(struct Rec *, struct Tag *);
void DumpStack();
int KillTop();
char * BinFind(char *, int, char *, int);
int BinCmp(char * , char * , int );
void UpdateJournal(char * , char * );
void JotDeleteChr(int, int);
void JotInsertChr(char);
void JotAddString(char *);
void JotSetAttr(int);
void JotClearToEOL();
void JotAddBoundedString(char *, struct ColourTag *, int, int, int *, int *);
int JotStrlenBytes(char *, int );
int JotStrlenChrs(char *, int );
int JotAddChr(char);
void JotGotoXY(int, int);
void JotScroll(int, int, int);
void PushInt(long long);
void PushFloat(double );
void PushBuf(struct Buf * );
void SetupTerm();
void InteractiveEditor(void);
int GetBreakArg(struct Buf *, int);
int GetBreakArg(struct Buf *, int);
int GetBreakArg(struct Buf *, int);
int GetBreakArg(struct Buf *, int);
int GetBreakArg(struct Buf *, int);
int GetBreakArg(struct Buf *, int);
void ExpandDeferredString(struct Buf * , char * );
void ErrorExit();
void DisplaySource(struct Com *, char *);
int AdvanceRecord(struct Buf *, int);
void BreakRecord(struct Buf *);
void ClearRecord(struct Buf *);
void DuplicateRecord(struct Buf *, struct Buf *);
char * ExtractKey(struct Buf *, struct Buf *);
int FreeBuffer(struct Buf *);
void FreeRecord(struct Buf *, int);
long long GetDec(struct Buf *);
int GetHex(struct Buf *);
void GetRecord(struct Buf *, int);
int CheckBufferKey(char );
struct Buf *GetBuffer(int, int);
char ImmediateCharacter(struct Buf *);
void JoinRecords(struct Buf *);
int ShiftRight(struct Buf *, int);
void MoveRecord(struct Buf *, struct Buf *);
void Abstract(struct Buf *, struct Buf *, int, int, char );
char NextCommand(void);
char NextNonBlank(struct Buf *);
void ResetBuffer(struct Buf *);
int SearchRecord(struct Buf *, char *, int Direction);
void SetPointer(struct Buf *, int);
char StealCharacter(struct Buf *);
void SubstituteString(struct Buf *, char *, int);
int VerifyAlpha(struct Buf *);
int VerifyCharacter(struct Buf *, int);
int VerifyDigits(struct Buf *);
int VerifyNonBlank(struct Buf *);
void UpdateWindowList(struct Buf * );
void JotDebug(char *);
void DisplayRecord(struct Buf *, int);
int JotTidyUp();
int TidyUp(int, char *);
void SynError(struct Buf * , char *, ...);
void Fail(char *, ...);
void RunError(char *, ...);
void Disaster(char *, ...);
void Message(struct Buf * , char *, ...);
void PriorityPrompt(char *, ...);
int Prompt(char *, ...);
void InitializeEditor(int , char *[], char ** );
int Noise(void);
int SwitchToComFile(char *, char *, char *);
char ReadCommand(struct Buf *, FILE *, char *, ...);
int TransEscapeSequence(struct Buf *, FILE *, unsigned char , char * );
int WriteString(char *, int, int, int, int, int,  struct ColourTag *, int []);
int JotUpdateWindow(void); 
void NewScreenLine(void);
char VerifyKey(int);
int NewCommandFile(FILE *, char *);
int EndCommandFile();
int ChrUpper(char);
void ExitNormally(char *);
void DumpSequence(struct Seq *, char *);
void FreeCommand(struct Com **);
void FreeSequence(struct Seq **);
void FreeBlock(struct Block **);
long long GetArg(struct Buf *, long long);
void AddBlock(struct Com *, struct Block *);
void AddNumericArg(struct Com *, long long);
void AddStringArg(struct Com *, struct Buf *);
void AddPercentArg(struct Com *, struct Buf *);
long long FetchIntArg(struct Arg **);
struct Block * FetchBlock(struct Arg **);
char * FetchStringArg(struct Arg **, char *);
int QuerySystem(struct Buf * , char );
void JOT_Sequence(struct Buf *, struct Seq * , int);
int ReadString(char * , int , FILE *);
//int ReadChr(FILE *);
#if defined(VC)
void OpenTextFile(int *, HANDLE **, char [], char *, char *, char *);
#else
void OpenTextFile(int *, FILE **, char [], char *, char *, char *);
#endif
void DisplayDiag(struct Buf *, int, char *);
long long PopInt();
double PopFloat();
int DoHash(struct Buf * , char );
int ParseSort(char * , int );
 
static int s_MouseMask = 0;                           //Controls mouse event handling.
static int s_DebugLevel = 0;                          //Shows nesting of debugger calls.
static int s_RecoveryMode = FALSE;                    //Disables %O writing and %I reads path from recovery script when set.
static int s_GuardBandSize = 0;                       //Scrolling guardbands at top and bottom of window.
static char * s_ModifiedCommandString = NULL;         //Used to redefine the keyboard input line from %s=commandstring
static int s_NewFile = FALSE;                         //Indicates that the -new arg was given - creates a new file without reading.
static int s_NoUnicode = FALSE;                       //Indicates state of NoUnicode flag in buffer currently being displayed.
//There's also g_TTYMode                              Disables curses and uses normal characters to indicate cursor position in line-editing mode.
static int s_StreamOut = FALSE;                       //Indicates that stdout is a stream.
static int s_StreamIn = FALSE;                        //Indicates that stdin is stream to read into the primary buffer.
static int s_ObeyStdin = FALSE;                       //Set by -Obey CLI qualifier - the stdin stream is commands.
static struct BucketBlock * s_CurrentBucketBlock = NULL; //Buffer block for ReadUnbufferedStreamRec
static int s_BombOut = FALSE;                         //A an error or an interrupt has occured - causes all call frames to unwind tidily and silently.
static char * s_JournalPath = NULL;                   //Directory holding journal files.
static FILE * s_JournalHand = NULL;                   //File handle for fournal file.
static int s_FileCount = 0;                           //Used to uniquify file names in journal dir - in case a file is read, then written and then read back in again.
static struct Buf * s_InitCommands = NULL;            //Buffer holding -init command sequence.
static int s_HoldScreen = FALSE;                      //Set by -Hold - holds the screen on exit.

//static struct intFrame g_Stack[StackSize+2]; 
static struct intFrame * g_Stack; 
static struct Window *g_FirstWindow;
static char * s_ScreenTable[ScreenTableSize];         //s_ScreenTable[1] -> text in line 1 of screen, ...

//Bitmasks for s_Verbose:
#define NonSilent 1                                   //Normal reporting of errors I/O etc.
#define QuiteChatty 2                                 //Reports more events and explains some failures.
#define AnnoyingBleep 4                               //Emits an annoying bleep on errors.
static int s_Verbose = 1;
static char g_TableSeparator = '\t';
static int g_ScreenMode = 0;
static int g_StackSize = 100;
static int g_StackPtr;
static int g_Failure;
static int g_CaseSensitivity;
static struct Buf *g_BufferChain;
static struct Buf *g_CommandBuf;
static struct Com *g_CurrentCommand = NULL;
static int g_FirstWindowLine;                     //The screen line no. of the first line in the next (or first) line in the window. 
static int g_LastWindowLine;                      //The screen line no. of the last line of the current window.
struct Buf *g_CurrentBuf;                         //The buffer that's currently the focus of the editor.
static struct CommandFile *g_EditorInput;         //The current command stream - could be g_EditorConsole or some jot script initiated by %r
static struct CommandFile *g_asConsole;           //If set, this is a command file being run as -asConsole.
static struct CommandFile *g_EditorConsole;       //The console stream - usually stdin.
static FILE *g_EditorOutput;                      //Destination stream for p commands.
//static struct Buf *g_LastDefaultWindow;
static int g_TraceMode;                           //Controls the editor debugger.
static int g_DefaultTraceMode =                   //Value for g_TraceMode that's assigned by the T command.
  Trace_Stack | Trace_Print | Trace_Break | Trace_AllCommands;
static int g_TTYMode;
static int g_ScreenHeight = 0;
static int g_ScreenWidth = 0;
static char * g_DefaultComFileName = "${JOT_HOME}/coms/startup.jot"; 
static int g_FileFlags = 0;
static int g_x, g_y;
static int g_MouseBufKey, g_MouseLine, g_Mouse_x, g_Mouse_y;
static char s_CommandChr;
static char s_InsertString[StringMaxChr];
static char s_SearchString[StringMaxChr];
static char s_QualifyString[StringMaxChr];
static char s_PromptString[StringMaxChr];
static char s_ArgString[StringMaxChr];
static int s_LastCurrentCharacter = 0;
static int s_NoteLine;
static int s_NoteCharacter;
static int s_TraceCounter = 0;
static struct Buf * s_NoteBuffer = NULL;
static struct Buf * s_TempBuf;                    //Used for expanding deferred arguments.
static struct Buf * s_DebugBuf;                   //Command buffer for the jot debugger.
static int s_AbstractWholeRecordFlag = TRUE;
static struct Buf * s_PrevDebugBuf;
//static struct Rec * s_PrevDebugRec;
//static struct Seq * s_PrevDebugSeq;
 
static int s_IndentLevel = 0;
static int s_InView;

#if defined(LINUX)
static WINDOW *mainWin = NULL;    //In non-curses implementations this is set to -1 - assumed to be NULL in g_TTYMode.
short s_NextColourPair;           //The next-available colour-pair ID number.
#endif


//----------------------------------------------TidyUp
int TidyUp(int status, char * ExitMessage)
  { // Editor-only TidyUp procedure called by Tripos. 
  if (JotTidyUp()) //Only fails if there are writeifchanged buffers to be saved.
    return 0;
  if (ExitMessage && s_Verbose)
    fprintf(stdout, "%s\n", ExitMessage);
  exit(status);
  } // TidyUp procedure ends.

//----------------------------------------------main
int main(int ArgC, char * ArgV[])
  { // Outer procedure - calls editor. 
  char * StartupFileName = NULL;
   
  InitializeEditor(ArgC, ArgV, &StartupFileName); 
  StartEdit(StartupFileName, s_InitCommands);
   
  //It only gets here if something has exited abnormally.
  FreeBuffer(s_InitCommands);
  s_InitCommands = NULL;
  TidyUp(0, "Abnormal exit");
  return 0; }

//----------------------------------------------InitializeEditor
void InitializeEditor(int ArgC, char *ArgV[], char ** StartupFileName)
  { // Editor initialization for editing and application use. 
  int Arg = 0;
  int CommandBufSize = 20;
  char *FileName = NULL;
  char *ToFile = NULL;
  char ActualFile[1000];
  struct Seq *FilterSequence = NULL;
  struct Buf * FilterCommands = NULL;
#if defined(VC)
  SMALL_RECT SrWindow;
#else
  
  if ( ! (s_Locale = getenv("XTERM_LOCALE")))
    s_Locale = "en_US.ISO8859-1";
  setlocale(LC_CTYPE, s_Locale);
#endif
  g_TTYMode = FALSE;
  s_StreamOut = ! isatty(fileno(stdout));
  s_StreamIn = ! isatty(fileno(stdin));
  g_FileFlags = 0;
  g_EditorInput = NULL;
  s_PromptString[0] = '\0';
  s_TempBuf = GetBuffer('t', AlwaysNew);
  GetRecord(s_TempBuf, StringMaxChr);
  s_DebugBuf = GetBuffer('d', AlwaysNew);
  GetRecord(s_DebugBuf, StringMaxChr);
  s_InsertString[0] = '\0';
  s_SearchString[0] = '\0';
  s_QualifyString[0] = '\0';
  g_StackSize = StackSize;
  g_StackPtr = 0;
  g_asConsole = NULL;
  
  while (++Arg < ArgC) // Argument extraction loop.
    if (ArgV[Arg][0] != '-') { //Argument looks like a filespec.
      if (FileName != NULL)
        Disaster("Multiple filenames \"%s\" and \"%s\"\n", FileName, ArgV[Arg]);
      else
        FileName = ArgV[Arg]; }
  
    else {
      int EqChr;
      
      for (EqChr = 0; EqChr <= strlen(ArgV[Arg]); ) 
        if (ArgV[Arg][EqChr++] == '=')
          break;
      
      switch (toupper(ArgV[Arg][1])) { // Qualifier switch block.
        //  -                               - edit stdin stream.
        //  -Codepage                       - Sets windows code page for display (Windows only).
        //  -Filter=<commandSequence>       - Filter input records using following commands 
        //  -Init=<commandSequence>         - Execute these commands before starting interactive session.
        //  -History=<n>                    - Lines of history to be held in g_CommandBuf.
        //  -Hold                           - holds the screen on exit from curses.
        //  -Journal                        - requests journal files, these are saved in <primaryPathname>.jnl
        //  -Locale=<locale>                - Sets the linux locale for display (Linux only).
        //  -New                            - Creating new file
        //  -Obey                           - commands from stdin stream. 
        //  -Quiet                          - no messages except from P commands etc.
        //  -SCreensize=<width>[x<height>]  - Screen size follows.
        //  -STartup=<startupfilename>      - use specified startup file   N.B. Text file not read until after startup file has been run.
        //  -STAcksize=<stackSize>          - sets the stack size, defaults to StackSize.
        //  -TO=<filename>                  - On exit write the main file to specified pathName.
        //  -TTy                            - simple, non-curses,  teletype mode - no screen attributes to be used.      
         
        case 'N': // -New - create new file
          s_NewFile = TRUE;
          break;
           
        case 'F': // -Filter=<JOT commands>
          FilterCommands = GetBuffer('f', AlwaysNew);
          for ( ; ; ) { //Identify any further args that form part of the filter string.
            GetRecord(FilterCommands, strlen(ArgV[Arg])-EqChr+1);
            strcpy(FilterCommands->CurrentRec->text, ArgV[Arg]+EqChr);
            EqChr = 0; 
            if (ArgC <= ++Arg)
              break;
            if (ArgV[Arg][0] == '-') {
              Arg--;
              break; } } 
          FilterCommands->CurrentRec = FilterCommands->FirstRec;
          break;
           
        case 'I': // -init=<JOT commands>  - Initialization before starting interactive session.
          s_InitCommands = GetBuffer('i', OptionallyNew);
          if (s_InitCommands->CurrentRec == NULL)
            GetRecord(s_InitCommands, 1);
          for ( ; ; ) { //Identify any further args that form part of the init string.
            s_InitCommands->CurrentChr = strlen(s_InitCommands->CurrentRec->text);
            s_InitCommands->SubstringLength = 0;
            SubstituteString(s_InitCommands, ArgV[Arg]+EqChr, -1);
            s_InitCommands->SubstringLength = 0;
            SubstituteString(s_InitCommands, " ", -1);
            EqChr = 0; 
            if (ArgC <= ++Arg)
              break;
            if (ArgV[Arg][0] == '-') {
              Arg--;
              break; } } 
          s_InitCommands->CurrentChr = 0;
          break;
           
        case 'S': // 'S' commands switch.
          switch (toupper(ArgV[Arg][2]))
            {
          case 'C': // -SCreen size follows.
            sscanf(ArgV[Arg]+EqChr, "%ix%i", &g_ScreenWidth, &g_ScreenHeight);
            g_FirstWindowLine = 1;
            break;
          case 'T':
            if (toupper(ArgV[Arg][4]) == 'C') //-STAcksize=<n> - set the operand stack size.
              sscanf(ArgV[Arg]+EqChr, "%d", &g_StackSize);
            else { // -STartup file name follows.
              char TempString[1000];
              sscanf(ArgV[Arg]+EqChr, "%s", TempString);
              if (strlen(ArgV[Arg]) <= EqChr)
                strcpy(TempString, NULL_STREAM);
              *StartupFileName = (char *)malloc(strlen(TempString)+1);
              strcpy(*StartupFileName, TempString);
              break; } }
             
          break;
      
        case 'T':
          switch (toupper(ArgV[Arg][2])) { // 'T' commands switch.
            case 'O': // -To = <O/P filespec.
              ToFile = ArgV[Arg]+EqChr;
              break;
            case 'T': // -tty mode
              g_TTYMode = TRUE;
              break; }
               
          break;
      
        case 'Q': // -Quiet mode - no messages except from P commands etc.
          s_Verbose = 0;
          break;
          
        case 'J': //-Journal - requests journal files, these are saved in <primaryPathname>.jnl
          s_JournalPath = "";
          break;
        
#if defined(VC)
        case 'C': //-Codepage=<codepage> - for a specific codepage.
          sscanf(ArgV[Arg]+EqChr, "%ix%i", &s_CodePage);
          break;
#else
        case 'L': //-Locale=<locale> - for a specific locale setting.
          s_Locale = ArgV[Arg]+EqChr;
          break;
#endif
        
        case '\0': //Arg is a simple '-' - this indicates a pipe in from stdin.
          s_StreamIn = TRUE;
          break;
        
        case 'O': //-Obey - jot commands from stdin.
          s_StreamIn = FALSE;
          s_ObeyStdin = TRUE;
          break;
         
        default:
          fprintf(stderr, "Unknown qualifier \"%s\"\n", ArgV[Arg]);
           
        case 'H': //-Help, -HOld, -HIstory
          if (toupper(ArgV[Arg][2]) == 'O') { // -HOld - hold the screen on exit from curses.
            s_HoldScreen = TRUE;
            break; }
          else if (toupper(ArgV[Arg][2]) == 'I') { // -History=<n> - no. of command lines to hold in g_CommandBuf.
            sscanf(ArgV[Arg]+EqChr, "%d", &CommandBufSize); 
            break; }
             
        fprintf(stderr, "\nDocumentation guide at at %s/docs/README.html\n", getenv("JOT_HOME"));
        fprintf(stderr, "Valid qualifiers follow:\n");
        fprintf(stderr, "  -    - edit stdin stream.\n");
        fprintf(stderr, "  -New - Creating new file\n");
        fprintf(stderr, "  -Filter=<commandSequence> - Filter input records using following commands \n");
        fprintf(stderr, "  -History=<n> - Lines of history to be held in g_CommandBuf.\n");
        fprintf(stderr, "  -Hold - holds the screen on exit from curses.\n");
        fprintf(stderr, "  -Init=<commandSequence> - Execute these commands before starting interactive session.\n");
        fprintf(stderr, "  -Journal - requests journal files, these are saved in ./<primaryPathname>.jnl/\n");
        fprintf(stderr, "  -Obey - obey commands from stdin stream\n");
        fprintf(stderr, "  -Quiet - no messages except from P commands etc.\n");
        fprintf(stderr, "  -SCreensize=<width>[x<height>] - Screen size follows.\n");
        fprintf(stderr, "  -STAcksize=<stackSize> - set the operand-stack size [defaults to %d].\n", StackSize);
        fprintf(stderr, "  -STartup=<startupfilename> - use specified startup file.\n");
        fprintf(stderr, "  -TO=<filename> - On exit write the main file to specified pathName.\n");
        fprintf(stderr, "  -TTy - simple, non-curses,  teletype mode - no screen attributes to be used.\n");
#if defined(VC)
        fprintf(stderr, "  -Codepage - sets codepage (windows only).\n");
#else
        fprintf(stderr, "  -Locale - sets locale (linux only).\n");
#endif
        fprintf(stderr, "Bye now.\n");
        exit(1); } }
    
  g_CurrentBuf = GetBuffer('.', AlwaysNew);
  g_CurrentBuf->LineNumber = 1;
   
  if (FilterSequence) {
    FilterSequence = (struct Seq *)malloc(sizeof(struct Seq));
    JOT_Sequence(g_CommandBuf, FilterSequence, FALSE); }
    
  if (getenv("JOT_JOURNAL")) //Journalling requested via the JOT_JOURNAL env.
    s_JournalPath = "";
  if (s_JournalPath) {
    char FullPath[1000], Actual[1000];
    if ( ! FileName)
      Disaster("Cannot use -journal in stream-in mode");
    s_JournalPath = (char *)malloc(strlen(FileName)+6);
    strcpy(s_JournalPath, FileName);
    strcat(s_JournalPath, ".jnl/");
#if defined(VC)
    if (mkdir(s_JournalPath))
#else
    if (mkdir(s_JournalPath, 00777))
#endif
      Disaster("Can't create journal directory %s\n", s_JournalPath);
    strcpy(FullPath, s_JournalPath);
    strcat(FullPath, "history.txt");
    OpenTextFile(NULL, &s_JournalHand, "w", FullPath, NULL, Actual);
    if ( s_JournalHand == NULL)
      Disaster("Failed to open journal file \"%s\"", FullPath); }
      
  //Read file/stdin or create an empty file image.
  if (s_NewFile) { //Create a new file.
    GetRecord(g_CurrentBuf, StringMaxChr);
    GetRecord(g_CurrentBuf, StringMaxChr);
    g_CurrentBuf->CurrentRec = g_CurrentBuf->FirstRec;
    ToFile = FileName; }
  else {
    if (s_StreamIn) // Reading from stdin stream.
      ReadBuffer(stdin, FALSE, 0, "-", '.', FilterCommands, 0);
    else { //Reading from a real file.
#if defined(VC)
      HANDLE Chan = NULL;
      OpenTextFile(&Chan, NULL, "r", FileName, "\0", ActualFile);;
#else
      int Chan = 0;
      OpenTextFile(&Chan, NULL, "r", FileName, "\0", ActualFile);;
#endif
      
      if (s_JournalHand) {
        char Temp[1000];
        sprintf(Temp, "<<Primary file %s>>", FileName);
        UpdateJournal(Temp, NULL); }
      if ( ! Chan)
        Disaster("Nonexistant file \"%s\"", FileName);
      if ( ! ReadBuffer(NULL, FALSE, Chan, ActualFile, '.', FilterCommands, 0)) 
        Disaster("Can't open \"%s\" for reading.", FileName);
#if defined(VC)
      CloseHandle(Chan);
#else
      close(Chan);
#endif
 } }

  if (ToFile) { //Rename the file.
    free(g_CurrentBuf->PathName);
    g_CurrentBuf->PathName = (char *)malloc(strlen(ToFile)+1);
    g_CurrentBuf->NewPathName = TRUE;
    strcpy(g_CurrentBuf->PathName, ToFile); }

  if (s_StreamIn) { //Primary text from stdin stream - duplicate and switch so that stdin can be used for console reads.
    freopen(KBD_NAME, "r", stdin); }
  //Reorganize stdin and stdout as appropriate for required operating mode.
#if defined(VC)
  if (s_StreamOut) //Piped output - in windows just force -tty mode and have done with it.
    g_TTYMode = TRUE;
  if ( ! g_TTYMode) { //Normal screen mode - use console screen-text routines.
    if (s_StreamOut) //No output pipe.
      AllocConsole();
    if ( ! s_ObeyStdin) { //No stdin switchover required.
      hStdin = GetStdHandle(STD_INPUT_HANDLE); }
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if ( ! GetConsoleScreenBufferInfo(hStdout, &csbiInfo)) 
      Disaster("GetConsoleScreenBufferInfo: stdout Console Error"); 
      SrWindow = csbiInfo.srWindow;
    if ( ! g_ScreenWidth)
      g_ScreenWidth = SrWindow.Right-SrWindow.Left+1;
    if ( ! g_ScreenHeight)
      g_ScreenHeight = SrWindow.Bottom-SrWindow.Top+1;
    wOldColorAttrs = csbiInfo.wAttributes; 
    if ( ! SetConsoleTextAttribute(hStdout, Normal_Text))
      Disaster("SetConsoleTextAttribute: Console Error");
  if ( ! SetConsoleOutputCP(s_CodePage))
    puts("SetConsoleOutputCP failed");
 }
#else
#if defined(LINUX)
  if ( ! g_TTYMode) { //Using screen attributes - curses/windows screen initialization happens here.
    mainWin = initscr();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    s_NextColourPair = 2;
    s_DefaultColourPair = COLOR_PAIR(1);
    raw();
    keypad(mainWin, TRUE);
    noecho();
    mousemask(s_MouseMask, NULL);
    halfdelay(1);
    if ( ! g_ScreenWidth)
      g_ScreenWidth = getmaxx(mainWin);
    if ( ! g_ScreenHeight)
      g_ScreenHeight = getmaxy(mainWin); }
#endif
#endif
 
  NewCommandFile(fopen(KBD_NAME, "r"), "<The Keyboard>");
  g_EditorConsole = g_EditorInput;
  g_EditorOutput = stdout;
  g_CommandBuf = g_EditorConsole->CommandBuf;
  if (CommandBufSize <= 0)
    Disaster("Invalid history-buffer size");
  while (CommandBufSize--)
    GetRecord(g_CommandBuf, StringMaxChr);
  g_CommandBuf->CurrentChr = 0;
  g_FirstWindow = NULL;
  g_FirstWindowLine = 1;
  g_CaseSensitivity = FALSE;
  
  g_TraceMode = 0;
  g_Failure = FALSE;
  
#if defined(VC)
  if( ! SetConsoleCtrlHandler( (PHANDLER_ROUTINE) Ctrl_C_Interrupt, TRUE ) ) 
    Disaster("Can't set up Ctrl+C handler");
  system("cls");
#elif defined(LINUX)
  if (signal(SIGINT, SIG_IGN) != SIG_IGN)
    sigset(SIGINT, Ctrl_C_Interrupt);
#endif
  
  g_Stack = (struct intFrame *)malloc((sizeof(struct intFrame)+1)*g_StackSize);
  g_Stack[0].type = StackVoidFrame;
  if (FileName == NULL) { //No file name given.
    if ( ! s_StreamIn)
      Disaster("No file to edit.\n"); }
  else { //File name specified.
    if (s_StreamIn)
      Disaster("File name (\"%s\") given when in stream-in mode", FileName); } }

//----------------------------------------------StartEdit
void StartEdit(char * StartupFileName, struct Buf * InitBuf)
  { //The initial text file is already read, renames and calls the main editor as requested.
  struct Seq * InitSequence = NULL;
   
  if (s_JournalHand) {
    char Temp[1000];
    sprintf(Temp, "<<Screen height %d, Guard band size %d>>", g_ScreenHeight, s_GuardBandSize);
    UpdateJournal(Temp, NULL);
    if (s_InitCommands) {
      UpdateJournal("<<Init commands follow>>", NULL);
      UpdateJournal(s_InitCommands->FirstRec->text, NULL); }
    sprintf(Temp, "<<Startup script>>");
    UpdateJournal(Temp, NULL); }
  g_CurrentBuf->AbstractWholeRecordFlag = TRUE;
  SwitchToComFile(StartupFileName, NULL, "");
  s_BombOut = 0;
 
  if (s_JournalHand) {
    char Temp[1000];
    sprintf(Temp, "<<Startup Sequence ends, buffer %c>>", g_CurrentBuf->BufferKey);
    UpdateJournal(Temp, NULL); }
  if (InitBuf) {
    InitSequence = (struct Seq *)malloc(sizeof(struct Seq));
    JOT_Sequence(InitBuf, InitSequence, FALSE);
    if ( ! Run_Sequence(InitSequence)) {
      Fail("-init sequence failed."); } }
       
  FreeSequence(&InitSequence); 
  InteractiveEditor();
  Message(NULL, "Abnormal exit");
  FreeBuffer(g_CurrentBuf); }

//----------------------------------------------InteractiveEditor
void InteractiveEditor()
  { //Main entry point for the C version of jot, called after reading the initial text-file 
    //image to buffer '.' and all hot-key mappings have been defined by the startup file.
  struct Seq *MainSequence = NULL;
  int status;
  
  for (;;) { //User command loop. 
    s_BombOut = FALSE;
    g_Failure = FALSE;
    if ( ! JotUpdateWindow() && (s_Verbose & NonSilent) )
      DisplayDiag(g_CurrentBuf, TRUE, "");
    g_CurrentCommand = NULL;
     
    g_Failure = ReadCommand(
      g_CommandBuf,
      g_EditorInput->FileHandle,
      ((g_CurrentBuf->CurrentRec) == (g_CurrentBuf->FirstRec->prev) ? "****End**** %d %c> " : "%d %c> "),
      g_CurrentBuf->LineNumber,
      g_CurrentBuf->BufferKey);
#if defined(LINUX)
    refresh();
#endif
    if (g_Failure) {
      if (g_Failure == EOF) //This can only happen at the end of an -asConsole script.
        break;
      else if (g_Failure == UNTRANSLATED_CHR) {
        continue; }
      else
        continue; }
    
    if (VerifyDigits(g_CommandBuf)) { //Repeat command.
      long long Repeat = GetDec(g_CommandBuf);
      int Count = 0;
      
      if (MainSequence == NULL) {
        RunError("Last command failed.");
        continue; }
      if (VerifyNonBlank(g_CommandBuf)) {
        SynError(g_CommandBuf, "Unexpected characters");
        continue; }
       
      for ( ; ; )
        if ( ! Run_Sequence(MainSequence)) { // Failure detected.
          if (0 < Repeat) {
            Message(NULL, "Command failed on iteration %d of %d", Count, Repeat);
            break; }
          else //That's OK it was an infinite repeat.
            break; }
        else
          if (++Count ==  Repeat) {
            if (s_Verbose & QuiteChatty)
              Message(NULL, "Completed all reiterations without error");
            break; }
             
      continue; }
    
    FreeSequence(&MainSequence);
    g_CommandBuf->CurrentChr = 0;
    MainSequence = (struct Seq *)malloc(sizeof(struct Seq));
    JOT_Sequence(g_CommandBuf, MainSequence, FALSE);
    if (s_CommandChr == ')')
      SynError(g_CommandBuf, "Surplus \')\'");
    if (g_Failure)
      continue;
    status = Run_Sequence(MainSequence);
    if ( ! status && ! s_BombOut)
      RunError("Command-sequence failed."); }
      
  FreeSequence(&MainSequence); }

//---------------------------------------------JOT_Sequence
void JOT_Sequence(struct Buf * CommandBuf, struct Seq * ThisSequence, int StopOnTag)
  { //The main entry point for the JOT compiler.
    //Compiles a sequence of JOT commands, terminated by one of the following:
    // ',' - an JOT-style else clause follows - calls itself recursively,
    // ')' - end of JOT-style block - pass it back to calling JOT_Block
    // '\0' - the end of a command line. */
    // StopOnTag is used to identify the end point when compiling a selected routine from the repositort using %h=call ...;
  struct Com * ThisCommand = NULL;
  struct Com * PrevCommand = NULL;
  
  ThisSequence->FirstCommand = NULL;
  ThisSequence->ElseSequence = NULL;
  ThisSequence->NextSequence = NULL;
  CommandBuf->UnchangedStatus |= SameSinceCompiled;
   
  for ( ; ; ) { // Command-sequence loop.
    int CommandChrNo;
    int CommandLineNo;
    
    s_CommandChr = ChrUpper(NextNonBlank(CommandBuf));
    if (StopOnTag && CommandBuf->CurrentRec->TagChain)
      return;
    CommandChrNo = (CommandBuf->CurrentChr)-1;
    CommandLineNo = CommandBuf->LineNumber;
    if (s_CommandChr == ',') { // ELSE Clause follows. 
      ThisSequence->ElseSequence = (struct Seq *)malloc(sizeof(struct Seq));
      JOT_Sequence(CommandBuf, ThisSequence->ElseSequence, StopOnTag);
      //DumpSequence(ThisSequence, "Completed ELSE sequence.");
      return; }
    else if (s_CommandChr == '\0') { // End of command line reached.
      //DumpSequence(ThisSequence, "End of command record.");
      if ( ! AdvanceRecord(CommandBuf, 1))
        continue;
      return; }
  
    ThisCommand = (struct Com *)malloc(sizeof(struct Com));
    if (ThisCommand == NULL) {
      SynError(NULL, "Failed to allocate space for command.");
      return; }
    ThisCommand->ArgList = NULL;
    ThisCommand->NextCommand = NULL;
    if (PrevCommand == NULL)
      ThisSequence->FirstCommand = ThisCommand;
    else
      PrevCommand->NextCommand = ThisCommand;
    ThisCommand->CommandKey = s_CommandChr;
    ThisCommand->ArgList = NULL;
    ThisCommand->NextCommand = NULL;
    ThisCommand->CommandBuf = CommandBuf;
    ThisCommand->CommandRec = CommandBuf->CurrentRec;
    ThisCommand->CommandChrNo = CommandChrNo;
    ThisCommand->CommandLineNo = CommandLineNo;
       
    switch (s_CommandChr) {;
    case '\'':  //Deferred command specification.
      AddNumericArg(ThisCommand, ChrUpper(ImmediateCharacter(CommandBuf)));
      break;
  
    case '(': { //Open a new Block. 
      struct Block *NewBlock = (struct Block *)malloc(sizeof(struct Block));
      NewBlock->BlockSequence = NULL;
      AddBlock(ThisCommand, NewBlock);
      JOT_Block(NewBlock, CommandBuf, StopOnTag);
      if (g_Failure) {
        return; }
      break; }
  
    case ')':  // End of Block. 
      //DumpSequence(ThisSequence, "End of JOT-style block.");
      ThisCommand->ComChrs = (CommandBuf->CurrentChr)-ThisCommand->CommandChrNo;
      return;
  
    case '?':  //Force Pass/fail state to TRUE.
    case '\\':  //Reverse Pass/Fail state.
      break;
  
    case 'A': { //A - Abstract using note record pointer. 
      char Chr;
      Chr = ImmediateCharacter(CommandBuf);
      if ( ! CheckBufferKey(Chr)) {
        SynError(CommandBuf, "Syntax invalid buffer identifier %c", Chr);
        continue; }
      AddNumericArg(ThisCommand, ChrUpper(Chr));
      AddNumericArg(ThisCommand, CheckNextCommand(CommandBuf, "+-."));
      AddNumericArg(ThisCommand, CheckNextCommand(CommandBuf, "*&"));
      break; }
  
    case 'Q':  //Q - qualify.
      AddNumericArg(ThisCommand, (VerifyCharacter(CommandBuf, '-') ? -1 : 1));
      AddStringArg(ThisCommand, CommandBuf);
      break;
  
    case 'V':  //V - verify text string
      AddNumericArg(ThisCommand, (VerifyCharacter(CommandBuf, '-') ? -1 : 1));
      AddStringArg(ThisCommand, CommandBuf);
      break;
  
    case 'M':  //M - move command.
    case 'Y':  //Y - Move in same column.
      {
      AddNumericArg(ThisCommand, CheckNextCommand(CommandBuf, "+*"));
      AddNumericArg(ThisCommand, GetArg(CommandBuf, 1)); }
      break;
  
    case 'T':  //Trace - debug
      break;
  
    case 'N':  //N - note current Record for later abstract.
      AddNumericArg(ThisCommand, VerifyCharacter(CommandBuf,'.'));
      break;
  
    case 'F':  //F - find Command.
      AddNumericArg(ThisCommand, (VerifyCharacter(CommandBuf, '-') ? -1 : 1));
      AddNumericArg(ThisCommand, ((s_CommandChr == 'F') ? GetArg(CommandBuf, 0) : GetArg(CommandBuf, 1)));
      AddStringArg(ThisCommand, CommandBuf);
      AddNumericArg(ThisCommand, GetArg(CommandBuf, 1));
      break;
  
    case 'U': //U - hashtable lookUp.
      AddStringArg(ThisCommand, CommandBuf);
      break;
  
    case 'H': { //H - here command.
      char Chr;
      int Repeats;
      Chr = ChrUpper(ImmediateCharacter(CommandBuf));
      AddNumericArg(ThisCommand, Chr);
      if ( ! CheckBufferKey(Chr)) {
        SynError(CommandBuf, "Invalid buffer.");
        continue; }
      Repeats = GetArg(CommandBuf, 1);
      AddNumericArg(ThisCommand, VerifyCharacter(CommandBuf, '*'));
      AddNumericArg(ThisCommand, Repeats);
      break; }
  
    case 'S':  //S - substitute command.
      AddNumericArg(ThisCommand, (VerifyCharacter(CommandBuf, '-') ? -1 : 1));
      AddStringArg(ThisCommand, CommandBuf);
      break;
  
    case 'I':  //I - insert command.
      AddNumericArg(ThisCommand, (VerifyCharacter(CommandBuf, '-') ? -1 : 1));
      AddStringArg(ThisCommand, CommandBuf);
      AddNumericArg(ThisCommand, (VerifyDigits(CommandBuf) ? GetDec(CommandBuf) : 1));
      break;
  
    case 'K':  //K - kill some lines.
    case 'P':  //P - print lines.
    case 'J':  //J - join line(s).
    case 'C':  //C - change case of characters.
//    case 'D':  //D - delete characters.
    case 'E':  //E - erase bytes or characters.
      AddNumericArg(ThisCommand, ((VerifyCharacter(CommandBuf, '-') ? -1 : 1)));
      AddNumericArg(ThisCommand, GetArg(CommandBuf, 1));
      break;
      
    case 'B': { //B - break line.
      AddNumericArg(ThisCommand, ((VerifyCharacter(CommandBuf, '-') ? -1 : 1)));
      AddNumericArg(ThisCommand, GetBreakArg(CommandBuf, 1));
      break; }
      
    case 'R':  //R - Move current character pointer right n bytes.
//    case 'L':  //L - Move character pointer left n bytes.
//    case 'X':  //X - Move character pointer right or left n characters.
    case 'G':  //G - Get - Input text from console.
      AddNumericArg(ThisCommand, GetArg(CommandBuf, 1));
      break;
  
    case 'W':  //W - update Window command.
      break;
  
    case 'O': {  //O - stack-Operation command.
      char Arg1 = ChrUpper(ImmediateCharacter(CommandBuf));
      char Arg2;
      AddNumericArg(ThisCommand, Arg1);
      switch (Arg1) {//Stack operation switch block.
      case 'L':  // OL<nnn> - Literal number to be pushed onto stack.
        AddNumericArg(ThisCommand, GetDec(CommandBuf));
        break;
      case 'O':  // OO<string> - formatted Output using string as sprintf format string.
        AddStringArg(ThisCommand, CommandBuf);
        break;
      case 'I':  // OI<string> - formatted Input - Convert string at CurentChr using string as sscanf format.
      case '^':  //O^ Test state
        AddNumericArg(ThisCommand, ChrUpper(ImmediateCharacter(CommandBuf)));
        break;
      case 'W':  //OW Scrolls up by lines set in top of stack.
      case '@':  //O@ reset stack
      case '?':  //O? Dump contents of stack
      case 'S':  //OS Swap 1st and 2nd items of stack.
      case '#':  //O# Duplicate top of stack.
      case '=':  //O= Fail if top two items on stack not equal.
      case '<':  //O< Test that item at top greater than (top-1).
      case '>':  //O> Test that item at top less than (top-1).
      case 'K':  //OK Kill - delete top of stack.
      case 'D':  //OD Decimal conversion - pop stack, decimal to current chr.
      case '~':  //O~ Increment - add 1 to item at top, fail if result=0.
      case 'A':  //OA rAndom - push a random number onto the stack.
      case '!':
      case '&':
      case '|':  //Bitwise NOT, AND and OR
      case '+':
      case '-':
      case '*':
      case '/':  //Add, Subtract, Multiply and Divide.
      case '%':  //O% Remainder
      case '.':  //O. Set current line number to <top of stack>.
      case 'B':  //OB Push pointer to current buffer onto stack.
      case 'E':  //OE Push Extent (i.e. length on the screen) taking unicode characters into account.
      case 'P':  //OP Push mouse Position - last detected mouse-click position
      case 'N':  //ON Push current line no. onto stack.
      case 'U':  //OU set current sUbstring length from value on stack.
//      case 'Y':  //OY Push current byte no. onto stack.
      case 'C':  //OC Push current chr. no. onto stack.
      case 'Z':  //OZ Zoom (change to buffer) pointer popped off stack.
      case 'R':  //OR shift Right (left if -ve) no. of bytes popped off stack.
//      case 'X':  //OX X-index Right (left if -ve) no. of chrs popped off stack.
      case 'M':  //OM Move (backwards if -ve) no. of lines popped off stack.
        break;
      default:
        SynError(CommandBuf, "Unknown stack operator");
      };//Stack-operation switch block ends.
    break;
  
    case 'Z':  //Z - zoom into another buffer.
      Arg2 = ChrUpper(ImmediateCharacter(CommandBuf));
      if ( ! CheckBufferKey(Arg2))
        SynError(CommandBuf, "Invalid buffer tag.");
      AddNumericArg(ThisCommand, Arg2);
      break; }
  
    case '%': { //% - Percent-command-argument block.
      s_CommandChr = ChrUpper(NextNonBlank(CommandBuf));
      
      if ( (s_CommandChr == 'G') &&  (CommandBuf != g_CommandBuf) ) { //%G command from a macro - copy the text block now.
        char Key = ChrUpper(NextNonBlank(CommandBuf));
        struct Buf * DestBuf = GetBuffer(Key, AlwaysNew);
         
        if ( ! DestBuf)
          continue;
        CommandBuf->CurrentChr--;
        AddNumericArg(ThisCommand, s_CommandChr);
        AddPercentArg(ThisCommand, CommandBuf);
        for ( ; ; ) { 
          if (AdvanceRecord(CommandBuf, 1))
            break;
          if ( (CommandBuf->CurrentRec->text[0] == ':' && CommandBuf->CurrentRec->text[1] == '\0') ) {
            CommandBuf->CurrentChr = 1;
            break; }
          GetRecord(DestBuf, strlen(CommandBuf->CurrentRec->text));
          strcpy(DestBuf->CurrentRec->text, CommandBuf->CurrentRec->text); } }
      else {
        char BufferKey;
        AddNumericArg(ThisCommand, s_CommandChr);
        if (s_CommandChr == '%') //Skip all the comment text.
          CommandBuf->CurrentChr = strlen(CommandBuf->CurrentRec->text);
        switch (s_CommandChr) { //Any these percent commands may have a buffer-key arg immediately after s_CommandChr followed by an equals sign.
          case 'D':
          case 'E':
          case 'F':
          case 'G':
          case 'H':
          case 'I':
          case 'Q':
            BufferKey = ChrUpper(NextNonBlank(CommandBuf));
              if (BufferKey == ';' && CommandBuf->CurrentRec->text[CommandBuf->CurrentChr] != '=') { //It's a terminating semicolon.
                CommandBuf->CurrentChr--;
                BufferKey = '\0'; }
              break;
          default:
            BufferKey= '\0'; }
        AddNumericArg(ThisCommand, BufferKey);
        AddPercentArg(ThisCommand, CommandBuf); } }
  
    switch (s_CommandChr) { // Percent-command switch block.
      case 'A':  //%A - Exit without writing file.
      case 'B':  //%B - Set buffer attributes.
      case 'C':  //%C - Exit writing new file.
      case 'D':  //%D - Define a buffer from console or last console command.
      case 'E':  //%E - Execute following CLI command line.
      case 'F':  //%F - Regular-expression search.
      case 'G':  //%G - Get - but from current command file or console (macro case dealt with above).
      case 'H':  //%H - Hashtable maintenance
      case 'I':  //%I - Secondary input file.
      case 'L':  //%L - set line Length.
      case 'M':  //%M - Message.
      case 'O':  //%O - Output current buffer as specified file.
      case 'P':  //%P - set user Prompt for G command.
      case 'Q':  //%Q - system Query.
      case 'R':  //%R - Run a command file.
      case 'S':  //%S - System settings 
      case 'U':  //%U - Undo last substitution.
      case 'W':  //%W - Set up a screen window.
      case 'X':  //%X - Exit via a run-time error message.
      case '~':  //%~ - Insert or display control character.
      case '%':  //%% - Comment line.
        break;
    
      default:
        SynError(CommandBuf, "Unknown %%%% command"); }
    break;
  
    default:  //Invalid command character.
      CommandBuf->CurrentChr--;
      if (s_JournalHand) {
        fputs("<InvalidCommand>\n", s_JournalHand);
        fflush(s_JournalHand); }
      SynError(CommandBuf, "Invalid command character.");
      return; } // Command case block ends.
       
    PrevCommand = ThisCommand;
    ThisCommand->ComChrs = (CommandBuf->CurrentChr)-ThisCommand->CommandChrNo; } }

//---------------------------------------------JOT_Block
struct Block * JOT_Block(struct Block *ThisBlock, struct Buf *CommandBuf, int StopOnTag)
  { //Compiles a Block of JOT commands, terminated by a ')'. 
  ThisBlock->BlockSequence = (struct Seq *)malloc(sizeof(struct Seq));
  JOT_Sequence(CommandBuf, ThisBlock->BlockSequence, StopOnTag);
  if (s_CommandChr != ')') {
    SynError(CommandBuf, "Missing \')\'");
    return NULL; }
  ThisBlock->Repeats = GetArg(CommandBuf, 1);
  return ThisBlock;  }

//---------------------------------------------Run_Sequence
int Run_Sequence(struct Seq * Sequence)
  { //Executes a sequence of compiled commands.
  int LocalFail = FALSE;
  struct Com * NextCommand = Sequence->FirstCommand;
  struct Seq * ElseCommands = Sequence->ElseSequence;
  struct Arg * ThisArg;
  char TraceMessage[1000]; 
  
  g_CurrentCommand = NULL;
  
  for (;;) { // Command-key loop. 
    if (s_BombOut) {
      g_Failure = TRUE;
      return FALSE; }
    if (g_CurrentCommand == NULL) { // First command in sequence.
      g_CurrentCommand = NextCommand;
      if (g_CurrentCommand == NULL)
        return TRUE;
      s_CommandChr = g_CurrentCommand->CommandKey;
      ThisArg = g_CurrentCommand->ArgList; }
    else
      if (NextCommand == NULL) { // Last command in sequence.
        if ( ! LocalFail)
          return TRUE;
        else if (ElseCommands == NULL)
          return ! LocalFail; }
      else { // Already executed at least one command and more to follow.
        s_CommandChr = NextCommand->CommandKey;
        ThisArg = NextCommand->ArgList;
        if (s_CommandChr == '?')
          LocalFail = FALSE;
        else if (s_CommandChr == '\\')
          LocalFail = ! LocalFail; }
          
    if (g_TraceMode && g_CurrentCommand) {
      TraceMessage[0] = '\0'; 
      if (g_TraceMode & Trace_AllCommands) 
        strcpy(TraceMessage, "New command.");
      else if ( (g_TraceMode & Trace_CommandLines) && (g_CurrentCommand->CommandChrNo == 0) ) 
        strcat(TraceMessage, "New command line.");
      else if ( (g_TraceMode & Trace_Macros) && (g_CurrentCommand->CommandBuf != s_PrevDebugBuf) ) 
        strcat(TraceMessage, "New macro.");
      else if ( (g_TraceMode & Trace_Failures) && LocalFail ) 
        strcat(TraceMessage, "Command failure.");
      if (TraceMessage[0]) {
        if (g_TraceMode & Trace_Backtrace)
          Backtrace(GetBuffer('?', OptionallyNew));
        if (g_TraceMode & Trace_Stack)
          DumpStack();
        if (g_TraceMode & Trace_Print)
          DisplayRecord(g_CurrentBuf, 1);
        if (g_TraceMode & Trace_Source)
          DisplaySource(g_CurrentCommand, TraceMessage);
        if (g_TraceMode & Trace_Break) {
          struct Com * CurrentCommandSave = g_CurrentCommand;
          int CommandChrSave = s_CommandChr;
          JotDebug(TraceMessage); 
          g_CurrentCommand = CurrentCommandSave;
          s_CommandChr = CommandChrSave; }
        s_PrevDebugBuf = g_CurrentCommand->CommandBuf; } }
//        s_PrevDebugRec = g_CurrentCommand->CommandRec;
//        s_PrevDebugSeq = Sequence; } }

    if (LocalFail) { // Last command failed.
      if (s_BombOut) { //Don't attempt to handle this failure - just unwind the block frames.
        g_Failure = TRUE;
        if (s_Verbose & QuiteChatty)
          Fail(NULL, "unwinding call frames following an error");
        return FALSE; }
      if (ElseCommands) {
        Fail("Command failed, taking else sequence");
        return Run_Sequence(ElseCommands); }
      else
        return FALSE; }
    
    g_CurrentCommand = NextCommand;
    NextCommand = g_CurrentCommand->NextCommand;
     
    LocalFail = TRUE;
    g_Failure = FALSE;
    switch (s_CommandChr) {

    case '\'': { //' - Macro call - deferred sequence block.
      char Key = FetchIntArg(&ThisArg);
      struct Buf * CommandBuf = GetBuffer(Key, NeverNew);
      struct Seq * DeferredSeq;
      struct BacktraceFrame ThisBacktraceFrame;
     
      if (CommandBuf == NULL) {
        Fail("Undefined buffer");
        continue; }
      if (CommandBuf->Predecessor) {
        RunError("Recursive use of macro in buffer %c", Key);
        continue; }
      ResetBuffer(CommandBuf);
      CommandBuf->CurrentRec = CommandBuf->FirstRec;
      CommandBuf->CurrentChr = 0;
      CommandBuf->SubstringLength = 0;
      CommandBuf->Predecessor = g_CurrentCommand;
       
      DeferredSeq = (struct Seq *)malloc(sizeof(struct Seq));
      JOT_Sequence(CommandBuf, DeferredSeq, FALSE);
      if (s_BombOut) { 
        struct Buf * TempBuf = GetBuffer(CommandBuf->BufferKey, NeverNew);
        if (TempBuf->FirstRec == CommandBuf->FirstRec)
          Message(CommandBuf, "Exiting macro %c, line no %d", Key, CommandBuf->LineNumber);
        else //The failing sequence changed the command buffer - this is allowable but the original command string is no longer available.
          Message(CommandBuf, "Exiting macro %c, line no %d (<original command-line text has been deleted>)", Key, CommandBuf->LineNumber); }
      if (s_CommandChr == ')') {
        SynError(CommandBuf, "Surplus \')\'");
        free(DeferredSeq);
        continue; }
      ThisBacktraceFrame.prev = s_BacktraceFrame;
      ThisBacktraceFrame.ParentCommand = g_CurrentCommand;
      ThisBacktraceFrame.Type = MacroFrame;
      s_BacktraceFrame = &ThisBacktraceFrame;
      g_Failure = ! Run_Sequence(DeferredSeq);
      if (CommandBuf == GetBuffer(Key, NeverNew)) //The command buffer may have been re-initialized as part of the command sequence - we allow this to happen.
        CommandBuf->Predecessor = NULL;
      FreeSequence(&DeferredSeq);
      g_CurrentCommand = NULL;
      s_BacktraceFrame = ThisBacktraceFrame.prev;
      if (g_Failure)
        continue;
      break; }

    case '\\': //\ - Reverse PASS/FAIL status of previous command - already done.
    case '?':  //? - Set status of previous command to PASS - already done.
      break;

    case '(': { //( - Block.
      struct Com * CurrentCom = g_CurrentCommand;
       
      g_Failure = ! Run_Block(FetchBlock(&ThisArg));
      g_CurrentCommand = CurrentCom;
      if (g_Failure)
        continue;
      break; }

    case ',':  //, - ELSE Clause follows.
      return ! s_BombOut;

    case ')':  //) - End of current Block.
      return TRUE;

    case 'N':  //N - Note current Record for later abstract.
      s_AbstractWholeRecordFlag = ! FetchIntArg(&ThisArg);
      s_NoteLine = g_CurrentBuf->LineNumber;
      s_NoteBuffer = g_CurrentBuf;
      s_NoteCharacter = s_AbstractWholeRecordFlag ? 0 : g_CurrentBuf->CurrentChr;
      break;

    case 'A': { //A - Abstract using note record pointer.
      char BufferKey = FetchIntArg(&ThisArg);
      char ModeQualifier = FetchIntArg(&ThisArg);
      char FillQualifier = FetchIntArg(&ThisArg);
      struct Buf * DestBuf;
       
      if ( (BufferKey <= ' ') || ('z' < BufferKey) ) {
        Fail("Invalid buffer key");
        continue; }
      if (FillQualifier != '&') {
        if (g_CurrentBuf->EditLock & ReadOnly) {
          Fail("Attempt to modify a readonly buffer");
          continue; }
        g_CurrentBuf->UnchangedStatus = 0; }
        
      if ( ! s_NoteBuffer) {
        continue; }
      if (s_NoteBuffer != g_CurrentBuf) {
        RunError("The note point is in a different buffer");
        continue; }
      s_NoteBuffer = NULL;
      if (g_CurrentBuf->BufferKey == BufferKey) {
        RunError("Abstract source and destination is the same buffer");
        continue; }
      DestBuf = GetBuffer(BufferKey, ModeQualifier ? OptionallyNew : AlwaysNew);
      if (DestBuf == NULL) {
        RunError("Invalid buffer");
        continue; }
         
      if ( ! DestBuf->FirstRec) //A newly-minted buffer with no records.
        GetRecord(DestBuf, 1);
      DestBuf->UnchangedStatus = 0;
       
      switch (ModeQualifier) {;
        case '+': //Append
          AdvanceRecord(DestBuf, INT_MAX);
          DestBuf->CurrentChr = strlen(DestBuf->CurrentRec->text);
          break;
        
        case '-': //Prepend
          DestBuf->CurrentRec = DestBuf->FirstRec;
          DestBuf->LineNumber = 1;
          DestBuf->CurrentChr = 0;
          DestBuf->SubstringLength = 0;
          break;
        
        case '.': //Insert
          break;
          
        case '\0':
          break;
          
        default: //Unrecognized qualifier character.
          RunError("Invalid qualifier to abstract command - must be one of +, - or .");
          break; }
      
      
      if (s_AbstractWholeRecordFlag) {
        g_CurrentBuf->CurrentChr = 0;
        g_CurrentBuf->SubstringLength = 0;
        if (s_NoteLine < g_CurrentBuf->LineNumber)
          g_Failure |= AdvanceRecord(g_CurrentBuf, 1);
        else 
          s_NoteLine++; }
      if (g_Failure)
        continue;
        
      Abstract(DestBuf, g_CurrentBuf, s_NoteLine, s_NoteCharacter, FillQualifier);    
      DestBuf->AbstractWholeRecordFlag = s_AbstractWholeRecordFlag;
      if (g_Failure)
        continue;
      break; }

    case 'Q': { //Q - Qualify.
      g_CurrentBuf->SubstringLength = 0; { // Qualify block. 
        long long Direction = FetchIntArg(&ThisArg);
        char * StringArgument = (char *)FetchStringArg(&ThisArg, s_QualifyString);
        char *RecordText = g_CurrentBuf->CurrentRec->text;
        int RecordTextLength = strlen(RecordText);
        int CurrentChrPtr = g_CurrentBuf->CurrentChr;
        char Chr;
        int Length, Index;
        
        //Extract match character, set to 0 if already at end stop. 
        if (Direction < 0)
          Chr = (CurrentChrPtr == 0)? '\0' : RecordText[CurrentChrPtr-1];
        else
          Chr = (RecordTextLength < CurrentChrPtr) ? '\0' : RecordText[CurrentChrPtr];
        
        if ((StringArgument == NULL) || (Chr == '\0'))
          continue;
        if ((Length = strlen(StringArgument)) <= 0)
          continue;
        if ((StringArgument[0] == Chr) || (StringArgument[Length-1] == Chr))
          LocalFail = FALSE;
        else
          for (Index = 1; Index < Length; Index +=1) { // Search loop. 
            if (StringArgument[Index] == '-') { // A range of characters.
              if ((StringArgument[Index-1] <= Chr) && (Chr <= StringArgument[Index+1])) { // Chr in range. 
                LocalFail = FALSE;
                break; } }
            else if (StringArgument[Index] == Chr) { // Chr match. 
              LocalFail = FALSE;
              break; } } }
               
      if (LocalFail)
        continue;
      break; }

    case 'V': { //V - Verify text string.
      long long Direction = FetchIntArg(&ThisArg);
      char * StringArgument = (char *)FetchStringArg(&ThisArg, s_SearchString);
      int InitialPointer = g_CurrentBuf->CurrentChr;
       
      g_CurrentBuf->SubstringLength = 0; 
      if (0 < Direction) { // Forward verify.
        if (SearchRecord(g_CurrentBuf, StringArgument, 1))
          LocalFail = ((g_CurrentBuf->CurrentChr) != InitialPointer); }
      else { // Backwards verify. 
        int StringArgLength = strlen(StringArgument);
        g_CurrentBuf->CurrentChr = InitialPointer-StringArgLength;
        if (SearchRecord(g_CurrentBuf, StringArgument, -1))
          LocalFail = (g_CurrentBuf->CurrentChr) != (InitialPointer-StringArgLength);
        g_CurrentBuf->SubstringLength = -StringArgLength; }
      g_CurrentBuf->CurrentChr = InitialPointer;
       
    if (LocalFail) { // Failure. 
      g_CurrentBuf->SubstringLength = 0;
      continue; }
    break; }

    case 'M':  //M - Move command.
    case 'Y':  //Y - Move in same column.
      {
      char Arg1 = FetchIntArg(&ThisArg);
      long long Arg2 = FetchIntArg(&ThisArg);
      int * TabStops = g_CurrentBuf->TabStops;
      int TabCount;
       
      //Column no., used by Y command, is reset if not at end of line.
      if (s_CommandChr == 'Y' && g_CurrentBuf->CurrentChr != strlen(g_CurrentBuf->CurrentRec->text))
        s_LastCurrentCharacter = JotStrlenBytes(g_CurrentBuf->CurrentRec->text, g_CurrentBuf->CurrentChr);
      if (Arg1 == '+')
        Arg2 = Arg2-(g_CurrentBuf->LineNumber);
      else { // Not '+' try '*' 
        struct Window *Window = g_FirstWindow;
        
        if (Arg1 == '*') { // Window-relative move. 
          while (Window != NULL) // Window search loop. 
            if ( ((Window->WindowHeight != 0) && ((Window->WindowKey) == (g_CurrentBuf->BufferKey)) ) || (Window->WindowKey == '\0')) { // Window found. 
              int WindowHeight = (Window->WindowHeight)-1;
              int OldLastLineNumber = Window->OldLastLineNumber;
              int CurrentGuard = (s_GuardBandSize*2 <= WindowHeight) ? s_GuardBandSize : 0;
            
              Arg2 = OldLastLineNumber-(g_CurrentBuf->LineNumber ) + ( (Arg2 < 0) ? (CurrentGuard-WindowHeight)*2-1 : WindowHeight-CurrentGuard*2+1);
              break; }
            else
              Window = Window->NextWindow; } }
      
      if (s_CommandChr == 'Y' && TabStops) { //Identify the TabStop in the original line.
        char * SubString = g_CurrentBuf->CurrentRec->text;
        TabCount = 0;
        while (SubString-g_CurrentBuf->CurrentRec->text <= g_CurrentBuf->CurrentChr)   {
          char * Pos = strchr(SubString, g_TableSeparator);
          if ( ! Pos) { //Current character is in the last cell.
            TabCount++;
            break; }
          if ( ! (SubString = Pos+1))
            break;
          TabCount++; } }
      g_Failure |= AdvanceRecord(g_CurrentBuf, Arg2);
      if (s_CommandChr == 'Y') { //Y Move in text column.
//        int Length = strlen(g_CurrentBuf->CurrentRec->text);
        if (TabStops) { //Y move in table column. 
          char * SubString = g_CurrentBuf->CurrentRec->text;
          s_LastCurrentCharacter = 0;
          while (--TabCount) {
            char * Pos = strchr(SubString, g_TableSeparator);
            if ( ! Pos) {
              g_Failure = TRUE;
              break; }
            SubString = strchr(SubString, g_TableSeparator)+1;
            s_LastCurrentCharacter = SubString-g_CurrentBuf->CurrentRec->text; } }
        g_CurrentBuf->CurrentChr = 0;
        g_CurrentBuf->SubstringLength = 0;
        ShiftRight(g_CurrentBuf, s_LastCurrentCharacter); }
      if (g_Failure && (Arg2 != INT_MIN) && (Arg2 != INT_MAX))
        continue;
      break; }

    case 'B': { //B - Break line.
      long long Direction = FetchIntArg(&ThisArg);
      long long Count = FetchIntArg(&ThisArg);
      int Index;
    
      if (g_CurrentBuf->EditLock & ReadOnly) {
        Fail("Attempt to modify a readonly buffer");
        continue; }
      g_CurrentBuf->UnchangedStatus = 0;
       
      for (Index = 1; Index <= Count; Index +=1) { // Record-loop. 
        BreakRecord(g_CurrentBuf);
        if (g_Failure)
          break;
        if (Direction < 0) { // Backwards.
          g_CurrentBuf->CurrentRec = g_CurrentBuf->CurrentRec->prev;
          g_CurrentBuf->CurrentChr = strlen(g_CurrentBuf->CurrentRec->text);
          g_CurrentBuf->LineNumber -= 2; }
        else
          g_CurrentBuf->LineNumber -= 1; }
    
      if (g_Failure)
        continue;
      break; }

    case 'T':  //T - set Trace mode.
      if (s_TraceCounter)
        s_TraceCounter--;
      else
        g_TraceMode = g_DefaultTraceMode;
      break;

    case 'J': { //J - Join line(s).
      long long Direction = FetchIntArg(&ThisArg);
      long long Count = FetchIntArg(&ThisArg);
      int Joins = 0;
    
      if (g_CurrentBuf->EditLock & ReadOnly) {
        Fail("Attempt to modify a readonly buffer");
        continue; }
      g_CurrentBuf->UnchangedStatus = 0;
      
      if (1 <= Direction)
        while (g_CurrentBuf->CurrentRec->next != g_CurrentBuf->FirstRec && Joins++ < Count)
          JoinRecords(g_CurrentBuf);
      else
        while (g_CurrentBuf->CurrentRec->prev != g_CurrentBuf->FirstRec->prev && Joins++ < Count) {
          AdvanceRecord(g_CurrentBuf, -1);
          JoinRecords(g_CurrentBuf); }
          
      if ( (Joins < Count) && (Count != INT_MAX) )
        continue;
      break; }

    case 'F': { //F - Find Command.
      long long Direction = FetchIntArg(&ThisArg);
      long long SearchSpan = FetchIntArg(&ThisArg);
      char * StringArgument = FetchStringArg(&ThisArg, s_SearchString);
      long long Count = FetchIntArg(&ThisArg);
      int Traverse = FALSE;
      
      if (g_Failure) 
        continue;
      if (Count < 0) { 
        Traverse = TRUE;
        Count = -Count; }
      for ( ; (0 < Count)  && ( ! g_Failure); Count--) {
        for ( ; ; ) { // Record loop, if not found move record pointer.
          int InitialPointer = g_CurrentBuf->CurrentChr;
          if (SearchRecord(g_CurrentBuf, StringArgument, Direction)) // Substring found.
            break;
          else if ( ! --SearchSpan) { // Hit the endstop.
            g_CurrentBuf->CurrentChr = InitialPointer;
            g_Failure = TRUE;
            break; }
          if (g_Failure |= AdvanceRecord(g_CurrentBuf, Direction))
            break;
          if (Direction < 0)
            g_CurrentBuf->CurrentChr = strlen(g_CurrentBuf->CurrentRec->text); } }
          
      if (g_Failure)
        continue;
      if (Traverse) {
        g_CurrentBuf->CurrentChr = (g_CurrentBuf->CurrentChr) + (g_CurrentBuf->SubstringLength);
        g_CurrentBuf->SubstringLength = - g_CurrentBuf->SubstringLength; }
      break; }

    case 'U': { //U  - hashtable lookUp.
      char * StringArgument = (char *)FetchStringArg(&ThisArg, s_SearchString);
      struct Buf * TargetBuf = QueryKey(StringArgument, g_CurrentBuf);
      if ( ! TargetBuf)
        continue;
      g_CurrentBuf = TargetBuf;
      break; }

    case 'G':  //G - Get - Input text from console.
      { //Get block. 
      struct Rec *PrevRecordSave = g_CurrentBuf->CurrentRec->prev;
      long long Count = FetchIntArg(&ThisArg);
      int Infinate = (Count == INT_MAX);
      
      if (g_CurrentBuf->EditLock & ReadOnly) {
        Fail("Attempt to modify a readonly buffer");
        continue; }
      g_CurrentBuf->UnchangedStatus = 0;
      if (g_ScreenWidth < strlen(s_PromptString)) {
        Message(NULL, "Prompt string truncated to %d characters", g_ScreenWidth-1);
        s_PromptString[g_ScreenWidth-1] = '\0'; }
    
      for ( ; Count; Count--) { // Insert-records loop.
        struct Rec *Rec;
    
        NewScreenLine();
        Prompt("%s", strlen(s_PromptString) ? s_PromptString : "> ");
        g_CurrentBuf->CurrentRec = g_CurrentBuf->CurrentRec->prev;
        if ( ! ReadNewRecord(g_CurrentBuf, g_asConsole ? g_asConsole->FileHandle : g_EditorConsole->FileHandle, FALSE)) {
          g_CurrentBuf->LineNumber--;
          if (s_JournalHand)
            UpdateJournal("<<G-command terminated>>", NULL);
          break; }
        Rec = g_CurrentBuf->CurrentRec;
        if ((strlen(Rec->text) == 1) && ((Rec->text)[0] == ':')) { // Last record. 
          FreeRecord(g_CurrentBuf, AdjustBack);
          g_Failure = TRUE;
          g_CurrentBuf->LineNumber--;
          break; }
        g_CurrentBuf->CurrentRec = Rec->next;
        g_EditorInput->LineNo++;
        JotUpdateWindow();
        if (g_Failure)
          break; }
  
      if ((g_CurrentBuf->CurrentRec) == (g_CurrentBuf->FirstRec))
        g_CurrentBuf->FirstRec = PrevRecordSave->next;
      if ( ! Infinate && (0 < Count) )
        continue;
      break; }

    case 'H': { //H - Here command.
      char BufferKey = FetchIntArg(&ThisArg);
      long long OverlayMode = FetchIntArg(&ThisArg);
      long long Count = FetchIntArg(&ThisArg);
      int OriginalCurrentChr = g_CurrentBuf->CurrentChr;
      int OriginalLineNumber = g_CurrentBuf->LineNumber;
      int Index;
      int Reverse = FALSE;
      int OrigSourceLineNumber, OrigSourceCurrentChr, OrigSourceSubstrLen;
      struct Rec * OrigSourceCurrentRec;
      struct Buf * SourceBuf = GetBuffer(BufferKey, NeverNew);
      
      if (  (! SourceBuf) || ! CheckBufferKey(BufferKey) || (SourceBuf == g_CurrentBuf)) {
        Fail("Invalid buffer.");
        continue; }
      OrigSourceCurrentRec = SourceBuf->CurrentRec;
      OrigSourceLineNumber = SourceBuf->LineNumber;
      OrigSourceCurrentChr = SourceBuf->CurrentChr;
      OrigSourceSubstrLen = SourceBuf->SubstringLength;
      if (g_CurrentBuf->EditLock & ReadOnly) {
        Fail("Attempt to modify a readonly buffer");
        continue; }
      g_CurrentBuf->UnchangedStatus = 0;
      ResetBuffer(SourceBuf);
      if (Count < 0) {
        Count = -Count;
        Reverse = TRUE; }
      
      for (Index = 1; Index <= Count; Index +=1) { // Copy-buffer loop. 
        if (OverlayMode) { // Overwrite existing records. 
          for ( ; ; ) { // Copy loop.
            int TextLength = strlen(g_CurrentBuf->CurrentRec->text);
            int SubstringLength = strlen(SourceBuf->CurrentRec->text);
            if (OriginalCurrentChr+SubstringLength <= TextLength)                          //Easy case - whole of new substring is within the original text string.
              g_CurrentBuf->SubstringLength = SubstringLength;
            else if (OriginalCurrentChr <= TextLength)                                     //SubstituteString will extend the string - just set the substring correctly.
              g_CurrentBuf->SubstringLength = TextLength-OriginalCurrentChr;
            else if (TextLength < OriginalCurrentChr) {                                    //Whole substring is beyond of the original text - get a bigger record.
                int NewTextLength = OriginalCurrentChr+SubstringLength+1;
                char * NewText = (char *)malloc(NewTextLength+1);
                strcpy(NewText, g_CurrentBuf->CurrentRec->text);
                memset(NewText+TextLength, ' ', NewTextLength-TextLength);
                NewText[NewTextLength] = '\0';
                g_CurrentBuf->CurrentRec->length = NewTextLength;
                free(g_CurrentBuf->CurrentRec->text);
                g_CurrentBuf->CurrentRec->text = NewText; 
                g_CurrentBuf->SubstringLength = SubstringLength; }
            g_CurrentBuf->CurrentChr = OriginalCurrentChr;
            SubstituteString(g_CurrentBuf, SourceBuf->CurrentRec->text, -1);
            if (AdvanceRecord(SourceBuf, 1))
              break;
            if (AdvanceRecord(g_CurrentBuf, 1)) {
              int NewTextLen = OriginalCurrentChr+strlen(SourceBuf->CurrentRec->text)+1;
              GetRecord(g_CurrentBuf, NewTextLen);
              memset(g_CurrentBuf->CurrentRec->text, ' ', NewTextLen);
              g_CurrentBuf->CurrentRec->text[NewTextLen] = '\0'; } }
            
          g_CurrentBuf->SubstringLength = 0;
          g_Failure = FALSE; }
        else { // Normal copy - non-block mode. 
          if (SourceBuf->AbstractWholeRecordFlag )
            g_CurrentBuf->CurrentChr = 0;
          BreakRecord(g_CurrentBuf);
          AdvanceRecord(g_CurrentBuf, -1);
          DuplicateRecord(g_CurrentBuf, SourceBuf);
          g_Failure |= AdvanceRecord(g_CurrentBuf, -1);
          JoinRecords(g_CurrentBuf);
    
          for (;;) { // Record loop. 
            g_Failure |= AdvanceRecord(SourceBuf, 1);
            if (g_Failure)
              break;
            DuplicateRecord(g_CurrentBuf, SourceBuf); }
    
          g_Failure = FALSE;
          JoinRecords(g_CurrentBuf);
          if (g_Failure)
            break;
          g_CurrentBuf->LineNumber -= 1; } 
        ResetBuffer(SourceBuf); } // Copy-buffer loop ends.
           
      if (Reverse) {
        MoveDown(g_CurrentBuf, g_CurrentBuf->LineNumber-OriginalLineNumber, NULL);
        g_CurrentBuf->CurrentChr = OriginalCurrentChr; }
      SourceBuf->CurrentRec = OrigSourceCurrentRec;
      SourceBuf->LineNumber = OrigSourceLineNumber;
      SourceBuf->CurrentChr = OrigSourceCurrentChr;
      SourceBuf->SubstringLength = OrigSourceSubstrLen;
      if (g_Failure)
        continue;
      break; }

    case 'S': { //S - Substitute command.
      long long Arg1 = FetchIntArg(&ThisArg);
      char * StringArgument = (char *)FetchStringArg(&ThisArg, s_InsertString);
       
      if (g_CurrentBuf->EditLock & ReadOnly) {
        Fail("Attempt to modify a readonly buffer");
        continue; }
      g_CurrentBuf->UnchangedStatus = 0;
       
      if ((g_CurrentBuf->SubstringLength) == 0)
        continue;
      else { // Non-null substitute string. 
        SubstituteString(g_CurrentBuf, StringArgument, -1);
        if (0 < Arg1) {
          g_CurrentBuf->SubstringLength = -g_CurrentBuf->SubstringLength;
          g_CurrentBuf->CurrentChr = (g_CurrentBuf->CurrentChr) - (g_CurrentBuf->SubstringLength); } }
      break; }

    case 'I': { //I - Insert command.
      long long Direction = FetchIntArg(&ThisArg);
      char * StringArgument = (char *)FetchStringArg(&ThisArg, s_InsertString);
      long long Arg2 = FetchIntArg(&ThisArg);
      int RepCount;
        
      if (g_CurrentBuf->EditLock & ReadOnly) {
        Fail("Attempt to modify a readonly buffer");
        continue; }
      g_CurrentBuf->UnchangedStatus = 0;
      
      if (Arg2 <= 0)
        break;
       
      for (RepCount = 1; RepCount <= Arg2; RepCount +=1) { // Repeat loop. 
        g_CurrentBuf->SubstringLength = 0;
        SubstituteString(g_CurrentBuf, StringArgument, -1); }
    
      if (0 < Direction) {
        g_CurrentBuf->SubstringLength = -strlen(StringArgument);
        g_CurrentBuf->CurrentChr = (g_CurrentBuf->CurrentChr) - (g_CurrentBuf->SubstringLength); }
      else
        g_CurrentBuf->SubstringLength = strlen(StringArgument);
      if (g_Failure)
        continue;
      break; }

//    case 'D':   //D - Delete characters.
    case 'E': { //E - Erase bytes or characters.
      long long Direction = FetchIntArg(&ThisArg);
      long long Count = FetchIntArg(&ThisArg);
      int NoUnicode = g_CurrentBuf->NoUnicode;
      
      if (g_CurrentBuf->EditLock & ReadOnly) {
        Fail("Attempt to modify a readonly buffer");
        continue; }
      g_CurrentBuf->UnchangedStatus = 0;
       
      if (Direction < 0) { // -ve. - Erase left Count charcaters. 
        int Current = g_CurrentBuf->CurrentChr;
        
        if (Count == INT_MAX)
          Count = Current;
        else if (! NoUnicode) { //Delete a finite no. of characters left of current character.
          int CurrentChr = g_CurrentBuf->CurrentChr;
          g_Failure |= ShiftRight(g_CurrentBuf, -Count);
          Count = CurrentChr-g_CurrentBuf->CurrentChr;
          g_CurrentBuf->CurrentChr = CurrentChr; }
        if (Current < Count)
          continue; 
        g_CurrentBuf->CurrentChr -= Count; }
      else { // +ve. - Erase right. 
        int Length = strlen(g_CurrentBuf->CurrentRec->text);
        int Current = g_CurrentBuf->CurrentChr;
  
        if (Count == INT_MAX)
          Count = Length-Current;
        else if (! NoUnicode) { //Delete a finite no. of characters right of current character.
          int CurrentChr = g_CurrentBuf->CurrentChr;
          g_Failure |= ShiftRight(g_CurrentBuf, Count);
          Count = g_CurrentBuf->CurrentChr-CurrentChr;
          g_CurrentBuf->CurrentChr = CurrentChr; }
//        else if (! NoUnicode) //Delete a finite no. of characters right of current character.
//          Count = JotStrlenChrs(g_CurrentBuf->CurrentRec->text + g_CurrentBuf->CurrentChr, Count);
        if (Length-Current < Count)
          continue; }
        
      g_CurrentBuf->SubstringLength = Count;
      SubstituteString(g_CurrentBuf, NULL, -1);
      if (g_Failure)
        continue;
      break; }

    case 'C':  //C - Change case of characters.
      { // Change case of alpha characters. 
      long long Direction = FetchIntArg(&ThisArg);
      long long Count = FetchIntArg(&ThisArg);
      char *RecordText = g_CurrentBuf->CurrentRec->text;
      int Current = g_CurrentBuf->CurrentChr;
      int Chr, Last;
      
      if (g_CurrentBuf->EditLock & ReadOnly) {
        Fail("Attempt to modify a readonly buffer");
        continue; }
       
      if (Direction < 0) { // -ve change hop back first.
        Chr = (Count == INT_MAX) ? 0 : Current-Count;
        if (Chr < 0)
          continue;
        Last = Current;
        g_CurrentBuf->CurrentChr = Chr; }
      else { // +ve change.
        int Length = strlen(RecordText);
          
        Last = (Count == INT_MAX) ? Length : Current+Count;
        if (Length < Last)
          continue;
        Chr = Current;
        g_CurrentBuf->CurrentChr = Last; }
      
      for ( ; Chr != Last; Chr++) { // Case-change loop.
        int C = RecordText[Chr];
        
        RecordText[Chr] = (('a' <=  C && C <= 'z') ? C-'a'+'A' : (('A' <=  C && C <= 'Z') ? C-'A'+'a' : C)); }
      
      g_CurrentBuf->SubstringLength = 0;
      g_CurrentBuf->CurrentRec->DisplayFlag = Redraw_Line;
      g_CurrentBuf->UnchangedStatus = 0;
      break; }

    case 'R': { //R - move current character pointer Right bytes.
      int Displacement = FetchIntArg(&ThisArg);
      g_CurrentBuf->SubstringLength = 0; 
      if (Displacement == INT_MAX)
        g_CurrentBuf->CurrentChr = strlen(g_CurrentBuf->CurrentRec->text);
      else if (Displacement == INT_MIN)
        g_CurrentBuf->CurrentChr = 0;
      else
        g_Failure = ShiftRight(g_CurrentBuf, Displacement);
      if (g_Failure)
        continue;
      break; }
//      int CurrentChr = g_CurrentBuf->CurrentChr, Displacement = FetchIntArg(&ThisArg);
//      g_CurrentBuf->SubstringLength = 0;
//      if (Displacement == INT_MAX) {
//        g_CurrentBuf->SubstringLength = 0; 
//        g_CurrentBuf->CurrentChr = strlen(g_CurrentBuf->CurrentRec->text); }
//      else if (Displacement == INT_MIN) {
//        g_CurrentBuf->SubstringLength = 0; 
//        g_CurrentBuf->CurrentChr = 0; }
//      else {
//        int Length = strlen(g_CurrentBuf->CurrentRec->text);
//        if ( (0 < Displacement) && (Length < CurrentChr+Displacement) ) {
//          g_CurrentBuf->CurrentChr = strlen(g_CurrentBuf->CurrentRec->text);
//          continue; }
//        else if (CurrentChr+Displacement < 0) {
//          g_CurrentBuf->CurrentChr = 0;
//          continue; }
//        g_CurrentBuf->CurrentChr += Displacement; }
//      break; }

//    case 'L': { //L - move character pointer Left bytes.
//      int CurrentChr = g_CurrentBuf->CurrentChr, Displacement = FetchIntArg(&ThisArg);
//      g_CurrentBuf->SubstringLength = 0;
//      if (Displacement == INT_MAX) {
//        g_CurrentBuf->SubstringLength = 0; 
//        g_CurrentBuf->CurrentChr = 0; }
//      else if (Displacement == INT_MIN) {
//        g_CurrentBuf->SubstringLength = 0; 
//        g_CurrentBuf->CurrentChr = strlen(g_CurrentBuf->CurrentRec->text); }
//      else {
//        int Length = strlen(g_CurrentBuf->CurrentRec->text);
//        if (CurrentChr-Displacement < 0) {
//          g_CurrentBuf->CurrentChr = 0;
//          continue; }
//        else if ( (0 < -Displacement) && (Length < g_CurrentBuf->CurrentChr-Displacement) ) {
//          g_CurrentBuf->CurrentChr = strlen(g_CurrentBuf->CurrentRec->text);
//          continue; }
//        g_CurrentBuf->CurrentChr = CurrentChr-Displacement; }
//      break; }

//    case 'X': { //X - move current character pointer Right (Left if -ve. n) by n characters.
//      int Displacement = FetchIntArg(&ThisArg);
//      g_CurrentBuf->SubstringLength = 0; 
//      if (Displacement == INT_MAX)
//        g_CurrentBuf->CurrentChr = strlen(g_CurrentBuf->CurrentRec->text);
//      else if (Displacement == INT_MIN)
//        g_CurrentBuf->CurrentChr = 0;
//      else
//        g_Failure = ShiftRight(g_CurrentBuf, Displacement);
//      if (g_Failure)
//        continue;
//      break; }

    case 'K': { //K - Kill some lines.
      long long Direction = FetchIntArg(&ThisArg);
      long long Count = FetchIntArg(&ThisArg);
      int Index;
    
      if (g_CurrentBuf->EditLock & ReadOnly) {
        Fail("Attempt to modify a readonly buffer");
        continue; }
      g_CurrentBuf->SubstringLength = 0;
      g_CurrentBuf->CurrentChr = 0;
       
      for (Index = 1; Index <= Count; Index+=1) { // Record Loop. 
        if ((g_CurrentBuf->CurrentRec->next) == (g_CurrentBuf->FirstRec)) { //Last record.
          struct Tag * ThisTag = g_CurrentBuf->CurrentRec->TagChain;
          g_CurrentBuf->CurrentRec->text[0] = '\0';
          //Check and adjust tags.
          while (ThisTag) {
            ThisTag->StartPoint = 0;
            if (ThisTag->Type == TargetType)
              ((struct HtabEntry * )ThisTag->Attr)->CurrentChr = 0;
            ThisTag = ThisTag->next; }
          g_Failure = TRUE; }
        if (g_Failure)
          break;
        FreeRecord(g_CurrentBuf, (Direction == 1) ? AdjustBack : AdjustForwards);
        g_CurrentBuf->UnchangedStatus = 0;
        if ( ! g_CurrentBuf->CurrentRec) {
          GetRecord(g_CurrentBuf, 1);
          break; }
        if (Direction < 0)
          g_Failure |= AdvanceRecord(g_CurrentBuf, -1); }
          
      if (g_Failure && (Count != INT_MAX))
        continue;
      break; }

    case 'P': { //P - Print line(s).
      long long Direction = FetchIntArg(&ThisArg);
      long long Count = FetchIntArg(&ThisArg);
      int Index;
       
      for (Index = 1; Index <= Count; Index++) { // Print loop. 
        if (Index == 1)
          DisplayRecord(g_CurrentBuf, 1);
        else {
          g_Failure |= AdvanceRecord(g_CurrentBuf, Direction);
          if (g_Failure)
            break;
          DisplayRecord(g_CurrentBuf, 0);
        if (s_Verbose == 0 && g_TTYMode)
          putchar('\n'); } }
         
#if defined(LINUX)
  refresh();
#endif
      if (g_Failure && (Count != INT_MAX))
        continue;
      break; }

    case 'W':  //W - update Window command.
      JotUpdateWindow();
      break;

    case 'O': { //O - Stack Operation.
      char Chr = FetchIntArg(&ThisArg);
      switch (Chr) {//Stack operation switch block.
      case '@': { //O@ reset stack
        long long Seed = (0 < g_StackPtr) && ((g_Stack[g_StackPtr-1].type == StackIntFrame)) ? PopInt() : 1;
        srand(Seed);
        while (g_StackPtr && ! KillTop())
          continue;
        break; }
         
      case '?': //O? Dump contents of stack
        DumpStack();
        break;
         
      case 'A': //OA rAndom - push a random number to top of stack.
        PushInt(rand());
        break;
       
      case 'W': { //OW Scrolls up by lines set in top of stack.
        struct Window *Win = g_FirstWindow;
        int WindowHeight, Offset;
         
        if (g_StackPtr < 1) {
          Fail("Stack underflow");
          continue; }
        if ( ! (Offset = PopInt()) )
          break;
        if ( ! Win)
          continue;
        g_FirstWindowLine = 1;
        while (Win != NULL) {
          WindowHeight = (Win->WindowHeight)-1;
          g_LastWindowLine = g_FirstWindowLine+WindowHeight;
          if ( ((Win->WindowKey) == '\0') || (Win->WindowKey == g_CurrentBuf->BufferKey) )
            break;
          g_FirstWindowLine = g_LastWindowLine+1;
          Win = Win->NextWindow; }
          
        if (g_CurrentBuf->OldLastLineNumber+Offset <= WindowHeight)
          Offset = WindowHeight-g_CurrentBuf->OldLastLineNumber+1;
        g_CurrentBuf->OldLastLineNumber += Offset;
        if (Win)
          Win->OldLastLineNumber = g_CurrentBuf->OldLastLineNumber;
        JotScroll(g_FirstWindowLine-1, g_LastWindowLine-1, Offset);
        JotUpdateWindow();
        break; }
           
      case 'S': { //OS Swap 1st and 2nd items of stack.
        struct intFrame TopCopy;
         
        if (g_StackPtr <= 1)
          continue;
        TopCopy.type = g_Stack[g_StackPtr-1].type;
        TopCopy.Value = g_Stack[g_StackPtr-1].Value;
        g_Stack[g_StackPtr-1].type = g_Stack[g_StackPtr-2].type;
        g_Stack[g_StackPtr-1].Value = g_Stack[g_StackPtr-2].Value;
        g_Stack[g_StackPtr-2].type = TopCopy.type;
        g_Stack[g_StackPtr-2].Value = TopCopy.Value; }
      break;
         
      case '#': { //O# Duplicate top of stack.
        if (g_StackPtr < 1) {
          Fail("Stack empty");
          continue; }
        if (g_StackSize <= g_StackPtr) {
          Fail("Stack overflow");
          continue; }
        g_Stack[g_StackPtr].Value = g_Stack[g_StackPtr-1].Value;
//        g_Stack[g_StackPtr].type = g_Stack[(g_StackPtr++)-1].type;
        g_Stack[g_StackPtr].type = g_Stack[g_StackPtr-1].type;
        g_StackPtr += 1;
        if (g_Failure)
          continue;
        break; }
         
      case 'L':  //OL Literal number to be pushed onto stack.
        PushInt(FetchIntArg(&ThisArg));
        if (g_Failure)
          continue;
        break;
         
      case '=': { //O= Fail if intergerized top two items on stack unequal.
        long long TopValue, NewTopValue;
        if (g_StackPtr < 2) {
          Fail("Stack underflow");
          continue; }
        if ((g_Stack[g_StackPtr-1].type == StackBufFrame) || (g_Stack[g_StackPtr-2].type == StackBufFrame)) {
          Fail("Buffer types not allowed in arithmetic operations.");
          continue; }
//        if (g_Stack[g_StackPtr-1].type == StackBufFrame) {
//          Fail("Attempt to compare a buffer object on the stack");
//          continue; }
        TopValue = (g_Stack[g_StackPtr-1].type == StackIntFrame) ? PopInt() : (int)PopFloat();
        NewTopValue = (g_Stack[g_StackPtr-1].type == StackIntFrame) ? g_Stack[g_StackPtr-1].Value  : ((struct floatFrame *)(&g_Stack[g_StackPtr-1]))->fValue;
        
        if (TopValue != NewTopValue) 
          continue;
        break; }
        
      case '<': { //O< Test that item at top greater than (top-1).
        if (g_StackPtr < 2) {
          Fail("Stack underflow");
          continue; }
        if (g_Stack[g_StackPtr-1].type == StackBufFrame) {
          Fail("Attempt to compare a buffer object on the stack");
          continue; }
        if ( (g_Stack[g_StackPtr-1].type == StackIntFrame) && (g_Stack[g_StackPtr-2].type == StackIntFrame) ) { //Both items are integer - no conversion required.
          long long OldTopValue = PopInt();
          if (OldTopValue > g_Stack[g_StackPtr-1].Value)
            break; 
          continue; }
        else if ((g_Stack[g_StackPtr-1].type == StackBufFrame) || (g_Stack[g_StackPtr-2].type == StackBufFrame)) {
          Fail("Buffer types not allowed in arithmetic operations.");
          continue; }
        else { 
          double OldTopValue = (g_Stack[g_StackPtr-1].type == StackIntFrame) ? (double)PopInt() : PopFloat();
          double NewTopValue = (g_Stack[g_StackPtr-1].type == StackIntFrame) ? (double)g_Stack[g_StackPtr-1].Value : ((struct floatFrame *)(&g_Stack[g_StackPtr-1]))->fValue;
          if (OldTopValue > NewTopValue)
            break; 
          continue; } }
         
      case '>': { //O> Test that item at top less than (top-1).
        if (g_StackPtr < 2) {
          Fail("Stack underflow");
          continue; }
        if ( (g_Stack[g_StackPtr-1].type == StackIntFrame) && (g_Stack[g_StackPtr-2].type == StackIntFrame) ) { //Both items are integer - no conversion required.
          long long OldTopValue = PopInt();
          if (OldTopValue < (long long)g_Stack[g_StackPtr-1].Value)
            break; 
          continue; }
        else if ((g_Stack[g_StackPtr-1].type == StackBufFrame) || (g_Stack[g_StackPtr-2].type == StackBufFrame)) {
          Fail("Buffer types not allowed in arithmetic operations.");
          continue; }
        else { 
          double OldTopValue = (g_Stack[g_StackPtr-1].type == StackIntFrame) ? (double)PopInt() : PopFloat();
          double NewTopValue = (g_Stack[g_StackPtr-1].type == StackIntFrame) ? (double)g_Stack[g_StackPtr-1].Value : ((struct floatFrame *)(&g_Stack[g_StackPtr-1]))->fValue;
          if (OldTopValue < NewTopValue)
            break; 
          continue; } }
         
         
      case 'K':  //OK Kill - delete top of stack.
        KillTop();
        if (g_Failure)
          continue;
        break;

      case 'I': { //OI formatted Input - Convert chr(s) at CurentChr using format key.
        char FormatChr = ChrUpper(FetchIntArg(&ThisArg));
        int Length;
        
        if (FormatChr == 'C') { //OIC
          if (strlen(g_CurrentBuf->CurrentRec->text) <= g_CurrentBuf->CurrentChr)
            continue;
          PushInt(ImmediateCharacter(g_CurrentBuf));
          g_CurrentBuf->SubstringLength = -1; }
        else if (FormatChr == 'D') { //OID
          long long IntValue;
          if (sscanf((g_CurrentBuf->CurrentRec->text)+(g_CurrentBuf->CurrentChr), "%lld%n", &IntValue, &Length) <= 0)
            continue;
          else {
            PushInt(IntValue);
            g_CurrentBuf->CurrentChr += Length;
            g_CurrentBuf->SubstringLength = -Length;
            break; } }
        else if (FormatChr == 'X') { //OIX
          long long IntValue;
          if (sscanf((g_CurrentBuf->CurrentRec->text)+(g_CurrentBuf->CurrentChr), "%llx%n", &IntValue, &Length) <= 0)
            continue;
          else {
            PushInt(IntValue);
            g_CurrentBuf->CurrentChr += Length;
            g_CurrentBuf->SubstringLength = -Length;
            break; } }
        else if (FormatChr == 'O') { //OIO
          long long IntValue;
          if (sscanf((g_CurrentBuf->CurrentRec->text)+(g_CurrentBuf->CurrentChr), "%llo%n", &IntValue, &Length) <= 0)
            continue;
          else {
            PushInt(IntValue);
            g_CurrentBuf->CurrentChr += Length;
            g_CurrentBuf->SubstringLength = -Length;
            break; } }
#if defined(VC)
        else if (FormatChr == 'F') { //OIF
          float Float;
          if (sscanf((g_CurrentBuf->CurrentRec->text)+(g_CurrentBuf->CurrentChr), "%f%n", &Float, &Length) <= 0) {
            PushFloat(1.0);
            Fail("Error converting floating-point number");
            continue; }
          else {
            PushFloat((double)Float);
            g_CurrentBuf->CurrentChr += Length;
            g_CurrentBuf->SubstringLength = -Length;
            break; } }
#else
        else if (FormatChr == 'F') { //OIF
          char * EndPtr;
          float Float = strtof((g_CurrentBuf->CurrentRec->text)+(g_CurrentBuf->CurrentChr), &EndPtr);
          errno = 0;
          Length = EndPtr-(g_CurrentBuf->CurrentRec->text);
          if (errno) {
            PushFloat(1.0);
            Fail("Error converting floating-point number");
            continue; }
          else {
            PushFloat(Float);
            g_CurrentBuf->CurrentChr += Length;
            g_CurrentBuf->SubstringLength = -Length;
            break; } }
#endif
        else 
          continue;
        break; }

      case 'O': { //OO Output to current text, using given string as format spec.
        char * StringArgument = (char *)FetchStringArg(&ThisArg, s_TempBuf->CurrentRec->text);
        char TempString[1000];
         
        if (g_CurrentBuf->EditLock & ReadOnly) {
          Fail("Attempt to modify a readonly buffer");
          continue; }
        g_CurrentBuf->UnchangedStatus = 0;
       
        if (g_StackPtr < 1) {
          Fail("Stack underflow");
          continue; }
        if (g_Stack[g_StackPtr-1].type == StackBufFrame) {
          struct Buf * Buf = ((struct bufFrame *)(&g_Stack[g_StackPtr-1]))->BufPtr;
          sprintf(TempString, StringArgument, Buf->CurrentRec->text); }
        else if (g_Stack[g_StackPtr-1].type == StackFloatFrame)
          sprintf(TempString, StringArgument, ((struct floatFrame *)(&g_Stack[g_StackPtr-1]))->fValue);
        else { //An integer 
          sprintf(TempString, StringArgument, g_Stack[g_StackPtr-1].Value); }
        KillTop();
        SubstituteString(g_CurrentBuf, TempString, -1);
        g_CurrentBuf->SubstringLength = -strlen(TempString);
        g_CurrentBuf->CurrentChr = g_CurrentBuf->CurrentChr - g_CurrentBuf->SubstringLength;
        break; }

      case '~': { //O~ Increment - (item at top of stack)++, fail if result=0.
        int ActualType;
        if (g_StackPtr < 1)
          continue;
        if ( (ActualType = g_Stack[g_StackPtr-1].type) != StackIntFrame) {
          RunError("Incorrect stack-frame type, expecting integer frame, found frame type %d", ActualType);
          continue; }
        if (++g_Stack[g_StackPtr-1].Value)
          break;
        continue; }
        
      case '!': { //O! bitwise NOT
        int ActualType;
        if (g_StackPtr < 1)
          continue;
        if ( (ActualType = g_Stack[g_StackPtr-1].type) != StackIntFrame) {
          RunError("Incorrect stack-frame type, expecting integer frame, found frame type %d", ActualType);
          continue; }
        g_Stack[g_StackPtr-1].Value = g_Stack[g_StackPtr-1].Value ^ -1;
        break; }
        
      case '&':   //O& - bitwise OR
      case '|': { //O| - bitwise AND
        if (g_StackPtr < 2) {
          Fail("Stack underflow");
          continue; }
        if ((g_Stack[g_StackPtr-1].type != StackIntFrame) && (g_Stack[g_StackPtr-2].type != StackIntFrame)) {
          RunError("Non integer types not allowed in bitwise AND/OR operations");
          continue; }
        PushInt(Chr == '&' ? PopInt()&PopInt() : PopInt()|PopInt());
        break; }
           
      case '+':   //O+ Add
      case '-':   //O- Subtract
      case '*':   //O* Multiply
      case '/':   //O/ Divide
      case '%': { //O% Remainder
        if (g_StackPtr < 2) {
          Fail("Stack underflow");
          continue; }
        if ((g_Stack[g_StackPtr-1].type == StackIntFrame) && (g_Stack[g_StackPtr-2].type == StackIntFrame)) {
          long long Arg2 = PopInt();
          long long Arg1 = PopInt();
          switch (Chr) {//Two-operand-stack-operations switch block.
            case '+':
              PushInt(Arg1+Arg2);
              break;
            case '-':
              PushInt(Arg1-Arg2);
              break;
            case '*':
              PushInt(Arg1*Arg2);
              break;
            case '/':
              if (Arg2 == 0) { // Divide by zero.
                g_StackPtr = g_StackPtr+2;
                continue; }
              PushInt(Arg1/Arg2);
              break;
            case '%':  //Remainder
              if (Arg2 == 0) { // Divide by zero.
                g_StackPtr = g_StackPtr+2;
                continue; }
              PushInt(Arg1%Arg2);
              break; } }
        else if ((g_Stack[g_StackPtr-1].type == StackBufFrame) || (g_Stack[g_StackPtr-2].type == StackBufFrame)) {
          Fail("Buffer types not allowed in arithmetic operations.");
          continue; }
        else if (Chr == '%') { //O% - convert both items to integer.
          long long Arg2 = (g_Stack[g_StackPtr-1].type == StackFloatFrame) ? (int)PopFloat() : PopInt();
          long long Arg1 = (g_Stack[g_StackPtr-1].type == StackFloatFrame) ? (int)PopFloat() : PopInt();
          if (Arg2 == 0) { // Divide by zero.
            g_StackPtr = g_StackPtr+2;
            continue; }
          PushInt(Arg1%Arg2);
          break; }
        else {
          double Arg2 = (g_Stack[g_StackPtr-1].type == StackIntFrame) ? (double)PopInt() : PopFloat();
          double Arg1 = (g_Stack[g_StackPtr-1].type == StackIntFrame) ? (double)PopInt() : PopFloat();
          switch (Chr) {//Two-operand-stack-operations switch block.
            case '+':
              PushFloat(Arg1+Arg2);
              break;
            case '-':
              PushFloat(Arg1-Arg2);
              break;
            case '*':
              PushFloat(Arg1*Arg2);
              break;
            case '/':
              if (Arg2 == 0.0) { // Divide by zero.
                g_StackPtr = g_StackPtr+1;
                continue; }
              PushFloat(Arg1/Arg2);
              break; } } }
          break;
    
      case '.':  //O. Set current line number to <top of stack>.
//        if (g_StackPtr < 1) {
//          Fail("Stack underflow");
//          continue; }
        g_CurrentBuf->LineNumber = PopInt();
        break;
         
      case 'B':  //OB Push current buffer key onto stack.
        PushInt(g_CurrentBuf->BufferKey);
        if (g_Failure)
          continue;
        break;
         
      case 'E': { //OE Push Extent (i.e. no. of substring bytes then characters) taking unicode characters into account.
        int ByteCount = g_CurrentBuf->SubstringLength ? g_CurrentBuf->SubstringLength : strlen(g_CurrentBuf->CurrentRec->text);
        
        if (ByteCount < 0)
          ByteCount = -ByteCount;
//        PushInt(ByteCount);
        if (g_CurrentBuf->NoUnicode) //Unicode is disabled - just duplicate the byte-count entry.
          PushInt(ByteCount);
        else {  //Unicode is enabled - add true character count.
          if (g_CurrentBuf->SubstringLength) { //Length of substring in display characters.
            if (g_CurrentBuf->SubstringLength < 0)
              PushInt(JotStrlenBytes(g_CurrentBuf->CurrentRec->text+g_CurrentBuf->CurrentChr+g_CurrentBuf->SubstringLength, -g_CurrentBuf->SubstringLength));
            else 
              PushInt(JotStrlenBytes(g_CurrentBuf->CurrentRec->text+g_CurrentBuf->CurrentChr, g_CurrentBuf->SubstringLength)); }
          else //Length of entire record in display characters.
            PushInt(JotStrlenBytes(g_CurrentBuf->CurrentRec->text, strlen(g_CurrentBuf->CurrentRec->text))); }
        break; }
       
      case 'P': { //OP push mouse Position - last detected mouse-click position
        char Buf[100];
        if (s_RecoveryMode) { //In recovery mode pick up the mouse-click point from the journal file.
          ReadString(Buf, 100, g_asConsole->FileHandle);
          sscanf(Buf, "%d %d %d\033MOUSE", &g_MouseBufKey, &g_MouseLine, &g_Mouse_x); }
        else if (s_JournalHand) {
          sprintf(Buf, "%d %d %d", g_MouseBufKey, g_MouseLine, g_Mouse_x);
          UpdateJournal(Buf, "MOUSE"); }
        PushInt(g_Mouse_x);
        PushInt(g_MouseLine);
        PushInt(g_MouseBufKey);
        break; }
       
      case 'N':  //ON Push current line no. onto stack.
        PushInt(g_CurrentBuf->LineNumber);
        if (g_Failure)
          continue;
        break;
         
      case 'U': //OU set currently-selected sUbstring length from value on stack.
        if (g_CurrentBuf->NoUnicode)
          g_CurrentBuf->SubstringLength = PopInt();
        else {
          int Chrs = PopInt();
          int CurrentChr = g_CurrentBuf->CurrentChr;
          ShiftRight(g_CurrentBuf, Chrs);
          g_CurrentBuf->SubstringLength = g_CurrentBuf->CurrentChr-CurrentChr;
          g_CurrentBuf->CurrentChr = CurrentChr; }
        if (g_Failure)
          continue;
        break;
         
//      case 'Y':  //OY Push current byte no. onto stack.
//        PushInt(g_CurrentBuf->CurrentChr);
//        if (g_Failure)
//          continue;
//        break;
         
      case 'C':  //OC Push current chr. no. onto stack.
        PushInt(g_CurrentBuf->NoUnicode ? g_CurrentBuf->CurrentChr : JotStrlenBytes(g_CurrentBuf->CurrentRec->text, g_CurrentBuf->CurrentChr));
        if (g_Failure)
          continue;
        break;
         
      case 'Z': { //OZ Zoom (change to buffer) pointer popped off stack.
        long long Arg1;
        struct Buf * DestBuf;
         
        if (g_StackPtr < 1) {
          Fail("Stack underflow");
          continue; }
           
        Arg1 = PopInt();
        if ( ! Arg1 || g_Failure)
          continue;
        if ( (Arg1 <= ' ' || 'a' <= Arg1 ) && (Arg1 != '~') ) {
          Fail("Invalid buffer key 0x%X", Arg1);
          continue; }
        DestBuf = GetBuffer(Arg1, NeverNew);
        if (DestBuf == NULL)
          continue;
        if ((DestBuf->CurrentRec) == NULL)
          GetRecord(DestBuf, 1);
        g_CurrentBuf = DestBuf;
        break; }
         
      case 'R': //OR shift Right (left if -ve) no. of bytes popped off stack.
        if (g_StackPtr < 1) {
          Fail("Stack underflow");
          continue; }
        g_Failure = ShiftRight(g_CurrentBuf, PopInt());
        if (g_Failure)
          continue;
        break;
//        long long Arg1;
//        int ByteCount = g_CurrentBuf->CurrentChr, Length = strlen(g_CurrentBuf->CurrentRec->text);
//        if (g_StackPtr < 1) {
//          Fail("Stack underflow");
//          continue; }
//        Arg1 = PopInt();
//        g_CurrentBuf->CurrentChr += Arg1;
//        g_CurrentBuf->SubstringLength = 0;
//        if ( (0 < Arg1) && (Length <= ByteCount+Arg1) ) {
//          g_CurrentBuf->CurrentChr = Length;
//          g_Failure = TRUE; }
//        else if ( (Arg1 < 0) && (ByteCount+Arg1 < 0) ) {
//          g_CurrentBuf->CurrentChr = 0;
//          g_Failure = TRUE; }
//        break; }
         
//      case 'X':  //OX X-index Right (left if -ve) no. of chrs popped off stack.
//        if (g_StackPtr < 1) {
//          Fail("Stack underflow");
//          continue; }
//        g_Failure = ShiftRight(g_CurrentBuf, PopInt());
//        if (g_Failure)
//          continue;
//        break;
         
      case 'M':  //OM Move (backwards if -ve) no. of lines popped off stack.
        if (g_StackPtr < 1) {
          Fail("Stack underflow");
          continue; }
//        g_Failure |= AdvanceRecord(g_CurrentBuf, PopInt());
        if (AdvanceRecord(g_CurrentBuf, PopInt()))
          continue;
        break;
         
      default:
        RunError("Unknown stack operator");
        break; } }
         
      break;

    case 'Z': { //Z - Zoom to another buffer.
      char Chr = FetchIntArg(&ThisArg);
      struct Buf * DestBuf;
      
      if (Chr == '~')
        DestBuf = GetBuffer(Chr, NeverNew);
      else {
        if ( (Chr <= ' ') || ('z' < Chr) ) {
          Fail("Invalid buffer key");
          continue; }
        DestBuf = GetBuffer(Chr, OptionallyNew); }
      if ( ! DestBuf)
        continue;
      if ( ! DestBuf->CurrentRec)
        GetRecord(DestBuf, 1);
      g_CurrentBuf = DestBuf;
      break; }

    case '%': { //% - Percent-commands block.
      char BufferKey, * ArgString, * FoundEquals, * RightArg;
       
      s_CommandChr = FetchIntArg(&ThisArg);
      BufferKey = FetchIntArg(&ThisArg);
      ArgString = FetchStringArg(&ThisArg, NULL);
      FoundEquals = strchr(ArgString, '=');
      RightArg = FoundEquals ? FoundEquals+1 : ArgString;
       
      switch (s_CommandChr) {

      case 'O': { //%O - Write buffer to file.
        char TempString[1000];
        FILE * FileHand = NULL;
        struct Rec *Record = g_CurrentBuf->FirstRec;
        struct Rec *LastRec = g_CurrentBuf->FirstRec->prev;
    
        g_Failure = FALSE;
        ExpandDeferredString(s_TempBuf, RightArg);
        if (s_RecoveryMode) {
          if (s_Verbose & NonSilent)
            Message(NULL, "Writing to \"%s\" was disabled", g_CurrentBuf->PathName); }
        else { //Really do the write.
          OpenTextFile(NULL, &FileHand, "w", s_TempBuf->CurrentRec->text, g_CurrentBuf->PathName, TempString);
          if ( ! FileHand) {
            Fail("Failed to open file \"%s\" for writing.", g_CurrentBuf->PathName);
            continue; }
          if (s_Verbose & NonSilent) {
            NewScreenLine();
            Prompt("Writing to \"%s\"", TempString);
            if (g_TTYMode)
              NewScreenLine(); }
      
          for (;!g_Failure;) { // Write out records loop. 
            if (Record == LastRec) {
              if(0 < strlen(Record->text))
                fprintf(FileHand, "%s", Record->text);
              break; }
            fprintf(FileHand, "%s\n", Record->text);
            Record = Record->next; }
    
          g_CurrentBuf->UnchangedStatus |= SameSinceIO;
          g_Failure = !fclose(FileHand); }
        break; }

      case 'C':  //%C - Exit writing new file.
        if (g_CurrentBuf->BufferKey != '.') {
          RunError("Can only %%%%C from main buffer.");
          break; }
        if (s_RecoveryMode) {
          Message(NULL, "Writing to \"%s\" was disabled", g_CurrentBuf->PathName);
          continue; }
        if (strlen(RightArg)) {
          ExpandDeferredString(s_TempBuf, RightArg);
          ExitNormally(s_TempBuf->CurrentRec->text); }
        else
          ExitNormally("Normal exit");
        break;

      case 'A': { //%A - Exit without writing file.
        int ExitStatus = 1;
        if (FoundEquals)
          sscanf(ArgString, "%d=", &ExitStatus);
        if (strlen(RightArg)) {
          ExpandDeferredString(s_TempBuf, RightArg);
          if (TidyUp(ExitStatus, s_TempBuf->CurrentRec->text))
            continue; }
        else 
          if (TidyUp(ExitStatus, "Edit abandoned"))
            continue;
        break; }

      case 'R': { //%R - Run a command file.
        char * asConsole = NULL, * FirstBlank = NULL;
         
        ExpandDeferredString(s_TempBuf, RightArg);
        if ((FirstBlank = strchr(s_TempBuf->CurrentRec->text, ' '))) {
          FirstBlank++[0] = '\0'; 
          if ((asConsole = strstr(FirstBlank, "-asConsole")))
            FirstBlank = strchr(FirstBlank, ' '); }
        SwitchToComFile(s_TempBuf->CurrentRec->text, asConsole, FirstBlank ? FirstBlank : "");
        break; }

      case 'I': { //%I - secondary input.
        char *FileName = g_CurrentBuf->PathName;
        char Actual[StringMaxChr];
        char RecoveryPathName[StringMaxChr];
        char * FilterQual, * BinaryQual;
        struct Buf * FilterCommands = NULL;
        struct Buf * SecondaryBuf;
        int BinaryRecordSize = 0;
#if defined(VC)
        HANDLE Chan;
#else
        int Chan = 0;
#endif
        
        ExpandDeferredString(s_TempBuf, RightArg);
        FilterQual = strstr(s_TempBuf->CurrentRec->text, "-filter=");
        BinaryQual = strstr(s_TempBuf->CurrentRec->text, "-binary");
        if (FilterQual) {
          FilterCommands = GetBuffer('f', AlwaysNew);
          GetRecord(FilterCommands, strlen(s_TempBuf->CurrentRec->text)-(FilterQual-s_TempBuf->CurrentRec->text+8));
          strcpy(FilterCommands->CurrentRec->text, FilterQual+8);
          FilterQual[0] = '\0'; }
        if (BinaryQual) {
          BinaryRecordSize = 16;
          BinaryQual[0] = '\0'; }
        if (FilterQual || BinaryQual)  {
          int End = strlen(s_TempBuf->CurrentRec->text)-1;
//          int End = strlen(s_TempBuf->CurrentRec->text);
//          while (s_TempBuf->CurrentRec->text[--End] == '\s')
//            continue;
          s_TempBuf->CurrentRec->text[End] = '\0'; }
        OpenTextFile(&Chan, NULL, "r", FoundEquals ? s_TempBuf->CurrentRec->text : FileName, FileName, Actual);
        if (s_RecoveryMode) { //In recovery mode prompt then read the journal-area pathname.
          if (Chan)
#if defined(VC)
            CloseHandle(Chan);
#else
            close(Chan);
#endif
          if ( ! g_asConsole) {
            RunError("Recovery script must be run with -asConsole set");
            continue; }
          NewScreenLine();
          Prompt("Enter recovery-substitution file for %s", FoundEquals ? s_TempBuf->CurrentRec->text : FileName);
          if (ReadString(RecoveryPathName, StringMaxChr, g_asConsole->FileHandle))
            Message(NULL, "Using recovery-substitution \"%s\"", RecoveryPathName);
          else {
            RunError("Failed to read recovery-substitution pathname");
            continue; }
#if defined(VC)
          if ( ! (Chan = CreateFile(RecoveryPathName, GENERIC_READ,  0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL)))
#else
          if ( ! (Chan = open(RecoveryPathName, O_RDONLY)))
#endif
          {
            RunError("Error opening recovery-substitution file \"%s\"", RecoveryPathName);
            continue; } }
            
        if (Chan) {
          SecondaryBuf = ReadBuffer(NULL, FALSE, Chan, Actual, BufferKey, FilterCommands, BinaryRecordSize); 
#if defined(VC)
          CloseHandle(Chan);
#else
          close(Chan);
#endif
       }
        else {
          if (s_JournalHand)
            UpdateJournal("<<Read file failed>>", NULL);
          continue; }
         
        if ( ! SecondaryBuf)
          continue;
        g_CurrentBuf->UnchangedStatus |= SameSinceIO;
        break; }

      case 'D':  //%D - Define a buffer from console or last console command.
      case 'G':  //%G - Get - but from current command file (or console).
        if ( (s_CommandChr == 'G') && (g_CurrentCommand->CommandBuf != g_CommandBuf) ) //%G command from a macro - buffer set up at compile time.
          break;
        else {
          int IsCurrentBuf = (BufferKey == g_CurrentBuf->BufferKey);
          struct Buf * DestBuf = NULL;
           
          if ( ! CheckBufferKey(BufferKey))
            return FALSE;
          if ( ! (DestBuf = GetBuffer(BufferKey, AlwaysNew)))
            return FALSE;
          if ( ! IsCurrentBuf && g_CurrentBuf->BufferKey == BufferKey)
            g_CurrentBuf = DestBuf;
          if (IsCurrentBuf) 
            g_CurrentBuf = DestBuf;
          if (DestBuf == NULL)
            return FALSE;
          
          if (DestBuf->EditLock & ReadOnly) {
            Fail("Attempt to redefine a readonly buffer");
            continue; }
          DestBuf->UnchangedStatus = 0;
          DestBuf->PathName = (char * )malloc(30);
          sprintf(DestBuf->PathName, "[ Set by %%%c command ]", s_CommandChr);
         
          if (s_CommandChr == 'D') { //%D
            GetRecord(DestBuf, strlen(RightArg)+1);
            SubstituteString(DestBuf, RightArg, -1); }
          else if (s_CommandChr == 'G') { //%G
            for ( ; ; ) { // Insert records loop. 
              char *Record;
              if (g_EditorInput == g_EditorConsole) {
                NewScreenLine();
                Prompt("%s", strlen(s_PromptString) ? s_PromptString : "> "); }
              else
                g_EditorInput->LineNo++;
              if ( ! ReadNewRecord(DestBuf, g_EditorInput->FileHandle, FALSE)) {
                if (s_JournalHand)
                  UpdateJournal("<<G-command terminated>>", NULL);
                break; }
              Record = (DestBuf->CurrentRec->text);
              if ((strlen(Record) == 1) && (Record[0] == ':')) { // Last record. 
                FreeRecord(DestBuf, AdjustBack);
                break; }
              if (g_Failure)
                break; }
            ResetBuffer(DestBuf); }
          break; }

      case 'P': { //%P - Set user Prompt for G command.
        ExpandDeferredString(s_TempBuf, RightArg);
        if (strlen(s_TempBuf->CurrentRec->text)) {
          if (StringMaxChr < strlen(s_TempBuf->CurrentRec->text))
            RunError("Overlong string to %%P command truncated to %d characters", StringMaxChr);
          strncpy(s_PromptString, s_TempBuf->CurrentRec->text, StringMaxChr); }
        else
          s_PromptString[0] = '\0';
        break; }

      case 'E': //%E - Execute following CLI command line.
        if (BufferKey != '=') {
          char * NameString;
#if defined(VC)
          HANDLE Chan;
#else
          int Chan = 0;
#endif
          FILE * FileHand = NULL;
          char * FilterQual;
          struct Buf * FilterCommands = NULL;
          char * SystemCommands = (char *)malloc(strlen(RightArg)+1);
          struct Buf * SecondaryBuf;
          int StreamInBuf = FALSE;
           
          strcpy(SystemCommands, RightArg); 
          if ( (FilterQual = strstr(SystemCommands, "-filter=")) ) {
            FilterCommands = GetBuffer('f', AlwaysNew);
            GetRecord(FilterCommands, strlen(SystemCommands)-(FilterQual-SystemCommands));
            strcpy(FilterCommands->CurrentRec->text, FilterQual+8); 
            SystemCommands[FilterQual-SystemCommands] = '\0'; }
          
          if (s_RecoveryMode) {
            char RecoveryPathName[StringMaxChr];
//            int Chr;
            NewScreenLine();
            if ( ! g_asConsole) {
              RunError("Recovery script must be run with -asConsole set");
              free(SystemCommands);
              continue; }
            Prompt("Enter recovery-substitution file for %s", SystemCommands);
            do
              ReadString(RecoveryPathName, StringMaxChr, g_asConsole->FileHandle);
              while ( RecoveryPathName[0] == '\0');
            Message(NULL, "Using recovery-substitution \"%s\"", RecoveryPathName);
#if defined(VC)
            if ( ! (Chan = CreateFile(RecoveryPathName, GENERIC_READ,  0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL)))
#else
            if ( (Chan = open(RecoveryPathName, O_RDONLY)) <= 0)
#endif
              {
                RunError("Error opening recovery-substitution file \"%s\"", RecoveryPathName);
                free(SystemCommands);
                continue; } }
          else {
#if defined(VC)
            Chan = OpenWindowsCommand(SystemCommands);
#else
            if (SystemCommands[0] == '|') {
              SystemCommands[0] = ' ';
              StreamInBuf = TRUE; }
            
            if(StreamInBuf) 
              Chan = DoublePopen(g_CurrentBuf, SystemCommands);
            else
              FileHand = popen(SystemCommands, "r");
#endif
            }             
          NameString = (char *)malloc(strlen(RightArg)+100);
          sprintf(NameString, "[ From CLI command %s ]", RightArg);
            
          SecondaryBuf = ReadBuffer(FileHand, TRUE, Chan, NameString, BufferKey, FilterCommands, 0);
           
          free(SystemCommands);
          free(NameString);
          if ( ! SecondaryBuf)
            continue;
          g_CurrentBuf = SecondaryBuf;
#if defined(VC)
          CloseHandle(Chan);
#else
          if (StreamInBuf || s_RecoveryMode)
            close(Chan);
          else
            fclose(FileHand);
#endif
          }
        else { //Just run and let it mess up the screen.
          char CommandString[1000];
          int status;
           
          strcpy(CommandString, RightArg);
          status = system(CommandString);
          NewScreenLine();
          if (status != 0)
            continue; }
        break;

      case 'F': { //%F - Regular-expression search.
        int RegexSearchBuffer(struct Buf *, char , char *, int );
#if defined(NoRegEx)
        break;
#else
        int i = 0;
        int direction = 1;
        char *regEx;
         
        if (BufferKey == '-') {
          direction = -1;
          BufferKey = ChrUpper(ArgString[i++]); }
        if (BufferKey == g_CurrentBuf->BufferKey) {
          Fail("Destination buffer for %%%%F is current buffer");
          continue; }
        if (ArgString[i++] != '=')
          continue;  //Syntax error.
        regEx = ArgString+i;
        
        ExpandDeferredString(s_TempBuf, regEx);
        if (strlen(s_TempBuf->CurrentRec->text) <= 1) {
          Fail("Single-character expressions not allowed in regular-expression searches.");
          continue; }
        if (RegexSearchBuffer(g_CurrentBuf, BufferKey, s_TempBuf->CurrentRec->text, direction) == 0)
          continue;
        break;
#endif
        }

      case 'U':  //%U - Undo last substitution.
        if (g_CurrentBuf->EditLock & ReadOnly) {
          Fail("Attempt to modify a readonly buffer");
          continue; }
        g_CurrentBuf->UnchangedStatus = 0;
         
        SubstituteString(g_CurrentBuf, s_SearchString, -1);
        break;

      case 'L': { //%L - set line Length.
          int NewLength, Line;
//          struct Window *Win = g_FirstWindow;
           
          if (g_TTYMode)
            break;
          NewLength = 0;
#if defined(VC)
          GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
          {
            SMALL_RECT SrWindow = csbiInfo.srWindow;
            g_ScreenWidth = SrWindow.Right-SrWindow.Left+1;
            g_ScreenHeight = SrWindow.Bottom-SrWindow.Top+1; }
#elif defined(LINUX)
          g_ScreenHeight = getmaxy(mainWin);
          g_ScreenWidth = getmaxx(mainWin);
#endif
          if ( (ArgString == NULL) || (strlen(ArgString) == 0) )
            NewLength = g_ScreenWidth;
          else
            sscanf(ArgString, "%i", &NewLength);
          if ( (StringMaxChr < NewLength) || (g_ScreenWidth < NewLength) )
            return FALSE;
          JotGotoXY(0, g_ScreenHeight);
          
//          g_CurrentBuf->OldLastLineNumber = 0;
//          while (Win) {
//            Win->OldLastLineNumber = 0;
//            Win = Win->NextWindow; }
          
          for (Line = 1; Line <= g_ScreenHeight; Line +=1) {
            s_ScreenTable[Line] = NULL;
            JotGotoXY(0, Line-1);
            JotClearToEOL(); } }
#if defined(LINUX)
        refresh(); 
#endif
        break;

      case '%':  //%% - Comment line.
        break;

      case '~':  //%~ - Insert or display control character.
        if (strlen(RightArg) != 0) { // Insert a byte. 
          char TempString[1000];
          int Value;
           
          if (g_CurrentBuf->EditLock & ReadOnly) {
            Fail("Attempt to modify a readonly buffer");
            continue; }
          g_CurrentBuf->UnchangedStatus = 0;
           
          sscanf(RightArg, "%x", &Value);
          TempString[0] = Value;
          TempString[1] = '\0';
          SubstituteString(g_CurrentBuf, TempString, -1);
          break; }
        Message(NULL, "\"%X\"", (int)(g_CurrentBuf->CurrentRec->text)[g_CurrentBuf->CurrentChr]);
        break;

      case 'W': //%W - Set up a screen window.
        ExpandDeferredString(s_TempBuf, RightArg);
        UpdateWindowList(s_TempBuf);
        break;

      case 'X': {  //%X - Exit with a run-time error message.
        ExpandDeferredString(s_TempBuf, RightArg);
        if ( ! g_TTYMode && g_ScreenWidth < strlen(s_TempBuf->CurrentRec->text)) {
          Message(NULL, "Over-long %%X message string truncated to %d characters", g_ScreenWidth-1);
          s_TempBuf->CurrentRec->text[g_ScreenWidth-1] = '\0'; }
        Message(NULL, "%s", s_TempBuf->CurrentRec->text);
        LocalFail = TRUE;
        g_Failure = TRUE;
        s_BombOut = TRUE;
        return FALSE; }

     case 'S': { //%S - System settings and utilities.
        char Qualifier[20], ValueString[StringMaxChr];
        if (sscanf(ArgString, "=%s %s", (char *)&Qualifier, (char *)&ValueString) <=1 )
          ValueString[0] = '\0';
        if (strstr(Qualifier, "commandmode") == Qualifier) { //CommandMode -  controls command/type-into-screen mode.
          int Value, OrigValue = g_ScreenMode;
          sscanf(ValueString, "%x", &Value);
          if (ValueString[0] == '+')
            g_ScreenMode = g_ScreenMode ^ Value;
          else
            g_ScreenMode = Value;
          Message(NULL, "Command mode was %X, now set to %X", OrigValue, g_ScreenMode);
          break; }
           
        else if (strstr(Qualifier, "recoverymode") == Qualifier) { //recoverymode -  disables %O command, %I prompts and reads filename from console (the recovery script).
          int Value;
          if (sscanf(ValueString, "%d", &Value) <= 0)
            Value = 1;
          s_RecoveryMode = Value;
          Message(NULL, "%%O and %%I %sabled", s_RecoveryMode ? "dis" : "en");
          if (Value == 1) {
            s_JournalPath = (char *)malloc(strlen(g_CurrentBuf->PathName)+6);
            strcpy(s_JournalPath, g_CurrentBuf->PathName);
            strcat(s_JournalPath, ".jnl/"); }
          else {
            char FullPath[1000], Actual[1000];
            strcpy(FullPath, s_JournalPath);
            strcat(FullPath, "history.txt");
            OpenTextFile(NULL, &s_JournalHand, "a", FullPath, NULL, Actual);
            if ( s_JournalHand == NULL)
              Disaster("Failed to open journal file \"%s\"", FullPath); } }
             
        else if (strstr(Qualifier, "guardband") == Qualifier) { //Set window guardband.
          int Value;
          if (sscanf(ValueString, "%d", &Value) <= 0)
            Value = 0;
          s_GuardBandSize = Value;
          Message(NULL, "Window guardband set to %d", s_GuardBandSize); }
         
        else if (strstr(Qualifier, "debug") == Qualifier) { //Set default trace vector set by d command.
          int Value;
          if (sscanf(ValueString, "%x", &Value) <= 0)
            Value = Trace_Stack | Trace_Print | Trace_Break | Trace_AllCommands;
          g_DefaultTraceMode = Value; }
         
        else if (strstr(Qualifier, "verbose") == Qualifier) { //Set/reset verbose mode.
          int Value;
          if (sscanf(ValueString, "%x", &Value) <= 0)
            Value = 0;
          s_Verbose = Value;
          if (s_Verbose & NonSilent)
            Message(NULL, "Verbose diagnostics set to %d", s_Verbose); }

#ifndef NOWIDECHAR
        else if (strstr(Qualifier, "copy") == Qualifier) { //Text between latest note point and current chr defines paste buffer.
          int Size = 0, Chr = 0, StatusBits; 
          char * Temp;
          int ToChrNo = s_AbstractWholeRecordFlag ? strlen(g_CurrentBuf->CurrentRec->text) : g_CurrentBuf->CurrentChr;
          int FromChrNo = s_AbstractWholeRecordFlag ? 0 : s_NoteCharacter;
          int FromLineNo = s_NoteLine;
          int ToLineNo = g_CurrentBuf->LineNumber;
#if defined(VC)
          HANDLE hData;
          char * ptrData;
#endif
          
          if ( (ToLineNo < FromLineNo) || ((ToLineNo == FromLineNo) && (ToChrNo < FromChrNo)) ) { //Abstracting backwards - switch around.
            FromLineNo = ToLineNo;
            ToLineNo = s_NoteLine;
            FromChrNo = ToChrNo; 
            ToChrNo = s_AbstractWholeRecordFlag ? 0 : s_NoteCharacter;
            g_Failure |= AdvanceRecord(g_CurrentBuf, FromLineNo-ToLineNo); }
          if ( ! s_NoteBuffer) {
            continue; }
          if (s_NoteBuffer != g_CurrentBuf) {
            RunError("The note point is in a different buffer");
            continue; }
          s_NoteBuffer = NULL;
          if (g_CurrentBuf->BufferKey == BufferKey) {
            RunError("Abstract source and destination is the same buffer");
            continue; }
            
          AdvanceRecord(g_CurrentBuf, FromLineNo-ToLineNo);
          while (g_CurrentBuf->LineNumber != ToLineNo) {
            Size += strlen(g_CurrentBuf->CurrentRec->text)+1;
            AdvanceRecord(g_CurrentBuf, 1); }
          Size += ToChrNo - FromChrNo;
          
          Temp = (char *)malloc(Size+1);
          AdvanceRecord(g_CurrentBuf, FromLineNo-ToLineNo);
          g_CurrentBuf->CurrentChr = FromChrNo;
          while (g_CurrentBuf->LineNumber != ToLineNo) {
            char * Line = g_CurrentBuf->CurrentRec->text + g_CurrentBuf->CurrentChr;
            strcpy(Temp+Chr, Line);
            Chr += strlen(Line);
            Temp[Chr++] = '\n';
            AdvanceRecord(g_CurrentBuf, 1); }
          strncpy(Temp+Chr, g_CurrentBuf->CurrentRec->text+g_CurrentBuf->CurrentChr, ToChrNo-g_CurrentBuf->CurrentChr);
          Chr += ToChrNo-g_CurrentBuf->CurrentChr;
          Temp[Chr++] = '\0';
          s_NoteBuffer = NULL;
          
#if defined(VC)
          hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, Size);
          ptrData = (char*)GlobalLock(hData);
          memcpy(ptrData, Temp, Chr);
          GlobalUnlock(hData);
          OpenClipboard(NULL);
          EmptyClipboard();
          SetClipboardData(CF_TEXT, hData);
          CloseClipboard();
          Message(NULL, "Sent %d bytes", Size);
#else
          Display * Disp = XOpenDisplay(NULL);
          Widget top;
          XtAppContext context;
          int Args = 0;
          int xtime_out = 300; // in ms
          jmp_buf *RestartPoint = (jmp_buf *)malloc(sizeof(jmp_buf));
          void CopyTimeout(XtPointer p, XtIntervalId* i) {
            longjmp(*RestartPoint, 1); }
          
          if (setjmp(*RestartPoint) != 0)
            return TRUE;

          top = XtVaAppInitialize(&context, "ClearCutBuffers",0 , 0, &Args, NULL, NULL,NULL);
          Disp = XtDisplay(top);
          StatusBits = XStoreBuffer(Disp, Temp, Size, 0);
          XtAppAddTimeOut(context, xtime_out, CopyTimeout, 0);
          XtAppMainLoop(context);
          Message(NULL, "Sent %d bytes, status %X", Size, StatusBits);
#endif
          free(Temp); }

        else if (strstr(Qualifier, "paste") == Qualifier) { //paste - returns contents of paste buffer to nominated buffer.
          char * PasteBuf;
          int FirstLineNo = g_CurrentBuf->LineNumber;
#if defined(VC)
          HANDLE hClipboardData;
         
          OpenClipboard(NULL);
          hClipboardData = GetClipboardData(CF_TEXT);
          if ( ! (PasteBuf = (char*)GlobalLock(hClipboardData))) {
            Fail("Failed to open windows paste buffer");
            return 1; }
#else
          unsigned int BufLength = 0;
          Display * Disp = XOpenDisplay(NULL);
          PasteBuf = XFetchBuffer(Disp, &BufLength, 0);
#endif
          BreakRecord(g_CurrentBuf);
          AdvanceRecord(g_CurrentBuf, -1);
          TextViaJournal(g_CurrentBuf, PasteBuf, "[ copy ]");
          AdvanceRecord(g_CurrentBuf, FirstLineNo-g_CurrentBuf->LineNumber+1);
          JoinRecords(g_CurrentBuf);
          g_CurrentBuf->LineNumber -= 1;
#if defined(VC)
          GlobalUnlock(hClipboardData);
          CloseClipboard();
#endif
           }
#endif

        else if (strstr(Qualifier, "mousemask") == Qualifier) { //Define the mouse mask
          if (sscanf(ValueString, "%x", &s_MouseMask) <= 0)
#if defined(VC)
            s_MouseMask = -1;
#else
            s_MouseMask = ALL_MOUSE_EVENTS;
          mousemask(s_MouseMask, NULL);
#endif
          }

        else if (strstr(Qualifier, "setenv") == Qualifier) { //SetEnv - set env variable for the session.
          char EnvName[StringMaxChr];
          int ChrPtr;
          ExpandDeferredString(s_TempBuf, RightArg);
          if (sscanf(s_TempBuf->CurrentRec->text+6, "%s %n", (char *)&EnvName, &ChrPtr) <= 0)
            RunError("Bad syntax for SetEnv");
          if (setenv(EnvName, RightArg+6+ChrPtr, -1))
            continue; }
             
        else if (strstr(Qualifier, "case") == Qualifier) { //case -  controls case sensitivity for F command.
          int Value;
          if (sscanf(ValueString, "%x", &Value) <= 0)
            Value = 0;
          g_CaseSensitivity = Value;
          if (g_CaseSensitivity)
            Message(NULL, "Case sensitivity on"); 
          else
            Message(NULL, "Case sensitivity off"); }
             
        else if (strstr(Qualifier, "tracecounter") == Qualifier) { //TraceCounter - sets the debug counter to some positive value.
          int Value, OrigValue = s_TraceCounter;
          ExpandDeferredString(s_TempBuf, ValueString);
          if (sscanf(s_TempBuf->CurrentRec->text, "%d", &Value) <= 0)
            Message(NULL, "TraceCounter currently set to %d", s_TraceCounter);
          else
            s_TraceCounter = Value;
          if (s_Verbose & NonSilent)
            Message(NULL, "Trace counter was %d, now set to %d", OrigValue, s_TraceCounter); }
        
        else if (strstr(Qualifier, "trace") == Qualifier) { //trace - set Trace mode. Prefix argument with '+' to XOR with current value.
          int Value;
          int OrigValue = g_TraceMode;
          ExpandDeferredString(s_TempBuf, ValueString);
          if (sscanf(s_TempBuf->CurrentRec->text, "%x", &Value) <= 0)
            Value = 0;
          if (ValueString[0] == '+')
            g_TraceMode = g_TraceMode ^ Value;
          else
            g_TraceMode = Value;
          if (s_Verbose & NonSilent)
            Message(NULL, "Trace mode was %X, now set to %X", OrigValue, g_TraceMode);
          break; }

        else if (strstr(Qualifier, "tab") == Qualifier) { //tab - table-column separator character.
          char Value;
          if (sscanf(ValueString, "%c", &Value) <= 0)
            Value = '\t';
          g_TableSeparator = Value; }

        else if (strstr(Qualifier, "commandstring") == Qualifier) { //commandstring - return modified string back to command-line input.
          char * OriginalCommandString = g_CurrentCommand->CommandRec->text+17;
          if (OriginalCommandString[0] == ' ')
            OriginalCommandString++;
          if (s_ModifiedCommandString) {
            RunError("Recursive use of %s=commandstring is not supported.");
            continue; }
          s_ModifiedCommandString = (char *)malloc(strlen(OriginalCommandString)+1);
          strcpy(s_ModifiedCommandString, OriginalCommandString); }

        else if (strstr(Qualifier, "setmouse") == Qualifier) { //setmouse - sets the mouse coordinates returned by OP and %q=window commands.
          struct Window *Win = g_FirstWindow;
//          int LineCount = 1;
//          struct Buf * ThisBuf;
          
          g_MouseBufKey = g_CurrentBuf->BufferKey;
          g_MouseLine = g_CurrentBuf->LineNumber;
          g_Mouse_x = g_CurrentBuf->CurrentChr - g_CurrentBuf->LeftOffset;
          g_Mouse_y = -1;
          
          while (Win != NULL) { // Window loop.
            if (Win->WindowKey == '\0' || Win->WindowKey == g_CurrentBuf->BufferKey) {
              int LastLineNumber = Win->OldLastLineNumber;
              int FirstLineNumber = LastLineNumber - Win->WindowHeight;  //Tack alert!!! if the text is smaller than the window, this value is negative but does this matter?
              if (FirstLineNumber < g_CurrentBuf->LineNumber || g_CurrentBuf->LineNumber < LastLineNumber) { //Current line appears in this wondow.
                g_Mouse_y = g_CurrentBuf->LineNumber - FirstLineNumber;
                break; } }
            Win = Win->NextWindow; } }
           
        else { 
          SynError(g_CommandBuf, "Invalid %%%%s command qualifier \"%s\"", Qualifier);
          continue; }
          
        break; }

      case 'M': { //%M - MessageText.
        ExpandDeferredString(s_TempBuf, RightArg);
        if ( ! g_TTYMode && g_ScreenWidth < strlen(s_TempBuf->CurrentRec->text)) {
          Message(NULL, "Over-long %%M message string truncated to %d characters", g_ScreenWidth-1);
          s_TempBuf->CurrentRec->text[g_ScreenWidth-1] = '\0'; }
        Message(NULL, "%s", s_TempBuf->CurrentRec->text);
#if defined(LINUX)
  refresh();
#endif
        break; }

      case 'B': { //%B - Buffer attributes and operations.
        int Line;
        char Qualifier[20], * Modifier, FirstChr;
        int Chrs;
        ExpandDeferredString(s_TempBuf, RightArg);
        sscanf(s_TempBuf->CurrentRec->text, "%s %c%n", (char *)&Qualifier, &FirstChr, &Chrs);
        Modifier = s_TempBuf->CurrentRec->text+Chrs-1;
         
        if (strstr(Qualifier, "unrestricted") == Qualifier) { //As it says - no restrictions.
          g_CurrentBuf->EditLock = 0;
          break; }
        
        else if (strstr(Qualifier, "readonly") == Qualifier) { //Prohibits changes to this buffer.
          int NewReadonlyState;
          g_CurrentBuf->EditLock |= ReadOnly;
          if ((0 < sscanf(s_TempBuf->CurrentRec->text+8, "%d", &NewReadonlyState)) && ! NewReadonlyState)
            g_CurrentBuf->EditLock ^= ReadOnly;
          break; }
        
        else if (strstr(Qualifier, "writeifchanged") == Qualifier) { //Insists that changes are written out.
          int NewWriteIfChangedState;
            g_CurrentBuf->EditLock |= WriteIfChanged;
          if ((0 < sscanf(s_TempBuf->CurrentRec->text+14, "%d", &NewWriteIfChangedState)) && ! NewWriteIfChangedState)
            g_CurrentBuf->EditLock ^= WriteIfChanged;
          break; }
          
        else if (strstr(Qualifier, "sameflag1") == Qualifier) { //Sets the user change flag.
          g_CurrentBuf->UnchangedStatus |= SameFlag1;
          break; }
           
#if defined(VC)
        else if (strstr(Qualifier, "codepage") == Qualifier) { //Define CodePage for this buffer.
          sscanf(s_TempBuf->CurrentRec->text+8, "%d", &g_CurrentBuf->CodePage);
          break; }
#endif

        else if (strstr(Qualifier, "header") == Qualifier) { //Define a header string for display above window.
          char * Header = (char * )malloc(strlen(s_TempBuf->CurrentRec->text+7)+1);
          strcpy(Header, s_TempBuf->CurrentRec->text+7);
          if (g_CurrentBuf->Header) 
            free(g_CurrentBuf->Header);
          g_CurrentBuf->Header = Header;
          for (Line = 1; Line <= g_ScreenHeight; Line +=1) {
            s_ScreenTable[Line] = NULL;
            JotGotoXY(0, Line-1);
            JotClearToEOL(); }
          JotUpdateWindow();
          break; }
         
        else if (strstr(Qualifier, "leftoffset") == Qualifier) { //Define LeftOffset for this buffer (leftmost character in view).
          int OrigLeftOffset = g_CurrentBuf->LeftOffset;
          int NewLeftOffset;
          if (sscanf(s_TempBuf->CurrentRec->text+10, "%d", &NewLeftOffset) <= 0) {
            g_CurrentBuf->LeftOffset = 0;
            continue; }
          if (1000000000 < NewLeftOffset) {
            Fail("Yikes!!! left offset setting too large (%d)", NewLeftOffset);
            continue; }
          g_CurrentBuf->LeftOffset = (NewLeftOffset < 0) ? 0 : NewLeftOffset;
          if (OrigLeftOffset == g_CurrentBuf->LeftOffset) 
            break;
          //Force JotUpdateWindow to redraw the whole lot.
          for (Line = 1; Line <= g_ScreenHeight; Line +=1) {
            s_ScreenTable[Line] = NULL;
            JotGotoXY(0, Line-1);
            JotClearToEOL(); }
          JotUpdateWindow();
          break; }
         
        else if (strstr(Qualifier, "tabstops") == Qualifier) { //Define TabStops - column widths for this buffer.
          struct Chain { int TabStop; struct Chain * next; };
          struct Chain * FirstLink, * ThisLink, * PrevLink = NULL;
          int Chrs, Index = 1, TabStop, LastTabStop = 0, * TabStops;
          char * Args = s_TempBuf->CurrentRec->text+8;
           
          if (g_CurrentBuf->TabStops) {
            free(g_CurrentBuf->TabStops);
            g_CurrentBuf->TabStops = NULL; }
          s_TempBuf->CurrentChr += 8; //Length of 'TabStops'.
          
          for ( ; ; ) {
            if(sscanf(Args, " %d%n", &TabStop, &Chrs) <= 0) {
              if ( ! PrevLink) //No tabstop setting supplied, buffer reverts to non-tabular behaviour.
                return TRUE;
              ThisLink->next = NULL;
              break; }
            if (PrevLink)
              ThisLink = PrevLink->next = (struct Chain *)malloc(sizeof(struct Chain));
            else
              ThisLink = FirstLink = (struct Chain *)malloc(sizeof(struct Chain));
            Args += Chrs;
            ThisLink->TabStop = TabStop;
            PrevLink = ThisLink;
            Index++; }
            
          if (Index == 1) //No TabStops. 
            break;
          g_CurrentBuf->AutoTabStops = (Index == 2) && (FirstLink->TabStop == -1);
          TabStops = (int * )malloc(sizeof(int)*(g_CurrentBuf->AutoTabStops ? MaxAutoTabStops : Index));
          TabStops[0] = Index-1;
          ThisLink = FirstLink;
          Index = 1;
          
          //Copy the tabstops into the buffer, performing a sanity check.
          while(ThisLink) {
            int TabStop = ThisLink->TabStop;
            if (TabStop <= LastTabStop && 1 < Index) {
              free(TabStops);
              RunError("TabStops must be an ascending sequence");
              break; }
            if (g_Failure)
              continue;
            TabStops[Index++] = LastTabStop = TabStop;
            ThisLink = (PrevLink = ThisLink)->next;
            free(PrevLink); }
             
          g_CurrentBuf->TabStops = TabStops; 
          //Force JotUpdateWindow to redraw the whole lot.
          for (Line = 1; Line <= g_ScreenHeight; Line +=1) {
            s_ScreenTable[Line] = NULL;
            JotGotoXY(0, Line-1);
            JotClearToEOL(); }
          JotUpdateWindow();
          break; }

        else if (strstr(Qualifier, "tagtype") == Qualifier) { //tagtype - define a user tag, currently only colour tags supported.
          char TagName[StringMaxChr];
          int Foreground = 0, Background = 7;
          struct ColourTag * NewColourTag;
          sscanf(s_TempBuf->CurrentRec->text+8, "%s colour %d %d", &TagName[0], &Foreground, &Background);
#if defined(VC)
          if (15 < Foreground || 15 < Background) {
            Fail("Invalid colour number for windows (must be in the range 0 to 15)");
            continue; }
#else
          if (7 < Foreground || 7 < Background) {
            Fail("Invalid colour number for unix (must be in the range 0 to 7)");
            continue; }
          if (COLOR_PAIRS-1 < s_NextColourPair) {
            Fail("System colour-pair limit has been reached");
            continue; }
#endif
          if (12 < strlen(TagName)) {
            Fail("Name string is longer than 12 characters");
            continue; }
            
          NewColourTag = (struct ColourTag *)malloc(sizeof(struct ColourTag));
          NewColourTag->Foreground = Foreground;
          NewColourTag->Background = Background;
          strcpy(NewColourTag->TagName, TagName);
          NewColourTag->next = g_CurrentBuf->FirstColTag;
          g_CurrentBuf->FirstColTag = NewColourTag;
#if defined(VC)
          NewColourTag->ColourPair = Foreground+(Background*16);
#else
          NewColourTag->ColourPair = s_NextColourPair;
          init_pair(s_NextColourPair++, Foreground, Background);
#endif
          break; }

        else if (strstr(Qualifier, "addtag") == Qualifier) { //addtag - applies a tag defined by %b=tagtype to the current text.
          char Name[StringMaxChr];
          struct ColourTag * ThisColourTag = g_CurrentBuf->FirstColTag;
          char * TextString = NULL;
          struct Tag * ThisTag = (struct Tag *)malloc(sizeof(struct Tag));
          
          sscanf(s_TempBuf->CurrentRec->text+7, "%s", &Name[0]);
          if (strstr(Name, "-text=") == Name) { //It's the %b=addtag -string=<string> form
            char * Text = strstr(s_TempBuf->CurrentRec->text+7, "-text=")+6;
            TextString = (char *)malloc(strlen(Text)+1);
            strcpy(TextString, Text);
            ThisTag->Attr = TextString; }
          else {
            g_CurrentBuf->CurrentRec->DisplayFlag = Redraw_Line;
            while (ThisColourTag) { //Tagname search loop.
              if ((strstr(Name, ThisColourTag->TagName) == Name) && (strlen(Name) == strlen(ThisColourTag->TagName)) ) {
                ThisTag->Attr = ThisColourTag;
                break; }
              else
                ThisColourTag = ThisColourTag->next; }
            if ( ! TextString && ! ThisColourTag)
              continue; }
              
          if (g_CurrentBuf->SubstringLength < 0) {
            ThisTag->StartPoint = g_CurrentBuf->CurrentChr+1;
            ThisTag->EndPoint = g_CurrentBuf->CurrentChr-g_CurrentBuf->SubstringLength-1; }
          else {
            ThisTag->StartPoint = g_CurrentBuf->CurrentChr;
            ThisTag->EndPoint = (0<g_CurrentBuf->SubstringLength) ? g_CurrentBuf->CurrentChr+g_CurrentBuf->SubstringLength-1 :  strlen(g_CurrentBuf->CurrentRec->text); }
          ThisTag->next = NULL;
          ThisTag->Type = TextString ? TextType : ColourType;
          AddTag(g_CurrentBuf->CurrentRec, ThisTag);
          break; }

        else if (strstr(Qualifier, "remove_tag") == Qualifier) { //remove_tag - identifies a tag and removes it from the record structure.
          int TagType;
          char * TagName;
//          int ChrNo, TagStartPoint, TagEndPoint;
          int TagStartPoint, TagEndPoint;
          struct Rec * CurrentRec = g_CurrentBuf->CurrentRec;
//          int LineNo = g_CurrentBuf->LineNumber;
          struct Tag * ThisTag, * StartTag = NULL;
          
          if (strstr(s_TempBuf->CurrentRec->text+10, " colour ")) {
            TagType = ColourType;
            TagName = s_TempBuf->CurrentRec->text+18; }
          else if (strstr(s_TempBuf->CurrentRec->text+10, " text ")) {
            TagType = TextType;
            TagName = s_TempBuf->CurrentRec->text+16; }
          else if (strstr(s_TempBuf->CurrentRec->text+10, " target ")) {
            TagType = TargetType;
            TagName = s_TempBuf->CurrentRec->text+18; }
          else {
            Fail("Syntax error in remove_tag command");
            continue; }
            
          if (g_CurrentBuf->SubstringLength < 0) {
            TagStartPoint = g_CurrentBuf->CurrentChr+1;
            TagEndPoint = g_CurrentBuf->CurrentChr-g_CurrentBuf->SubstringLength-1; }
          else {
            TagStartPoint = g_CurrentBuf->CurrentChr;
            TagEndPoint = (0<g_CurrentBuf->SubstringLength) ? g_CurrentBuf->CurrentChr+g_CurrentBuf->SubstringLength-1 :  strlen(g_CurrentBuf->CurrentRec->text); }
            
          ThisTag = CurrentRec->TagChain;
          while (ThisTag) {
            if (ThisTag->Type == TagType) {
              if (TagType == ColourType || TagType == TextType) {
                if (ThisTag->StartPoint == TagStartPoint && ThisTag->EndPoint == TagEndPoint) { 
                  if ( (TagType == ColourType && ! strcmp(((struct ColourTag *)ThisTag->Attr)->TagName, TagName) ) || ( TagType == TextType && ! strcmp((char *)ThisTag->Attr, TagName) ) )
                    StartTag = ThisTag; } }
              else if (TagType == TargetType) {
                struct HtabEntry * HashTabEntry = ThisTag->Attr;
                if ( ! strcmp(HashTabEntry->HashKey, TagName) && ThisTag->StartPoint == TagStartPoint) {
                  HashTabEntry->TargetRec = NULL;
                  StartTag = ThisTag;
                  break; } } }
            ThisTag = ThisTag->next; }
            
          if ( ! StartTag) {
            Fail("Can't locate the tag");
            continue; }
          FreeTag(CurrentRec, StartTag); }

        else if (strstr(Qualifier, "sort") == Qualifier) //sort - perfom a qsort on the records in this buffer.
          if (ParseSort(RightArg, 0))
            continue;
          else
            break;
        
        else if (strstr(Qualifier, "tabsort") == Qualifier) //tabsort - perfom a qsort on the records in this buffer.
          if (ParseSort(RightArg, 1))
            continue;
          else
            break;
             
        else if (strstr(Qualifier, "unicode") == Qualifier) { //Controls unicode support for this buffer.
          int Line, UnicodeMode = TRUE;
          if (sscanf(s_TempBuf->CurrentRec->text+8, "%d", &UnicodeMode) < 0)
            g_CurrentBuf->NoUnicode = FALSE;
          else
            g_CurrentBuf->NoUnicode = (UnicodeMode == 0);
          
          for (Line = 1; Line <= g_ScreenHeight; Line +=1) {
            s_ScreenTable[Line] = NULL;
            JotGotoXY(0, Line-1);
            JotClearToEOL(); }
#if defined(LINUX)
        refresh(); 
#endif
          break; }
           
        else if (strstr(Qualifier, "pathname") == Qualifier) { //Sets the pathName for when the buffer is written.
          int Line;
           
//          ExpandDeferredString(s_TempBuf, Modifier);
          if (g_CurrentBuf->PathName) 
            free(g_CurrentBuf->PathName);
//          g_CurrentBuf->PathName = strcpy((char *)malloc(strlen(s_TempBuf->CurrentRec->text)+1), s_TempBuf->CurrentRec->text);
          g_CurrentBuf->PathName = strcpy((char *)malloc(strlen(Modifier)+1), Modifier);
          for (Line = 1; Line <= g_ScreenHeight; Line +=1)
            s_ScreenTable[Line] = NULL;
          JotUpdateWindow();
          break; }
          
        else {
          RunError("Unrecognized %%%%B qualifier \"%s\"", Qualifier);
          continue; }
           
        break; }

      case 'H': { //%H - Hashtable maintenance.
        ExpandDeferredString(s_TempBuf, RightArg);
        if (DoHash(s_TempBuf, BufferKey))
          continue;
        break; }

      case 'Q': { //%Q - system Query.
        ExpandDeferredString(s_TempBuf, RightArg);
        if (BufferKey == '=') { //With no destination buffer, dir and file just return status, version prints version to console, others options fail.
          if (QuerySystem(s_TempBuf, '\0'))
            continue; }
        else {
          if (QuerySystem(s_TempBuf, BufferKey))
            continue;
          GetRecord(g_CurrentBuf, 1);
          ResetBuffer(g_CurrentBuf); }
        break; }

      default: { // Should never get here - this error should be detected at compile time.
        RunError("Unknown %%%% command"); 
        continue; } } } // Percent-commands block ends.
      break;

    default:  //Invalid command character.
      RunError("Unknown command"); 
      return FALSE; } // Main s_CommandChr case block ends. 
       
    if (ThisArg != NULL) {
      Fail("Argument(s) remain");
      return FALSE; }
    LocalFail = FALSE; } // Command-key loop ends.
  return ! LocalFail; }

//---------------------------------------------Run_Block
int Run_Block(struct Block *ThisBlock)
  { // Calls Run_Sequence to execute embedded sequence,
    //     checks for ')' and repeat-block count.
  struct Seq * Sequence;
  int RepeatCount;
  
  if ( ! ThisBlock) 
    return FALSE;
     
  Sequence = ThisBlock->BlockSequence;
  RepeatCount = ThisBlock->Repeats;
  
  if (RepeatCount == INT_MAX) { // Infinite-loop block.
    while (TRUE)
      if ( ! Run_Sequence(Sequence) || s_BombOut)
        return TRUE; }
  else
    for ( ; 0 < RepeatCount; RepeatCount--)
      if ( ! Run_Sequence(Sequence) || s_BombOut)
        return FALSE;
       
    return TRUE;  }

//----------------------------------------------ExitNormally
void ExitNormally(char * ExitMessage)
  { // Exits after writing file. 
  FILE * FileHand = NULL;
  char TempString[1000];
  
  OpenTextFile(NULL, &FileHand, "w", g_CurrentBuf->PathName, "\0", TempString);
  if (FileHand == NULL) { // Denied write access.
    RunError("Error opening file \"%s\" for writing.", TempString);
    return; }
  else { // File opend OK. 
    struct Rec *Record = g_CurrentBuf->FirstRec;
    
    if (s_Verbose & NonSilent)
      Message(NULL, "Writing to \"%s\"", TempString);
    for ( ; ; ) { // Write out records loop. 
      if (Record==(g_CurrentBuf-> FirstRec->prev))
        break;
      if (fputs(Record->text, FileHand) == EOF)
        break;
      fputc('\n', FileHand);
      Record = Record->next;
      if (g_Failure) 
        break; }
    
    if (fclose(FileHand) == 0) { // File wriitten OK.
      g_CurrentBuf->UnchangedStatus |= SameSinceIO;
      NewScreenLine();
      if (TidyUp(0, ExitMessage))
        return; }
    Fail("Error Closing your file"); }
  return;  }

//----------------------------------------------PushInt
void PushInt(long long Value)
  { //Checks for stack overflow and Pushes Value onto stack.
  if (g_StackSize <= g_StackPtr) {
    Fail("Stack overflow");
    return; }
  g_Stack[g_StackPtr].Value = Value; 
  g_Stack[g_StackPtr++].type = StackIntFrame;
  g_Stack[g_StackPtr].type = StackVoidFrame; }

//----------------------------------------------PushFloat
void PushFloat(double Value)
  { //Checks for stack overflow and Pushes Value onto stack.
  if (g_StackSize <= g_StackPtr) {
    Fail("Stack overflow");
    return; }
  ((struct floatFrame *)(&g_Stack[g_StackPtr]))->fValue = Value; 
  g_Stack[g_StackPtr++].type = StackFloatFrame;
  g_Stack[g_StackPtr].type = StackVoidFrame; }

//----------------------------------------------PushBuf
void PushBuf(struct Buf * Buffer)
  { //Checks for stack overflow and Pushes Buffer onto stack.
  if (g_StackSize <= g_StackPtr) {
    Fail("Stack overflow");
    return; }
  ((struct bufFrame *)(&g_Stack[g_StackPtr]))->BufPtr = Buffer; 
  g_Stack[g_StackPtr++].type = StackBufFrame;
  g_Stack[g_StackPtr].type = StackVoidFrame; }

//----------------------------------------------PopInt
long long PopInt()
  { //Checks for stack underflow and and frame type, then pops  Value from stack.
  int ActualType;
   
  if (g_StackPtr < 1) {
    Fail("Stack underflow");
    return 0; }
  ActualType = (&g_Stack[--g_StackPtr])->type;
  if (ActualType != StackIntFrame) {
    g_StackPtr++;
    RunError("Incorrect stack-frame type, expecting integer frame, found frame type %d", ActualType);
    return 0; }
     
  return g_Stack[g_StackPtr].Value; }

//----------------------------------------------PopFloat
double PopFloat()
  { //Checks for stack underflow and and frame tye, then pops  Value from stack.
  int ActualType;
   
  if (g_StackPtr < 1) {
    Fail("Stack underflow");
    return 0; }
  ActualType = (&g_Stack[--g_StackPtr])->type;
  if (ActualType != StackFloatFrame) {
    g_StackPtr++;
    RunError("Incorrect stack-frame type, expecting floating-point frame, found frame type %d", ActualType);
    return 0; }
     
  return ((struct floatFrame *)(&g_Stack[g_StackPtr]))->fValue; }

//---------------------------------------------KillTop
int KillTop()
  { //Moves the stack pointer down one frame.
  if (1 <= g_StackPtr) {
    if (g_Stack[g_StackPtr-1].type == StackBufFrame) {
      if (((struct bufFrame *)(&g_Stack[g_StackPtr-1]))->BufPtr != g_CurrentBuf) {
        int TempSP;
        for (TempSP = g_StackPtr-2; 0 <= TempSP; TempSP--) { //Check for duplicated copies of the buffer.
          if ((&g_Stack[TempSP])->type != StackBufFrame)
            continue;
          if (((struct bufFrame *)(&g_Stack[TempSP]))->BufPtr == ((struct bufFrame *)(&g_Stack[g_StackPtr-1]))->BufPtr) {
            g_StackPtr--;
            return 0; } }
        if (FreeBuffer(((struct bufFrame * )(&g_Stack[g_StackPtr-1]))->BufPtr))
          return 1; }  //FreeBuffer() fails - probably a write-if-changed buffer at top of stack.
      else {
        Fail("Can't delete top of stack when it is the current buffer.");
        return 1; } }
    g_StackPtr--;
    return 0; }
  else {
    Fail("Stack underflow");
    return 1; } }

//---------------------------------------------DumpStack
void DumpStack()
  { //Like it says on the tin - it dumps the stack to the editor console.
  int SP;
  Message(NULL, "Stack currently holds %d item%s:", g_StackPtr, g_StackPtr == 1 ? "" : "s");
  for (SP = g_StackPtr-1; 0 <= SP; SP--) { // Stack-dump loop.
    if (g_Stack[SP].type == StackIntFrame)
      Message(NULL, "%4d: (int)  %lld", SP, g_Stack[SP].Value);
    else if ((&g_Stack[SP])->type == StackFloatFrame) { //Doing it this way works ... but I don't know why I can't pass it all straight into Message???
//      char Temp[100]; 
      Message(NULL, "%4d: (double) %f", SP, ((struct floatFrame *)(&g_Stack[SP]))->fValue); }
    else if ( (((struct bufFrame *)(&g_Stack[SP]))->BufPtr)->CurrentRec )
      Message(NULL, "%4d: (buf)  \"%s\"", SP, ((struct bufFrame *)(&g_Stack[SP]))->BufPtr->CurrentRec->text);
    else
      Message(NULL, "Invalid buffer record, buffer %c, line %d", ((struct bufFrame *)(&g_Stack[SP]))->BufPtr->BufferKey, ((struct bufFrame * )(&g_Stack[SP]))->BufPtr->LineNumber); } }

//
//  The procedures which follow are for string handling_
//
//  g_Character pointers indicate next next empty g_character cell, when building
//    new strings,
//    first g_character of matched substring after searches.
//  While extracting characters from the text using ImmediateCharacter(TextBuf)
//    which returns the next g_character or NextNonBlank which skips spaces.
//    the global pointer indicates
//    the byte address of the next g_character to extract, the BCPL length
//    descriptor byte is unaffected.
//  Substrings are found in a record by the procedure SearKeyecord(),
//    SearchBuffer() will search sections of the Buffer.
//  Having found a substring, the procedure SubstituteString() can be used to
//    substitute some other substring
//  SubstituteString() can also be called to delete a substring (by providing
//    a null parameter in the substitute-string field, and can be used to
//    insert substrings  - when there is no existing substring the substitute
//    string is inserted at the current g_character position.
//  The absence of a substring is flagged by a zero in the SubstringLength field
//    of the Buffer descriptor.
//  After insertion of new text into an existing record,
//    the global current g_character pointer indicates the cell following
//    the inserted substring_
//
//  Data structure for strings is (all pointers indicate start of BCPL string):
//
//  (StringPointer->SubstringLength):     length of substring
//
//   StringPointer->CurrentRec :    Address of currently active record.
//   *(StringPointer+CurrentRecKeyMan): Contains descriptor for current
//   record and current g_character in that record.
//   these are accessable separately using CurrentKey and CurrentRec
//   or the complete descriptor can be taken using CurrentRecKeyMan.
//   StringPointer->FirstRec:      Address of first text record.
//   *(StringPointer+0):                   First word of BCPL string_
//
//
//
//  Data structure for continuation strings is:
//   *(ContinuationStringPointer+0               )Start of BCPL string_
//   ContinuationStringPointer->prev : Address (abs) of previous record.
//   ContinuationStringPointer->next :  Address (abs) of next record.
//  The next record descriptor of the last record points to the first record.
//  The previous record descriptor of the first record points to the last record
//
//   Each record is separately taken off the heap by calls to GetVec
//
//  The g_Character pointers in the header and in the record (ie
//    the record length byte), are always assumed to be consistant
//    following operations which change record length.
//
//  The procedures specialize either on Buffers - ie multi-record file images,
//    or Records, the naming scheme reflects this:
//
//   ..Buffer or Buf - Operates over complete Buffer.
//
//   ..Record or Rec - Operates over all of current record.
//
//       ..Substring - Operates only on text delimited by Current g_character
//                     and Substring End descriptors.
//
//      ..Characters - Operates on characters remaining after
//                     (exceptionally before) current g_character, ignores
//                     substring end descriptor.
//

//--------------------------------------------------FreeRecord
void FreeRecord(struct Buf *TextBuf, int AdjustMode)
  {
  //
  //Releases current record of Buffer.
  //If asked to delete first record, sets first record descriptor to next rec.
  //AdjustMode controls shifing of htab entries.
  //
  struct Rec *Record = TextBuf->CurrentRec;
  struct Rec * NextRecord = Record->next;
  int FirstRecord = (TextBuf->FirstRec) == Record;
  struct Tag * ThisTag = Record->TagChain;
  if ( ! TextBuf->CurrentRec)
    return;
  if (TextBuf->EditLock & ReadOnly) {
    Fail("Can't delete records from read-only buffer %c", TextBuf->BufferKey);
    return; }
    
  while (ThisTag) {
    struct Tag * NextTag = ThisTag->next;
    
    if (ThisTag->Type == TargetType) {
      struct HtabEntry * ThisEntry = (struct HtabEntry *)(ThisTag->Attr);
      int HtabBufKey = ThisEntry->HashBufKey;
      struct Buf * RemoteBuf = (HtabBufKey==g_CurrentBuf->BufferKey) ? g_CurrentBuf : GetBuffer(HtabBufKey, NeverNew);
      
      if (RemoteBuf) {
        if (RemoteBuf->HashtableMode == DestroyHashtables) {
          DestroyHtab(RemoteBuf);
          //N.B. DestroyHtab() also takes out tags pointing to the hashtable - including the one we're currently holding.
          NextTag = Record->TagChain; }
        else if (RemoteBuf->HashtableMode == DeleteEntries)
          ThisEntry->TargetRec = NULL;
        else if (RemoteBuf->HashtableMode == AdjustEntries) {
          if (AdjustMode == DeleteNotAdjust)
            ThisEntry->TargetRec = NULL;
          else {
            struct Tag * NewTag = (struct Tag *)malloc(sizeof(struct Tag));
            ThisEntry->TargetRec = (AdjustMode == AdjustForwards) ?
              ((ThisEntry->TargetRec->next == TextBuf->FirstRec) ? ThisEntry->TargetRec->prev : ThisEntry->TargetRec->next) :
              ((ThisEntry->TargetRec == TextBuf->FirstRec) ? ThisEntry->TargetRec->next : ThisEntry->TargetRec->prev);
            ThisEntry->CurrentChr = 0;
            NewTag->next = NULL;
            NewTag->StartPoint = 0;
            NewTag->Type = TargetType;
            NewTag->Attr = (void *)ThisEntry;
            AddTag(ThisEntry->TargetRec, NewTag); } }
        else if (RemoteBuf->HashtableMode == ProtectEntries) {
          Fail("Can't delete a protected record from buffer %c", RemoteBuf->BufferKey);
          return; } } }
    else {
//      if (ThisTag->Attr)
//        free(ThisTag->Attr);
      FreeTag(Record, ThisTag); }
    ThisTag = NextTag; }
     
  ThisTag = Record->TagChain;
  while (ThisTag) {
    struct Tag * NextTag = ThisTag->next;
    free(ThisTag);
    ThisTag = NextTag; }
    
  if (NextRecord == Record) { //This is the one-and-only record in the buffer.
    NextRecord = TextBuf->FirstRec = NULL;
    TextBuf->SubstringLength = 0; }
  else { // Haven't deleted one and only record so relink other records.
    struct Rec *PrevRecord = Record->prev;
    PrevRecord->next = NextRecord;
    NextRecord->prev = PrevRecord; }
  free(Record->text);
  free(Record);
  if (FirstRecord)
    TextBuf->FirstRec = NextRecord;
  TextBuf->CurrentRec = NextRecord;
  TextBuf->CurrentChr = 0;
  TextBuf->SubstringLength = 0; }

//----------------------------------------------FreeBuffer
int FreeBuffer(struct Buf *TextBuf)
  { //Releases Buffer records returns 1 if error, otherwise 0.
  struct ColourTag * ThisColourTag = TextBuf->FirstColTag;
  struct Buf ** ThisBufPtr = &g_BufferChain;
  
  if ( ! TextBuf)
    return 0;
  if (TextBuf->EditLock & ReadOnly) {
    Fail("Attempt to redefine readonly buffer %c", TextBuf->BufferKey);
    return 1; }
  if (TextBuf->EditLock & WriteIfChanged && (TextBuf->UnchangedStatus & SameSinceIO) == 0) {
    Fail("Attempt to redefine changed WriteIfChanged buffer %c", TextBuf->BufferKey);
    return 1; }
     
  TextBuf->UnchangedStatus = 0;
    
  while (ThisColourTag) {
    struct ColourTag * NextColourTag = ThisColourTag->next;
    free(ThisColourTag);
    ThisColourTag = NextColourTag; }
    
  if (TextBuf->PathName)
    free(TextBuf->PathName);
  if (TextBuf->Header)
    free(TextBuf->Header);
  if (TextBuf->TabStops)
    free(TextBuf->TabStops);
  TextBuf->TabStops = NULL;
  DestroyHtab(TextBuf);
  
  if (TextBuf->CurrentRec) {
    TextBuf->CurrentRec = TextBuf->FirstRec;
    while (TextBuf->FirstRec) {
      FreeRecord(TextBuf, DeleteNotAdjust); } }
  
  //Search for TextBuf and remove it from chain.
  while ( * ThisBufPtr != NULL) {
    struct Buf ** NextBufPtr = &(( * ThisBufPtr)->NextBuf);
    if ( * ThisBufPtr == TextBuf) {
      * ThisBufPtr = * NextBufPtr;
      break; }
    else
      ThisBufPtr = NextBufPtr; }
      
  free(TextBuf);
  return 0; }

//--------------------------------------------------DuplicateRecord
void DuplicateRecord(struct Buf *OutBuf, struct Buf *InBuf)
  { //Gets a record of the right size then copies in text.
  char *InRecord = InBuf->CurrentRec->text;
  
  GetRecord(OutBuf, strlen(InRecord)+1);
  if (g_Failure)
    return;
  strcpy(OutBuf->CurrentRec->text, InRecord);  }

//--------------------------------------------------CheckBufferKey
int CheckBufferKey(char Key)
  { // Verifies that the given key is in the valid range.
  return ( (Key <= ' ') || ('}' < Key) || (Key != '~') ); }

//--------------------------------------------------GetBuffer
struct Buf * GetBuffer(int Key, int CreateOrDelete)
  { //Creates new buffer, if the buffer already exists then the existing buffer is returned unchanged.
    //If buffer is ~ creates a new buffer on stack if CreateInStack is true - even if top frame is a buffer.
    //CreateOrDelete controls creation of new buffers, three valid values are recognized:
    //  0 - AlwaysNew      - Deletes any preexisting buffer, returned buffer is always empty.
    //  1 - OptionallyNew  - Returns preexisting buffer or creates a new buffer not a stack buffer.
    //  2 - NeverNew       - Returns preexisting buffer or NULL.
    //
  struct Buf *NewBuf = NULL;
  struct Buf *ThisBuf = g_BufferChain;
  
  if (Key == '~') {
    if (CreateOrDelete == AlwaysNew) { //Creation of new buffers on the stack only allowed when tagged AlwaysNew.
      NewBuf = (struct Buf *)malloc(sizeof(struct Buf));
      NewBuf->NextBuf = NULL;
      PushBuf(NewBuf); }
    else {
      if (g_StackPtr < 1) {
        Fail("Stack underflow");
        return NULL; }
      return (g_Stack[g_StackPtr-1].type == StackBufFrame) ? ((struct bufFrame *)(&g_Stack[g_StackPtr-1]))->BufPtr : NULL; } }
  else {
    while (ThisBuf != NULL)
      if ((ThisBuf->BufferKey) == Key) { //Found a preexisting buffer.
        if (CreateOrDelete == AlwaysNew) { //Check locks on preexisting buffer before clearing.
          if (ThisBuf->EditLock & ReadOnly) {
            Fail("Attempt to redefine readonly buffer %c", ThisBuf->BufferKey);
            return NULL; }
          if (ThisBuf->EditLock & WriteIfChanged && (ThisBuf->UnchangedStatus & SameSinceIO) == 0) {
            Fail("Attempt to redefine changed WriteIfChanged buffer %c", ThisBuf->BufferKey);
            return NULL; }
          NewBuf = (struct Buf *)malloc(sizeof(struct Buf));
          FreeBuffer(ThisBuf);
          break; }
        return ThisBuf; }
      else
        ThisBuf = ThisBuf->NextBuf;
        
    if ( ! NewBuf && CreateOrDelete != NeverNew)
      NewBuf = (struct Buf *)malloc(sizeof(struct Buf)); }
  if ( ! NewBuf)
    return NULL;
      
  NewBuf->NextBuf = g_BufferChain;
  g_BufferChain = NewBuf;
  NewBuf->CurrentRec = NULL;
  NewBuf->FirstRec = NULL;
  NewBuf->SubstringLength = 0;
  NewBuf->AbstractWholeRecordFlag = FALSE;
  NewBuf->CurrentChr = 0;
  NewBuf->TabStops = NULL;
  NewBuf->LeftOffset = 0;
  NewBuf->LineNumber = 0;
  NewBuf->OldLastLineNumber = 0; 
  NewBuf->BufferKey = Key;
  NewBuf->Predecessor = NULL;
  NewBuf->Header = NULL;
  NewBuf->PathName = NULL;
  NewBuf->NewPathName = TRUE;
  NewBuf->NoUnicode = FALSE;
  NewBuf->UnchangedStatus = 0;
  NewBuf->EditLock = 0;
  NewBuf->AutoTabStops = FALSE; 
  NewBuf->FirstColTag = NULL;
  NewBuf->htab = NULL;
  NewBuf->FirstColTag = NULL;
  NewBuf->FirstEntry = NULL;
  NewBuf->FileType = 0;
  NewBuf->HashtableMode = NoHashtable;
#if defined(VC)
  NewBuf->CodePage = s_CodePage;
#endif
  return NewBuf;  }

//--------------------------------------------------GetRecord
void GetRecord(struct Buf *TextBuf, int Size)
  { //Extracts new record from heap to follow original current record,
    //  then sets it to current and initializes it.
    //If null Buffer then create first record.
  struct Rec *Record = TextBuf->CurrentRec;
  struct Rec *NewRecord;
  char *NewText;
  
  if (Size < 0) {
    RunError("Negative record size in GetRecord()");
    return; }
  NewRecord = (struct Rec *)malloc(sizeof(struct Rec));
  NewText = (char *)malloc(Size+1);
  if ((NewRecord == NULL) || (NewText == NULL)) { //Run out of heap.
    RunError("GetRecord:run out of heap space.");
    return; }
  if (Record == NULL) { //Empty buffer - set up Buffer block descriptor.
    TextBuf->FirstRec = NewRecord;
    NewRecord->prev = NewRecord;
    NewRecord->next = NewRecord; }
  else { // Non-null Buffer, just link in new record.
    struct Rec *NextRecord = Record->next;
  
    NewRecord->prev = Record;
    NewRecord->next = NextRecord;
    NextRecord->prev = NewRecord;
    Record->next = NewRecord; }
  TextBuf->CurrentRec = NewRecord;
  TextBuf->CurrentChr = 0;
  TextBuf->SubstringLength = 0;
  NewRecord->text = NewText;
  *(NewRecord->text) = '\0';
  NewRecord->DisplayFlag = Redraw_Line;
  NewRecord->length = Size;
  NewRecord->TagChain = NULL;
  TextBuf->LineNumber += 1;
  return; }

//-------------------------------------------------AddTag
void AddTag(struct Rec * Record, struct Tag * NewTag)
  { //Adds the tag to the correct place in the record's tag chain.
  struct Tag ** PrevTag = &(Record->TagChain);
  
  while ( *PrevTag && ( (*PrevTag)->StartPoint <= NewTag->StartPoint ) ) {
    PrevTag = PrevTag;
    PrevTag = &((*PrevTag)->next); }
    
  NewTag->next = *PrevTag;
  *PrevTag = NewTag; }

//-------------------------------------------------Abstract
void Abstract(struct Buf * DestBuf, struct Buf * SourceBuf, int ToLineNo, int ToChrNo, char FillQualifier)
  { //Moves records from one buf to another (i.e. abstraction.), copying starts at the current character of the source buffer and progresses to ChrNo of LineNo.
  int FromLineNo = SourceBuf->LineNumber;
  int FromChrNo = SourceBuf->CurrentChr;
  int SourceLim = strlen(SourceBuf->CurrentRec->text);
  int Index;
  int CopyLen;
  
  SourceBuf->SubstringLength = 0;
  if ( (ToLineNo < FromLineNo) || ((ToLineNo == FromLineNo) && (ToChrNo < FromChrNo)) ) { //Abstracting backwards - switch around.
    FromLineNo = ToLineNo;
    FromChrNo = ToChrNo; 
    ToLineNo = SourceBuf->LineNumber;
    ToChrNo = SourceBuf->CurrentChr;
    g_Failure |= AdvanceRecord(SourceBuf, FromLineNo-ToLineNo); }
  CopyLen = (FromLineNo == ToLineNo) ? ToChrNo-FromChrNo : strlen(SourceBuf->CurrentRec->text)-FromChrNo;
  if (g_Failure)
    return;
  if (SourceLim < ToChrNo) {
    g_Failure = TRUE;
    return; }
  
  if (strlen(SourceBuf->CurrentRec->text) < FromChrNo || strlen(SourceBuf->CurrentRec->text) < CopyLen) {
    RunError("Garbled note point");
    return; }
  //Copy the first (sub)record.
  DestBuf->SubstringLength = 0;
  SubstituteString(DestBuf, SourceBuf->CurrentRec->text+FromChrNo, CopyLen);
  if (FillQualifier == '*') { //Backfill with blanks.
    memset(SourceBuf->CurrentRec->text+FromChrNo, ' ', CopyLen);
    SourceBuf->CurrentChr = FromChrNo;
    SourceBuf->CurrentRec->DisplayFlag = Redraw_Line; }
  else if (FillQualifier != '&') { //Destroy.
    SourceBuf->CurrentChr = FromChrNo;
    SourceBuf->SubstringLength = CopyLen;
    SubstituteString(SourceBuf, "", -1); }
  else if (FillQualifier == '&')
    SourceBuf->CurrentChr = ToChrNo;
  if (FromLineNo < ToLineNo) {
    g_Failure |= AdvanceRecord(SourceBuf, 1);
    DestBuf->CurrentChr += CopyLen;
    BreakRecord(DestBuf);
    g_Failure |= AdvanceRecord(DestBuf, -1);
    if (g_Failure)
      return; }
    
  //Move any complete records between first and last.
  for (Index = 1; Index < ToLineNo-FromLineNo; Index++) { //Move records loop.
    if (FillQualifier == '*') { //Backfill with blanks.
      int OriginalLength = strlen(SourceBuf->CurrentRec->text);
      MoveRecord(DestBuf, SourceBuf);
      SourceBuf->CurrentRec = SourceBuf->CurrentRec->prev;
      GetRecord(SourceBuf, OriginalLength+1);
      memset(SourceBuf->CurrentRec->text, ' ', OriginalLength); 
      SourceBuf->CurrentRec->text[OriginalLength] = '\0'; 
      SourceBuf->CurrentRec = SourceBuf->CurrentRec->next; }
    else if (FillQualifier == '&') { //Preserve original.
      int OriginalLength = strlen(SourceBuf->CurrentRec->text);
      GetRecord(DestBuf, OriginalLength+1);
      strcpy(DestBuf->CurrentRec->text, SourceBuf->CurrentRec->text); 
      g_Failure |= AdvanceRecord(SourceBuf, 1); }
    else //Destroy.
      MoveRecord(DestBuf, SourceBuf);
    if (g_Failure)
      return; }
    
  if (FromLineNo < ToLineNo) { //Copy the final (sub)record.
    DestBuf->LineNumber -= 1;
    g_Failure |= AdvanceRecord(DestBuf, 1);
    DestBuf->CurrentChr = 0;
    DestBuf->SubstringLength = 0;
    if (strlen(SourceBuf->CurrentRec->text) < ToChrNo) {
      RunError("Garbled note point");
      return; }
    SubstituteString(DestBuf, SourceBuf->CurrentRec->text, ToChrNo);
    if (FillQualifier == '*') { //Backfill with blanks.
      memset(SourceBuf->CurrentRec->text, ' ', ToChrNo);
      SourceBuf->CurrentRec->DisplayFlag = Redraw_Line; }
    else if (FillQualifier != '&') { //Remove the source substring.
      SourceBuf->CurrentChr = 0;
      SourceBuf->SubstringLength = ToChrNo;
      SubstituteString(SourceBuf, "", -1); }
    if (FillQualifier == '\0') { //For some reason (yet to be determined) the final joinup is not required in '&' mode.
      g_Failure |= AdvanceRecord(SourceBuf, -1);
      JoinRecords(SourceBuf); }
    else 
      SourceBuf->CurrentChr = ToChrNo; }
      
  return; }

//-------------------------------------------------MoveRecord
void MoveRecord(struct Buf *ToBuf, struct Buf *FromBuf)
  {
  //Extracts record from FromBuf and places it to follow original current record in ToBuf it then becomes the current record.
  struct Rec *ToRecord = ToBuf->CurrentRec;
  struct Rec *FromRecord = FromBuf->CurrentRec;
  struct Tag * ThisTag = FromRecord->TagChain;
  
  while (ThisTag) {
    struct Tag * NextTag = ThisTag->next;
    
    if (ThisTag->Type == TargetType) {
      struct HtabEntry * ThisEntry = (struct HtabEntry *)(ThisTag->Attr);
      int HtabBufKey = ThisEntry->HashBufKey;
      struct Buf * RemoteBuf = (HtabBufKey==g_CurrentBuf->BufferKey) ? g_CurrentBuf : GetBuffer(HtabBufKey, NeverNew);
      
      if (RemoteBuf->HashtableMode == DestroyHashtables) {
        DestroyHtab(RemoteBuf);
        //N.B. DestroyHtab() also takes out tags pointing to the hashtable - including the one we're currently holding.
        NextTag = FromRecord->TagChain; }
      else if (RemoteBuf->HashtableMode == DeleteEntries)
        ThisEntry->TargetRec = NULL;
      else if (RemoteBuf->HashtableMode == AdjustEntries) { //Normally push the target to the next record
        struct Tag * NewTag = (struct Tag *)malloc(sizeof(struct Tag));
        FreeTarget(FromBuf, ThisEntry);
        ThisEntry->TargetRec = (ThisEntry->TargetRec->next == FromBuf->FirstRec) ? ThisEntry->TargetRec->prev : ThisEntry->TargetRec->next;
        ThisEntry->CurrentChr = 0;
        NewTag->next = NULL;
        NewTag->StartPoint = 0;
        NewTag->Type = TargetType;
        NewTag->Attr = (void *)ThisEntry;
        AddTag(ThisEntry->TargetRec, NewTag); }
      else if (RemoteBuf->HashtableMode == ProtectEntries) {
        Fail("Can't abstract a protected record from buffer %c", RemoteBuf->BufferKey);
        return; } }
//    else if (ThisTag->Type == TextType)
//      FreeTag(FromRecord, ThisTag);
    else {
//      if (ThisTag->Attr)
//        free(ThisTag);
      FreeTag(FromRecord, ThisTag); }
      
    ThisTag = NextTag; }
      
  if (FromRecord == FromBuf->FirstRec->prev) { //No records in FromBuf.
    g_Failure = TRUE;
    return; }
  if (FromRecord == (FromRecord->next))
    FromBuf->CurrentRec = NULL;
  else { // Haven't deleted one and only record so relink other records.
    FromRecord->prev->next = FromRecord->next;
    FromRecord->next->prev = FromRecord->prev; }
   //If moving first record, then put in new first record descriptor.
  if ((FromBuf->FirstRec) == FromRecord)
     FromBuf->FirstRec = FromRecord->next;
  //Update Current record pointer.
  FromBuf->CurrentRec = FromRecord->next;
  FromBuf->CurrentChr = 0;
  if (ToRecord == ToBuf->FirstRec->prev) { //Null Buffer, move in first record.
    ToRecord = FromRecord;
    ToBuf->CurrentRec = ToRecord;
    ToBuf->FirstRec = ToRecord;
    ToRecord->next = ToRecord;
    ToRecord->prev = ToRecord; }
  else { // Add new record after current record.
    ToBuf->CurrentRec = FromRecord;
    ToBuf->CurrentRec->prev = ToRecord;
    ToBuf->CurrentRec->next = ToRecord->next;
    ToRecord->next->prev = ToBuf->CurrentRec;
    ToRecord->next = ToBuf->CurrentRec; }
  ToBuf->CurrentChr = 0;
  ToBuf->LineNumber += 1; }

//----------------------------------------------BreakRecord
void BreakRecord(struct Buf *TextBuf)
  { //Break current record at current character.
  int BreakPointer = TextBuf->CurrentChr;
  struct Rec *BreakRec = TextBuf->CurrentRec;
  struct Rec *FirstRec;
  struct Rec *SecondRec;
  struct Tag ** ThisTagPtr = &(BreakRec->TagChain);
  struct Tag * FirstRecLastTag = NULL;
  
  GetRecord(TextBuf, BreakPointer+1);
  FirstRec = TextBuf->CurrentRec;
  GetRecord(TextBuf, (strlen(BreakRec->text))-BreakPointer+1);
  SecondRec = TextBuf->CurrentRec;
  if (g_Failure)
    return;
    
  strncpy(FirstRec ->text, BreakRec->text, BreakPointer);
  strcpy(SecondRec->text, (BreakRec->text)+BreakPointer);
  
  //Now split any tags between the two halves adjusting target points as appropriate.
  if (*ThisTagPtr) {
    if ((*ThisTagPtr)->StartPoint < BreakPointer)
      FirstRec->TagChain = BreakRec->TagChain;
    while ((*ThisTagPtr) && (*ThisTagPtr)->StartPoint < BreakPointer) {
      if ((*ThisTagPtr)->Type == TargetType)
        ((struct HtabEntry * )(*ThisTagPtr)->Attr)->TargetRec = FirstRec;
      else if (BreakPointer < (*ThisTagPtr)->EndPoint)
        (*ThisTagPtr)->EndPoint = BreakPointer;
      FirstRecLastTag = *ThisTagPtr;
      (*ThisTagPtr) = (*ThisTagPtr)->next; }
    
    SecondRec->TagChain = *ThisTagPtr;
    while ((*ThisTagPtr)) {
      (*ThisTagPtr)->StartPoint -= BreakPointer;
      if ((*ThisTagPtr)->Type == TargetType) {
        ((struct HtabEntry * )(*ThisTagPtr)->Attr)->TargetRec = SecondRec;
        ((struct HtabEntry * )(*ThisTagPtr)->Attr)->CurrentChr -= BreakPointer; }
      (*ThisTagPtr) = (*ThisTagPtr)->next; }
      
    if (FirstRecLastTag)
      FirstRecLastTag->next = NULL; }
    
  BreakRec->TagChain = NULL;
  TextBuf->CurrentRec = BreakRec;
  FreeRecord(TextBuf, AdjustForwards);
  TextBuf->CurrentRec = SecondRec;
  *((TextBuf->CurrentRec->prev->text)+BreakPointer) = '\0';
  TextBuf->CurrentChr = 0; }

//----------------------------------------------JoinRecords
void JoinRecords(struct Buf *TextBuf)
  {  //Join current record to next.
  struct Rec *FirstRecord = TextBuf->CurrentRec;
  char *FirstText = FirstRecord->text;
  struct Rec *SecondRecord = FirstRecord->next;
  char *SecondText = SecondRecord->text;
  struct Rec * NewRecord;
  char *NewText;
  int FirstLength = strlen(FirstText);
  struct Tag ** FirstTagsPtr = &(FirstRecord->TagChain);
  struct Tag * SecondTags = SecondRecord->TagChain;
  
  if (SecondRecord == (TextBuf->FirstRec)) { // End-flag record.
    g_Failure = TRUE;
    return; }
  
  TextBuf->CurrentRec = SecondRecord;
  GetRecord(TextBuf, FirstLength+strlen(SecondText)+1);
  NewRecord = TextBuf->CurrentRec;
  if (g_Failure)
    return;
  NewText = NewRecord->text;
  
  strcpy(NewText, FirstText);
  strcat(NewText, SecondText);
  
  //Concatinate tags, adjusting tags, target points and, in 2nd. record, the character no.
  NewRecord->TagChain = *FirstTagsPtr ? *FirstTagsPtr : SecondTags;
  while (*FirstTagsPtr) {
    if ((*FirstTagsPtr)->Type == TargetType)
      ((struct HtabEntry * )(*FirstTagsPtr)->Attr)->TargetRec = NewRecord;
    if ((*FirstTagsPtr)->next)
      *FirstTagsPtr = (*FirstTagsPtr)->next;
    else {
      (*FirstTagsPtr)->next = SecondTags;
      break; } }
  
  while (SecondTags) {
    SecondTags->StartPoint += FirstLength;
    if (SecondTags->Type == TargetType) {
      ((struct HtabEntry * )SecondTags->Attr)->TargetRec = NewRecord;
      ((struct HtabEntry * )SecondTags->Attr)->CurrentChr += FirstLength; }
    SecondTags = SecondTags->next; }
  FirstRecord->TagChain = NULL;
  SecondRecord->TagChain = NULL;
  
  TextBuf->CurrentRec = FirstRecord;
  FreeRecord(TextBuf, AdjustForwards);
  FreeRecord(TextBuf, AdjustBack);
  TextBuf->LineNumber -= 1;
  TextBuf->CurrentChr = FirstLength; }

//----------------------------------------------SearchRecord
int SearchRecord(struct Buf *TextBuf, char * SearchString, int Direction) {
  //
  //Function searches just one record for substring,
  //  if found then returns TRUE and adjusts substring start pointer,
  //  if not then returns FALSE, substring start descriptor points to
  //  end of current record.
  //
  char *Text = TextBuf->CurrentRec->text;
  int BufferCharacter;
  int SubCharacter;
  int SubstringStart = 0;
  int Character = SearchString[0];
  int SearchStringLength;
  int EndPoint;
  int SubstringLength;
  int FirstCharacterPtr;
  if (Character == '\0') //The search string is empty - this is deemed to match to an empty substring in the buffer.
    return (0<Direction) ? (strlen(Text)==TextBuf->CurrentChr) : (TextBuf->CurrentChr==0);
  SearchStringLength = strlen(SearchString);
  EndPoint = strlen(Text)-SearchStringLength+1;
  SubstringLength = TextBuf->SubstringLength;
  FirstCharacterPtr = TextBuf->CurrentChr;
   
  if (SubstringLength < 0)
    FirstCharacterPtr = TextBuf->CurrentChr + SubstringLength + Direction;
  else if (SubstringLength)
    FirstCharacterPtr = TextBuf->CurrentChr + Direction;
  if (Direction < 0) { // Backwards search.
    if (EndPoint <= FirstCharacterPtr)
      FirstCharacterPtr = EndPoint-1;
    if (FirstCharacterPtr < 0)
      return FALSE;
    EndPoint = 0; }
  else if (EndPoint < FirstCharacterPtr)
    return FALSE;
  
  if (g_CaseSensitivity) { // Respect case of alpha. characters.
    for ( ;FirstCharacterPtr != (EndPoint+Direction); ) { //1st. Character search loop.
      if (Text[FirstCharacterPtr] == Character) {// 1st character match found, now test the rest.
        int SubCharacterPtr;
  
        for (SubCharacterPtr = SubstringStart+1; SubCharacterPtr < SearchStringLength; SubCharacterPtr +=1)
          if (Text[FirstCharacterPtr+SubCharacterPtr] != SearchString[SubCharacterPtr])
            goto tryNext;
        TextBuf->CurrentChr = FirstCharacterPtr;
        (TextBuf->SubstringLength) = SearchStringLength;
        TextBuf->CurrentChr = FirstCharacterPtr;
        return TRUE; }
tryNext:
      FirstCharacterPtr = FirstCharacterPtr+Direction; }
    TextBuf->SubstringLength = 0;
    return FALSE; }
  
  //Case insensitive search then .
  Character = ChrUpper(Character);
  for (;FirstCharacterPtr != (EndPoint+Direction) ;) { //1st. Character search loop.
    BufferCharacter = ChrUpper(Text[FirstCharacterPtr]);
    if (BufferCharacter == Character) {// 1st character match found, now test the rest.
      int SubCharacterPtr;
      
      for (SubCharacterPtr = SubstringStart+1; SubCharacterPtr < SearchStringLength; SubCharacterPtr +=1) { // Try each substring chr.
        SubCharacter = ChrUpper(SearchString[SubCharacterPtr]);
        BufferCharacter = ChrUpper(Text[FirstCharacterPtr+SubCharacterPtr]);
        if (SubCharacter != BufferCharacter)
          goto tryNext_D; }
      //Complete string match.
      TextBuf->CurrentChr = FirstCharacterPtr;
      (TextBuf->SubstringLength) = SearchStringLength;
      TextBuf->CurrentChr = FirstCharacterPtr;
      return TRUE; }
tryNext_D:
    FirstCharacterPtr = FirstCharacterPtr+Direction; }
  TextBuf->SubstringLength = 0;
  return FALSE; }

#if defined(NoRegEx)
//----------------------------------------------RegexSearchBuffer
int RegexSearchBuffer(struct Buf *TextBuf, char BufferKey, char *SearchString, int Direction)
  { // RegEx is not currently available in DOSland
  RunError("Regular-expression matching not available in the windows version.");
  return FALSE; }

#else
//----------------------------------------------RegexSearchBuffer
int RegexSearchBuffer(struct Buf *TextBuf, char BufferKey, char *SearchString, int Direction)
  { // Regular-expression Search of buffer, limited to a number of records.
  regex_t re;
  regmatch_t pmatch[100];
  int TempCount = 0;  //Indicates no. of records searched.
  int frame = 0;      //Used to allocate each regexec call a new frame in pmatch in backwards searches.
  size_t frameSize;   //Used in management of and extraction from frames in pmatch array.
  int IgnoreCase = g_CaseSensitivity ? 0 : REG_ICASE;
  int startPoint = TextBuf->CurrentChr;
  struct Buf * DestBuf;
  int i;
  
  TextBuf->SubstringLength = 0;
   
  DestBuf = GetBuffer(BufferKey, AlwaysNew);
  if (DestBuf == NULL)
    return FALSE;
  
  if (TextBuf->SubstringLength) 
    startPoint += Direction;
  
  if (regcomp(&re, SearchString, IgnoreCase|REG_EXTENDED) != 0) {
//    char temp[1000];
//    sprintf(temp, "Invalid regular expression %s", SearchString);
//    Fail(temp);
    Fail("Invalid regular expression %s", SearchString);
    regfree(&re);
    if (DestBuf->CurrentRec == NULL)
      GetRecord(DestBuf, 1);
    DestBuf->CurrentRec->text[0] = '\0';
    return FALSE; }
  else
    GetRecord(DestBuf, 100);
     
  frameSize = re.re_nsub+1;
  
  if (0 < Direction) { //Forward search.
    for (;;) { //Record loop, if not found move record pointer.
      if ( ! regexec(&re, (TextBuf->CurrentRec->text)+startPoint, 100, pmatch, 0) )
        break;
      startPoint = 0;
      if (AdvanceRecord(TextBuf, Direction)) {
        regfree(&re);
        GetRecord(DestBuf, 0);
        return FALSE;
        TempCount++; } }
    TextBuf->CurrentChr = startPoint+pmatch[frame].rm_so;
    TextBuf->SubstringLength = pmatch[frame].rm_eo - pmatch[frame].rm_so; }
  else {  //For reverse searches, do repeated forwards searches until last match before current character.
    int endStop = TextBuf->CurrentChr;
    
    for (;;) { //  Backwards-search record loop - implemented by repeated forward searches in the same line.
              //  Endstop indicates the rightmost allowable extent for valid matches.
              //  Repeat RE search in this line until endstop reached then return previous match.
      char * rec = TextBuf->CurrentRec->text;
      int firstChr;
      int pass2OrMore = 0; //Flag indicating 2nd or subsequent retry in this record, also an increment to offset the start point after first successful match.
       
      startPoint = 0;
       
      //Inner loop locates the last matching substring before the endStop
      //If there was a previous successful match in this record then offset the start point.
      for ( ; ; ) {
        int status = regexec(&re, rec+startPoint+pass2OrMore, 100-frame, pmatch+frame, 0);
        firstChr = pmatch[frame].rm_so;
        if ( status || firstChr <= 0 || (endStop <= (startPoint+pass2OrMore+firstChr)) ) { //Exit RE-match loop if match fails or we've gone past the endStop.
          if (pass2OrMore)
            goto reverseRegexMatchFound;
          break; }
        startPoint += firstChr+pass2OrMore;
        frame += frameSize;
        pass2OrMore = 1; }
      g_Failure |= AdvanceRecord(TextBuf, Direction);
      frame = 0; 
      endStop = strlen(TextBuf->CurrentRec->text);
      if (g_Failure) {
        regfree(&re);
        GetRecord(DestBuf, 0);
        return FALSE; }
      TempCount--; }
      
reverseRegexMatchFound: //Last regexec call failed but the previous match was successful - rewind pmatch and startPoint.
    TextBuf->CurrentChr = startPoint;
    frame -= frameSize; 
    TextBuf->SubstringLength = pmatch[frame].rm_eo-pmatch[frame].rm_so;
    startPoint -= pmatch[frame].rm_so; }
  
  for (i = 0; i < frameSize; i++) {
    regoff_t start = pmatch[frame+i].rm_so;
    regoff_t end = pmatch[frame+i].rm_eo;
    GetRecord(DestBuf, end-start);
    strncat(DestBuf->CurrentRec->text, (g_CurrentBuf->CurrentRec->text)+startPoint+start, end-start); }
     
  ResetBuffer(DestBuf);
  DestBuf->AbstractWholeRecordFlag = TRUE;
  regfree(&re);
  return TRUE; }
#endif

//----------------------------------------------SubstituteString
void SubstituteString(struct Buf *DestBuf, char * SourceString, int Length)
  {
  //
  //Replaces current substring of DestBuf (identified by SubstringStart and
  //SubstringLength) with string in SourceString, Length is the no. of SourceString 
  //characters to be used - any negative value indicates all of the SourceString.
  //If no substring in DestBuf (SubstringLength == 0) then inserts from SourceString.
  //A special case, delete string, when SourceString == NULL is also recognized.
  //
  struct Rec *DestRecord = DestBuf->CurrentRec;
  char *DestText = DestRecord->text;
  int SubLength = DestBuf->SubstringLength;  //The substitution substring length.
  int DestTotalLength = strlen(DestText);    //The overall size of the destination string.
  int RemainderLength;                       //The length of the destination substring following the current substring.
  int SubStart;                              //The character no. in the dest record to receive the first chr from the source string.
  int NewLength;                             //The malloc size allowed for the destination string after substitution.
  int SourceLength;                          //The size of the string to be pushed in.
  int Offset;                                //The change in length of the destination record.
  struct Tag * ThisTag = DestRecord->TagChain;
  int DestIndex;
  
  if (0 <= SubLength)
    SubStart = DestBuf->CurrentChr;
  else {
    SubLength = -SubLength; 
    SubStart = DestBuf->CurrentChr - SubLength; }
  DestIndex = SubStart;
  DestRecord->DisplayFlag = Redraw_Line;
  if (SourceString != NULL)
    SourceLength = (0 <= Length) ? Length : strlen(SourceString);
  else
    SourceLength = 0;
  RemainderLength = DestTotalLength-SubStart-SubLength;
  
  if (SubStart < 0) {
    Fail("Invalid SubstringLength");
    return; }
  
  if (SubLength != 0)
    Offset = SourceLength-SubLength;
  else { // Null substring in destination - insert.
    if (SourceLength == 0)
      return;
    Offset = SourceLength;
    SubStart = DestBuf->CurrentChr;
    DestBuf->CurrentChr = SubStart;
    DestIndex = SubStart; }
  
  //Check and adjust tags.
  while (ThisTag) {
    int TagOffset = Offset;
    if (SubStart < ThisTag->StartPoint) { //Tag is after original substring - adjust ChrNo.
      if (ThisTag->StartPoint+TagOffset < DestBuf->CurrentChr) //Ooops - we've removed some of the tagged text.
        TagOffset =  DestBuf->CurrentChr-ThisTag->StartPoint;
      ThisTag->StartPoint += TagOffset;
      if (ThisTag->Type == TargetType) {
//        int HtabBufKey = ((struct HtabEntry * )ThisTag->Attr)->HashBufKey;
//        struct Buf * HtabBuf = (HtabBufKey==g_CurrentBuf->BufferKey) ? g_CurrentBuf : GetBuffer(HtabBufKey, NeverNew);
        ((struct HtabEntry * )ThisTag->Attr)->CurrentChr += TagOffset; } }
        
    ThisTag = ThisTag->next; }
  
  NewLength = (SubStart+SourceLength+RemainderLength <= DestTotalLength) ? DestTotalLength : SubStart+SourceLength+RemainderLength;
  if ((DestRecord->length) < NewLength+1) { //Insufficient space for insertion, get a bigger record.
    char *NewText = (char *)malloc(NewLength+1);
    
    if (NewText == NULL) {
      RunError("SubstituteString:run out of heap space.");
      return; }
    strncpy(NewText, DestText, SubStart);
    strncpy(NewText+SubStart, SourceString, SourceLength);
    strncpy(NewText+SubStart+SourceLength, DestText+SubStart+SubLength, DestTotalLength-SubStart-SubLength);
    NewText[SubStart+SourceLength+RemainderLength] = '\0';
    free(DestText);
    DestBuf->CurrentRec->text = NewText;
    DestRecord->length = NewLength+1; }
  else { //Sufficient space in existing record. Shift right-hand end of old text to the left or the right before copying in from SourceString.
    if (0 < Offset) { // +ve. offset - shifting right.
      memmove(DestText+SubStart+Offset, DestText+SubStart, DestTotalLength-SubStart);
      DestText[DestTotalLength+Offset] = '\0'; }
    else { // -ve. offset - shifting left.
      memmove(DestText+SubStart, DestText+SubStart-Offset, DestTotalLength-SubStart+Offset);
      DestText[DestTotalLength+Offset] = '\0'; }
         
    if (SourceString != NULL) //Insert the string now.
      strncpy(DestText+DestIndex, SourceString, SourceLength); }
         
  DestBuf->SubstringLength = SourceLength;
  DestBuf->CurrentChr = SubStart; }

//----------------------------------------------CheckNextCommand
char CheckNextCommand(struct Buf * Buffer, char * MatchChrsString)
  { //If the next character in the buffer matches any in the list then it extracts and returns that character, otherwise returns '\0'.
  char Chr = StealCharacter(Buffer);
  return strchr(MatchChrsString, Chr) ? ImmediateCharacter(Buffer) : '\0'; }

//----------------------------------------------StealCharacter
char StealCharacter(struct Buf *TextBuf)
  { //Returns next non-blank character in buffer TextBuf if buffer empty then returns 0.
  char TempChr = 0;
  int Pointer;
  
  for (Pointer = TextBuf->CurrentChr; Pointer <= strlen(TextBuf->CurrentRec->text); Pointer+=1)
    { // Skip spaces repeat loop.
    TempChr = (TextBuf->CurrentRec->text)[Pointer];
    if (TempChr == ' ')
      continue;
    TextBuf->CurrentChr = Pointer;
    return TempChr; }

  return 0; }

//----------------------------------------------NextNonBlank
char NextNonBlank(struct Buf *TextBuf)
  { //Returns next character in string and updates pointer.
  for ( ; ; ) { //Skip records loop.
    char *Text = TextBuf->CurrentRec->text;
    int Pointer = TextBuf->CurrentChr;
    int Length = strlen(Text);
    
    for ( ;Pointer < Length; Pointer++) { //Skip spaces repeat loop.
      if (*(Text+Pointer) == ' ')
        continue;
      TextBuf->CurrentChr = Pointer+1;
      return Text[Pointer]; }
    
    if (AdvanceRecord(TextBuf, 1))
      break; }
  
  TextBuf->CurrentChr = strlen(TextBuf->CurrentRec->text);
  return 0;  }

//----------------------------------------------ImmediateCharacter
char ImmediateCharacter(struct Buf *TextBuf)
  { //Returns next Character in string and updates pointer.
  int Pointer = TextBuf->CurrentChr;
  char *Text = TextBuf->CurrentRec->text;
  
  if ((strlen(Text)) <= Pointer)
    return 0;
  TextBuf->CurrentChr = Pointer+1;
  return *(Text+Pointer);  }

//----------------------------------------------ChrUpper
int ChrUpper(char Chr)
  {
  //
  //Function returns upper-case of any lower-case characters it is passed.
  //
  return ((('a' <= Chr) && (Chr <= 'z')) ? Chr-'a'+'A' : Chr);  }

//----------------------------------------------VerifyNonBlank
int VerifyNonBlank(struct Buf *TextBuf)
  { //Returns TRUE if any nonblank characters in string and updates pointer.
    for (;;) { //Skip records loop.
      char *Text = TextBuf->CurrentRec->text;
      int Pointer = TextBuf->CurrentChr;
  
      for ( ; Pointer < strlen(Text); Pointer++) { //Skip spaces repeat loop.
        char TempChr = *(Text+Pointer);
  
        if (TempChr == ' ')
          continue;
        TextBuf->CurrentChr = Pointer;
        return TRUE; }
      if (AdvanceRecord(TextBuf, 1)) { //No non-blank characters found.
        TextBuf->CurrentChr = Pointer;
        return FALSE; } }
  
  return FALSE; }

//----------------------------------------------VerifyKey
char VerifyKey(int Chr)
  {
  //
  //Function upper-case Chr if next character is a valid buffer key,
  //otherwise returns NULL.
  //
  Chr = ChrUpper(Chr);
  return (' ' < Chr) && (Chr < 'a') ? Chr : '\0';  }

//----------------------------------------------VerifyDigits
int VerifyDigits(struct Buf *TextBuf)
  {
  //
  //Function returns TRUE if next Character is an ASCII digit.
  //
  int Character = StealCharacter(TextBuf);
  
  return (((Character>='0') && (Character <= '9')) || ((Character == '+') || (Character == '-')));  }

//----------------------------------------------VerifyAlpha
int VerifyAlpha(struct Buf *TextBuf)
  {
  //
  //Function returns TRUE if next character is an ASCII A-Z or a-z.
  //  Does not affect any pointers.
  //
  int Character = 0;
   
  Character = ChrUpper(StealCharacter(TextBuf));
  return ('A' <= Character) && (Character <= 'Z');  }

//----------------------------------------------VerifyCharacter
int VerifyCharacter(struct Buf *TextBuf, int Character)
  {
  //Returns TRUE, and moves pointer if next character same as Character, FALSE and leaves pointer otherwise.
  if (StealCharacter(TextBuf) == Character) { //Yes it matches.
    TextBuf->CurrentChr++;
    return TRUE; }
  else 
    return FALSE; }

//----------------------------------------------GetDec
long long GetDec(struct Buf *TextBuf)
  { //Returns decimal translation of text in TextBuf.
  long long Total = 0;
  long long Character = NextNonBlank(TextBuf);
  char * Text = TextBuf->CurrentRec->text;
  int CharacterPointer;
  long long Minus = FALSE;
  int CharacterLimit = strlen(Text);
  
  switch (Character) {
  default:
    CharacterPointer = TextBuf->CurrentChr-1;
    break;
  case '-':
    Minus = TRUE;
  case '+':
    CharacterPointer = TextBuf->CurrentChr;
    break; } //] End of case block.
  
  for (; CharacterPointer < CharacterLimit; ) { // Main loop.
    Character = (Text)[CharacterPointer++];
    if ((Character < '0') || ('9' < Character)) {
      CharacterPointer--;
      break; }
    Total = (Total*10)+Character-'0'; }
     
  TextBuf->CurrentChr = CharacterPointer;
  return Minus ? -Total : Total; }

//----------------------------------------------GetOct
int GetOct(struct Buf *TextBuf)
  {
  //
  //Function returns Octal translation of text in TextBuf.
  //
  int Total = 0;
  int Character = NextNonBlank(TextBuf);
  struct Rec *Record = TextBuf->CurrentRec;
  int CharacterPointer = TextBuf->CurrentChr;
  int Flag = FALSE;
  int CharacterLimit = strlen(Record->text);
  int Pointer;
  
  for (Pointer = CharacterPointer; Pointer < CharacterLimit; Pointer +=1) { // Main loop.
    Total = (Total*8)+Character-'0';
    Character = *((Record->text) + Pointer);
    if (('0' <= Character) && (Character <= '7'))
      continue;
    TextBuf->CurrentChr = Pointer;
    if (Flag)
      Total = -Total;
    return Total;
    } //[)
  
  Total = (Total*8)+Character-'0';
  TextBuf->CurrentChr = CharacterLimit;
  if (Flag)
    Total = -Total;
  return Total;  }

//----------------------------------------------GetHex
int GetHex(struct Buf *TextBuf)
  { //Function returns Hex. translation of string
  int Total = 0;
  int Character = 0;
  
  Character = NextNonBlank(TextBuf);
  
  for (;;) { 
    Character = ChrUpper(Character);
    if ('A' <= Character) { // A or bigger.
      if ('F' < Character)
        break;
      Total = (Total<<4)+(Character-'A'+10); }
    else { //Less than 'A'
      if ((Character < '0') || ('9' < Character))
        break;
      Total = (Total<<4)+(Character-'0'); }
    Character = ImmediateCharacter(TextBuf); }
  if (Character != 0)
    TextBuf->CurrentChr--;
  return Total;  }

//----------------------------------------------GetFormatted
int GetFormatted(struct Buf *TextBuf, char FormatKey)
  {
  //
  //Function returns scanf conversion of text in TextBuf.
  //
  char *Record = TextBuf->CurrentRec->text;
  int CharacterPointer = TextBuf->CurrentChr;
  int Result;
  char Format[20];
  int CharCount;
  
  sprintf(Format, "%%%c%%n", FormatKey);
  sscanf(Record+CharacterPointer, Format, &Result, &CharCount);
  TextBuf->SubstringLength = CharCount;
  TextBuf->CurrentChr = CharacterPointer+CharCount;
  return Result;  }

//----------------------------------------------ClearRecord
void ClearRecord(struct Buf *TextBuf)
  {
  //
  //Resets Substring length to zero, does not remove old characters.
  //
  *(TextBuf->CurrentRec->text) = '\0';
  TextBuf->CurrentChr = 0;
  TextBuf->SubstringLength = 0;
  return;  }

//----------------------------------------------ResetBuffer
void ResetBuffer(struct Buf *TextBuf)
  {
  //
  //Resets character pointer to begining of first record.
  //
  TextBuf->CurrentRec = TextBuf->FirstRec;
  TextBuf->CurrentChr = 0;
  TextBuf->SubstringLength = 0;
  TextBuf->LineNumber = 1;
  return;  }

//----------------------------------------------SetPointer
void SetPointer(struct Buf *TextBuf, int NewPointer)
  {
  //
  //sets current character pointer of TextBuf
  //
  TextBuf->CurrentChr = NewPointer;
  TextBuf->SubstringLength = 0;
  return;  }

//----------------------------------------------AdvanceRecord
int AdvanceRecord(struct Buf *TextBuf, int Requested)
  {
  //
  //Resets character pointer to begining of record, 'Requested' records
  //  away from the current record. If one end of the text image is
  //  hit then the first/last record is taken.
  //
  //Function returns 0 if it actually moved by Requested records.
  //
  int Actual = 0;
  struct Rec *FirstRecordOfBuffer = TextBuf->FirstRec;
  struct Rec *LastRecordOfBuffer = FirstRecordOfBuffer->prev;
  struct Rec *OriginalRecord = TextBuf->CurrentRec;
  struct Rec *Record = OriginalRecord;
  int Count;
    
  TextBuf->CurrentChr = 0;
  TextBuf->SubstringLength = 0;
  
  if (0 <= Requested)
    for (Count = 1; Count <= Requested; Count++) {//+ve. or zero Requested: forwards loop.
      if (Record == LastRecordOfBuffer)
        break;
      Record = Record->next;
      Actual++; }
  else
    for (Count = 0; Requested < Count; Count--) {// -ve.  backwards loop.
      if (Record == FirstRecordOfBuffer)
        break;
      Record = Record->prev;
      Actual--; }
  
  TextBuf->CurrentRec = Record;
  TextBuf->LineNumber += Actual;
  return (Actual != Requested);
  } //AdvanceRecord procedure ends.

//----------------------------------------------ShiftRight
int ShiftRight(struct Buf *TextBuf, int Displacement)
  { //Moves character pointer by Displacement characters, text may contain UTF-8 or ISO-8859 unicode and Displacement may be negative to count backwards.
  int ChrCount = 0;
  int Bytes;
  int FirstByte = TextBuf->CurrentChr;
  int ByteCount = 0;
  char * String = TextBuf->CurrentRec->text;
  int Length = strlen(String);
#if defined(VC)
  int CodePage;
#endif
  int Fail = FALSE;
  
  TextBuf->SubstringLength = 0; 
   
  if (TextBuf->NoUnicode) {
    TextBuf->CurrentChr += Displacement;
    if ( (0 < Displacement) && (Length <= FirstByte+Displacement) ) { // Overflow - Set to end of string.
      TextBuf->CurrentChr = Length;
      Fail = TRUE; }
    else if ( (Displacement < 0) && (FirstByte+Displacement < 0) ) { //Underflow - Set to first byte in record.
      TextBuf->CurrentChr = 0;
      Fail = TRUE; }
       
    return Fail; }
     
  else {   
#if defined(VC)
    CodePage = TextBuf->CodePage;
    if (0 <= Displacement) {
      while (ChrCount < Displacement) {
        ChrCount = MultiByteToWideChar(CodePage, MB_ERR_INVALID_CHARS, String+FirstByte, ++ByteCount, NULL, 0);
        if (Length <= FirstByte+ByteCount) {
          ChrCount = (ChrCount == Displacement) ? Length-FirstByte : 0;
          break; } } }
    else {
      for (ByteCount = -1; 0 <= FirstByte+ByteCount; ByteCount--) {
        ChrCount -= (String[FirstByte+ByteCount] < 0 && !MultiByteToWideChar(CodePage, MB_ERR_INVALID_CHARS, String+FirstByte+ByteCount, -ByteCount, NULL, 0)) ? 0 : 1;
        if (ChrCount <= Displacement)
          break; } }
#else
    if (0 <= Displacement)
      while ( (ChrCount < Displacement) && (FirstByte+ByteCount <= Length) ) {
        if ((Bytes = mblen(String+FirstByte+ByteCount, MB_CUR_MAX)) < 0 )
          Bytes = 1;
        else if (Bytes == 0)
          break;
        ChrCount++;
        ByteCount += Bytes; }
    else { //Reading backwards, it's not easy to distinguish a mid/end UTF-8 byte from an ISO-8859 character.
           //So we sniff back until we see either a genuine UTF-8 start byte, an ASCII chr. or the start of the string.
      for (ByteCount = 0; Displacement < ChrCount && 0 < FirstByte+ByteCount; ) {
        if (0 < (Bytes = mblen(String+FirstByte+ByteCount-1, MB_CUR_MAX))) {
          ByteCount -= Bytes;
          ChrCount--; }
        else {
          int BackByte;
          for (BackByte = 1; BackByte <= MB_CUR_MAX; BackByte++) {
            if ( (FirstByte-BackByte-1 < 0) || (0 < String[FirstByte+ByteCount-BackByte-1]) ) { //Start of the line or an ASCII chr - original character was ISO-8859.
              ByteCount--;
              ChrCount--;
              break; }
            else if (BackByte == MB_CUR_MAX) { //Encountered a string of more than MB_CUR_MAX ISO-8859 characters - count only the first one.
              ByteCount--;
              ChrCount--; }
            else if (0 < mblen(String+FirstByte+ByteCount-BackByte-1, MB_CUR_MAX)) {  //It was part of a UTF-8 sequence.
              ByteCount -= BackByte+1;
              ChrCount--;
              break; } } } } }
          
#endif
      if (ChrCount != Displacement) {
        TextBuf->CurrentChr = (0 < Displacement) ? Length : 0;
        Fail = TRUE; }
      else
        TextBuf->CurrentChr += ByteCount; }
  return Fail; }

//----------------------------------------------Indent
void Indent()
  { // Puts in correct number of indents for dump procedures.
  int Count;
  
  for (Count = s_IndentLevel+1; -s_IndentLevel <= Count; Count--)
    JotAddChr(' ');  }

//----------------------------------------------DumpCommand
void DumpCommand(struct Com * ThisCommand)
  { // Dumps one command block.
  struct Arg * FirstArg = ThisCommand->ArgList;
  struct Arg * ThisArg = FirstArg;
  char Result[StringMaxChr];
  int Ptr = 0;
  
  if ((ThisCommand->CommandKey) == '(')
    ThisArg = NULL;
  Ptr = sprintf(Result, "%s'%c'%s", (g_CurrentCommand == ThisCommand) ? "Current command = " : "", ThisCommand->CommandKey, (ThisArg == NULL) ? " " : "(");
  
  while (ThisArg != NULL) { //Arg loop.
    int Type = ThisArg->type;
    long long Value = (long long)(ThisArg->intArg);
  
    if ((Type != NumericArg) && (Type != StringArg)) { //Error in type.
      NewScreenLine();
      NewScreenLine();
      Ptr += sprintf(Result+Ptr, "Non-standard type, Type %i, ArgValue %p", Type, &Value);
      Message(NULL, Result);
      return; }
  
    if (Type == NumericArg) { //A numeric arg.
      Ptr += sprintf(Result+Ptr, "NumericArg %lli", Value);
      if ((' ' <= Value) && (Value <= '~'))
        Ptr += sprintf(Result+Ptr, "=\'%c\'", (int)(Value&0x000000FF)); }
    else
      Ptr += sprintf(Result+Ptr, "StringArg \"%s\"", (char *)(ThisArg->pointer));
    ThisArg = ThisArg->next;
    if (ThisArg != NULL)
      Ptr += sprintf(Result+Ptr, " "); }
  
  Ptr += sprintf(Result+Ptr, "%s%s", (FirstArg == NULL) ? "" : ")", ((ThisCommand->NextCommand) == NULL) ? "" : ", ");
  Message(NULL, Result); }

//----------------------------------------------DumpBlock
void DumpBlock(struct Block * ThisBlock)
  { // Dumps one Block.
  struct Seq * BlockSequence = ThisBlock->BlockSequence;
   
  Indent();
  Message(NULL, "( ");
  s_IndentLevel++;
  if (BlockSequence != NULL)
    DumpSequence(BlockSequence, "Block sequence"); 
  s_IndentLevel--;
  Message(NULL, " ) %i  ", ThisBlock->Repeats);  }

//----------------------------------------------DumpSequence
void DumpSequence(struct Seq * ThisSequence, char * Text)
  { // Dumps one Sequence.
  struct Com * ThisCom = ThisSequence->FirstCommand;
   
  Indent();
  Message(NULL, "DumpSequence - %s", Text);
  s_IndentLevel++;
     
  while (ThisCom != NULL) { // Com loop.
    DumpCommand(ThisCom);
    if ((ThisCom->CommandKey) == '(')
      DumpBlock((struct Block *)(ThisCom->ArgList->pointer));
    ThisCom = ThisCom->NextCommand; }
     
  s_IndentLevel--;
  if ((ThisSequence->ElseSequence) != NULL) { // An else sequence.
    Indent();
    NewScreenLine();
    DumpSequence(ThisSequence->ElseSequence, "ELSE sequence"); }
     
  s_IndentLevel--;
  if ((ThisSequence->NextSequence) != NULL) { // An next sequence.
    Indent();
    NewScreenLine();
    DumpSequence(ThisSequence->NextSequence, "NEXT sequence"); } }

//----------------------------------------------FreeCommand
void FreeCommand(struct Com ** ThisCommand)
  { // Frees one command block.
  struct Arg * ThisArg = (*ThisCommand)->ArgList;
  
  if ((*ThisCommand)->CommandKey == '(')
    FreeBlock((struct Block **)&((*ThisCommand)->ArgList->pointer));
    
  while (ThisArg) { // Arg loop.
    struct Arg * Next = ThisArg->next;
    if (ThisArg->type == StringArg && ThisArg->pointer)
      free(ThisArg->pointer);
    free(ThisArg);
    ThisArg = Next; }
  
  free(*ThisCommand); 
  *ThisCommand = NULL; }

//----------------------------------------------FreeSequence
void FreeSequence(struct Seq ** ThisSequence)
  { // Frees one Sequence.
  struct Com * ThisCom;
  
  if (*ThisSequence == NULL)
    return;
  ThisCom = (*ThisSequence)->FirstCommand;
  while (ThisCom != NULL) { // Com loop.
    struct Com * Next = ThisCom->NextCommand;
    FreeCommand(&ThisCom);
    ThisCom = Next; }
     
  FreeSequence(&(*ThisSequence)->ElseSequence);
  FreeSequence(&(*ThisSequence)->NextSequence);
  free(*ThisSequence);
  * ThisSequence = NULL; }

//----------------------------------------------FreeBlock
void FreeBlock(struct Block ** ThisBlock)
  { // Frees one Block.
  if (*ThisBlock == NULL)
    return;
  FreeSequence(&(*ThisBlock)->BlockSequence);
  free(*ThisBlock);
  ThisBlock = NULL; }

//----------------------------------------------GetArg
long long GetArg(struct Buf *TextBuf, long long DefaultValue)
  { // Returns JOT style numeric argument. 
  int Direction = VerifyCharacter(TextBuf,'-') ? -1 : 1;
  if (VerifyDigits(TextBuf)) { // Get the arg.
    DefaultValue = GetDec(TextBuf);
    if (DefaultValue < 0) {
      SynError(TextBuf, "Overflow occured while reading decimal argument");
      return 0; } }
  if (DefaultValue == 0)
    return (Direction == 1) ? INT_MAX : INT_MIN;
  else
    return Direction*DefaultValue;  }
       
//----------------------------------------------GetBreakArg
int GetBreakArg(struct Buf *TextBuf, int DefaultValue)
  { //Similar to GetArg but must be positive and non-zero.
  if (VerifyDigits(TextBuf)) {
    long long Value = GetDec(TextBuf);
    if (Value <= 0)
      SynError(TextBuf, "Invalid break count");
    else
      return Value; }
  return DefaultValue;  }

//---------------------------------------------AddNumericArg
void AddNumericArg(struct Com *CommandBlock, long long ArgValue)
  { //Adds argument to current command.
  struct Arg * NewArg = (struct Arg *)malloc(sizeof(struct Arg));
  struct Arg * ThisArg = CommandBlock->ArgList;
   
  NewArg->type = NumericArg;
  NewArg->intArg = ArgValue;
  NewArg->next = NULL;
  if (ThisArg == NULL)
    CommandBlock->ArgList = NewArg;
  else
    while (TRUE)
      if ((ThisArg->next) == NULL) { // Found last Arg.
        ThisArg->next = NewArg;
        return; }
      else
        ThisArg = ThisArg->next;  }

//---------------------------------------------AddBlock
void AddBlock(struct Com *CommandBlock, struct Block * Block)
  { //Adds argument to current command.
  struct Arg * NewArg = (struct Arg *)malloc(sizeof(struct Arg));
  struct Arg * ThisArg = CommandBlock->ArgList;
   
  NewArg->type = BlockArg;
  NewArg->pointer = Block;
  NewArg->next = NULL;
  if (ThisArg == NULL)
    CommandBlock->ArgList = NewArg;
  else
    while (TRUE)
      if ((ThisArg->next) == NULL) { // Found last Arg.
        ThisArg->next = NewArg;
        return; }
      else
        ThisArg = ThisArg->next;  }

//---------------------------------------------AddStringArg
void AddStringArg(struct Com *CommandBlock, struct Buf * CommandBuf)
  { //
    //Takes next nonblank character in CommandBuf as a delimeter,
    //and copies over all text to OutputString until 2nd. delimeter, or end of
    //text encountered.
    //
    //Since these string arguments are later copied into strings of StringMaxChr
    //the length is checked here.
    //
  int OutputIndex = 0;
  int DelimiterCharacter;
  char * InputText = CommandBuf->CurrentRec->text;
  char ResultString[StringMaxChr];
  int InputIndex;
  struct Arg *NewArg;
  struct Arg *ThisArg = CommandBlock->ArgList;
  int Type = StringArg;
  char * FinalArgValue;
  int Length;
  char BufferKey;
  
  NewArg = (struct Arg *)malloc(sizeof(struct Arg));
  if (VerifyCharacter(CommandBuf, '\'')) { // Deferred argument just save the buffer key.
    BufferKey = ChrUpper(NextNonBlank(CommandBuf));
    Type = DeferredStringArg; }
  else { // Extract string argument now.
    DelimiterCharacter = ImmediateCharacter(CommandBuf);
    if (DelimiterCharacter == '\0')
      ResultString[0] = '\0';
    if ( (('A' <= DelimiterCharacter) && (DelimiterCharacter <= 'Z')) || (('a' <= DelimiterCharacter) && (DelimiterCharacter <= 'z')) || 
        (('0' <= DelimiterCharacter) && (DelimiterCharacter <= '9')) ) {
      SynError(CommandBuf, "Invalid delimiter");
      free(NewArg);
      return; }
  
    for (InputIndex = CommandBuf->CurrentChr; InputIndex < strlen(InputText); InputIndex++) { // Copy-over-text loop.
      char Chr = InputText[InputIndex];
      if (StringMaxChr <= OutputIndex) {
        SynError(CommandBuf, "Argument string too long");
        free(NewArg);
        return; }
      if (Chr == DelimiterCharacter)
        break;
      if (Chr == '\\') {
        if (InputText[++InputIndex] == 't') //Translate tab.
          Chr = '\t';
        else if (InputText[InputIndex] == 'r') //Translate carriage return.
          Chr = '\r';
        else if (InputText[InputIndex] == 'n') //Translate new line.
          Chr = '\n';
        else
          InputIndex--; }
      ResultString[OutputIndex++] = Chr; }
    
    ResultString[OutputIndex] = '\0';
    CommandBuf->CurrentChr = InputIndex+1;
    
    Length = strlen(ResultString);
    
    if ( (Length == 0) || (ResultString == NULL) )
      FinalArgValue = NULL;
    else { // Non-null string.
      FinalArgValue = (char *)malloc(Length+1);
      strcpy(FinalArgValue, ResultString); } }
    
  NewArg->type = Type;
  if (Type == DeferredStringArg)
    NewArg->intArg = BufferKey;
  else 
    NewArg->pointer = (void * )FinalArgValue;
  NewArg->next = NULL;
  if (ThisArg == NULL)
    CommandBlock->ArgList = NewArg;
  else
    while (TRUE)
      if ((ThisArg->next) == NULL) { // Found last Arg.
        ThisArg->next = NewArg;
        return; }
      else
        ThisArg = ThisArg->next;  }

//---------------------------------------------AddPercentArg
void AddPercentArg(struct Com *CommandBlock, struct Buf * CommandBuf)
  { //Copies original percent-command argument respecting escaped semicolons into next arg struct.
  struct Arg * NewArg = (struct Arg *)malloc(sizeof(struct Arg));
  struct Arg * ThisArg = CommandBlock->ArgList;
  char * String = (CommandBuf->CurrentRec->text)+(CommandBuf->CurrentChr);
  char * OutputString = (char * )malloc(strlen(String)+1);
  int IPtr = 0, OPtr = 0;
  char * commandEnd = strstr(String, " -hereEndsThePercentCommand;");
   
  if (commandEnd) {
    int EndPoint = commandEnd-String;
    CommandBuf->CurrentChr += EndPoint+28;
    strncpy(OutputString, String, EndPoint);
    OutputString[EndPoint] = '\0'; }
  else {
    while (IPtr < strlen(String)) {
      if (String[IPtr] == ';') {
        IPtr++;
        break; }
      if (String[IPtr] == '\\' && String[IPtr+1] == ';')
        IPtr++;
      OutputString[OPtr++] = String[IPtr++]; }
    OutputString[OPtr++] = '\0';
    CommandBuf->CurrentChr += IPtr; }
     
  NewArg->type = StringArg;
  NewArg->pointer = (void * )OutputString;
  NewArg->next = NULL;
  if (ThisArg == NULL)
    CommandBlock->ArgList = NewArg;
  else
    while (TRUE)
      if ((ThisArg->next) == NULL) { // Found last Arg.
        ThisArg->next = NewArg;
        return; }
      else
        ThisArg = ThisArg->next;  }

//---------------------------------------------FetchIntArg
long long FetchIntArg(struct Arg **CurrentArg_a)
  { //Extracts arg value and indexes to next.
  long long Value;
   
  if (*CurrentArg_a == NULL) {
    RunError("Insufficient args in list.");
    return 0; }
  if ((*CurrentArg_a)->type != NumericArg)
    RunError("Expecting an integer argument, found some other type.");
  Value = (*CurrentArg_a)->intArg;
  *CurrentArg_a = (*CurrentArg_a)->next;
  return Value;  }

//---------------------------------------------FetchBlock
struct Block *  FetchBlock(struct Arg **CurrentArg_a)
  { //Extracts arg value and indexes to next.
  struct Block * Value;
   
  if (*CurrentArg_a == NULL) {
    RunError("Insufficient args in list.");
    return NULL; }
  if ((*CurrentArg_a)->type != BlockArg) {
    RunError("Expecting a block argument, found some other type.");
    return NULL; }
  Value = (struct Block * )(*CurrentArg_a)->pointer;
  *CurrentArg_a = (*CurrentArg_a)->next;
  return Value;  }

//-----------------------------------------------CompareRecs
int  CompareRecs(struct Rec * X, struct Rec * Y, int ColumnStarts[], int ColumnWidths[])
  { //Compares two records - used by quicksort
    //ColumnStarts is a list of start characters of columns to be compared in order, terminated by -1.
    //ColumnWidths is the corresponding list of column widths.
    int i;
    
    for (i=0; ; ) {
      int ColumnStart=ColumnStarts[i], ColumnWidth=ColumnWidths[i++];
      if (ColumnStart < 1)
        return strcmp(X->text, Y->text);
      else {
        int Result = memcmp(X->text+ColumnStart, Y->text+ColumnStart, ColumnWidth+1);
        if (Result != 0)
          return Result; } } }

//-----------------------------------------------CompareRecsTab
int CompareRecsTab(struct Rec * X, struct Rec * Y, int TabNos[])
  { //Compares two records - used by quicksort, TabNos is a list of tab-delimited columns in priority order and terminated by -1
    int i, Result;
    char TempX[StringMaxChr], TempY[StringMaxChr];
    
    for (i = 0; ; ) {
      int TabNo = TabNos[i++];
      if (TabNo < 1)
        return strcmp(X->text, Y->text);
      else {
        char * ColX = X->text;
        char * ColY = Y->text;
        char * nxTabX, * nxTabY;
        
        for ( ; 0 < TabNo; TabNo--) {
          ColX = strchr(ColX, g_TableSeparator);
          ColY = strchr(ColY, g_TableSeparator);
          if ( ! ColX++ || ! ColY++) 
            return strcmp(X->text, Y->text); }
             
        if ( (nxTabX = strchr(ColX, g_TableSeparator) )) {
          strncpy(TempX, ColX, nxTabX-ColX);
          TempX[nxTabX-ColX] = '\0'; }
        else
          strcpy(TempX, ColX);
        if ( (nxTabY = strchr(ColY, g_TableSeparator) )) {
          strncpy(TempY, ColY, nxTabY-ColY);
          TempY[nxTabY-ColY] = '\0'; }
        else
          strcpy(TempY, ColY);
        if ( (Result = strcmp(TempX, TempY)) )
          return Result; } } }

//-----------------------------------------------Swap
void  Swap(struct Rec * v[], int i, int j)
  { //Element swapper - used by quicksort only.
  struct Rec * temp;
  
  temp = v[i];
  v[i] = v[j];
  v[j] = temp; }

//-----------------------------------------------Quicksort
void Quicksort(struct Rec * v[], int left, int right, int (* comp)(struct Rec *, struct Rec *, int *, int *), int ColumnStarts[], int ColumnWidths[])
  { //The classic quicksort algorithm adapted for sorting JOT records
    //This version sorts fields delimited by <firstCol>-<lastCol>.
  int i, last;
  
  if (right <= left)
    return;
  Swap(v, left, (left + right) / 2);
  last = left;
  
  for (i = left + 1; i <= right; ++i) {
    if ((*comp)(v[i], v[left], ColumnStarts, ColumnWidths) < 0)
      Swap(v, ++last, i); }
  
  Swap(v, left, last);
  Quicksort(v, left, last - 1, comp, ColumnStarts, ColumnWidths);
  Quicksort(v, last + 1, right, comp, ColumnStarts, ColumnWidths); }

//-----------------------------------------------QuicksortTab
void QuicksortTab(struct Rec * v[], int left, int right, int (* comp)(struct Rec *, struct Rec *, int []), int TabNos[])
  { //The classic quicksort algorithm adapted for sorting JOT records
    //This version sorts by field numbers marked by tabs.
  int i, last;
  
  if (right <= left)
    return;
  Swap(v, left, (left + right) / 2);
  last = left;
  
  for (i = left + 1; i <= right; ++i) {
    if ((*comp)(v[i], v[left], TabNos) < 0)
      Swap(v, ++last, i); }
  
  Swap(v, left, last);
  QuicksortTab(v, left,     last - 1, comp, TabNos);
  QuicksortTab(v, last + 1, right,    comp, TabNos); }

//-----------------------------------------------ParseSort
int ParseSort(char * Text, int TabSort)
  { //Analyzes command line given to %S and launches the sort.
  //First set up the buffer's records in an array for sorting. 
  int CompareRecsTab(struct Rec *, struct Rec *, int []);
  int CompareRecs(struct Rec *, struct Rec *, int [], int []);
  int ColumnStarts[100];
  int ColumnWidths[100];
  struct Rec *Record = g_CurrentBuf->FirstRec;
  struct Rec *LastRec = g_CurrentBuf->FirstRec->prev;
  struct Rec * * Array;
  int Length = 0;
  int i, Col=0, Chrs;
  struct Rec * PrevRec;
  int Relinks;
  
  if (g_CurrentBuf->EditLock & ReadOnly) {
    Fail("Attempt to modify a readonly buffer");
    return 1; }
  //Extract any tabstops. 
  if (TabSort) { 
    for (i = 7; i < strlen(Text); ) {
      int TabNo;
      if (sscanf(Text+i, "%d%n", &TabNo, &Chrs) <= 0)
        return 1;
      ColumnStarts[Col++] = TabNo;
      i += Chrs+1; }
    ColumnStarts[Col++] = -1; }
  else {
    for (i = 4; i < strlen(Text); ) {
      int Start, Width;
      char Delim;
      if (sscanf(Text+i, "%d%c%d%n", &Start, &Delim, &Width, &Chrs) <= 0)
        return 1;
      if (Delim == '-')
        Width -= Start;
      ColumnStarts[Col] = Start;
      ColumnWidths[Col++] = Width;
      i += Chrs+1; }
    ColumnStarts[Col++] = -1; }
  
  //Get array length.
  for (; ; ) { 
    Length++;
    if (Record == LastRec)
      break; 
    Record = Record->next; }
    
  //Create and populate the array with Rec structs from the current buffer.
  Array = (struct Rec **)malloc(sizeof(struct Rec **)*(Length+1));
  Record = g_CurrentBuf->FirstRec;
  for (i = 0; i < Length; i++) { //Copy-records-to-array loop.
    Array[i] = Record;
    Record = Record->next; }
    
  if (TabSort) {
    QuicksortTab(Array, 0, Length-1, CompareRecsTab, ColumnStarts); }
  else {
    Quicksort(Array, 0, Length-1, CompareRecs, ColumnStarts, ColumnWidths); }
   
  //After sorting the array, reorder buffer to match the array.
  PrevRec = NULL;
  Relinks = 0;    //Counts number of relinked records.
   
  Record = (struct Rec *)Array[0];
  g_CurrentBuf->FirstRec = Record;
  g_CurrentBuf->CurrentRec = Record;
  
  for (i = 0; i < Length; i++) {
    Record = Array[i];
    if (Record->prev != PrevRec)
      Relinks++;
    Record->prev = PrevRec;
    if (PrevRec)
      PrevRec->next = Record;
    PrevRec = Record; }
       
  Record->next = g_CurrentBuf->FirstRec;
  g_CurrentBuf->FirstRec->prev = Record;
  g_CurrentBuf->CurrentChr = 0;
  g_CurrentBuf->SubstringLength = 0;
  g_CurrentBuf->LineNumber = 1;
  free(Array);
  return 0; }

//-----------------------------------------------AddFormattedRecord
void AddFormattedRecord(struct Buf * Buffer, char *String, ...)
  { //Appends a new record to buffer using specified format string.
  va_list ap;
  char TempString[StringMaxChr];
   
  va_start(ap, String);
//  vsprintf(TempString, String, ap);
  vsnprintf(TempString, StringMaxChr, String, ap);
  va_end(ap); 
  GetRecord(Buffer, strlen(TempString)+1);
  strcpy(Buffer->CurrentRec->text, TempString); }

//----------------------------------------------GetChrNo
int GetChrNo(struct Buf * ThisBuf)
  { //Returns perceived character no. for an actual character no. taking into account the effect of tabstops.
  int * TabStops = ThisBuf->TabStops; 
   
  if (TabStops) {
    int ChrNo;
    int TabStop = 0;
    int TabCount = 1;
    int PerceivedChr = 0;
    
    for (ChrNo = 0; ChrNo <= ThisBuf->CurrentChr; ChrNo++) {
      char Chr = ThisBuf->CurrentRec->text[ChrNo];
      if (Chr == g_TableSeparator) {
        TabStop = (TabStops[0] < TabCount) ? TabStop+(TabCount < 2 ? 8 : TabStops[TabCount-1]-TabStops[TabCount-2]+1) : TabStops[TabCount++];
        PerceivedChr += TabStop; } }
     return PerceivedChr; }    
    else 
      return ThisBuf->CurrentChr; }

//----------------------------------------------QuerySystem
int QuerySystem(struct Buf * Query, char BufferKey)
  { //Reports on directory, file or other system tables.
  char * Text = Query->CurrentRec->text;
  struct Buf * DestBuf = NULL, * SourceBuf = g_CurrentBuf;
  int SourceBufKey = SourceBuf->BufferKey;
#if defined(VC)
  HANDLE DirHand = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA FileData;
  FILETIME ftCreate, ftAccess, ftWrite;
  SYSTEMTIME stUTC, stLocal;
  DWORD dwRet;
  int Chrs, EndStop;
  struct stat Stat;
  WORD AttrWords[StringMaxChr];
#endif
  
  if (BufferKey != '\0') {
    DestBuf = GetBuffer(BufferKey, AlwaysNew);
    if (DestBuf) { 
      GetRecord(DestBuf, strlen(Query->CurrentRec->text)+1);
      strcpy(DestBuf->FirstRec->text, Query->CurrentRec->text);
      DestBuf->AbstractWholeRecordFlag = TRUE;
      g_CurrentBuf = DestBuf; }
    else
      return 1; }
   
  if (strstr(Text, "version") == Text) { //version - print version or send it to a buffer.
    if ( ! DestBuf) {
      Message(NULL, VERSION_STRING);
      return 0; }
    GetRecord(DestBuf, strlen(VERSION_STRING)+1);
    strcpy(DestBuf->CurrentRec->text, VERSION_STRING); }

  else if (strstr(Text, "windows") == Text) //windows - fails if not operating under windows.
#if defined(VC)
    return 0;
#else
    return 1;
#endif

  else if (strstr(Text, "linux") == Text) //linux fails if not operating under linux.
#if defined(VC)
    return 1;
#else
    return 0;
#endif

  else if (strstr(Text, "case") == Text) //case - fails if not case sensitive.
    return ! g_CaseSensitivity;

  else if (strstr(Text, "system") == Text) { //System %s settings etc.
    struct Buf *ThisBuf = g_BufferChain;
    if ( ! DestBuf)
      return 1;
    AddFormattedRecord(DestBuf, "            Trace vector = %4X", g_TraceMode);
    AddFormattedRecord(DestBuf, "    Default trace vector = %4X", g_DefaultTraceMode);
    AddFormattedRecord(DestBuf, "           Trace counter = %4d", s_TraceCounter);
    AddFormattedRecord(DestBuf, "        Case sensitivity = %4X", g_CaseSensitivity);
    AddFormattedRecord(DestBuf, "            Command mode = %4X", g_ScreenMode);
    AddFormattedRecord(DestBuf, "              Mouse Mask = %4X", s_MouseMask);
    AddFormattedRecord(DestBuf, "             Note buffer = %4c", s_NoteBuffer ? s_NoteBuffer->BufferKey : ' ');
    AddFormattedRecord(DestBuf, "         Note linenumber = %4d", s_NoteLine);
    AddFormattedRecord(DestBuf, "          Note character = %4d", s_NoteCharacter);
    AddFormattedRecord(DestBuf, "          Stream-in mode = %4d", s_StreamIn);
    AddFormattedRecord(DestBuf, "         Stream-out mode = %4d", s_StreamOut);
    AddFormattedRecord(DestBuf, " Initialization commands = %s", s_InitCommands ? s_InitCommands->CurrentRec->text : "");
#if defined(VC)
    AddFormattedRecord(DestBuf, "                CodePage = %d", s_CodePage);
#else
    AddFormattedRecord(DestBuf, "                  Locale = %s", s_Locale);
#endif
    AddFormattedRecord(DestBuf, "   Table-entry separator = '\\x%02X'", g_TableSeparator);
    AddFormattedRecord(DestBuf, "     Hold screen on exit = %4s", s_HoldScreen ? "On" : "Off");
    AddFormattedRecord(DestBuf, "           Recovery mode = %4s", s_RecoveryMode ? "On" : "Off");
    AddFormattedRecord(DestBuf, "Buffers:");
  
    while (ThisBuf != NULL) {
      char TruncatedRec[100];
      strncpy(TruncatedRec, ThisBuf->CurrentRec->text, 100);
      if (100 <= strlen(ThisBuf->CurrentRec->text))
        TruncatedRec[100] = '\0';
      AddFormattedRecord(DestBuf, "  buffer %c: %4d \"%s\"", ThisBuf->BufferKey, ThisBuf->LineNumber, TruncatedRec);
      ThisBuf = ThisBuf->NextBuf; } }

  else if (strstr(Text, "hashtables") == Text) { //hashtables - find buffers with hashtables, list internal and external references and target buffers
    struct Buf *ThisBuf = g_BufferChain;
    char Buffers[512];
    if ( ! DestBuf)
      return 1;
    
    while (ThisBuf != NULL) {
      Buffers[0] = '\0';
      if (ThisBuf->FirstEntry) {
        struct HtabEntry * ThisEntry = ThisBuf->FirstEntry;
        while (ThisEntry) {
          char Key = ThisEntry->BufferKey;
          if ( ! strchr(Buffers, Key)) {
            strcat(Buffers, "  ");
            Buffers[strlen(Buffers)-1] = Key; }
          ThisEntry = ThisEntry->next; } }
        if (strlen(Buffers))
          AddFormattedRecord(DestBuf, "  Buffer %c has hash-table entries pointing to these buffers:%s", ThisBuf->BufferKey, Buffers);
        ThisBuf = ThisBuf->NextBuf; } }

  else if (strstr(Text, "wd") == Text) { //wd - return current directory only.
    char * trans;
     
    if ( ! DestBuf)
      return 1;
    trans = getenv("PWD");
    if (trans == 0) {
      char temp[100];
      GetRecord(DestBuf, 20);
      strcpy(temp, " - <Undefined> - ");
      strcpy(DestBuf->CurrentRec->text, temp);
      return 1; }
    GetRecord(DestBuf, strlen(trans)+1);
    strcpy(DestBuf->CurrentRec->text, trans); }

  else if (strstr(Text, "heap") == Text) { //heap - report heap stats to stdout.
#if defined(LINUX)
    if ( ! DestBuf)
      return 1;
    struct mallinfo MallInfo = mallinfo();
    AddFormattedRecord(DestBuf, "    arena = %d", MallInfo.arena);
    AddFormattedRecord(DestBuf, "  ordblks = %d", MallInfo.ordblks);
    AddFormattedRecord(DestBuf, "   smblks = %d", MallInfo.smblks);
    AddFormattedRecord(DestBuf, "    hblks = %d", MallInfo.hblks);
    AddFormattedRecord(DestBuf, "   hblkhd = %d", MallInfo.hblkhd);
    AddFormattedRecord(DestBuf, "  usmblks = %d", MallInfo.usmblks);
    AddFormattedRecord(DestBuf, "  fsmblks = %d", MallInfo.fsmblks);
    AddFormattedRecord(DestBuf, " uordblks = %d", MallInfo.uordblks);
    AddFormattedRecord(DestBuf, " fordblks = %d", MallInfo.fordblks);
    AddFormattedRecord(DestBuf, " keepcost = %d", MallInfo.keepcost);
#endif
    }

  else if (strstr(Text, "backtrace") == Text) //Writes diagnostic backtrace to nominated buffer.
    return Backtrace(DestBuf);

  else if (strstr(Text, "history") == Text) { //history - dumps history to specified buffer.
    struct Rec * CommandRec = g_CommandBuf->FirstRec->next;
    if ( ! DestBuf)
      return 1;
    while (CommandRec != g_CommandBuf->FirstRec) {
      AddFormattedRecord(DestBuf, "%s", CommandRec->text);
      CommandRec = CommandRec->next; }
      
    DestBuf->CurrentRec = DestBuf->FirstRec->prev;
    return 0; }

  else if (strstr(Text, "tabstops") == Text) //tabstops - pushes TRUE if tabstops are set otherwise FALSE.
    return  ! SourceBuf->TabStops;

  else if (strstr(Text, "keys") == Text) { //keys - sends a list of keys and truncated endpoint strings to nominated buffer.
    struct HtabEntry * ThisEntry;
    if ( ! DestBuf)
      return (SourceBuf->htab == NULL);
    if (SourceBufKey == BufferKey)
      return 1;
      
    AddFormattedRecord(DestBuf, "Keys for buffer %c:", SourceBuf->BufferKey);
    ThisEntry = SourceBuf->FirstEntry;
    
    while (ThisEntry) {
      char TruncText[50];
      memset(TruncText, '\0', 50);
      if (ThisEntry->TargetRec)
        strncpy(TruncText, ThisEntry->TargetRec->text, 30);
      else
        strcpy(TruncText, "deleted key");
      AddFormattedRecord(
        DestBuf, 
        "key %20s, buf %c, line no. %4d, chr no. %3d, Rec:%s", ThisEntry->HashKey, ThisEntry->BufferKey, ThisEntry->LineNumber, ThisEntry->CurrentChr, TruncText);
      ThisEntry = ThisEntry->next; } }

  else if (strstr(Text, "tags") == Text) { //tags - sends a list of record tags in current buffer to nominated buffer.
    int LineNo = 1, ErrorRecs = 0, ErrorTags = 0;
    int Here = FALSE;
    struct Rec * ThisRec;
    struct Rec * LastRec;
    char TruncText[50];
    
    if ( ! DestBuf || SourceBufKey == BufferKey)
      return 1;
      
    if (strstr(Text, "-here")) { //Report only tags active at current character position.
      ThisRec = LastRec = SourceBuf->CurrentRec;
      AddFormattedRecord(DestBuf, "Reporting tags at Line no. %d Character no. %2d of Buffer %c", SourceBuf->LineNumber, SourceBuf->CurrentChr, SourceBuf->BufferKey);
      LineNo = SourceBuf->LineNumber;
      Here = TRUE; }
    else
      AddFormattedRecord(DestBuf, "Reporting all tags in Buffer %c", SourceBuf->BufferKey);
      
    ThisRec = Here ? SourceBuf->CurrentRec : SourceBuf->FirstRec;
    LastRec = SourceBuf->FirstRec->prev;
                      
    while (TRUE) {    
      int ErrorRec = 0;
      struct Tag * ThisTag = ThisRec->TagChain;
      
      if ( Here || ThisTag ) {
        memset(TruncText, '\0', 50);
        strncpy(TruncText, ThisRec->text, 49);
        AddFormattedRecord(DestBuf, "Rec %3d: \"%s\"", LineNo, TruncText); }
          
      while (ThisTag) {
        if ( ! Here || ( (ThisTag->StartPoint <= SourceBuf->CurrentChr) && ( (ThisTag->Type == TargetType) || (SourceBuf->CurrentChr <= ThisTag->EndPoint) ) ) ) {
          if (ThisTag->Type == TargetType) {
            struct HtabEntry * Entry = ThisTag->Attr;
            int ErrorTag = Entry->TargetRec == ThisRec ? 0 : 1;
            AddFormattedRecord(DestBuf, "  Type target %c, Chr %2d, %s", Entry->HashBufKey, ThisTag->StartPoint, ErrorTag ? "  ** Error HtabEntry-record mismatch **" : Entry->HashKey);
            ErrorTags += ErrorTag;
            ErrorRec |= ErrorTag; }
          else if (ThisTag->Type == TextType)
            AddFormattedRecord(DestBuf, "  Type text from chr %3d to %3d = \"%s\"", ThisTag->StartPoint, ThisTag->EndPoint, ThisTag->Attr);
          else if (ThisTag->Type == ColourType) {
//            struct ColourTag * TheColourTag = (struct ColourTag *)ThisTag->Attr;
            AddFormattedRecord(DestBuf, "  At rec %3d, chr %2d to %2d, active colour is %s",
              LineNo, ThisTag->StartPoint, ThisTag->EndPoint, ((struct ColourTag * )ThisTag->Attr)->TagName); }
          else if (ThisTag->Type == FontType)
            AddFormattedRecord( DestBuf, "  Type Font at chr %2d to %2d", ThisTag->StartPoint, ThisTag->EndPoint); }
        ThisTag = ThisTag->next; }
          
      if (Here)
        break;
      LineNo++;
      if (ErrorRec)
        ErrorRecs++;
      if (ThisRec == LastRec)
        break;
      ThisRec = ThisRec->next; }
    
    if (ErrorRecs)
       AddFormattedRecord(DestBuf, "There were %d records containing inconsistent hashtable entries, %d tag/entries were in error", ErrorRecs, ErrorTags); }

  else if (strstr(Text, "buffer") == Text) { //buffer - returns info on buffer.
    char temp[StringMaxChr];
    struct stat Stat;
    char * PathName;
    int HTabCount = 0;
    struct HtabEntry * ThisEntry;
    struct ColourTag * TagType;
    
    if ( ! DestBuf)
      return 1;
    if (SourceBufKey == BufferKey)
      return 1;
      
    TagType = SourceBuf->FirstColTag;
    PathName = SourceBuf->PathName;
    ThisEntry = SourceBuf->FirstEntry;
    while (ThisEntry) {
      HTabCount++;
      ThisEntry = ThisEntry->next; }
       
    AddFormattedRecord(DestBuf, "                  key = %c", SourceBuf->BufferKey); 
    AddFormattedRecord(DestBuf, "             pathName = %s", SourceBuf->PathName); 
    if (SourceBuf->PathName) //This is the image of a real file.
      if ( ! stat(PathName, &Stat)) { 
        strftime(temp, StringMaxChr, "%Y/%m/%d, %H:%M:%S", localtime(&Stat.st_mtime));
        AddFormattedRecord(DestBuf, "     currentDatestamp = %s", temp); }
    AddFormattedRecord(DestBuf, "          SameSinceIO = %s", SourceBuf->UnchangedStatus & SameSinceIO ? "TRUE" : "FALSE"); 
    AddFormattedRecord(DestBuf, "     SameSinceIndexed = %s", SourceBuf->UnchangedStatus & SameSinceIndexed ? "TRUE" : "FALSE"); 
    AddFormattedRecord(DestBuf, "    SameSinceCompiled = %s", SourceBuf->UnchangedStatus & SameSinceCompiled ? "TRUE" : "FALSE"); 
    AddFormattedRecord(DestBuf, "            SameFlag1 = %s", SourceBuf->UnchangedStatus & SameFlag1 ? "TRUE" : "FALSE"); 
    AddFormattedRecord(DestBuf, "            NoUnicode = %s", SourceBuf->NoUnicode ? "TRUE" : "FALSE"); 
    AddFormattedRecord(DestBuf, "           lineNumber = %d", SourceBuf->LineNumber); 
    AddFormattedRecord(DestBuf, "           CurrentChr = %d", SourceBuf->CurrentChr); 
    AddFormattedRecord(DestBuf, "      SubstringLength = %d", SourceBuf->SubstringLength); 
    AddFormattedRecord(DestBuf, "         wholeRecords = %s", SourceBuf->AbstractWholeRecordFlag ? "TRUE" : "FALSE"); 
    AddFormattedRecord(DestBuf, "          predecessor = %c", (SourceBuf->Predecessor) ? SourceBuf->Predecessor->CommandBuf->BufferKey : ' '); 
    AddFormattedRecord(DestBuf, "             editLock = %s %s", SourceBuf->EditLock & ReadOnly ? "ReadOnly" : "", SourceBuf->EditLock & WriteIfChanged ? "WriteIfChanged" : ""); 
    AddFormattedRecord(DestBuf, "           LeftOffset = %d", SourceBuf->LeftOffset);
    AddFormattedRecord(DestBuf, "    CurrentRec length = %d", SourceBuf->CurrentRec->length);
    AddFormattedRecord(DestBuf, "               Header = %s", SourceBuf->Header);
    if (0 < SourceBuf->FileType)
      AddFormattedRecord(DestBuf, "             FileType = binary, fixed length %d-byte records", SourceBuf->FileType);
    else
      AddFormattedRecord(DestBuf, "             FileType = ascii");
#if defined(VC)
    AddFormattedRecord(DestBuf, "             CodePage = %d", SourceBuf->CodePage);
#endif
    AddFormattedRecord(DestBuf, "                        %s", TagType ? "Tag-types follow" : "No tag types");
    while (TagType) {
      AddFormattedRecord(DestBuf, "              TagType = Name %12s, Forground Colour %d, Background colour %d", TagType->TagName, TagType->Foreground, TagType->Background);
      TagType = TagType->next; }
    AddFormattedRecord(DestBuf, "        HashtableMode = %d", SourceBuf->HashtableMode);
    if (SourceBuf->TabStops) {
      int * TabStops = SourceBuf->TabStops;
      char Temp[StringMaxChr];
      int Chrs = 0, TabStopsIndex = 1;
      while (TabStopsIndex <= TabStops[0]) 
        Chrs += sprintf(Temp+Chrs, "%d ", TabStops[TabStopsIndex++]);
      AddFormattedRecord(DestBuf, "             TabStops = %s", Temp); }
    else
      AddFormattedRecord(DestBuf, "             TabStops = 0 entries");
    AddFormattedRecord(DestBuf, "                 htab = %d entries", HTabCount); }

  else if (strstr(Text, "window") == Text) { //window - returns window size and allocation.
    char String[StringMaxChr];
    struct Window *Win = g_FirstWindow;
    int LineCount = 0;
    int WinCount = 0;
    int ValidMousePoint = FALSE;
#ifndef NOWIDECHAR
    int x = 0, y = 0;
    char AttrString[StringMaxChr];
#endif
     
    if ( ! DestBuf)
      return 1;
    AddFormattedRecord(DestBuf, " screenHeight = %d", g_ScreenHeight);
    AddFormattedRecord(DestBuf, "  screenWidth = %d", g_ScreenWidth);
    
    while (Win != NULL) { // Window loop.
      int WindowHeight = Win->WindowHeight;
      char delim[50] = "";
      char Key = Win->WindowKey;
       
      if (Key == '\0')
        strcpy(String, "current buffer");
      else
        sprintf(String, "buffer %c%s", Key,  (Key == SourceBufKey) ? " (current buffer)" : "");
      if (Win->DisplayName) {
        strcpy(delim, ", followed by a delimiter line");
        LineCount++; }
      AddFormattedRecord(DestBuf, "      win:%-2d = %s, %d lines%s", WinCount++, String, WindowHeight, delim);
      LineCount += WindowHeight; 
      Win = Win->NextWindow; }
      
    AddFormattedRecord(DestBuf, "  unallocated = %d", g_ScreenHeight-LineCount);
    AddFormattedRecord(DestBuf, "");
    
    if (ValidMousePoint)
      AddFormattedRecord(DestBuf, "Mouse cursor at: (%d, %d), Buffer %c, Line %d, Chr %d", g_Mouse_x, g_Mouse_y, g_MouseBufKey, g_MouseLine, g_Mouse_x);
    else
      AddFormattedRecord(DestBuf, "No valid mouse point set.");
    AddFormattedRecord(DestBuf, "");
      
#if defined(VC)
    AddFormattedRecord(DestBuf, "Screen dump follows:");
    for (y = 0; y < g_ScreenHeight; y++) {
      COORD Pos = { (short)0, (short)y };
      int AttrChr = 0, Attrs, PrevAttr = Normal_Text;
      
      ReadConsoleOutputCharacter(hStdout, String, StringMaxChr, Pos, &Chrs);
      ReadConsoleOutputAttribute(hStdout, AttrWords, StringMaxChr, Pos, &Attrs);
      String[g_ScreenWidth] = '\0';
      AddFormattedRecord(DestBuf, "%s", String);
      AttrString[0] = '\0';
      for (x = 0; x < g_ScreenWidth; x++) {
        int Attr = AttrWords[x];
        if (Attr != PrevAttr) {
          if (Attr==Normal_Text)
            AttrChr += sprintf(AttrString+AttrChr, "(X=%d Normal_Text) ", x);
          else if (Attr==Selected_Substring)
            AttrChr += sprintf(AttrString+AttrChr, "(X=%d Selected_Substring) ", x);
          else if (Attr==Reverse_Video)
            AttrChr += sprintf(AttrString+AttrChr, "(X=%d Reverse_Video) ", x);
          else if (Attr==Current_Chr)
            AttrChr += sprintf(AttrString+AttrChr, "(X=%d Current_Chr) ", x);
          else
            AttrChr += sprintf(AttrString+AttrChr, "(X=%d colour %X/%X) ", x, Attr&0xF, (Attr>>4)&0xF);
          PrevAttr = AttrWords[x]; } }
      if (AttrChr)
        AddFormattedRecord(DestBuf, "Attrs: %s", AttrString); }
#elif !defined(NOWIDECHAR)
    AddFormattedRecord(DestBuf, "Screen dump follows:");
    for (y = 0; y < g_ScreenHeight; y++) {
      int Byte = 0, PrevAttr = 0, AttrChr = 0;
      short PrevColourPair = 0;
      AttrString[0] = '\0';
      for (x = 0; x < g_ScreenWidth; x++) {
        int Attr;
        cchar_t C_chr;
        wchar_t wch;
        attr_t attrs;
        short ColourPair, Forground, Background;
        
        mvin_wch(y, x, &C_chr);
        Attr = C_chr.attr & 0xFFEFF;
        if (Attr != PrevAttr) {
          getcchar(&C_chr, &wch, &attrs, &ColourPair, NULL);
          if (Attr==Normal_Text)
            AttrChr += sprintf(AttrString+AttrChr, "(X=%d Normal_Text) ", x);
          else if (Attr==Selected_Substring)
            AttrChr += sprintf(AttrString+AttrChr, "(X=%d Selected_Substring) ", x);
          else if (Attr==Reverse_Video)
            AttrChr += sprintf(AttrString+AttrChr, "(X=%d Reverse_Video) ", x);
          else if (Attr==Current_Chr)
            AttrChr += sprintf(AttrString+AttrChr, "(X=%d Current_Chr) ", x);
          else if (ColourPair != PrevColourPair) {
            pair_content(ColourPair, &Forground, &Background);
            AttrChr += sprintf(AttrString+AttrChr, "(X=%d colour %X/%X) ", x, Forground, Background);
            PrevColourPair = ColourPair; }
          PrevAttr = Attr; }
        Byte += wcstombs(String+Byte, C_chr.chars, MB_CUR_MAX); }
      String[g_ScreenWidth] = '\0';
      AddFormattedRecord(DestBuf, "%s", String);
      if (AttrChr)
        AddFormattedRecord(DestBuf, "Attrs: %s", AttrString); }
#endif
 }

  else if (strstr(Text, "date") == Text) { //date - return todays date only.
    char temp[100];
    time_t DateTime;
     
    if ( ! DestBuf)
      return 1;
    DateTime = time(NULL);
    strftime(temp, 100, "%d/%m/%y, %H:%M:%S", localtime(&DateTime));
    TextViaJournal(DestBuf, temp, "[ date ]"); }

  else if (strstr(Text, "inview") == Text) { //Is current character visible with current LeftOffset setting.
    int PerceivedChr = JotGetPerceivedChr(SourceBuf);
     
    PushInt(SourceBuf->LeftOffset);
    PushInt(g_ScreenWidth);
    PushInt(PerceivedChr);
    return ! (SourceBuf->LeftOffset < PerceivedChr && PerceivedChr <= SourceBuf->LeftOffset+g_ScreenWidth); }
       
  else if (strstr(Text, "commandmode") == Text) //Query in command mode
    return g_ScreenMode;
  
  else if (strstr(Text, "samesinceio") == Text) //Query SameSinceIO in this buffer.
    return ((SourceBuf->UnchangedStatus) & SameSinceIO) == 0;
   
  else if (strstr(Text, "samesinceindexed") == Text) //Query SameSinceIndexed in this buffer.
    return ((SourceBuf->UnchangedStatus) & SameSinceIndexed) == 0;
   
  else if (strstr(Text, "samesincecompiled") == Text) //Query SameSinceCompiled in this buffer.
    return ((SourceBuf->UnchangedStatus) & SameSinceCompiled) == 0;
   
  else if (strstr(Text, "sameflag1") == Text) //Query or set SameFlag1 in this buffer.
    return  ((SourceBuf->UnchangedStatus) & SameFlag1) == 0;
   
  else if (strstr(Text, "pid") == Text) { //pid - Process ID of current process.
    char PID[StringMaxChr];
    if (sprintf(PID, "%d", getpid()) <= 1)
      return 1;
    GetRecord(DestBuf, strlen(PID)+1);
    strcpy(DestBuf->CurrentRec->text, PID); }
  
  else if (strstr(Text, "env") == Text) { //env - return translation of env variable.
    char * trans;
    char Name[1000];
     
    if ( ! DestBuf)
      return 1;
    sscanf(Text+4, "%s", (char *)&Name);
    trans = getenv(Name);
    if (trans == 0) {
      char temp[100];
      GetRecord(DestBuf, 20);
      strcpy(temp, " - <Undefined> - ");
      strcpy(DestBuf->CurrentRec->text, temp);
      return 1; }
    GetRecord(DestBuf, strlen(trans)+1);
    strcpy(DestBuf->CurrentRec->text, trans); }

  else if (strstr(Text, "dir") == Text) { //dir <path> - return directory contents.
#if defined(VC)
    char * Qualifier;
#else
    DIR * PathElemDIR;
    struct dirent * Entry = NULL;
    struct stat Stat;
//    char * Qualifier;
#endif
    char temp[StringMaxChr], Path[StringMaxChr];
    int EndQual = 4;
    int PathLen;
    
    if (strlen(Text+4) < 1)
      return 1;
    if ( ! DestBuf)
      return 0;
      
    while (0 < strlen(Text+EndQual)) {
      if(Text[EndQual] == ' ')
        EndQual++;
      else {
        if (Text[EndQual] == '-')
          DestBuf->FirstRec->text[EndQual-1] = '\t';
        if (strstr(Text+EndQual, "-mtime") == Text+EndQual)
          EndQual +=6;
        else if (strstr(Text+EndQual, "-atime") == Text+EndQual)
          EndQual +=6;
        else if (strstr(Text+EndQual, "-ctime") == Text+EndQual)
          EndQual +=6;
        else if (strstr(Text+EndQual, "-uid") == Text+EndQual)
          EndQual +=4;
        else if (strstr(Text+EndQual, "-gid") == Text+EndQual)
          EndQual +=4;
        else if (strstr(Text+EndQual, "-size") == Text+EndQual)
          EndQual +=5;
        else if (strstr(Text+EndQual, "-mode") == Text+EndQual)
          EndQual +=5;
        else if (strstr(Text+EndQual, "-inode") == Text+EndQual)
          EndQual +=6;
        else 
          break; } }
        
    Text[EndQual-1] = '\0';
    strcpy(Path, Text+EndQual);
    ExpandEnv(Path);
    PathLen = strlen(Path);
    if (Path[PathLen-1] == '/' || Path[PathLen-1] == '\\')
      Path[(PathLen--)-1] = '\0';
    if (stat(Path, &Stat)) {
      Fail("Nonexistent directory \"%s\"", Path); 
      return 1; }
#if defined(VC)
    if ( ! DestBuf) 
      return 0;
    while (Qualifier = strstr(DestBuf->FirstRec->text, " -"))
      Qualifier[1] = '\t';
    if ((Stat.st_mode & S_IFMT) != S_IFDIR) {
      Fail("Not a directory \"%s\"", Path); 
      return 1; }
    if ( ! DestBuf)
      return 0;
    strcpy(temp, Path);
    strcat(temp, "\\*");
    DirHand = FindFirstFile(temp, &FileData);
    if (DirHand == INVALID_HANDLE_VALUE)
      return 1;
       
    while ( 1 ) {
      strcpy(temp, Path);
      strcat(temp, "/");
      strcpy(temp, FileData.cFileName);
      stat(temp, &Stat);
      if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        strcat(temp, "/");
#else
    if ((Stat.st_mode & S_IFMT) != S_IFDIR) {
      Fail("Not a directory \"%s\"", Path); 
      return 1; }
    PathElemDIR = opendir(Path);
    if (PathElemDIR)
      Entry = readdir(PathElemDIR);
      
    while (Entry) {
      strcpy(temp, Path);
      strcat(temp, "/");
      strcat(temp, Entry->d_name);
      stat(temp, &Stat);
      if ((Stat.st_mode & S_IFMT) == S_IFDIR) 
        strcat(temp, "/");
#endif
      EndQual = 4;
         
      while (0 < strlen(Text+EndQual)) {
        char FmtResult[100];
//        int ChrsRead;
        
        if(Text[EndQual] == ' ')
          EndQual++;
        else if (strstr(Text+EndQual, "-mtime") == Text+EndQual) {
          EndQual +=6;
          strftime(FmtResult, 100, "\t%Y/%m/%d-%H:%M:%S", localtime(&Stat.st_mtime));
          strcat(temp, FmtResult); }
        else if (strstr(Text, "-atime") == Text+EndQual) {  
          EndQual +=6;
          strftime(FmtResult, 100, "\t%Y/%m/%d-%H:%M:%S", localtime(&Stat.st_atime));
          strcat(temp, FmtResult); }
        else if (strstr(Text, "-ctime") == Text+EndQual) {  
          EndQual +=6;
          strftime(FmtResult, 100, "\t%Y/%m/%d-%H:%M:%S", localtime(&Stat.st_ctime));
          strcat(temp, FmtResult); }
        else if (strstr(Text, "-uid") == Text+EndQual) {  
          EndQual +=4;
          sprintf(FmtResult, "\t%4d", Stat.st_uid); 
          strcat(temp, FmtResult); }
        else if (strstr(Text, "-gid") == Text+EndQual) {  
          EndQual +=4;
          sprintf(FmtResult, "\t%4d", Stat.st_gid); 
          strcat(temp, FmtResult); }
        else if (strstr(Text, "-size") == Text+EndQual) {  
          EndQual +=5;
          sprintf(FmtResult, "\t%10d", (int)Stat.st_size); 
          strcat(temp, FmtResult); }
        else if (strstr(Text, "-mode") == Text+EndQual) {  
          EndQual +=5;
          sprintf(FmtResult, "\t%6o", Stat.st_mode); 
          strcat(temp, FmtResult); }
        else if (strstr(Text, "-inode") == Text+EndQual) {  
          EndQual +=6;
          sprintf(FmtResult, "\t%9d", (int)Stat.st_ino); 
          strcat(temp, FmtResult); }
        else
          break; }
#if defined(VC)
      GetRecord(DestBuf, strlen(temp)+2);
      strcpy(DestBuf->CurrentRec->text, temp);
      if (FindNextFile(DirHand, &FileData) == 0)
        break; }
        
    FindClose(DirHand);
#else
      GetRecord(DestBuf, strlen(temp+PathLen+1)+2);
      strcpy(DestBuf->CurrentRec->text, temp+PathLen+1);
      Entry = readdir(PathElemDIR); }
    closedir(PathElemDIR);
#endif
  }

  else if (strstr(Text, "file") == Text) { //file <pathName> - a selection of info on the selected file.
    struct stat Stat;
    char PathName[StringMaxChr];
    
    if (SourceBufKey == BufferKey)
      return 1;
    Query->CurrentChr += 4;
    VerifyNonBlank(Query);
    strcpy(PathName, Text+Query->CurrentChr);
//    strcpy(PathName, Text+5);
    ExpandEnv(PathName);
    if (stat(PathName, &Stat)) { //Error - the pathName does not point to a valid file.
      if (DestBuf) {
        char temp[100];
        sprintf(temp, "Nonexistent file \"%s\"", PathName); 
        GetRecord(DestBuf, strlen(temp)+1);
        strcpy(DestBuf->CurrentRec->text, temp); }
      return 1; }
    else { //PathName points to a valid file.
      char temp[1000];
      if ( ! DestBuf)
        return 0;
      AddFormattedRecord(DestBuf, "                 Name = \"%s\"", PathName);
#if defined(VC)
      AddFormattedRecord(DestBuf, "                Drive = %c:", Stat.st_dev+'A');
#else
      AddFormattedRecord(DestBuf, "                inode = %d", Stat.st_ino);
#endif
      AddFormattedRecord(DestBuf, "                 Mode = %6o", Stat.st_mode);
      AddFormattedRecord(DestBuf, "                  uid = %d", Stat.st_uid);
      AddFormattedRecord(DestBuf, "                  gid = %d", Stat.st_gid);
      AddFormattedRecord(DestBuf, "                 size = %d", Stat.st_size);
      AddFormattedRecord(DestBuf, " writable by this UID = %s", (euidaccess(PathName, W_OK) == 0) ? "yes" : "no");
      AddFormattedRecord(DestBuf, "            directory = %d", (Stat.st_mode & S_IFMT) == S_IFDIR);
      strftime(temp, 100, "%Y/%m/%d, %H:%M:%S", localtime(&Stat.st_atime));
      AddFormattedRecord(DestBuf, "          Access time = %s", temp);
      strftime(temp, 100, "%Y/%m/%d, %H:%M:%S", localtime(&Stat.st_mtime));
      AddFormattedRecord(DestBuf, "          Modify time = %s", temp);
      strftime(temp, 100, "%Y/%m/%d, %H:%M:%S", localtime(&Stat.st_ctime));
      AddFormattedRecord(DestBuf, "        Creation time = %s", temp); } }

  else if (strstr(Text, "stack") == Text) { //stack - dumps stack to selected buffer.
    char temp[1000];
    int Arg1;
    if ( ! DestBuf)
      return 1;
    sprintf(temp, "Stack currently holds %d item%s (maximum %d):", g_StackPtr, g_StackPtr == 1 ? "" : "s", g_StackSize);
    GetRecord(DestBuf, strlen(temp)+1);
    strcpy(DestBuf->CurrentRec->text, temp);
    sprintf(temp, "%4s %8s %8s %8s", "Item", "Dec", "Hex", "Chr");
    GetRecord(DestBuf, strlen(temp)+1);
    strcpy(DestBuf->CurrentRec->text, temp);
    for (Arg1 = g_StackPtr-1; 0 <= Arg1; Arg1--) { // Stack-dump loop.
      if (g_Stack[Arg1].type == StackIntFrame) {
        long long value = g_Stack[Arg1].Value;
        sprintf(temp, "%4d %8lld %8llX %8c", Arg1, value, value, (int)(value&0x000000FF));
        GetRecord(DestBuf, strlen(temp)+1);
        strcpy(DestBuf->CurrentRec->text, temp); }
      else if (g_Stack[Arg1].type == StackFloatFrame) {
        double value = ((struct floatFrame *)(&g_Stack[Arg1]))->fValue;
        sprintf(temp, "%4d %f", Arg1, value);
        GetRecord(DestBuf, strlen(temp)+1);
        strcpy(DestBuf->CurrentRec->text, temp); }
      else { //It's a buffer frame then.
        struct Buf * Buf = ((struct bufFrame *)(&g_Stack[Arg1]))->BufPtr;
        char TruncatedText[1000];
        strncpy(TruncatedText, Buf->CurrentRec->text, 100);
        AddFormattedRecord(DestBuf, "%4d      Buffer -> \"%s\"", Arg1, TruncatedText); } } }

  else if (strstr(Text, "verify") == Text) { //Verifies the basic integrity of the buffer and the sequence.
    struct Rec * ThisRec = SourceBuf->FirstRec;
    int LineNo = 1, TagCount;
//    struct Com *ThisCom = g_CurrentCommand;
    struct Tag * ThisTag;
//    struct HTabEntry * HTEntry;
    struct Buf *ThisBuf = g_BufferChain;
    struct Buf * ActiveBuffers[256];
    int ErrorsDetected = 0;
    int TotalLen = strlen(SourceBuf->CurrentRec->text);
    
    if (DestBuf)
      AddFormattedRecord(DestBuf, "Verifying records in buffer %c", SourceBuf->BufferKey);
    if (SourceBuf->CurrentChr < 0) {
      ErrorsDetected++;
      if (DestBuf)
        AddFormattedRecord(DestBuf, "Current Character is negative (%d)",  SourceBuf->CurrentChr); }
    if (TotalLen < SourceBuf->CurrentChr || TotalLen < SourceBuf->CurrentChr+SourceBuf->SubstringLength) {
      ErrorsDetected++;
      if (DestBuf) {
        AddFormattedRecord(DestBuf, "Overall string length %d, is exceeded by either (CurrentChr=%d) or CurrentChr+(SubstringLength=%d)",
          TotalLen, SourceBuf->CurrentChr, SourceBuf->SubstringLength);
        AddFormattedRecord(DestBuf, "  for current record: \"%s\"", SourceBuf->CurrentRec->text); } }
       
    do { 
      int Length = strlen(ThisRec->text);
      if (ThisRec->next->prev != ThisRec) {
        ErrorsDetected++;
        if (DestBuf)
          AddFormattedRecord(DestBuf, "Ooops! at line %d of buffer %c, the next record does not point back to this record.", LineNo, SourceBuf->BufferKey);
        break; }
      if (ThisRec->length < Length) {
        ErrorsDetected++;
        if (DestBuf)
          AddFormattedRecord(DestBuf, "Ooops! at line %d of buffer %c, the text (%d chrs.) is longer than the allocated size of the record (%d chrs).", 
            LineNo, SourceBuf->BufferKey, Length, ThisRec->length);
        break; }
      if (DestBuf)
        AddFormattedRecord(DestBuf, "Rec %4d:\"%s\"", LineNo++, ThisRec->text);
      ThisTag = ThisRec->TagChain;
      TagCount = 0;
      while (ThisTag) {
        if (ThisTag->StartPoint < 0 || Length < ThisTag->StartPoint) {
          ErrorsDetected++;
          if (DestBuf)
            AddFormattedRecord(DestBuf, "Ooops: Tag has an out-of-range character pointer"); }
        if (ThisTag->Type == TargetType) {
          struct HtabEntry * ThisHTEntry = (struct HtabEntry * )ThisTag->Attr;
          if (ThisTag->StartPoint != ThisHTEntry->CurrentChr) {
            ErrorsDetected++;
            if (DestBuf)
              AddFormattedRecord(DestBuf, "  Target-point tag at chr %d, does not agree with Htab Target-point character (%d)", ThisTag->StartPoint, ThisHTEntry->CurrentChr); }
          if (DestBuf)
            AddFormattedRecord(DestBuf, "  HTabEntry: Buffer %c, Key %s", ThisHTEntry->HashBufKey, ThisHTEntry->HashKey); }
        if(10 < ++TagCount) {
          ErrorsDetected++;
          if (DestBuf)
            AddFormattedRecord(DestBuf, "Ooops: Tag chain of more than 10 entries");
          break; }
        ThisTag = ThisTag->next; }
      } while ( (ThisRec = ThisRec->next) != SourceBuf->FirstRec->prev);
      
    memset(ActiveBuffers, 0, sizeof(struct Buf *)*256);
    while (ThisBuf != NULL) {
      if ( ThisBuf->BufferKey != '~' && ActiveBuffers[(int)ThisBuf->BufferKey]) {
        ErrorsDetected++;
        if (DestBuf)
          AddFormattedRecord(DestBuf, "  Duplicate buffer found key=\'%c\', current line (this one) \"%s\" (last one) \"%s\"", 
            ThisBuf->BufferKey, ThisBuf->CurrentRec->text, ActiveBuffers[(int)ThisBuf->BufferKey]->CurrentRec->text); }
      ActiveBuffers[(int)ThisBuf->BufferKey] = ThisBuf;
      ThisBuf = ThisBuf->NextBuf; }
    
    return ErrorsDetected; }

  else {
    RunError("Unrecognized %%Q qualifier %s", Text);
    return 0; }
    
  return 0;  }

//-----------------------------------------------BinCmp
int BinCmp(char * String, char * x, int n) {
  //Returns TRUE if first n bytes of String are the same as those in x, works even if string contains '\0' bytes.
  int i;
  for (i = 0; i < n; i++)
    if (String[i] != x[i])
      return FALSE;
  return TRUE; }

//-----------------------------------------------Backtrace
int Backtrace(struct Buf * DestBuffer) {
  //Writes backtrace to nominated buffer.
  struct BacktraceFrame * ThisFrame = s_BacktraceFrame;
  struct Com * Command = g_CurrentCommand;
   
  if ( ! DestBuffer)
    return 1;
     
  while (Command) {
//    int Last = strlen(Command->CommandRec->text)-1;
    int ComChrNo = Command->CommandChrNo;
    int ComChrs = Command->ComChrs;
    char * Temp1 = (char *)malloc(ComChrNo+1);
    char * Temp2 = (char *)malloc(ComChrs+2);
    
    strncpy(Temp1, Command->CommandRec->text, ComChrNo);
    Temp1[ComChrNo] = '\0';
    strncpy(Temp2, Command->CommandRec->text+ComChrNo, ComChrs);
    Temp2[ComChrs] = '\0';
    if (ThisFrame && ThisFrame->Type == ScriptFrame) {
      struct CommandFile * ThisCommFile = ThisFrame->EditorInput;
      AddFormattedRecord(DestBuffer, "  Line %4d of file %s: %s[%s]%s", ThisCommFile->LineNo, ThisCommFile->FileName, Temp1, Temp2, Command->CommandRec->text+ComChrNo+ComChrs); }
    else {
      char Key = Command->CommandBuf->BufferKey;
      AddFormattedRecord(DestBuffer, "  Line %4d of macro %c: %s[%s]%s", Command->CommandLineNo, Key ? Key : ' ', Temp1, Temp2, Command->CommandRec->text+ComChrNo+ComChrs); }
    free(Temp1);
    free(Temp2);
    
    if ( ! ThisFrame)
      break;
    if (Command == ThisFrame->ParentCommand)
      ThisFrame = ThisFrame->prev;
    if ( ! ThisFrame)
      break;
    Command = ThisFrame->ParentCommand; }
    
  GetRecord(DestBuffer, 0);
  return 0; }

//---------------------------------------------FetchStringArg
char * FetchStringArg(struct Arg **CurrentArg_a, char *DefaultString)
  { //
  //Extracts string arg value and indexes to next.
  //If the type is DeferredStringArg then returns pointer to text,
  //if no arg, returns DefaultString unchanged
  //other args are copied into the DefaultString, returning the default string.
  //
  char * Value = 0;
  
  if (*CurrentArg_a == NULL) {
    RunError("Insufficient args in list.");
    return ""; }
    
  if (((*CurrentArg_a)->type) == DeferredStringArg) { // Indirection via buffer.
    char Key = (*CurrentArg_a)->intArg;
    struct Buf * ArgStringBuf;
    
    if (Key == '~') { //It's an indirect reference to the stack, but there's not a buffer at the top - just do the obvious decimal conversion.
      int Type;
      
      if (g_StackPtr < 1) {
        Fail("Stack underflow");
        return ""; }
      Type = g_Stack[g_StackPtr-1].type;
      if (Type == StackBufFrame)
        ArgStringBuf = ((struct bufFrame *)(&g_Stack[g_StackPtr-1]))->BufPtr;
      else {
        if (Type == StackIntFrame)
          sprintf(s_ArgString, "%lld", (long long)g_Stack[g_StackPtr-1].Value);
        else
          sprintf(s_ArgString, "%f", ((struct floatFrame *)(&g_Stack[g_StackPtr-1]))->fValue);
        KillTop();
        *CurrentArg_a = (*CurrentArg_a)->next;
        return s_ArgString; } }
    else
      ArgStringBuf = GetBuffer((int)Key, NeverNew);
       
    if (ArgStringBuf)
      Value = ArgStringBuf->CurrentRec->text;
    else {
      Fail("Undefined buffer %c", Key);
      return ""; } }
       
  else if ((*CurrentArg_a)->type != StringArg) {
    RunError("Expecting a string argument, found some other type.");
    return ""; }
     
  else { // A literal string was specified.
    Value = (char *)(*CurrentArg_a)->pointer;
    if (Value == NULL) {
      *CurrentArg_a = (*CurrentArg_a)->next;
      return DefaultString; }
    else if (DefaultString)
      strncpy(DefaultString, Value, StringMaxChr); }
       
  *CurrentArg_a = (*CurrentArg_a)->next;
  return Value; }

//-----------------------------------------------ExpandDeferredString
void ExpandDeferredString(struct Buf * DestinationBuf, char * Arg)
  { //Expands an indirect reference to a string in nominated buffer - only called by percent commands.
  char * Quote;
  char * Rec;
  int StartPoint = 0;
  char RecursionCheck[256];
  int Index = 0;
  
  if (DestinationBuf->CurrentRec->length < strlen(Arg)) {
    FreeRecord(DestinationBuf, AdjustBack);
    GetRecord(DestinationBuf, strlen(Arg)+100); }
//    GetRecord(DestinationBuf, StringMaxChr);
  strcpy(DestinationBuf->CurrentRec->text, Arg);
  RecursionCheck[0] = '\0';
   
  while ((Quote = strchr((Rec = DestinationBuf->CurrentRec->text)+StartPoint, '\''))) {
    char Key = ChrUpper(Quote[1]);
    struct Buf * ArgStringBuf;
    
    if (Key == '\0')
      return;
    if (Key == '~') { //It's an indirect reference to the stack, but there's not a buffer at the top - just do the obvious decimal conversion.
      int Type;
      
      if (g_StackPtr < 1) {
        Fail("Stack underflow");
        return; }
      Type = g_Stack[g_StackPtr-1].type;
      if (Type == StackBufFrame)
        ArgStringBuf = ((struct bufFrame *)(&g_Stack[g_StackPtr-1]))->BufPtr;
      else {
        ArgStringBuf = NULL;
        if (Type == StackIntFrame)
          sprintf(s_ArgString, "%lld", (long long)g_Stack[g_StackPtr-1].Value);
        else
          sprintf(s_ArgString, "%f", ((struct floatFrame *)(&g_Stack[g_StackPtr-1]))->fValue); } }
    else
      ArgStringBuf = GetBuffer((int)Key, NeverNew);
         
    if (strchr(RecursionCheck, Key))  //Recursion detected.
      StartPoint = Quote-Rec+2;
    else  {  //No recursion, 
      DestinationBuf->CurrentChr = Quote-Rec;
      DestinationBuf->SubstringLength = 2;
      SubstituteString(DestinationBuf, ArgStringBuf ? ArgStringBuf->CurrentRec->text : s_ArgString, -1); }
      
    if ( ! ArgStringBuf && Key == '~') //Only kill top entry if it's numeric.
      KillTop();
       
    RecursionCheck[Index++] = Key;
    RecursionCheck[Index] = '\0'; }
    
  DestinationBuf->CurrentChr = 0; }

//----------------------------------------------UpdateWindowList
void UpdateWindowList(struct Buf * CmdBuf)
  { //Maintains the g_FirstWindow list, interpreting %w commands passed from main command scanner.
  char Key = '\0', Chr = '\0';
  int Arg1;
  struct Window *Win = g_FirstWindow;
  struct Window *PrevWin = Win;
    
  if (g_TTYMode)
    return;
  Chr = StealCharacter(CmdBuf);
   
  if (! Chr) { //No arguments - delete all windows.
    while (Win != NULL) { // Window-clear loop.
      struct Window *NextWin = Win->NextWindow;
      free(Win);
      Win = NextWin; }
       
    g_FirstWindow = NULL;
    g_FirstWindowLine = 1;
    g_LastWindowLine = g_ScreenHeight;
    return; }
    
  //Args given - add new window or delimiter line.
  sscanf(CmdBuf->CurrentRec->text, "%i %c", &Arg1, &Key);
  if ( PrevWin && (Chr == '-' || Chr == '+')) { //Incremental change current window size.
    while ( Win ) {
      if (Win->WindowKey == '\0' || Win->WindowKey == g_CurrentBuf->BufferKey)
        break;
      PrevWin = Win;
      Win = Win->NextWindow; }
       
    if (0 <= (Win->WindowHeight + Arg1))
      Win->WindowHeight += Arg1;
    else
      Win->WindowHeight = 0;
    return; }
       
  else if (Arg1 == 0) { //Add delimiter with filename after last-defined window.
    while ( Win && (Win = Win->NextWindow) )
      PrevWin = Win;
    if ( ! PrevWin)
      return;
    PrevWin->DisplayName = TRUE;
    return; }
     
  else { //Add another window, to follow last-defined window.
    int Index = 1;
    struct Window *NewWin = (struct Window *)malloc(sizeof(struct Window));
    struct Buf *Buffer;
    struct Rec *FirstRec;
    int TotalSize = 0;
    
    while (Win) {
      int Size = Win->WindowHeight;
      TotalSize += (Size == 0) ? 1 : Size;
      PrevWin = Win;
      Win = Win->NextWindow; }
       
    if (NewWin == NULL) { //Run out of heap.
      RunError("UpdateWindowList:run out of heap space.");
      return; }
    if ( g_ScreenHeight <= (TotalSize+Arg1) ) {
      free(NewWin);
      RunError("Total window size exceeds terminal height");
      return; }
    if (PrevWin == NULL)
      g_FirstWindow = NewWin;
    else
      PrevWin->NextWindow = NewWin;
    NewWin->NextWindow = NULL;
    NewWin->OldLastLineNumber = 0;
    NewWin->LastSubstringStart = 0;
    NewWin->LastSubstringLength = 0;
    NewWin->WindowHeight = Arg1;
    NewWin->DisplayName = FALSE;
    Key = ChrUpper(Key);
    NewWin->WindowKey = Key;
    if (Key == '\0')
      return;
    Buffer = GetBuffer(Key, OptionallyNew);
    FirstRec = Buffer->FirstRec;
    if (FirstRec != NULL)
      FirstRec->prev->DisplayFlag = Redraw_Line;
    else
      GetRecord(Buffer, 1);
    for (Index = 1; Index <= g_ScreenHeight; Index +=1) {
      s_ScreenTable[Index] = NULL;
      JotGotoXY(0, Index-1);
      JotClearToEOL(); } }
    
  JotUpdateWindow(); }

//----------------------------------------------AnalyzeTabs
void AnalyzeTabs(struct Buf * TextBuf, struct Rec * LastRec, int LineCount, int * TabStops)
  { //LastRec (the last record in the screen window) and LineCount (the number of lines in window), together define a sequence of visible records in the buffer, 
    //thisFinds position of each tab in the section and assigns auto-TabStops
    //If the buffer has a header record defined, this may contain tabs so this record is also included in the analysis.
    //
  struct Rec * Record = LastRec;    //The current record, at the bottom of the window.
  int TabsHWM = 0;                  //The most tabs that have been found in any one line, to date.
  int TabIndex;                     //Used as an index for final tweaks.
  int LineIndex;                    //Counts lines.
  int TabstopsChange = FALSE;       //Flag indicates that the tabstops have been changed.
  int NewTabStops[MaxAutoTabStops]; //The newly-assigned set of tabstops.
  char * String = Record->text;     //The record text from a record currently in the display.
  
  NewTabStops[0] = 0; 
   
  for (LineIndex = 0; LineIndex <= LineCount+1; LineIndex++) { //Note the width of each column.
    int CellWidth;                                             //The width of the text in the current cell.
    int TabStopIndex = 0;                                      //Counts tabs in the current record.
    char * SubString;                                          //The text following the most recently-found tab.
    
    
    if (strchr(String, g_TableSeparator))
      while (strlen(String)) {
        if ( ! (SubString = strchr(String, g_TableSeparator)))
          SubString = String+strlen(String);
        if (MaxAutoTabStops < ++TabStopIndex) {
          Fail("More than %d tabs in record %s while assigning TabStops", MaxAutoTabStops, String);
          return; }
        CellWidth = JotStrlenBytes(String, SubString-String)+1;
           
        String = SubString+1;
        if (TabsHWM < TabStopIndex) {
          NewTabStops[TabStopIndex] = 0;
          TabsHWM = TabStopIndex;
          NewTabStops[0] = TabStopIndex; }
        if (NewTabStops[TabStopIndex] < CellWidth)
          NewTabStops[TabStopIndex] = CellWidth; }
      
    if (LineIndex < LineCount-1) {
      Record = Record->prev;
      String = Record->text; }
    else //The extra line is reserved for the header - if specified.
      if (TextBuf->Header)
        String = TextBuf->Header;
      else
        break; }
    
  for (TabIndex = 2; TabIndex <= NewTabStops[0]; TabIndex++) { //Currently the table only has column widths - change to absolute column numbers.
    NewTabStops[TabIndex] += NewTabStops[TabIndex-1];
    if ( (TabStops[0] < TabIndex) || (TabStops[TabIndex] != NewTabStops[TabIndex]) )
      TabstopsChange = TRUE; }
    
  if ( ! TabstopsChange)
    return;
     
  for (TabIndex = 0; TabIndex <= NewTabStops[0]; TabIndex++) //Copy new tabstops.
    TabStops[TabIndex] = NewTabStops[TabIndex];
     
  for (LineIndex = 1; LineIndex <= g_ScreenHeight; LineIndex +=1)
    s_ScreenTable[LineIndex] = NULL;
  return; }

//----------------------------------------------WriteString
int WriteString(char * OrigString, int ScreenLine, int Attr, int New, int LastByte, int LeftOffset, struct ColourTag * ColourTag, int OrigTabStops[])
  { //
    //Checks length of string then outputs text string to window, trapping control characters.
    //New is set true at the start of a new line.
    //The static variables CurrentByte and CurrentChr, for normal ascii text contining no tabs or unicode characters,  will always track one another.
    //Lines containing tabstops tends to make the CurrentChr overtake CurrentByte - as WriteString inserts padding blanks to align the columns.
    //Lines containing unicode tends to make CurrentByte overtake CurrentChr as additional bytes are extracted to define one unicode character on the screen.
    //The static variable TabCount indicates the current TabChrs[] entry of the current character.
    //N.B. This proceedure is designed to be called in sequence, with different Attr settings starting at byte 0 of the record.
    //Returns TRUE if, on exit, CurrentChr is in view.
    //
  static int CurrentByte;         //Points to next byte in string.
  static int CurrentChr;          //Counts characters as displayed on the screen, ignoring LeftOffset - i.e. the X coord in an imaginary very wide screen.
  static int TabIndex;            //Counts tabs found in String.
  static int * TabBytes = NULL;   //Indicates character nos. of each tab point, TabBytes[0] must bt set to 0.
  static int * TabChrs = NULL;    //Expansion of OrigTabStops, TabChrs[0] is 0.
  static int TabIndexMax;         //Last valid entry in TabBytes and TabChrs.
  static char * String = NULL;    //Copy of originnal string with control characters changed to tildes.
  static int EndStopChr;          //The last character of thcell to appear on screen (normally CellChrs[CellIndex+1]-1, but sometimes limited by screenwidth.
  static int PaddingChrs;         //No. of blanks to be used as a prefic to cell characters. If negative, the cell text is to be truncated.
  static int InView = TRUE;       //Set FALSE by various out-of-view checks.
      
  int Len = strlen(OrigString);
  int ByteCount, ChrCount;
  int i;
  
  if (New) {
    InView = TRUE;
    CurrentByte = 0;
    CurrentChr = 0;
    if (TabBytes) {
      free(TabBytes);
      free(TabChrs); }
    TabBytes = TabChrs = NULL;
     
    if (OrigTabStops) { //Map tab points
      char * EndPoint = OrigString;
      int OrigTabIndex = 1;
      int PrevTabStop = 0;
       
      TabBytes = (int *)malloc(sizeof(int)*(Len+2));
      TabChrs = (int *)malloc(sizeof(int)*(Len+2));
      TabBytes[0] = 0;
      TabChrs[0] = 0;
      TabIndex = 0;
      PrevTabStop = 0;
      while ( (EndPoint = strchr(EndPoint, g_TableSeparator)) ) {
        TabBytes[++TabIndex] = (EndPoint++)-OrigString+1;
        TabChrs[TabIndex] = (OrigTabIndex <= OrigTabStops[0]) ? OrigTabStops[OrigTabIndex++] : (TabChrs[TabIndex-1]*2)-PrevTabStop;
        PrevTabStop = TabChrs[TabIndex-1]; }
      TabBytes[++TabIndex] = Len+1;
      TabChrs[TabIndex] = (OrigTabIndex <= OrigTabStops[0]) ? OrigTabStops[OrigTabIndex++] : (TabChrs[TabIndex-1]*2)-PrevTabStop;
      TabIndexMax = TabIndex;
      TabIndex = -1;
      EndStopChr = LeftOffset+g_ScreenWidth; }
     
    if (String)
      free(String);
    String = (char *)malloc(Len+1);
    for (i=0; i < Len; i++) { //Replace nonprinting characters with tildes.
      char Chr = OrigString[i];
      if (Chr < ' ') {
        if (Chr < 0) {
          if (s_NoUnicode)
            Chr = '~'; }
        else {
          if ( (Chr = g_TableSeparator) )
          Chr = '~'; } }
      String[i] = Chr; }
    String[Len] = '\0';
       
    JotGotoXY(0, ScreenLine);
    JotClearToEOL(); }
       
  JotSetAttr(Attr);
  
  if (TabChrs && 1 < TabIndexMax) { //Display a section of tabular-text record.
    // TabBytes[TabIndex] = Byte at start of current-cell text.
    // TabBytes[TabIndex+1]-2 = Last Byte of current cell.
    // TabChrs[TabIndex] = (X-coord-LeftOffset) of character at start of cell display
    // TabChrs[TabIndex+1]-1 = (X-coord-LeftOffset) of character at end of cell display.
      
    for ( ; CurrentByte < LastByte && CurrentChr <= TabChrs[TabIndexMax]; ) {
      if (TabIndex < 0 || TabBytes[TabIndex+1]-2 < CurrentByte) {     //New cell - setup.
        int CellChrs;
        if(TabIndexMax <= ++TabIndex)
          break;
        if (TabChrs[TabIndex+1]-1 < LeftOffset) {                     //All of this cell is left of the LeftOffset - skip to next cell.
          CurrentByte = TabBytes[TabIndex+1];
          InView = FALSE;
          continue; }
        else
          InView = TRUE;
        CurrentByte = TabBytes[TabIndex];
        CurrentChr = TabChrs[TabIndex]+1;
        EndStopChr = LeftOffset+g_ScreenWidth < TabChrs[TabIndex+1] ? LeftOffset+g_ScreenWidth : TabChrs[TabIndex+1];
        if (EndStopChr < CurrentChr)
          break;
        CellChrs = JotStrlenBytes(String+TabBytes[TabIndex], TabBytes[TabIndex+1]-TabBytes[TabIndex]-1);
        PaddingChrs = TabChrs[TabIndex+1]-CurrentChr-CellChrs+1;
        if (g_ScreenWidth+LeftOffset < CurrentChr+PaddingChrs)        //All of current cell's text is off the right margin of the screen - exit now.
          PaddingChrs = g_ScreenWidth+LeftOffset-CurrentChr+1;
        if (CurrentChr <= LeftOffset) {                               //Current cell has text left of LeftOffset - truncation required.
          int ChrsToLop = LeftOffset-CurrentChr+1;
          if (PaddingChrs < ChrsToLop)                                //Truncate leftmost characters.
            CurrentByte += JotStrlenChrs(String+CurrentByte, ChrsToLop-PaddingChrs);
          PaddingChrs -= ChrsToLop;
          CurrentChr += ChrsToLop; }
        if (0 < PaddingChrs) {                                        //Text is shorter than allocated cell - justify with blanks.
          char Padding[StringMaxChr];
          memset(Padding, ' ', PaddingChrs);
          Padding[PaddingChrs] = '\0';
          JotAddBoundedString(Padding, ColourTag, PaddingChrs, g_ScreenWidth, &ByteCount, &ChrCount);
          CurrentChr += ChrCount; }
        else if (PaddingChrs < 0) {                                   //Negative padding indicates a truncated cell.
          JotAddBoundedString("!", ColourTag, 1, 1, NULL, NULL);
          CurrentByte += JotStrlenChrs(String+CurrentByte, 1);
          CurrentChr++;
          InView = FALSE; }
        if (g_ScreenWidth+LeftOffset <= TabChrs[TabIndex+1]-1 && CurrentChr <= EndStopChr) { //Current cell truncated by right margin.
          JotAddBoundedString("!", ColourTag, 1, 1, NULL, NULL);
          CurrentChr++;
          InView = FALSE; } }
          
      if (g_ScreenWidth < EndStopChr-CurrentChr) //No point in attempting to display anything beyond the right margin  - especially if the leftoffset is off in hyperspace, then crashes can happen.
        return FALSE;
      JotAddBoundedString(String+CurrentByte, ColourTag, LastByte-CurrentByte, EndStopChr-CurrentChr+1, &ByteCount, &ChrCount);
      CurrentByte += ByteCount;
      CurrentChr += ChrCount;
      if (g_ScreenWidth+LeftOffset < CurrentChr)
        return LastByte <= CurrentByte && InView;
         
      if (CurrentByte < LastByte)
        CurrentByte = TabBytes[TabIndex+1]; }
         
    return LastByte <= CurrentByte && InView; }
  
  else {                                                               //Linear text and no TabChrs and not currently off the screen.
    int LastChr = JotStrlenBytes(String, LastByte);
    int StartOffsetByte = 0;
    int StartOffsetChr = 0, EndOffsetChr = 0;
    int Bytes, Chrs;
    
    InView = TRUE;
    if (g_ScreenWidth+LeftOffset < CurrentChr)
      return FALSE;
    if (CurrentChr < LeftOffset)                                       //Suppress printing before LeftOffset chr.
      StartOffsetChr = LeftOffset-CurrentChr;
    StartOffsetByte = JotStrlenChrs(String, StartOffsetChr);
    if (g_ScreenWidth+LeftOffset < LastChr) {                          //Suppress printing beyond g_ScreenWidth+LeftOffset chr.
      InView = FALSE;
      EndOffsetChr = LastChr - g_ScreenWidth - LeftOffset; }
       
    if (0 < (Chrs = LastChr-CurrentChr-StartOffsetChr-EndOffsetChr) ) {
      JotAddBoundedString(String+CurrentByte+StartOffsetByte, ColourTag, Len-CurrentByte-StartOffsetByte, Chrs, &Bytes, NULL);
       
      CurrentChr = LastChr;
      CurrentByte += Bytes+StartOffsetByte; }
    else if (CurrentByte == Len && Attr == Current_Chr && CurrentChr < g_ScreenWidth+LeftOffset)
      JotAddBoundedString("~", ColourTag, 1, 1, NULL, NULL);
    return ( (LeftOffset <= CurrentChr+1) && InView ); } }

//----------------------------------------------MoveDown
int MoveDown(struct Buf *TextBuf, int Offset, char * EndRecord)
  { // Resets character pointer to begining of record, 'Offset' records away from the current record.  
    //If one end of the text image or the optional specified EndRecord is encountered then it stops there.
    //Function returns no. (+ve. or -ve.) records actually moved. 
     
  int Distance = 0;
  struct Rec *Record = TextBuf->CurrentRec;
  struct Rec *FirstRecordOfBuf = TextBuf->FirstRec;
  struct Rec *LastRecordOfBuf = FirstRecordOfBuf->prev;
  int Count;
  
  if (0 <= Offset)
    for (Count = 1; Count <= Offset; Count++) {// +ve. or zero offset: forwards loop.
      if (Record == LastRecordOfBuf || Record->text == EndRecord)
        break;
      Record = Record->next;
      Distance = Distance+1; }
  else
    for (Count = 1; Count <= -Offset; Count++) {// -ve. offset backwards loop.
      if (Record == FirstRecordOfBuf || Record->text == EndRecord)
        break;
      Record = Record->prev;
      Distance = Distance-1; }
  
  TextBuf->CurrentRec = Record;
  return Distance; }

//-----------------------------------------------JotGetPerceivedChr
int JotGetPerceivedChr(struct Buf * TextBuf)
  { //Returns perceived position of cursor for linear or tabular text.
  int PerceivedChr = 0;
  int * TabStops = TextBuf->TabStops;
  int CurrentChr = TextBuf->CurrentChr;
  int TabCount = 1;
  int TabStop = 0;
  int Bytes = 0;
  int FirstByte = 0;
  int CellWidth, FullWidth;
  char * CellStart, * CellEnd;
   
  if (TabStops) {
    for ( ; Bytes <= CurrentChr; Bytes++) {
      if (TextBuf->CurrentRec->text[Bytes] == g_TableSeparator) {
        if (TabStops[0] <= TabCount)
          TabStop += TabStops[TabCount] - (TabCount==1 ? 0 : TabStops[TabCount-1]);
        else
          TabStop = TabStops[TabCount++];
        FirstByte = Bytes+1;
        PerceivedChr = TabStop; } }
    CellStart = TextBuf->CurrentRec->text+FirstByte;
    CellEnd = strchr(CellStart, '\t');
    CellWidth = TabStops[TabCount] - (TabCount==1 ? 0 : TabStops[TabCount-1]);
    FullWidth = JotStrlenBytes(CellStart, (CellEnd ? CellEnd-CellStart : strlen(CellStart)));
    PerceivedChr += CellWidth - FullWidth + JotStrlenBytes(CellStart, Bytes-FirstByte); }
  else
    PerceivedChr = JotStrlenBytes(TextBuf->CurrentRec->text, TextBuf->CurrentChr+1);
    
  return PerceivedChr; }

//-----------------------------------------------JotUpdateWindow
int JotUpdateWindow()
  { //Calculates new start and end points then displays new and changed lines, returns FALSE if current buffer has not been displayed.
  struct Rec *LastUpdatedRecord = NULL;
  struct Window *Win = g_FirstWindow;
  int DisplayedCurrentBuf = FALSE;
  
  if (g_TTYMode || (g_CurrentBuf->CurrentRec) == NULL) 
    return DisplayedCurrentBuf;
#if defined(VC)
  //This rewinds the screen to the top of history.
  JotGotoXY(0, 0);
#endif
     
  g_EditorOutput = stdout;
  g_FirstWindowLine = 1;
   
  while (Win != NULL) { // Window loop.
    int ThisLine;
    char WindowKey = Win->WindowKey; 
    struct Buf *TextBuf = (WindowKey == '\0') ? g_CurrentBuf : GetBuffer(WindowKey, NeverNew);
    int HeaderSize = TextBuf->Header ? 1 : 0;
    int WindowHeight = Win->WindowHeight - HeaderSize;    //No. of lines available for display.
    short int TextHeight = WindowHeight;                  //No. of lines in display - normally TextHeight == WindowHeight, except for very short buffers
    int LastDrawnLine;                                    //Line no of the line at the bottom of the window.
    int OldLastLineNumber =                               //The last line in display when this buffer was visited last time.
      (Win->WindowKey == '\0') ? TextBuf->OldLastLineNumber : Win->OldLastLineNumber;
    int LastLineNumber = OldLastLineNumber;               //The last line in display for this visit.
    int LineNumber = TextBuf->LineNumber;                 //The line number of the current line.
    int TopOffset = LineNumber;                                        //No. of lines between top of window and current record of text.
    struct Rec *CurrentRec = TextBuf->CurrentRec;
    struct Rec *Record = CurrentRec;
    int CurrentChr = (TextBuf->CurrentChr);
    int CurrentGuard = (s_GuardBandSize*2 <= WindowHeight) ? s_GuardBandSize : WindowHeight/2;
    int SubstringLength = TextBuf->SubstringLength;
    int LeftOffset = TextBuf->LeftOffset;
    int * TabStops = TextBuf->TabStops;
    int Line;

    s_NoUnicode = TextBuf->NoUnicode;
    if(TextBuf == g_CurrentBuf) 
      DisplayedCurrentBuf = TRUE;
    g_LastWindowLine = g_FirstWindowLine+WindowHeight+HeaderSize-1;
    if (TextBuf->Header)
      g_FirstWindowLine++;
    
    //Identify the record to appear in the last screen line (LastLineNumber) and calculate scroll shift.
    TextBuf->CurrentRec = TextBuf->FirstRec;
    
    for (Line = 1; Line <= WindowHeight; Line++) {
      if (TextBuf->CurrentRec == Record)
        TopOffset = Line-1;
      if ((TextBuf->CurrentRec = TextBuf->CurrentRec->next) == TextBuf->FirstRec)
        TextHeight = Line;
      if (TextBuf->CurrentRec == TextBuf->FirstRec)
        break; }
        
    TextBuf->CurrentRec = Record;
    if (LineNumber <= 0)
      OldLastLineNumber = LastLineNumber = LineNumber+WindowHeight-1;
    else if (OldLastLineNumber < TextHeight)
      OldLastLineNumber = LastLineNumber = LineNumber+WindowHeight-TopOffset-1;
 
    LastDrawnLine = g_FirstWindowLine+TextHeight-1;
    LastDrawnLine = g_FirstWindowLine+TextHeight-1;
    TextBuf->CurrentRec = Record;
    
    if (LineNumber < OldLastLineNumber-CurrentGuard) {                     //Line is above old lower guard band.
      if (LineNumber <= OldLastLineNumber-WindowHeight+CurrentGuard)       //Current line is also in or above old top guard.
        LastLineNumber = (LineNumber <= CurrentGuard) ? TextHeight : LineNumber+TextHeight-CurrentGuard-1; }
    else if (WindowHeight-CurrentGuard <= LineNumber)                      //Current line is below the top page.
      LastLineNumber = LineNumber+CurrentGuard;

    //Plod down to record assigned to last line of screen, if lines have been deleted near the end revise LastLineNumber appropriately.
    Record = TextBuf->CurrentRec = CurrentRec;
    for (ThisLine = 1; ThisLine <= LastLineNumber-LineNumber; ThisLine++)
      if (Record == TextBuf->FirstRec->prev) {
        LastLineNumber = LineNumber+ThisLine-1;
        break; }
      else
        Record = Record->next;
     
    //Scroll display while making suitable adjustments to s_ScreenTable - only if there are more records than screen lines.
    if (WindowHeight <= TextHeight)
      JotScroll(g_FirstWindowLine-1, g_LastWindowLine-1, LastLineNumber-OldLastLineNumber);
     
    //Clear any empty lines at end of window when window has more lines than file.
    for (ThisLine = g_LastWindowLine+TextHeight-WindowHeight; ThisLine <= g_LastWindowLine; ThisLine++)
      if (s_ScreenTable[ThisLine] != NULL) { // Clearing unused lines loop.
        JotGotoXY(0, ThisLine-1);
        JotClearToEOL();
        s_ScreenTable[ThisLine] = NULL; }

    if (TabStops && TextBuf->AutoTabStops) //Automatically-assigned TabStops.
      AnalyzeTabs(TextBuf, Record, TextHeight, TabStops);
    if (TextBuf->Header && s_ScreenTable[g_FirstWindowLine-1] != TextBuf->Header) {
      WriteString(TextBuf->Header, g_FirstWindowLine-2, Current_Chr, TRUE, strlen(TextBuf->Header), LeftOffset, NULL, TabStops);
      s_ScreenTable[g_FirstWindowLine-1] = TextBuf->Header; }
    for (ThisLine = LastDrawnLine; g_FirstWindowLine <= ThisLine; ThisLine--) { // Record update loop.
      struct Tag * ThisTag = Record->TagChain;
      if (Record == TextBuf->CurrentRec && TextBuf == g_CurrentBuf) { //Current line of current buffer, overlay line with cursor and substring highlights.
        int CurrentChrEndByte = CurrentChr+JotStrlenChrs(Record->text+CurrentChr, 1);
        int New = TRUE;
        struct ColourTag * Colour = NULL;
        int EndStop = strlen(Record->text);
        int NextColour, LastStop = -1, NextHighlight;
        int Attr = Normal_Text;
        
        while (ThisTag && ThisTag->Type != ColourType) 
          ThisTag = ThisTag->next;
        NextColour = ThisTag ? ThisTag->StartPoint : EndStop;
        NextHighlight = (0 <= SubstringLength) ? CurrentChr : CurrentChr+SubstringLength;
        Win->LastSubstringLength = SubstringLength;
        Win->LastSubstringStart = NextHighlight;
        
        while ( 1 ) {
          if (LastStop < NextHighlight && NextHighlight <= NextColour) { //Reached the NextHighlight point.
            LastStop = NextHighlight;
            if (SubstringLength < 0 && NextHighlight == CurrentChr+SubstringLength) { //At the start of a negative substring.
              WriteString(Record->text, ThisLine-1, Normal_Text, New, CurrentChr+SubstringLength, LeftOffset, Colour, TabStops);
              New = FALSE;
              Attr = Selected_Substring;
              NextHighlight = CurrentChr; }
            else if (NextHighlight == CurrentChr) { //Highlight the current character now.
              WriteString(Record->text, ThisLine-1, Attr, New, CurrentChr, LeftOffset, Colour, TabStops);
              s_InView = WriteString(Record->text, ThisLine-1, Current_Chr, (New = FALSE), CurrentChrEndByte, LeftOffset, Colour, TabStops);
              if (SubstringLength < 0) {
                NextHighlight = EndStop;
                Attr = Normal_Text; }
              else if (0 <SubstringLength) {
                NextHighlight = CurrentChr+SubstringLength;
                Attr = Selected_Substring; }
              else { //No substring.
                NextHighlight = EndStop;
                Attr = Normal_Text; } }
            else if (0 < SubstringLength && NextHighlight == CurrentChr+SubstringLength) {
              WriteString(Record->text, ThisLine-1, Attr, New, NextHighlight, LeftOffset, Colour, TabStops);
              NextHighlight = EndStop;
              Attr = Normal_Text; } }
          else if (ThisTag) {
            LastStop = NextColour;
            if (NextColour <= ThisTag->EndPoint) { //A new colour tag.
              WriteString(Record->text, ThisLine-1, Attr, New, ThisTag->StartPoint, LeftOffset, NULL, TabStops);
              New = FALSE;
              Colour = ThisTag->Attr;
              NextColour = ThisTag->EndPoint+1; }
            else { //Tagged text pending from previous iteration.
              WriteString(Record->text, ThisLine-1, Attr, FALSE, ThisTag->EndPoint+1, LeftOffset, ThisTag->Attr, TabStops);
              Colour = NULL;
              while ( (ThisTag = ThisTag->next) ) 
                if (ThisTag->Type == ColourType)
                  break;
              NextColour = ThisTag ? ThisTag->StartPoint : EndStop; } }
          else { //Current-character highlights and colour tags all done - write to end of line and exit.
            WriteString(Record->text, ThisLine-1, Normal_Text, New, EndStop, LeftOffset, Colour, TabStops);
            break; } }

        g_y = ThisLine-1;
        g_x = JotStrlenBytes(g_CurrentBuf->CurrentRec->text, g_CurrentBuf->CurrentChr);
        Record->DisplayFlag = Remove_Cursor; }
         
      else if (((Record->DisplayFlag) == Redraw_Line) || (Record->DisplayFlag) == Remove_Cursor || s_ScreenTable[ThisLine] != Record->text) { // Update this record on screen.
        struct ColourTag * ColourTag = NULL;
        int New = TRUE;
        while (ThisTag && ThisTag->Type != ColourType) 
          ThisTag = ThisTag->next;
          
        while (ThisTag && (ThisTag->StartPoint < strlen(Record->text))) {
          WriteString(Record->text, ThisLine-1, Normal_Text, New, ThisTag->StartPoint, LeftOffset, NULL, TabStops);
          New = FALSE;
          ColourTag = (struct ColourTag *)ThisTag->Attr;
          WriteString(Record->text, ThisLine-1, Normal_Text, New, ThisTag->EndPoint+1, LeftOffset, ColourTag, TabStops);
          while ( (ThisTag = ThisTag->next) ) 
            if (ThisTag->Type == ColourType)
              break; }
        WriteString(Record->text, ThisLine-1, Normal_Text, New, strlen(Record->text), LeftOffset, NULL, TabStops);
        LastUpdatedRecord = Record;
        Record->DisplayFlag = Redraw_None; }
         
      s_ScreenTable[ThisLine] = Record->text;
      if(Record == TextBuf->FirstRec)
        break;
      Record = Record->prev; }

    Win->OldLastLineNumber = LastLineNumber;
    TextBuf->OldLastLineNumber = LastLineNumber;
    Win->TextHeight = TextHeight;
    g_FirstWindowLine = g_LastWindowLine+1;
    
    if (Win->DisplayName) { // Display file name here.
      char * PathName = TextBuf->PathName ? TextBuf->PathName : "";
      if ((s_ScreenTable[g_FirstWindowLine] != PathName) || (LastUpdatedRecord && (LastUpdatedRecord->text == PathName))) { // The last record was updated.
        int Chr;
        int MaxNameLength = g_ScreenWidth-10;
        int Actual;
          
        if (WindowKey == '\0')
          WindowKey = TextBuf->BufferKey;
        JotGotoXY(0, g_FirstWindowLine-1);
        JotClearToEOL();
        JotSetAttr(Reverse_Video);
        if(strlen(PathName) < MaxNameLength) {
          Actual = strlen(PathName);
          JotAddString(PathName); }
        else {
          Actual = MaxNameLength;
          JotAddBoundedString(PathName, NULL, g_ScreenWidth, MaxNameLength-4, NULL, NULL);
          JotAddString(" ..."); }
        for (Chr = Actual; Chr++ < g_ScreenWidth; )
#if defined(VC)
          putchar(' ');
#else
          addch(' ');
#endif
        JotGotoXY(g_ScreenWidth-5, g_FirstWindowLine-1);
        JotAddChr(WindowKey);
        JotSetAttr(Normal_Text);
        s_ScreenTable[g_FirstWindowLine] = TextBuf->PathName; }
      
      g_FirstWindowLine = g_FirstWindowLine+1; }
    if ( ! s_InView) {
      JotGotoXY(0, g_ScreenHeight-1);
      DisplayDiag(TextBuf, 1, ""); }
    Win->LastKey = TextBuf->BufferKey;
    Win = Win->NextWindow; } // Window loop ends.
     
  g_LastWindowLine = g_ScreenHeight;
  JotGotoXY(0, g_ScreenHeight-1);
  JotSetAttr(Normal_Text);
#if defined(LINUX)
  refresh(); 
#endif
  return DisplayedCurrentBuf; }

//----------------------------------------------TextViaJournal
void TextViaJournal(struct Buf * DestBuf, char * Text, char * Name)
  { //
    //Copies the text to the nominated buffer, text records are separated by \n characters.
    //If journal file is being created then also copies text to a recovery file in the journal area.
    //In recovery, it reads the text from the recovery file, ignoring the given text.
    //
  char * EndChr;
  int RecStart = 0, RecEnd = 0;
#if defined(VC)
  static HANDLE Chan;
#else
  static int Chan;
#endif
  
  if (s_RecoveryMode) { //Reading text from backup file.
    char RecoveryPathName[StringMaxChr];
    NewScreenLine();
    if ( ! g_asConsole) {
      RunError("Recovery script must be run with -asConsole set");
      return; }
    Prompt("Enter recovery-substitution file for %s", Name);
    do
      ReadString(RecoveryPathName, StringMaxChr, g_asConsole->FileHandle);
      while ( RecoveryPathName[0] == '\0');
    Message(NULL, "Using recovery-substitution \"%s\"", RecoveryPathName);
#if defined(VC)
    if ( ! (Chan = CreateFile(RecoveryPathName, GENERIC_READ,  0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL)))
#else
    if ( (Chan = open(RecoveryPathName, O_RDONLY)) <= 0)
#endif
      {
        RunError("Error opening recovery-substitution file \"%s\"", RecoveryPathName);
        return; }
        
  while (ReadUnbufferedStreamRec(DestBuf, Chan, 0))
    { }
  JoinRecords(DestBuf); }
  
  else { //Using Text.
    if (s_JournalHand) { //Save Text to recovery file.
      char Temp[1000];
      FILE * JournalHand = NULL;
      if (Name[0] == '[' || euidaccess(Name, W_OK) == 0) {
//        struct Rec * rec = DestBuf->FirstRec;
//        struct Rec * LastRec = DestBuf->FirstRec->prev;
        char * LastPathElem = strrchr(Name, '/');
        char * ShortName;
        
        ShortName = (LastPathElem && Name[0] != '[') ? LastPathElem : Name;
        sprintf(Temp, "%s/%s_%d_%d", s_JournalPath, ShortName, getpid(), ++s_FileCount);
        if ( ! (JournalHand = fopen(Temp, "w"))) {
          Fail("Can't write to backup file %s in journal area", Temp);
          return; }
        fprintf(JournalHand, "%s", Text);
        fclose(JournalHand);
        sprintf(Temp, "<<ReadFile %s (%d_%d)  to buffer %c >>", Name, getpid(), s_FileCount, DestBuf->BufferKey);
        UpdateJournal(Temp, NULL); } }
      
    EndChr = strchr(Text+RecStart, '\n');
    RecEnd = EndChr ? EndChr-Text : strlen(Text);
    
    while ( (EndChr = strchr(Text+RecStart, '\n')) ) {
      RecEnd = EndChr - Text;
      GetRecord(g_CurrentBuf, RecEnd-RecStart+1);
      strncpy(g_CurrentBuf->CurrentRec->text, Text+RecStart, RecEnd-RecStart);
      g_CurrentBuf->CurrentRec->text[RecEnd-RecStart] = '\0';
      RecStart = RecEnd+1; }
      
    GetRecord(g_CurrentBuf, strlen(Text+RecStart)+1);
    strcpy(g_CurrentBuf->CurrentRec->text, Text+RecStart);
    JoinRecords(g_CurrentBuf); }
      
  return; }

//----------------------------------------------UpdateJournal
void UpdateJournal(char * TextArg, char * EscapeString)
  { //Adds TextArg to journal file.
  char TempText[1000];
  if (TextArg)
    strcpy(TempText, TextArg);
  else
    TempText[0] = '\0';
  if (EscapeString) {
    strcat(TempText, "\033");
    strcat(TempText, EscapeString); }
  strcat(TempText, "\n");
  if (fputs(TempText, s_JournalHand) < 0) {
    Fail("Journal write failed");
    return; }
  fflush(s_JournalHand); }

//----------------------------------------------ReadBuffer
#if defined(VC)
struct Buf * ReadBuffer(FILE * FileHand, int NonUnicode, HANDLE Chan, char * Name, char BufferKey, struct Buf * FilterBuf, int BinaryRecSize)
#else
struct Buf * ReadBuffer(FILE * FileHand, int NonUnicode, int Chan, char * Name, char BufferKey, struct Buf * FilterBuf, int BinaryRecSize)
#endif
  { //Reads stream (from either FileHand or Chan) into specified buffer if buffer already exists then buffer is scrubbed.
    //If the FilterBuf is given, then commands from that buffer are applied to each record of the stream as it is read.
    //When Chan is set it reads from Chan with ReadUnbufferedStreamRec() otherwise it reads from FileHand using ReadNewRecord()
    //With BinaryRecSize set it reads fixed-length records ignoring any '\0','\n' bytes in the stream.
  int NotEOF = TRUE;
  struct Buf * Buffer;
  struct Seq * FilterSequence = NULL;
  
  if (s_JournalHand) {
    char Temp[1000];
    if (Name[0] == '[' || euidaccess(Name, W_OK) == 0) {
      if (Name[0] == '[') { //Remove forward slashes - these bugger things up good and proper.
        int i;
        for (i=0; i < strlen(Name); i++)
          if (Name[i] == '/')
            Name[i] = '_'; }
      sprintf(Temp, "<<ReadFile %s (%d_%d)  to buffer %c >>", Name, getpid(), ++s_FileCount, BufferKey);
      UpdateJournal(Temp, NULL); }
    else {
      sprintf(Temp, "<<ReadFile %s to buffer %c >>", Name, BufferKey);
      UpdateJournal(Temp, NULL); } }
    
  if ( (('{' <= BufferKey) || (BufferKey <= ' ')) && (BufferKey != '~') ) {
    Fail("Invalid buffer key \'%c\'", BufferKey);
    return NULL; }
  Buffer = GetBuffer(BufferKey, AlwaysNew);
  if (Buffer == NULL)
    return NULL;
  g_CurrentBuf = Buffer;
    
  if (FilterBuf) {
    FilterSequence = (struct Seq *)malloc(sizeof(struct Seq));
    GetRecord(g_CurrentBuf, 0);
    GetRecord(g_CurrentBuf, 0);
    g_CurrentBuf->CurrentRec = g_CurrentBuf->CurrentRec->prev;
    JOT_Sequence(FilterBuf, FilterSequence, FALSE); }
    
  NewScreenLine();
  Prompt("Reading \"%s\"", Name);
  if (g_TTYMode)
    NewScreenLine();
             
  while (NotEOF && ! s_BombOut) { //Record-read loop.
    NotEOF = Chan ? ReadUnbufferedStreamRec(Buffer, Chan, BinaryRecSize) : ReadNewRecord(Buffer, FileHand, NonUnicode);
    if (s_BombOut) {
      Message(NULL, "Abandoning read \"%s\", (line %d \"%s\")", Name, g_EditorInput->LineNo, g_CommandBuf->CurrentRec->text);
      break; }
    if (FilterSequence) {
      Run_Sequence(FilterSequence);
      g_CurrentBuf->CurrentRec = g_CurrentBuf->FirstRec->prev->prev; } }
    
  if ( ! Buffer->FirstRec)
    GetRecord(Buffer, 0);
  Buffer->CurrentRec = Buffer->FirstRec;
  if (FilterBuf) {
    FreeRecord(Buffer, AdjustBack);
    FreeSequence(&FilterSequence); }
  Buffer->PathName = (char *)malloc(strlen(Name)+1);
  strcpy(Buffer->PathName, Name);
  Buffer->NewPathName = TRUE;
  ResetBuffer(Buffer);
  Buffer->UnchangedStatus |= SameSinceIO;
   
  if (s_JournalHand) {
    char Temp[1000];
    FILE * JournalHand = NULL;
    if (Name[0] == '[' || euidaccess(Name, W_OK) == 0) {
      struct Rec * rec = Buffer->FirstRec;
      struct Rec * LastRec = Buffer->FirstRec->prev;
      char * LastPathElem = strrchr(Name, '/');
      char * ShortName;
      
      ShortName = (LastPathElem && Name[0] != '[') ? LastPathElem : Name;
      sprintf(Temp, "%s/%s_%d_%d", s_JournalPath, ShortName, getpid(), s_FileCount);
      if ( ! (JournalHand = fopen(Temp, "w"))) {
        Fail("Can't write to backup file %s in journal area", Temp);
        return NULL; }
      for ( ; ; ) {
        fprintf(JournalHand, "%s\n", rec->text);
        rec = rec->next;
        if (rec == LastRec)
        break; }
    fclose(JournalHand); } }
    
  return Buffer; }

//----------------------------------------------ReadUnbufferedStreamRec
#if defined(VC)
int ReadUnbufferedStreamRec(struct Buf * ThisBuf, HANDLE Chan, int BinaryRecSize)
#else
int ReadUnbufferedStreamRec(struct Buf * ThisBuf, int Chan, int BinaryRecSize)
#endif
  { //Reads from Chan (typically stdin) to a new record in nominated buffer - only called when stdin is a pipe, all other reads go via ReadNewRecord().
    //returns -1 is more records follow, otherwise 0.
    //
  static int Its_Binary;
  int RecStartChr;                       //The startpoint of the record in it's first bucket.
  int RecEndChr;                         //The endpoint of the record in it's last bucket.
  int SubstringStart;                    //The startpoint of the next substring in the current bucket.
  int ChrsRead;                          //Counts characters read to bucket.
  int TotalChrs = 0;                     //Total record length.
  int RecordChr = 0;                     //Counts characters copied into destination record.
  int NotEOF = TRUE;                     //Set FALSE when last record has been detected.
  char * Terminator;                     //Position of the terminator character (e.g. '\n' for ascii).
  struct BucketBlock * ThisBucketBlock;
  
  if ( ! s_CurrentBucketBlock) { //First call - initialize.
    Its_Binary = FALSE;
    s_CurrentBucketBlock = (struct BucketBlock *)malloc(sizeof(struct BucketBlock));
    s_CurrentBucketBlock->Next = 0;
    if ((ChrsRead = ReadBucket(Chan, s_CurrentBucketBlock->Bucket)) < 0) {
      Fail("Error reading that stream");
      free(s_CurrentBucketBlock);
      s_CurrentBucketBlock = NULL;
      return 0; }
    s_CurrentBucketBlock->BufEndChr = ChrsRead;
    s_CurrentBucketBlock->NextRecStartChr = 0; 
    if (BinaryRecSize) {
      s_CurrentBucketBlock->NextRecStartChr = 0;
      Its_Binary = TRUE;
      ThisBuf->FileType = BinaryRecSize; }
    else {
      s_CurrentBucketBlock->NextRecStartChr = 0;
      ThisBuf->FileType = ASCII; } }
        
  if (s_CurrentBucketBlock->BufEndChr == 0) { //Already read an EOF from file.
    free(s_CurrentBucketBlock);
    s_CurrentBucketBlock = NULL;
    return 0; }
       
  ThisBucketBlock = s_CurrentBucketBlock;
  RecStartChr = SubstringStart = ThisBucketBlock->NextRecStartChr;
  
  while ( 1 ) { //Bucket-read loop - exit when it reads a valid terminator, an EOL or an empty bucket.
    struct BucketBlock * NextBucketBlock;
    
    if (Its_Binary) {
      if (SubstringStart+BinaryRecSize < ThisBucketBlock->BufEndChr) {
        ThisBucketBlock->NextRecStartChr = RecEndChr = SubstringStart+BinaryRecSize;
        TotalChrs += RecEndChr-SubstringStart;
        break; } }
    else if ( (Terminator = (char *)memchr(ThisBucketBlock->Bucket+SubstringStart, '\n', ThisBucketBlock->BufEndChr-SubstringStart)) ) {
       //This bucketful contains a valid EOL marker - we're now ready to copy bucket(s) to a new record.
      ThisBucketBlock->NextRecStartChr = RecEndChr = Terminator - ThisBucketBlock->Bucket + 1;
      TotalChrs += RecEndChr-SubstringStart;
      break; }
    
    NextBucketBlock = (struct BucketBlock *)malloc(sizeof(struct BucketBlock));
    TotalChrs += ThisBucketBlock->BufEndChr-SubstringStart;
    RecEndChr = ReadBucket(Chan, NextBucketBlock->Bucket);
    if (RecEndChr == 0) { //Attempting to read past EOF.
      free(NextBucketBlock);
      NotEOF = FALSE;
      if (ThisBucketBlock->NextRecStartChr <= ThisBucketBlock->BufEndChr) { //Some bytes left over at the end of the final bucketful.
        ThisBucketBlock->NextRecStartChr = ThisBucketBlock->BufEndChr+1;
        break; } }
    SubstringStart = 0;
    ThisBucketBlock->NextRecStartChr = 0;
    ThisBucketBlock->Next = NextBucketBlock;
    NextBucketBlock->Next = NULL;
    NextBucketBlock->NextRecStartChr = 0;
    NextBucketBlock->BufEndChr = RecEndChr;
    ThisBucketBlock = NextBucketBlock; }
        
  GetRecord(ThisBuf, (Its_Binary ? TotalChrs*3 : TotalChrs)+1);
  ThisBuf->CurrentRec->text[0] = '\0';
  ThisBucketBlock = s_CurrentBucketBlock;
   
  while ( ThisBucketBlock ) { //Record  loop - Intermediate blocks each contain a full bucketload, the last one has a '\n' at RecEnd.
    struct BucketBlock * NextBucketBlock = ThisBucketBlock->Next;
    
    if (Its_Binary) {
      int ChrsThisBucket = (ThisBucketBlock->NextRecStartChr ? ThisBucketBlock->NextRecStartChr-RecStartChr-1 : ThisBucketBlock->BufEndChr-RecStartChr);
      int i;
      for (i = RecordChr; i <= ChrsThisBucket; i++)
        sprintf(ThisBuf->CurrentRec->text+(i*3), "%02X ", (unsigned char)(ThisBucketBlock->Bucket+RecStartChr)[i]);
      RecordChr += ChrsThisBucket*3; }
    else {
      if (ThisBucketBlock->NextRecStartChr) { //End-of-record mark ( '\n' ) found - copy up to penultimate character.
        int ChrsThisBucket = ThisBucketBlock->NextRecStartChr-RecStartChr-1;
        memcpy(ThisBuf->CurrentRec->text+RecordChr, ThisBucketBlock->Bucket+RecStartChr, ChrsThisBucket);
        RecordChr += ChrsThisBucket; }
      else {
        int ChrsThisBucket = ThisBucketBlock->BufEndChr-RecStartChr;
        memcpy(ThisBuf->CurrentRec->text+RecordChr, ThisBucketBlock->Bucket+RecStartChr, ChrsThisBucket);
        RecordChr += ChrsThisBucket; } }
      
    if (NextBucketBlock) {
      free(ThisBucketBlock);
      ThisBucketBlock = NextBucketBlock; }
    else
      break;
    RecStartChr = 0;
    s_CurrentBucketBlock = ThisBucketBlock; }
    
#if defined(VC)
  if (ThisBuf->CurrentRec->text[RecordChr-1] =='\r')
    RecordChr--;
#endif
  ThisBuf->CurrentRec->text[RecordChr] = '\0';
  if (NotEOF)
    return TRUE;
  ThisBucketBlock = s_CurrentBucketBlock;
  while (ThisBucketBlock) {
    struct BucketBlock * NextBucketBlock = ThisBucketBlock->Next;
    free(ThisBucketBlock);
    ThisBucketBlock = NextBucketBlock; }
  s_CurrentBucketBlock = NULL;
  return FALSE; }

//----------------------------------------------ReadBucket
#if defined(VC)
int ReadBucket(HANDLE Chan, char * Bucket) {
  //Reads a bucketfull from either a windows command or copy of stdin handle, returns number of bytes read.
  LPDWORD dwRead = 0, Avail = 0;
  unsigned long long bread = 0;
   
  if (Chan) {
    if (GetFileType(Chan) == FILE_TYPE_PIPE) {
      PeekNamedPipe(Chan, NULL, 0, NULL, &Avail, NULL);
      if (Avail == 0)
        sleep(1);
      PeekNamedPipe(Chan, NULL, 0, NULL, &Avail, NULL);
      if (Avail == 0) {
        if (s_Verbose & QuiteChatty)
          Message(NULL, "ReadBucket: Empty pipe");
        return 0; }
      ReadFile(Chan, Bucket, BucketSize, &dwRead, NULL);
      return (int)dwRead; }
    else if (GetFileType(Chan) == FILE_TYPE_DISK) {
      ReadFile(Chan, Bucket, BucketSize, &dwRead, NULL);
      return (int)dwRead; } }
  else
    return read(hStdin, Bucket, BucketSize); }
#else

int ReadBucket(int Chan, char * Bucket) {
  //Reads a bucketfull from copy of stdin handle.
  return read(Chan, Bucket, BucketSize); }
#endif

//----------------------------------------------ReadNewRecord
int ReadNewRecord(struct Buf *TextBuf, FILE * FileHand, int NonUnicode)
  {
  //Reads one line from currently selected I/P channel, returns number of bytes read.
  //If EndOfFile then returns FALSE, otherwise does a GetRecord of the
  //  right size and returns TRUE with text in TextBuf->CurrentRec
  //Gracefully handles oversize records.
   
  struct TempRec{
    char Bucket[BucketSize+1];
    struct TempRec *PrevRec;
    struct TempRec *NextRec; };
  struct TempRec *ThisRec = NULL;
  struct TempRec *NextRec;
  int TotalSize = 0;
  int ChrIndex = BucketSize;
  int Chr;
  int ExitFlag = FALSE;
  int FromKeyboard = g_EditorConsole && (FileHand == g_EditorConsole->FileHandle);
  
  while ( ! ExitFlag) { // Chr read loop.
    if (BucketSize <= ChrIndex) { // Bucket full - time to extend.
      NextRec = (struct TempRec *)malloc(sizeof(struct TempRec));
      if (ThisRec)
        ThisRec->NextRec = NextRec; 
      NextRec->PrevRec = ThisRec;
      ThisRec = NextRec;
      ThisRec->NextRec = NULL;
      (ThisRec->Bucket)[0] = '\0';
      ChrIndex = 0; }
       
    if (FromKeyboard) { // Reading from TTY - do rubouts and reflect to screen.
      Chr = JotGetCh(FileHand);
      if (s_BombOut)
        break;
      switch (Chr) {;
        case Control_U: //Erase all of current input line.
          while (ThisRec->PrevRec)
            ThisRec = ThisRec->PrevRec;
          while (ThisRec) { // Record-erase loop.
            NextRec = ThisRec->NextRec;
            free(ThisRec);
            ThisRec = NextRec; }
          for (ChrIndex = 0; ChrIndex++ < TotalSize; )
            Prompt("\b \b");
          TotalSize = 0;
          ChrIndex = 0;
          ThisRec = (struct TempRec *)malloc(sizeof(struct TempRec));
          break;
         
        case RubOut: // Erase last character from bucket. 
          if (ChrIndex == 0 && ThisRec->PrevRec) { // Delete this rec and erase chr from predecessor.
            ThisRec = ThisRec->PrevRec;
            free(ThisRec->NextRec);
            ThisRec->NextRec = NULL;
            ChrIndex = BucketSize; }
          if (0 < ChrIndex) { // Still some characters left to rub out.
            ChrIndex--;
            TotalSize--;
            Prompt("\b \b"); }
          break;
      
        case EOF:
        case Control_D:
          // End of File mark. 
          ExitFlag = EOF;
          break;
      
        case '\n':
        case '\r':
          // End of record. 
          ExitFlag = TRUE;
          break;
      
        default:
          { // Any other character. 
            ThisRec->Bucket[ChrIndex] = Chr;
            ChrIndex++;
            TotalSize++;
            Prompt("%c", Chr); }
          break; } }
           
//      else { // For anything other than TTY read using fgets.
//        if ( ! fgets(ThisRec->Bucket, BucketSize+1, FileHand))
      else { // For anything other than TTY read using ReadString.
        if ( ! (NonUnicode ? fgets(ThisRec->Bucket, BucketSize+1, FileHand)!=NULL : ReadString(ThisRec->Bucket, BucketSize+1, FileHand)))
          ExitFlag = EOF;
        else { // No EOF detected - have we seen an end-of-line mark?.
          ChrIndex = strcspn(ThisRec->Bucket, "\n\r");
          if (ChrIndex < BucketSize)
            ExitFlag = TRUE;
          else
            ChrIndex = BucketSize; }
        TotalSize = TotalSize+ChrIndex; } }
    
  GetRecord(TextBuf, TotalSize+1);
  ThisRec->Bucket[ChrIndex] = '\0';
  while (ThisRec->PrevRec)
    ThisRec = ThisRec->PrevRec;
  TotalSize = 0;
  while (ThisRec) { // Final-record-copy loop.
    strcpy((TextBuf->CurrentRec->text)+TotalSize, ThisRec->Bucket);
    TotalSize = TotalSize+BucketSize;
    NextRec = ThisRec->NextRec;
    free(ThisRec);
    ThisRec = NextRec; }
  if (s_JournalHand && FromKeyboard)
    UpdateJournal(TextBuf->CurrentRec->text, NULL);
  return (ExitFlag != EOF); }

//----------------------------------------------ReadString
int ReadString(char * Buffer, int BufferLength, FILE * FileHandle)
  { //Reads an entire record from command stream.
  int Chr, i = 0;
  
  while (i < BufferLength) {
    Chr = JotGetCh(FileHandle);
    if (Chr == '\n' || Chr == '\r' || Chr == EOF)
      break;
    Buffer[i++] = Chr; }
  Buffer[i] = '\0';
  return Chr != EOF; }

//----------------------------------------------JotGetCh
#if defined(VC)
int JotGetCh(FILE * FileHandle)
  { //Replacement for curses getch() - returns next character from keyboard buffer.
    //See ../ed/wine/MSDKs/v7.1/Include/WinCon.h
  static int count = 0;
  static char OutBuf[100];
  static int InPtr = 0;
  static int OutPtr = 0;
  static int Button = 0, Release = FALSE, xp, yp, x, y;  //Identity of button pressed, flag indicating a release-event is pending then mouse x, y at button-press time.
  static int MouseBufKey, MouseLine, Mouse_x, Mouse_y;
  INPUT_RECORD EvBuf[100];
  DWORD EvsRead = 0;
  unsigned long long EventsPending = 0; 
  int i, Chr;
  wchar_t WChr;
  
  if (OutPtr < InPtr)
    return OutBuf[OutPtr++];
  else { //Collect and filter some more keyboard events
    OutPtr = InPtr = 0;
    if (Release) { //A button-release event is pending - send the escape-sequence now.
      g_MouseBufKey = MouseBufKey;
      g_MouseLine = MouseLine;
      g_Mouse_x = x;
      g_Mouse_y = y;
      InPtr = sprintf(OutBuf, "MB%dR", Button);
      OutPtr = 0;
      Button = 0;
      Release = FALSE;
      return ASCII_Escape; } }
    
  while (InPtr == 0) {
    if (s_ObeyStdin && FileHandle==g_EditorConsole->FileHandle)
      return fgetc(stdin); 
    else if ( ! g_EditorConsole || (FileHandle != g_EditorConsole->FileHandle) ) {
      WChr = fgetwc(FileHandle);
      if (WChr == WEOF)
        return EOF;
      else if ((((int)WChr) & 255) == 0xC2)  //Some sort of Microsoft UTC prefix???
        continue;
      InPtr = WideCharToMultiByte(CP_UTF8, 0, &WChr, 1, OutBuf, 100,  NULL, NULL);
      return OutBuf[OutPtr++]; }
    else
      ReadConsoleInputW(hStdin, &EvBuf, 100, &EvsRead);
        
    for (i = 0; i < EvsRead; i++) {
      if (EvBuf[i].Event.KeyEvent.wRepeatCount > 0 ) 
        count = EvBuf[i].Event.KeyEvent.wRepeatCount - 1;
        
      switch (EvBuf[i].EventType) {
        case MOUSE_EVENT:
          if (s_MouseMask != 0) {
            struct Window *Win = g_FirstWindow;
            int WinTop_y = 0;
            DWORD MouseEvent = EvBuf[i].Event.MouseEvent.dwEventFlags;
            
            if (MouseEvent == 0) { //A mouse-button event.
              InPtr = OutPtr = 0;
              if (EvBuf[i].Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
                Button = 1; //Mouse Button 1 Pressed.
              else if (EvBuf[i].Event.MouseEvent.dwButtonState == FROM_LEFT_2ND_BUTTON_PRESSED)
                Button = 2; //Mouse Button 2 Pressed.
              else if (EvBuf[i].Event.MouseEvent.dwButtonState == FROM_LEFT_3RD_BUTTON_PRESSED)
                Button = 3; //Mouse Button 3 Pressed.
              else if (Button)
                Release = TRUE; } 
                
            else if (MouseEvent == DOUBLE_CLICK) {
              if (EvBuf[i].Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
                InPtr = sprintf(OutBuf, "MB1C"); //Mouse Button 1 double-click.
              else if (EvBuf[i].Event.MouseEvent.dwButtonState == FROM_LEFT_2ND_BUTTON_PRESSED)
                InPtr = sprintf(OutBuf, "MB2C"); //Mouse Button 2 double-click.
              else if (EvBuf[i].Event.MouseEvent.dwButtonState == FROM_LEFT_3RD_BUTTON_PRESSED)
                InPtr = sprintf(OutBuf, "MB3C"); //Mouse Button 3 Presseddouble-click.
              return ASCII_Escape; }
              
            else //Mouse movement etc. - these are of no interest.
              continue;
         
            x = EvBuf[i].Event.MouseEvent.dwMousePosition.X;
            y = EvBuf[i].Event.MouseEvent.dwMousePosition.Y;
              
            if (Release) { //The release event has arrived.
              if (xp == x && yp == y) { //No change in mouse position - count it as a click.
                InPtr = sprintf(OutBuf, "MB%dc", Button);
                Button = 0;
                Release = FALSE; }
              else { //Mouse moved - it's a mouse-drag event - first send the button-press details.
                InPtr = sprintf(OutBuf, "MB%dP", Button); }
              g_MouseBufKey = MouseBufKey;
              g_MouseLine = MouseLine;
              g_Mouse_x = xp;
              g_Mouse_y = yp;
              OutPtr = 0; }
            else if (Button) { //A button-press.
              xp = x;
              yp = y; }
            
            while (Win != NULL) { // Window loop - search windows for mouse-event x, y.
              if (WinTop_y <= g_Mouse_y) {
                struct Buf * ThisBuf;
                int FirstLineNumber, LastLineNumber;
                int BufKey = (Win->WindowKey == '\0') ? g_CurrentBuf->BufferKey : Win->WindowKey;
                
                ThisBuf = GetBuffer(BufKey, NeverNew);
                LastLineNumber = ThisBuf->OldLastLineNumber;
                FirstLineNumber = LastLineNumber-Win->WindowHeight+1;
                if (WinTop_y <= g_Mouse_y && g_Mouse_y < WinTop_y + Win->WindowHeight) {
                  MouseBufKey = BufKey;
                  MouseLine = FirstLineNumber + y - WinTop_y;
                  Mouse_x = x;
                  Mouse_y = y;
                  break; } }
              WinTop_y += Win->WindowHeight + (Win->DisplayName ? 1 : 0); 
              Win = Win->NextWindow; }
              
            if ( ! Win) { //Out-of bounds event - ignore.
              InPtr = 0;
              continue; }
              
            if (InPtr)
              return ASCII_Escape;
            else
              continue; }
            
        case KEY_EVENT:
          Chr = EvBuf[i].Event.KeyEvent.wVirtualKeyCode; 
          if ( ! (EvBuf[i].Event.KeyEvent.bKeyDown) || (0x10 <= Chr && Chr <= 0x12) )
            continue;
             
          else if (Chr == '\0')
            continue;
            
          else if ( (0x21 <= Chr && Chr <= 0x2F) || (0x5F <= Chr && Chr <= 0xB8) || (EvBuf[i].Event.KeyEvent.dwControlKeyState & ENHANCED_KEY) ) {
            if (Chr == 0x57 || Chr == 0x58)  // fix F11 and F12
              Chr = Chr - 0x12;
            InPtr = sprintf(OutBuf, "\x1BX%03X",  ( Chr | 0x100 | 
              (EvBuf[i].Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)      << 5  |
              (EvBuf[i].Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED)  << 7  |
              (EvBuf[i].Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED) << 8  |
              (EvBuf[i].Event.KeyEvent.dwControlKeyState & LEFT_ALT_PRESSED)   << 10 |
              (EvBuf[i].Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED)  << 11)); }
             
          else //Normal typewriter keys returning normal ascii codes.
            InPtr = WideCharToMultiByte(CP_UTF8, 0, &EvBuf[i].Event.KeyEvent.uChar.UnicodeChar, 1, OutBuf, 100,  NULL, NULL);
                 
        default:  //Ignore all other (mouse-button, focus, resize or menu) events
          continue; } } }
          
  return OutBuf[OutPtr++]; }

#else
int JotGetCh(FILE * FileHandle)
  { //Replacement for curses getch() - returns next character from keyboard buffer.
  static char Result[100];
  static int InPtr = 0, OutPtr = 0;
  MEVENT event;
  int Chr;
  wchar_t WChr;
  
  if (OutPtr < InPtr)
    return Result[OutPtr++];
  InPtr = OutPtr = 0;
    
  while ( 1 ) {
    if (s_BombOut)
      return '\0';
    if (g_EditorConsole && (FileHandle == g_EditorConsole->FileHandle) )
      Chr = (g_TTYMode || s_ObeyStdin) ? fgetc(stdin) : getch();
    else {
      WChr = fgetwc(FileHandle);
      if (WChr == WEOF)
        return (int)EOF;
      else if ((((int)WChr) & 255) == 0xC2)  //Some sort of Microsoft UCS-BOM prefix???
        continue;
      InPtr = wctomb(Result, WChr);
      return Result[OutPtr++]; }
    if (Chr == ERR)
      continue;
    else if (Chr < KEY_MIN)
      return Chr;
    else if (Chr == KEY_BACKSPACE)
      return RubOut;
    else if (Chr == KEY_MOUSE) {
      if (getmouse(&event) == OK) {
        struct Window *Win = g_FirstWindow;
        int WinTop_y = 0;
        int x = event.x;
        
        g_Mouse_y = event.y;
        while (Win != NULL) { // Window loop.
          if (WinTop_y <= g_Mouse_y) {
            struct Buf * ThisBuf;
            int FirstLineNumber, LastLineNumber;
            int BufKey = (Win->WindowKey == '\0') ? g_CurrentBuf->BufferKey : Win->WindowKey;
            
            ThisBuf = GetBuffer(BufKey, NeverNew);
            LastLineNumber = ThisBuf->OldLastLineNumber;
            FirstLineNumber = LastLineNumber-Win->TextHeight+1;
            if (ThisBuf->Header)
              WinTop_y++;
            if (WinTop_y <= g_Mouse_y && g_Mouse_y < WinTop_y + Win->TextHeight) {
              g_Mouse_x = x;
              g_MouseBufKey = BufKey;
              g_MouseLine = FirstLineNumber + g_Mouse_y - WinTop_y;
              break; } }
          WinTop_y += Win->WindowHeight + (Win->DisplayName ? 1 : 0); 
          Win = Win->NextWindow; }
        if ( ! Win)
          continue;
        InPtr = sprintf(Result, "\eM%04X", (unsigned int)event.bstate); }
      else
        continue; }
    else if (Chr < 0x023D)
      InPtr = sprintf(Result, "\eX%04X", Chr);
    else //Assume it's unicode.
      return Chr;
    if (InPtr < 0)
      continue;
    return Result[OutPtr++]; } }
#endif

#if defined(VC)
//----------------------------------------------OpenWindowsCommand
HANDLE OpenWindowsCommand(char * Command)
  { //In windows, performs a function similar to popen() in unix.
  PROCESS_INFORMATION piProcInfo; 
  STARTUPINFO siStartInfo;
  int StreamInBuf = FALSE;
  char * FullCommand = (char *)malloc(strlen(Command)+100);
  DWORD avail = 0;
  unsigned long long bread = 0;
  static HANDLE s_hChildStd_IN_Rd = NULL;
  static HANDLE s_hChildStd_IN_Wr = NULL;
  static HANDLE s_hChildStd_OUT_Rd = NULL;
  static HANDLE s_hChildStd_OUT_Wr = NULL;
  static SECURITY_ATTRIBUTES s_saAttr; 
  
  if (Command[0] == '|') {
    Command[0] = ' ';
    StreamInBuf = TRUE; }
  sprintf(FullCommand, "cmd /c%s\0", Command); 
     
  // Set the bInheritHandle flag so pipe handles are inherited. 
  s_saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
  s_saAttr.bInheritHandle = TRUE; 
  s_saAttr.lpSecurityDescriptor = NULL; 
  
  CreatePipe(&s_hChildStd_OUT_Rd, &s_hChildStd_OUT_Wr, &s_saAttr, 0);
  SetHandleInformation(s_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);
  if (StreamInBuf) { //Create a pipe to push the current-buffers text down the child's stdin.
    CreatePipe(&s_hChildStd_IN_Rd, &s_hChildStd_IN_Wr, &s_saAttr, 0);
    SetHandleInformation(s_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0); }
  
  ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION) );
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFO) );
  siStartInfo.cb = sizeof(STARTUPINFO); 
  siStartInfo.hStdError = s_hChildStd_OUT_Wr;
  siStartInfo.hStdOutput = s_hChildStd_OUT_Wr;
  siStartInfo.hStdInput = s_hChildStd_IN_Rd;
  siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
  
  if ( ! CreateProcess(NULL, FullCommand, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo)) {
    Fail("CreateProcess failed");
    return NULL; }
  else {
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread); }
 
  if (StreamInBuf) { //Copy current-buffer text to child's stdin.
    struct Rec * ThisRec = g_CurrentBuf->FirstRec;
     
    for ( ; ; ) { 
      if ( ! WriteFile(s_hChildStd_IN_Wr, ThisRec->text, strlen(ThisRec->text), NULL, NULL))
        Fail("Failed to write to child's stdin pipe");
      ThisRec = ThisRec->next;
      if (ThisRec == g_CurrentBuf->FirstRec)
        break;
      WriteFile(s_hChildStd_IN_Wr, "\n", 1, NULL, NULL); }
    CloseHandle(s_hChildStd_IN_Wr); }
     
  while (avail == 0) {
    sleep(1);
    if (s_BombOut)
      break;
    PeekNamedPipe(s_hChildStd_OUT_Rd, NULL, 0, NULL, &avail, NULL); }
  s_BombOut = FALSE;
  return s_hChildStd_OUT_Rd; }

#else
//----------------------------------------------DoublePopen
int DoublePopen(struct Buf * TextBuf, char * Commands)
  { //Similar to popen execpt that it can read it's stdin and write to it's stdout, popen can only do one or the other.
  int pipeIn[2];
  int pipeOut[2];
  pid_t child;
  char * ArgV[100];
  char * Command = strtok(Commands, " ");
  int i = 1;
  
  ArgV[0] = Command; 
  do 
    ArgV[i] = strtok(NULL, " ");
    while (ArgV[i++]);
         
  if ((pipe(pipeIn)) == -1) {
    RunError("pipe");
    return -1; }
  if ((pipe(pipeOut)) == -1) {
    RunError("pipe");
    return -1; }
  
  child = fork();
  
  if (child == 0) { // This is child! 
    if ((dup2(pipeIn[0], STDIN_FILENO)) == -1) {
      RunError("dup2");
      return -1; }
    if ((dup2(pipeOut[1], STDOUT_FILENO)) == -1) {
        RunError("dup2");
        return -1; }
    if ((dup2(pipeOut[1], STDERR_FILENO)) == -1) {
        RunError("dup2");
        return -1; }
    if ((close(pipeIn[0])) == -1) {
        RunError("close");
        return -1; }
    if ((close(pipeOut[1])) == -1) {
        RunError("close");
        return -1; }
    if ((close(pipeIn[1])) == -1) {
      RunError("close");
      return -1; }
    if ((close(pipeOut[0])) == -1) {
      RunError("close");
      return -1; }
    if ((execvp(Command, ArgV)) == -1) {
      RunError("execvp");
      return -1; } }
           
  else if (child == -1) {
    RunError("fork");
    return -1; }
       
  else { // This is parent!
    if ((close(pipeIn[0])) == -1) {
        RunError("close");
        return -1; }
    if ((close(pipeOut[1])) == -1) {
      RunError("close");
      return -1; }
      
    if (TextBuf) {
      struct Rec * ThisRec = TextBuf->FirstRec;
       
      do { //Record loop - copy TextBuf's text to child's stdin.
        if ((write(pipeIn[1], ThisRec->text, strlen(ThisRec->text))) == -1 || (write(pipeIn[1], "\n", 1) == -1)) {
          RunError("write 1");
          return -1; }
        ThisRec = ThisRec->next; 
        } while (ThisRec != TextBuf->FirstRec); }
    
    if ((close(pipeIn[1])) == -1) {
      RunError("close");
      return -1; }
    
    return pipeOut[0]; }
  return -1; }
#endif

//----------------------------------------------BinFind
char * BinFind(char * String, int Lim, char * SubString, int SubLim) {
  //Finds substring in string and returns byte offset or -1, works with strings containing '\0' characters.
  char * i;
  int j;
  
  for (i = String; i < String+Lim-SubLim; i++) {
    for (j = 0; j < SubLim; j++)
      if (i[j] != SubString[j])
        goto Next_i;
      return i;
Next_i: continue; }
  return NULL; }

//----------------------------------------------ReadCommand
char ReadCommand(struct Buf * DestBuf, FILE * FileHandle, char *FormatString, ...)
  { // Reads command from TTY prompting as specified by Prompt arguments into
    // current record of TextBuf.
    //Text goes in starting at current character position.
    //The terminating character is not copied in.
    //Returns EOF or 1 if error detected, otherwise 0.
  va_list ap;
  char string[StringMaxChr];
  int Chr;
//  wchar_t WideChr;
  int BytePtr = 0;                                                      //Counts characters typed into DestBuf;
  int Untranslated = FALSE;                                             //A non-ascii character string was not in the translations table - it's probably unicode.
  char EscapeString[EscapeSequenceLength+1];                            //Escape string derived from curses key code.
  int ScreenMode = (g_ScreenMode & Screen_ScreenMode);
  char * CommandText = DestBuf->CurrentRec->text;
  
  if (ScreenMode && g_CurrentBuf->EditLock) {
    Message(NULL, "Buffer is readonly - exiting type-to-screen mode.");
    ScreenMode = g_ScreenMode = 0; }
  va_start(ap, FormatString);
  vsnprintf(string, StringMaxChr, FormatString, ap);
  va_end(ap); 
  s_BombOut = FALSE;
  if (s_ModifiedCommandString) { //The command string has been redefined by %s=commandstring <command> 
    BytePtr = strlen(s_ModifiedCommandString);
    strcpy(DestBuf->CurrentRec->text, s_ModifiedCommandString);
    DestBuf->CurrentChr = BytePtr;
    free(s_ModifiedCommandString);
    s_ModifiedCommandString = NULL;
    NewScreenLine();
    Prompt(string);
    Prompt("%s", DestBuf->CurrentRec->text); }
  else {
    if ( ! ScreenMode) {
      int LeftOffset = g_CurrentBuf->LeftOffset;
      if ( ! g_TTYMode)
        JotGotoXY(-LeftOffset, g_ScreenHeight-1);
        if ( ! g_TTYMode)
          NewScreenLine();
        Prompt(string); }
      if (g_TTYMode)
        fflush(g_EditorOutput);
      DestBuf->CurrentRec = DestBuf->CurrentRec->next;
      DestBuf->FirstRec = DestBuf->CurrentRec->next;
      DestBuf->CurrentChr = 0; }

  for ( ; ; ) { // Main character loop.
    int RecordLength = DestBuf->CurrentRec->length;
    CommandText = DestBuf->CurrentRec->text;
    Chr = JotGetCh(FileHandle);
    if (s_BombOut)
      return CTRL_C_INTERRUPT;
    switch ((Chr) ) {
    
    case EOF: //Force exit at end of an -asConsole command file script. 
      if (g_EditorInput->asConsole)
        return EOF;

    case RubOut:
      if (ScreenMode) {
        int LeftOffset = g_CurrentBuf->LeftOffset;
        if (0 < BytePtr) //Easy - delete characters from CommandBuf.
          JotDeleteChr(g_x = --BytePtr+(g_CurrentBuf->TabStops ? JotGetPerceivedChr(g_CurrentBuf)-1 : g_CurrentBuf->CurrentChr)-LeftOffset, g_y);
        else { //Delete character from current buffer.
          if (g_CurrentBuf->CurrentChr <= 0) { //No more characters before cursor in current record first join with predecessor record.
            AdvanceRecord(g_CurrentBuf, -1);
            JoinRecords(g_CurrentBuf); }
          else { //Delete main-buffer characters to the left of the cursor.
            int ChrPtr = --g_CurrentBuf->CurrentChr;
            g_CurrentBuf->SubstringLength = 1; 
            SubstituteString(g_CurrentBuf, NULL, -1);
            JotDeleteChr(g_x = ChrPtr - LeftOffset, g_y); }
          JotUpdateWindow(); 
          if (s_JournalHand)
            UpdateJournal(NULL, "<<DEL>>"); } 
#if defined(LINUX)
          refresh();
#endif
          }
      else if (0 < BytePtr) { // Some characters left in buffer to rub out.
        BytePtr--;
        Prompt("\b \b"); }
      break;

    default:
    case '\t':
#if defined(VC)
      if ( (32 <= Chr || Chr < 0 || Chr == '\t') && Chr < 127 ) //An ordinary printable ASCII character a TAB, or a unicode byte.
#else
      if ( 32 <= Chr || Chr < 0 || Chr == '\t') //An ordinary printable ASCII character a TAB, or a unicode byte.
#endif
        {
        if (ScreenMode)
          JotInsertChr(Chr);
        else if( ! g_TTYMode)
          JotAddChr(Chr);
#if defined(LINUX)
        refresh();
#endif
        fflush(g_EditorOutput);
        CommandText[BytePtr] = Chr;
        if (RecordLength < BytePtr++)
          return 1;
        break; }

    case ASCII_Escape: { //Also dropthrough from default case - for control keys only.
      int TransStatus;
      CommandText[BytePtr] = '\0';
      //N.B. TransEscapeSequence() will add characters directly to DestBuf->CurrentRec->text (i.e. CommandText).
      TransStatus = TransEscapeSequence(DestBuf, FileHandle, Chr, EscapeString);
      if (TransStatus == UNTRANSLATED_CHR) {
        if (Untranslated && 0 < mblen(CommandText, MB_CUR_MAX)) {
          Untranslated = FALSE;
          Prompt("%s", CommandText); }
        else {
          Untranslated = TRUE; }
        if (RecordLength < BytePtr++)
          return 1;
        continue; }
      return TransStatus; }

    case '\n':
    case '\r':
      //End of command line detected.
      if (ScreenMode) { //There are typed-into-screen characters insert them to g_CurrentBuf then exit - deal with the escape sequence next time around.
        g_CurrentBuf->SubstringLength = 0;
        SubstituteString(g_CurrentBuf, CommandText, BytePtr);
        g_CurrentBuf->CurrentChr += BytePtr;
        g_CurrentBuf->LineNumber--;
        g_CurrentBuf->SubstringLength = 0;
        BreakRecord(g_CurrentBuf);
        JotUpdateWindow();
        if (s_JournalHand) {
          CommandText[BytePtr] = '\0';
          UpdateJournal(CommandText, "<<BRK>>"); }
        BytePtr = 0; }
      else {
        CommandText[BytePtr] = '\0';
        if (s_JournalHand) {
          UpdateJournal(CommandText, NULL); }
        DestBuf->SubstringLength = 0;
        DestBuf->CurrentChr = 0;
        return Untranslated; } } } }

//----------------------------------------------TransEscapeSequence
int TransEscapeSequence(struct Buf * DestBuf, FILE * FileHandle, unsigned char Chr, char * EscapeString)
  { //Translates the escape string using '^' buffer translations.
    //Either an escape sequence follows or a control character was hit.
    //If translation exists for the escape sequence then translate,
    //otherwise copy in sequence minus escape character.
    //
  int ChrPointer = strlen(DestBuf->CurrentRec->text);
  int asConsole = g_EditorInput->asConsole;             //
  int ScreenMode = (g_ScreenMode & Screen_ScreenMode);
  char * CommandText = DestBuf->CurrentRec->text;
  struct Buf * TranslationBuf;
  int Parameter = FALSE;                                 //TRUE if translated string contains ## - response to prompt follows escape string.
  char * TransText;                                      //Translated command text from ^ buffer.
  char ThisChr;
  int DelIndex;
  int SearchIndex = 0;
     
  g_Failure = FALSE;
  if (g_ScreenMode & Screen_ToggleScreen)
    g_ScreenMode ^= (Screen_ToggleScreen | Screen_ScreenMode);
  if (ChrPointer && ScreenMode) { //There are typed-into-screen characters insert them to g_CurrentBuf then deal with the escape sequence.
    g_CurrentBuf->SubstringLength = 0;
    SubstituteString(g_CurrentBuf, CommandText, ChrPointer);
    g_CurrentBuf->CurrentChr += ChrPointer;
    g_CurrentBuf->SubstringLength = 0;
    if (s_JournalHand)
      UpdateJournal(CommandText, "<<INS>>");
    CommandText[0] = '\0'; }
  TranslationBuf = GetBuffer('^', NeverNew);
  if ( ! TranslationBuf)
    return FALSE;
     
  //Some translations to try - read characters one at a time and try matching with translations buffer.
  ResetBuffer(TranslationBuf);
  if (Chr == ASCII_Escape) { //Escape sequence has not been read by ReadCommend - get it now.
    memset(EscapeString, '\0', EscapeSequenceLength+1);
    EscapeString[0] = JotGetCh(FileHandle); }

  for ( ; ; ) { // Search translation records loop.
    SearchIndex = 0;
    
    for ( ; SearchIndex < EscapeSequenceLength; )  { // Escape sequence loop.
      ThisChr = *((TranslationBuf->CurrentRec->text)+SearchIndex);
      if (ThisChr == ' ')
        EscapeString[SearchIndex] = ' ';
      else if (EscapeString[SearchIndex] == 0)
        EscapeString[SearchIndex] = JotGetCh(FileHandle);
      if (EscapeString[SearchIndex] == ASCII_Escape)
        EscapeString[SearchIndex] = JotGetCh(FileHandle);
      if (EscapeString[SearchIndex++] != ThisChr)
        break; }
    
    if (SearchIndex == EscapeSequenceLength)
      break;
    if ( (g_Failure = (AdvanceRecord(TranslationBuf, 1))) )
      break; }
    
  if (g_Failure) { // No match.
    unsigned char Temp[2] = {Chr, '\0'};
     
    DestBuf->SubstringLength = 0;
    DestBuf->CurrentChr = ChrPointer;
    SubstituteString(DestBuf, (char *)Temp, -1);
    TranslationBuf->SubstringLength = 0;
    ChrPointer = strlen(CommandText);
    return UNTRANSLATED_CHR; }
  else { // Translations Match found.
    int Index;
    int PromptStart = 0;
    int PromptEnd = 0;
    
    DestBuf->SubstringLength = 0;
    TransText = TranslationBuf->CurrentRec->text;
    TranslationBuf->SubstringLength = strlen(TransText)-EscapeSequenceLength;
    
    for (Index = EscapeSequenceLength; Index <= strlen(TransText); Index++) { // Parameter-flag search loop.
      if (TransText[Index] == '#' && TransText[Index+1] == '#') { // Parameter tag '##' found.
        char PromptString[StringMaxChr];
        PromptString[0] = '\0';
        PromptStart = Index;
        PromptEnd = Index+2;
        if (TransText[Index+2] == '\"') { //Prompt string follows.
          char * Terminator = strchr(TransText+Index+3, '\"');
          if ( ! Terminator) {
            SynError(NULL, "No terminating \" found for prompt string");
            break; }
          PromptEnd = Terminator - TransText+1;
          strncpy(PromptString, TransText+PromptStart+3, PromptEnd-PromptStart-4);
          PromptString[PromptEnd-PromptStart-4] = '\0'; }
        if (ScreenMode) { // Prompt for parameters.
          NewScreenLine();
          JotGotoXY(0, g_ScreenHeight-1);
          Prompt(strlen(PromptString) ? PromptString : "Parameter<CR> ");
          Chr = '\0';
              
          while (Chr != '\r') { // Parameter character read loop.
            Chr = JotGetCh(FileHandle);
            if (s_BombOut)
              break;
            Prompt("%c", Chr);
            if (Chr == RubOut) {
              if (ChrPointer == 0)
                continue;
              Prompt("\b \b");
              ChrPointer--; }
            else if (Chr ==  '\r' || Chr == '\n') {
              DestBuf->CurrentChr = ChrPointer;
              CommandText[ChrPointer] = '\0';
              break; }
            else
              CommandText[ChrPointer++] = Chr; } }
             
        if (s_JournalHand) {
          EscapeString[EscapeSequenceLength] = '\0';
          UpdateJournal(CommandText, EscapeString); }
        Parameter = TRUE;
        DestBuf->CurrentChr = ChrPointer;
        SubstituteString(DestBuf, TransText+PromptEnd, -1);           //Suffix
        TranslationBuf->SubstringLength = Index-EscapeSequenceLength;
        break; } }
    
    if (s_JournalHand && ! Parameter) {
      EscapeString[EscapeSequenceLength] = '\0';
      UpdateJournal(CommandText, EscapeString); }
    DestBuf->CurrentChr = 0;
    for (DelIndex = 1; DelIndex <= ChrPointer; DelIndex++)
      Prompt("\b \b");
    DestBuf->SubstringLength = 0;
    SubstituteString(DestBuf, TransText + 8, PromptStart - 8);  //Prefix
    TranslationBuf->SubstringLength = 0;
    ChrPointer = strlen(CommandText);
    } // Translations-Match-found block ends.
  
  DestBuf->SubstringLength = 0;
  DestBuf->CurrentChr = 0;
  DestBuf->SubstringLength = 0;
  CommandText[ChrPointer] = '\0';
  if (asConsole) //Suck out any trailing whitespace and the '\n' from the recovery file.
    while ( (Chr = JotGetCh(FileHandle) == ' ') )
      { }
  return asConsole ? g_Failure : FALSE; }

//----------------------------------------------Ctrl_C_Interrupt
#if defined(VC)
BOOL Ctrl_C_Interrupt( DWORD fdwCtrlType ) 
  { //Ctrl+C handler for windows.
  if (fdwCtrlType == CTRL_C_EVENT) { // Handle the CTRL-C signal. 
    g_ScreenMode = 0;
    g_Failure = TRUE;
    if (g_TraceMode & Trace_Int) {
      JotDebug("Ctrl+C interrupt"); }
    if (s_BombOut) {
      Message(NULL, "Unanswered Ctrl+C interrupt");
      return FALSE; }
    if (s_JournalHand)
      UpdateJournal(NULL, "<<INT>>"); }
      
  RunError("Interrupt detected");
  return TRUE; }

#elif defined(LINUX)
void Ctrl_C_Interrupt(int sig)
  { // Sets flag for control+c interrupt. 
  g_ScreenMode = 0;
  g_Failure = TRUE;
  if (sig == SIGINT) {
    sigrelse(sig);
    if (s_BombOut) {
      Message(NULL, "Unanswered Ctrl+C interrupt");
      return; }
    if (s_JournalHand)
      UpdateJournal(NULL, "<<INT>>"); 
    RunError("Interrupt detected");
    if (g_TraceMode & Trace_Int) {
      JotDebug("Ctrl+C interrupt"); } }
  else
    RunError("Unknown signal detected"); 
  return; }
#endif

//----------------------------------------------ExpandEnv
void ExpandEnv(char *Full)
  { // Recursive, expands any environmental variables at start of name. 
  char Temp[StringMaxChr];
  char * EnvEnd, * Trans, * Ptr;
  
  while ( (Ptr = strstr(Full, "${")) ) { 
    if ( ! (EnvEnd = strstr(Ptr, "}"))) {
      RunError("Invalid env reference in \"%s\"", Ptr);
      return; }
    strncpy(Temp, Ptr+2, EnvEnd-Ptr-2);
    Temp[EnvEnd-Ptr-2] = '\0';
    if ( ! (Trans = getenv(Temp))) {
      RunError("Invalid env variable name \"%s\"", Temp);
      Temp[0] = '\0';
      return; }
    strcpy(Temp, EnvEnd+1);
    strcpy(Ptr, Trans);
    strcat(Ptr, Temp); } }

//----------------------------------------------ExpandFileName
void ExpandFileName(struct PathElemDesc * Path, struct PathElemDesc * Name, struct PathElemDesc * Extn, char * Full)
  { // Extracts the various fields of file spec. 
   
  ExpandEnv(Full);
   
  Path->Start = Full;
  if ( (Name->Start = strrchr(Full, PathDelimChr)) ) { 
    Path->Length = Name->Start-Full+1;
    Name->Start++; }
  else {
    Name->Start = Full;
    Path->Length = 0; }
  if ( (Extn->Start = strrchr(Name->Start, ExtnDelimChr)) ) { 
    Name->Length = Extn->Start-Name->Start; }
  else {
    Name->Length = strlen(Name->Start);
    Extn->Start = Name->Start+Name->Length; }
  Extn->Length = strlen(Extn->Start); }

//----------------------------------------------IsAFile
int IsAFile(char * PathName)
  { //Returns TRUE if the pathname points to a file in the filing system, FALSE if it doesn't exist or is a directory.
  struct stat Stat;
   
  if ( ! stat(PathName, &Stat))
    //Object exists, but is it a directory?
    return (Stat.st_mode & S_IFMT) != S_IFDIR;
  return FALSE; }

//----------------------------------------------OpenTextFile
void OpenTextFile(int * Chan, FILE ** FileHand, char AccessKey[], char *FileSpec, char *Default, char * Actual)
  { // FileSpec and Default are given, sets Actual to the pathname actually used - all simple BCPL strings.
  struct PathElemDesc Path, Name, Extn, DefPath, DefName, DefExtn;
  char ActCopy[StringMaxChr];
  char DefCopy[StringMaxChr];
#if defined(VC)
  struct OFSTRUCT * status;
  FILE * File;
#endif
  
  if ( ! FileSpec) 
    FileSpec = "";
  if ( ! Default) 
    Default = "";
  if (StringMaxChr < strlen(FileSpec)) {
    RunError("FileSpec too long.");
    return; }
  if (StringMaxChr < strlen(Default)) {
    RunError("Default filespec too long.");
    return; }
  
  strcpy(ActCopy, FileSpec ? FileSpec : "");
  strcpy(DefCopy, Default ? Default : "");
  ExpandFileName(&Path,    &Name,    &Extn,    ActCopy);
  ExpandFileName(&DefPath, &DefName, &DefExtn, DefCopy);
  Actual[0] = '\0';
   
  if (Path.Length != 0)  
    strncat(Actual, Path.Start, Path.Length);
  else
    strncat(Actual, DefPath.Start, DefPath.Length);
     
  if (Name.Length != 0)  
    strncat(Actual, Name.Start, Name.Length);
  else
    strncat(Actual, DefName.Start, DefName.Length);
     
  if (Extn.Length != 0) {
    strncat(Actual, Extn.Start, Extn.Length); }
  else
    if ( (AccessKey[0] == 'w') || ! IsAFile(Actual))
      strncat(Actual, DefExtn.Start, DefExtn.Length);
     
  if ( (AccessKey[0] == 'w') || IsAFile(Actual) ) {
#if defined(VC)
    if (Chan)
      * Chan = ((AccessKey[0] == 'r') ? CreateFile(Actual, GENERIC_READ,  0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL) :
                                        CreateFile(Actual, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,   NULL));
    else
      * FileHand = fopen(Actual, AccessKey);
#else
    if(Chan)
      *Chan = open(Actual, (AccessKey[0] == 'r') ? O_RDONLY : O_WRONLY);
    else
      *FileHand = fopen(Actual, AccessKey);
#endif
  
  return; } }

//---------------------------------------------NewCommandFile
int NewCommandFile(FILE * FileHand, char * Name)
  { //Sets up CommandFile structure for this input file,
    //does not use NewBuffer() because it has no BufferKey field.
  struct CommandFile *NewFile;
  struct Buf *NewBuf;
     
  if (FileHand == NULL)
    return FALSE;
  NewFile = (struct CommandFile *)malloc(sizeof (struct CommandFile));
  NewFile->FileHandle = FileHand;
  NewFile->ReturnFile = g_EditorInput;
  NewFile->OldCommandBuf = g_CommandBuf;
  NewFile->FileName = (Name == NULL) ? NULL : strcpy((char *)malloc(strlen(Name)+1), Name);
  NewFile->LineNo = 0;
  NewFile->asConsole = FALSE;
  NewFile->Pred = g_EditorInput;
  NewBuf = (struct Buf *)malloc(sizeof(struct Buf));
  NewBuf->BufferKey = 'c';
  NewBuf->CurrentRec = NULL;
  NewBuf->Predecessor = g_CurrentCommand;
  NewFile->CommandBuf = NewBuf;
  GetRecord(NewBuf, StringMaxChr);
  NewBuf->SubstringLength = 0;
  NewBuf->LineNumber = 1;
  g_EditorInput = NewFile;
  return TRUE; }

//---------------------------------------------EndCommandFile
int EndCommandFile()
  { //Deletes CommandFile structure for current input file.
  struct CommandFile *OldComFile;
  int ReturnValue;
  
  if ( ! g_EditorInput)
    return FALSE;
  ReturnValue = (g_EditorInput->ReturnFile) != NULL;
  fclose(g_EditorInput->FileHandle);
  g_CommandBuf = g_EditorInput->OldCommandBuf;
  OldComFile = g_EditorInput->ReturnFile;
  g_EditorInput->CommandBuf->CurrentRec = g_EditorInput->CommandBuf->FirstRec;
//  FreeBuffer(g_EditorInput->CommandBuf);
  while ( 1 ) {
    struct Rec * NextRec = g_EditorInput->CommandBuf->CurrentRec->next;
    free(g_EditorInput->CommandBuf->CurrentRec->text);
    free(g_EditorInput->CommandBuf->CurrentRec);
    g_EditorInput->CommandBuf->CurrentRec = NextRec;
    if (g_EditorInput->CommandBuf->CurrentRec == g_EditorInput->CommandBuf->FirstRec)
      break; }
  free(g_EditorInput->CommandBuf);
  if (g_EditorInput->FileName)
    free(g_EditorInput->FileName);
  free(g_EditorInput);
  g_EditorInput = OldComFile;
  return ReturnValue; }

//---------------------------------------------SwitchToComFile
int SwitchToComFile(char * NameString, char * asConsole, char * Args)
  { 
  //
  //  Parses and checks file name substituting undefined bits with default
  //  startup file name (defined by g_DefaultComFileName or g_DefaultComFileName,
  //  then searches the users default directory for the specified file
  //  if the file is not there then searches the ${JOT_HOME}/coms directory.
  //
  FILE * FileHand = NULL;
  char Actual[1000];
  int status;
  struct BacktraceFrame ThisBacktraceFrame;
  struct Buf * ArgsBuf;
  int IsDollar = g_CurrentBuf->BufferKey == '$';
  
  if (NameString)
    OpenTextFile(NULL, &FileHand, "r", NameString, g_DefaultComFileName, Actual);
  else { //No pathname given - use defaults.
    OpenTextFile(NULL, &FileHand, "r", NameString, "startup.jot", Actual);
    if ( ! FileHand)
      OpenTextFile(NULL, &FileHand, "r", NameString, g_DefaultComFileName, Actual); }
      
  if ( ! (ArgsBuf = GetBuffer('$', AlwaysNew))) {
    Fail("Can't copy args to $ buffer.");
    return FALSE; }
  if (IsDollar)
    g_CurrentBuf = ArgsBuf;
  GetRecord(ArgsBuf, strlen(Args)+1);
  strcpy(ArgsBuf->FirstRec->text, Args);
     
  if (s_JournalHand) {
    char Temp[1000];
    sprintf(Temp, "<<Running script %s>>", Actual);
    UpdateJournal(Temp, NULL); }
  if (FileHand == NULL) {
    Fail("Can't find jot command script.");
    return FALSE; }
  if (s_Verbose & NonSilent)
    Message(NULL, "Running command file \"%s\"", Actual);
  NewCommandFile(FileHand, Actual);
  g_EditorInput->asConsole = (asConsole != NULL); 
  if (asConsole) {
    if (g_asConsole) {
      Fail("Attempt to nest -asConsole scripts"); 
      return FALSE; }
    g_asConsole = g_EditorInput; }
  g_CommandBuf =  asConsole ? g_EditorConsole->CommandBuf : g_EditorInput->CommandBuf;
  if (asConsole) 
    while( ! feof(g_EditorInput->FileHandle))
      InteractiveEditor();
  else {
    ThisBacktraceFrame.prev = s_BacktraceFrame;
    ThisBacktraceFrame.ParentCommand = g_CurrentCommand;
    ThisBacktraceFrame.Type = ScriptFrame;
    ThisBacktraceFrame.EditorInput = g_EditorInput;
    s_BacktraceFrame = &ThisBacktraceFrame;
    
    for (;;) { // Command-file-record loop. 
      struct Seq *ComFileSequence = NULL;
      
      g_Failure = FALSE;
      g_EditorInput->LineNo++;
      if ( ! ReadString(g_CommandBuf->CurrentRec->text, g_CommandBuf->CurrentRec->length, g_EditorInput->FileHandle))
        break;
      if (s_BombOut) {
        if (s_Verbose & QuiteChatty)
          Message(NULL, "Exiting script \"%s\", line %d \"%s\"", Actual, g_EditorInput->LineNo, g_CommandBuf->CurrentRec->text);
        break; }
         
      g_CommandBuf->CurrentChr = 0;
      
      ComFileSequence = (struct Seq *)malloc(sizeof(struct Seq));
      JOT_Sequence(g_CommandBuf, ComFileSequence, FALSE);
      status = Run_Sequence(ComFileSequence);
      if ( ! status && ! s_BombOut) {
        if (g_EditorInput->asConsole)
          Message(NULL, "Command-file failure, line %d of \"%s\"", g_EditorInput->LineNo, Actual);
        else {
          Fail("Command-file failure, line %d of \"%s\"", g_EditorInput->LineNo, Actual);
          FreeSequence(&ComFileSequence);
          break; } }
      FreeSequence(&ComFileSequence); }
      
    s_BacktraceFrame = ThisBacktraceFrame.prev; 
    g_CurrentCommand = ThisBacktraceFrame.ParentCommand; }
    
  if (asConsole)
    g_asConsole = NULL;
  return EndCommandFile(); }

//----------------------------------------------NewScreenLine
void NewScreenLine()
  { // Performs scroll-up on section of window above cursor.
  if ( ! g_TTYMode)  { //Normal curses terminal - scroll the console region only.
    int TotalSize = 0;
    struct Window * Win = g_FirstWindow;
    while (Win) {
      int Size = Win->WindowHeight;
      TotalSize += (Size == 0) ? 1 : Size;
      Win = Win->NextWindow; }
       
    JotGotoXY(0, g_ScreenHeight-1);
    JotScroll(TotalSize+1, g_ScreenHeight-1, 1); }
  else if (s_Verbose & NonSilent) //It's writing simple text to stdout
    fputs("\n", stdout); }

//----------------------------------------------JotDeleteChr
void JotDeleteChr(int x, int y)
  { //Replacement for curses delch() - deletes the character below the cursor
#if defined(VC)
  const SMALL_RECT start = { (short)x, (short)y, (short)g_ScreenWidth-1, (short)y };
  const COORD      dest = { x-1, y };
  const CHAR_INFO  fill = { { ' ' }, Normal_Text };
   
  JotGotoXY(x, y);
  ScrollConsoleScreenBuffer(hStdout, &start, NULL, dest, &fill);
#elif defined(LINUX)
  JotGotoXY(x, y);
  delch();
#endif
  }

//----------------------------------------------JotStrlenBytes
int JotStrlenBytes(char * String, int Bytes)
  { //Returns the number of characters for string limited to Bytes bytes - String may contain unicode.
  int Chrs = 0, ByteCount = 0;
   
#if defined(VC)
  int CodePage = g_CurrentBuf->CodePage;
  Chrs = MultiByteToWideChar(CodePage, 0, String, Bytes, NULL, 0);
     
  return Chrs;
#else
  while (ByteCount < Bytes) {
    int ChrBytes = mblen(String+ByteCount, MB_CUR_MAX);
    ByteCount += (ChrBytes <= 0) ? 1 : ChrBytes;
    Chrs++; }
     
  //If the last byte is part-way through a multi-byte character the no. of characters should be reduced.   
  return Chrs+Bytes-ByteCount;
#endif
  }

//----------------------------------------------JotStrlenChrs
int JotStrlenChrs(char * String, int Chrs)
  { //Returns the number of bytes for String limited to Chrs characters - String may contain unicode.
  int TotalChrs = 0;
  int ByteCount = 0;
  int Length = strlen(String);
#if defined(VC)
  int CodePage = g_CurrentBuf->CodePage;
  
  if (Length < Chrs)
    Chrs = Length;
  while (TotalChrs < Chrs) {
    if (Length <= ++ByteCount)
      return Length;
    TotalChrs = MultiByteToWideChar(CodePage, MB_ERR_INVALID_CHARS, String, ByteCount, NULL, 0); }
#else
  int MBLen;
 
  if (Length < Chrs)
    Chrs = Length;
  while (TotalChrs++ < Chrs) {
    if (0 < (MBLen = mblen(String+ByteCount, MB_CUR_MAX)))
      ByteCount += MBLen;
    else if (String[ByteCount])
      ByteCount++; }
#endif
  return ByteCount; }

//----------------------------------------------JotInsertChr
void JotInsertChr(char Chr)
  { //Replacement for curses insch() - inserts one character in (g_x, g_y) location, pushes characters rightwards rather than overwriting rest of line.
#if defined(VC)
  SMALL_RECT start = { (short)g_x, (short)g_y, (short)g_ScreenWidth-1, (short)g_y };
  COORD dest = { g_x+1, g_y };
  CHAR_INFO fill = { { ' ' }, Normal_Text };
   
  JotGotoXY(g_x, g_y);
  ScrollConsoleScreenBuffer(hStdout, &start, NULL, dest, &fill);
  if (JotAddChr(Chr))
    g_x++;
  else {
    SMALL_RECT NewStart = { (short)g_x+1, (short)g_y, (short)g_ScreenWidth-1, (short)g_y };
    COORD NewDest = { g_x, g_y };
    ScrollConsoleScreenBuffer(hStdout, &NewStart, NULL, NewDest, &fill); }
#elif defined(LINUX)
  JotGotoXY(g_x, g_y);
  insch(' ');
  if (JotAddChr(Chr)) {
    JotGotoXY(g_x, g_y);
    g_x++; }
  else
    delch();
#endif
  }

//----------------------------------------------JotAddChr
int JotAddChr(char Chr)
  { //Replacement for curses addch() - adds a character in screen at current cursor position, overwriting any previous character.
     //Returns 1 when a complete character has been added.
  static char UTF_8_Chr[10];  
  static int NextByteOfUTF_8 = 0;
  int CurrentChrSave = g_CurrentBuf->CurrentChr;
  int Chrs = 1;
#if defined(VC)
  const CHAR_INFO  fill = { { ' ' }, Normal_Text };
  PCONSOLE_SCREEN_BUFFER_INFO Info;
#endif
  
  if (Chr < 0) {
    if (s_NoUnicode)
      Chr = '~';
    else {
      int Chrs;
      UTF_8_Chr[NextByteOfUTF_8++] = Chr;
      UTF_8_Chr[NextByteOfUTF_8] = '\0';
      JotAddBoundedString(UTF_8_Chr, NULL, NextByteOfUTF_8, 1, NULL, &Chrs);
      if (1 <= Chrs) {
        NextByteOfUTF_8 = 0;
        return 1; }
      else
        return 0; }
      return 1; }
//  else if (Chr == '\0')
//    return 0;
  else if (Chr < ' ')
    Chr = '~';
#if defined(VC)
  printf("%c", Chr);
#else
  Chrs = addch(Chr) ? 0 : 1;
  refresh();
#endif
  g_CurrentBuf->CurrentChr = CurrentChrSave;
  return Chrs; }

//----------------------------------------------JotAddBoundedString
void JotAddBoundedString(char * OrigString, struct ColourTag * ColourTag, int MaxBytes, int MaxChrs, int * ByteCount, int * ChrCount)
  { //Writes OrigString to screen, string is bounded by MaxBytes and/or MaxChrs, sets ByteCount (bytes consumed) and ChrCount (screen-columns occupied).
#if defined(NOWIDECHAR)
  int Bytes = MaxBytes;
  if (MaxChrs <= MaxBytes)
    Bytes = MaxChrs;
  addnstr(OrigString, Bytes);
  if (ByteCount)
    *ByteCount = Bytes;
  if (ChrCount)
    *ChrCount = Bytes;
  return;
#elif defined(VC)
  WCHAR WideBuf[StringMaxChr];
  int ThisByteChrLen, FirstByte = 0;
  int LocalByteCount, LocalChrCount, Chrs;
  int CodePage = g_CurrentBuf->CodePage;
  
  if ( ! ByteCount)
    ByteCount = &LocalByteCount;
  if ( ! ChrCount)
    ChrCount = &LocalChrCount;
  *ChrCount = 0;
  if (MaxBytes == 0 || MaxChrs == 0) {
    *ByteCount = 0;
    return; }
  *ByteCount = ((MaxBytes < MaxChrs) ? MaxBytes : MaxChrs)-1; //Start-point for MaxChrs search.
  
  while (*ChrCount < MaxChrs && *ByteCount < MaxBytes)
    //Plod through trying every string length up to MaxBytes, until we find one meeting the required MaxChrs character count.
    *ChrCount = MultiByteToWideChar(CodePage, MB_ERR_INVALID_CHARS, OrigString, ++(*ByteCount), NULL, 0);
    
  SetConsoleOutputCP(CodePage);
  //CodePages, arrrrgggghhhh! one of the great sorowful mysteries, I've no idea how this manages to do UTF-8 conversions but if it works it don't want fixing.
  Chrs = MultiByteToWideChar(28591, MB_ERR_INVALID_CHARS, OrigString, *ByteCount, WideBuf, StringMaxChr);
  WideBuf[Chrs] = L'\0';
  if (ColourTag)
    SetConsoleTextAttribute(hStdout, ColourTag->ColourPair);
  wprintf(L"%s", WideBuf);
#elif defined(LINUX)
  int StringLen = strlen(OrigString);
  int LocalChrCount, LocalByteCount;
  mbstate_t ShiftState;
  wchar_t WideString[MaxChrs+1];
  char * Temp = OrigString;
  char * EndChr;
  
  if ( ! ByteCount)
    ByteCount = &LocalByteCount;
  if ( ! ChrCount)
    ChrCount = &LocalChrCount;
  *ChrCount = 0;
  *ByteCount = 0;
  if (StringLen == 0 || MaxBytes <= 0 || MaxChrs <= 0)
    return;
  setlocale(LC_CTYPE, "");
    
//  memset(&ShiftState, '\0', sizeof(ShiftState));
  memset(&ShiftState, '\0', sizeof(mbstate_t));
  wmemset(&WideString[0], L'\0', MaxChrs+1);
  EndChr = OrigString+MaxBytes;
  
  while ( 1 ) {
    if (MaxChrs <= *ChrCount || !Temp || MaxBytes <= Temp-OrigString)
      break;
    int ChrsThisPass = mbsnrtowcs(WideString+*ChrCount, &Temp, EndChr-Temp, MaxChrs-*ChrCount, &ShiftState);
    if (0 <= ChrsThisPass)
      *ChrCount += ChrsThisPass;
    else {
      *ChrCount += wcslen(WideString+*ChrCount);
      setlocale(LC_CTYPE, s_Locale);
      mbsnrtowcs(WideString+*ChrCount, &Temp, 1, MaxBytes, &ShiftState);
      setlocale(LC_CTYPE, "");
      (*ChrCount)++; } }
  if (ColourTag)
    attron(COLOR_PAIR(ColourTag->ColourPair));
  else
    attron(s_DefaultColourPair);
  addwstr(WideString);
  *ByteCount = Temp-OrigString;
#endif
 }

//----------------------------------------------JotAddString
void JotAddString(char * String)
  { //Replacement for curses addstr() - writes string to window.
#if defined(NOWIDECHAR)
  addstr(String);
#else
#if defined(VC)
  printf("%s", String);
#elif defined(LINUX)
  int Size = strlen(String)+MB_CUR_MAX;
  int Len;
  wchar_t WideString[Size+1];
  memset(WideString, '\0', Size+1);
  if ((Len = mbstowcs(WideString, String, Size)) < 0) {
    Fail("Invalid multi-byte character");
    return; }
  WideString[Len] = L'\0';
  addnwstr(WideString, Len);
#endif
#endif
  }

//----------------------------------------------JotClearToEOL
void JotClearToEOL()
  { //Replacement for curses clrtoeol().
#if defined(VC)
  int x, y;
  COORD start;
  DWORD Chrs, ChrsWritten;
  CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
   
  GetConsoleScreenBufferInfo(hStdout, &csbiInfo); 
  start.X = csbiInfo.dwCursorPosition.X;
  start.Y = csbiInfo.dwCursorPosition.Y;
  Chrs = g_ScreenWidth - start.X;
  FillConsoleOutputAttribute(hStdout, Normal_Text, Chrs, start, &ChrsWritten);
  FillConsoleOutputCharacter(hStdout, ' ', Chrs, start, &ChrsWritten);
#elif defined(LINUX)
  clrtoeol();
#endif
  }

//----------------------------------------------JotSetAttr
void JotSetAttr(int DisplayAttrs)
  { //Replacement for curses attrset() - sets forground/background colour/boldness/highligting attributes.
#if defined(VC)
  SetConsoleTextAttribute(hStdout, DisplayAttrs);
#elif defined(LINUX)
  attrset(DisplayAttrs);
#endif
  }

//----------------------------------------------JotScroll
void JotScroll(int Top, int Bot, int Offset)
  { //defines the scroll region (Top-to-Bot) and scrolls up (+ve) Lines or down (-ve) by Offset lines.
  int Line; 
   
#if defined(VC)
  if (0 < Offset) {
    const SMALL_RECT start = { 0, (short)Top+Offset, (short)g_ScreenWidth, (short)Bot };
    const COORD dest = { 0, Top };
    const CHAR_INFO fill = { { ' ' }, Normal_Text };
     
    ScrollConsoleScreenBuffer(hStdout, &start, NULL, dest, &fill); }
  else {
    const SMALL_RECT start = { 0, (short)Top, (short)g_ScreenWidth, (short)Bot+Offset };
    const COORD dest = { 0, Top-Offset };
    const CHAR_INFO fill = { { ' ' }, Normal_Text };
     
    ScrollConsoleScreenBuffer(hStdout, &start, NULL, dest, &fill); }
     
#elif defined(LINUX)
  if (Offset == 0)
    return;
  setscrreg(Top, Bot);
  scrollok(mainWin, TRUE);
  //scrl(Offset);
  //scrl() messes up sometimes - e.g: release/v1.6.2/bin/lin64/jot l99.t -in="%w; %w 9; %w 0; zai/ol0(o# %m=abc'~; o~)30 ok/z.'a"
  //Console-area should display abc1 to abc29
  if (Top == Bot)
    JotClearToEOL();
  if (0 < Offset) //Scrolling up.
    for (Line = 1; (Line <= Offset) && (Line <= Bot-Top); Line++)
      scrl(1);
  else //Scrolling down.
    for (Line = 1; (Line <= -Offset) && (Line < Bot-Top); Line++)
      scrl(-1);
  scrollok(mainWin, FALSE);
#endif
 
  if (0 < Offset) //Scrolling up.
    for (Line = g_FirstWindowLine; Line <= g_LastWindowLine; Line++)
      s_ScreenTable[Line] = (Line+Offset <= g_LastWindowLine) ? s_ScreenTable[Line+Offset] : NULL;
  else //Scrolling down.
    for (Line = g_LastWindowLine; g_FirstWindowLine <= Line; Line--)
      s_ScreenTable[Line] = (Line+Offset < g_FirstWindowLine) ? NULL : s_ScreenTable[Line+Offset]; }

//----------------------------------------------JotGotoXY
void JotGotoXY(int x, int y)
  { //Replacement for curses move(), TC gotoxy() etc - moves cursor to specified coordinate.
#if defined(VC)
  const COORD pos = { (short)x, (short)y };
   
  SetConsoleCursorPosition(hStdout, pos);
#elif defined(LINUX)
  move(y, x);
#endif
  }

//----------------------------------------------SynError
void SynError(struct Buf * CommandBuf, char *FormatString, ...)
  { // Flags syntax errors.
  va_list ap;
  char String[StringMaxChr];
   
  va_start(ap, FormatString);
  vsnprintf(String, StringMaxChr, FormatString, ap);
  va_end(ap); 
  if (CommandBuf) {
    DisplayDiag(CommandBuf, TRUE, String);
    CommandBuf->Predecessor = NULL; }
  if (s_Verbose & AnnoyingBleep)
#if defined(VC)
    Beep(750, 300);
#else
    beep();
#endif
  if (g_TraceMode & Trace_Backtrace)
    Backtrace(GetBuffer('?', OptionallyNew));
  g_Failure = TRUE;
  s_BombOut = TRUE;
  return; } 

//----------------------------------------------RunError
void RunError(char *FormatString, ...)
  { // Flags serious cockups detected at run time.
  va_list ap;
  char String[StringMaxChr];
   
  va_start(ap, FormatString);
  vsnprintf(String, StringMaxChr, FormatString, ap);
  va_end(ap); 
  DisplaySource(g_CurrentCommand, String);
  if (s_Verbose & AnnoyingBleep)
#if defined(VC)
    Beep(750, 300);
#else
    beep();
#endif
  g_Failure = TRUE;
  s_BombOut = TRUE;
  if (g_TraceMode & Trace_Backtrace)
    Backtrace(GetBuffer('?', OptionallyNew));
  return; }

//----------------------------------------------Fail
void Fail(char *FormatString, ...)
  { //Gives optional diagnostic help with selected failures.
  if (s_Verbose & QuiteChatty) {
    va_list ap;
    char String[StringMaxChr];
  
    va_start(ap, FormatString);
    vsnprintf(String, StringMaxChr, FormatString, ap);
    va_end(ap); 
    if (s_Verbose & QuiteChatty)
      DisplaySource(g_CurrentCommand, String); }
  if (g_TraceMode & Trace_Backtrace)
    Backtrace(GetBuffer('?', OptionallyNew));
  g_Failure = TRUE;
  return; }

//----------------------------------------------Message
void Message(struct Buf * CommandBuf, char *FormatString, ...)
  { //Throws a new line, then uses prompt to write the string.
  va_list ap;
  char string[StringMaxChr];
   
  va_start(ap, FormatString);
//  vsprintf(string, FormatString, ap);
  vsnprintf(string, StringMaxChr, FormatString, ap);
  if (CommandBuf)
    DisplayDiag(CommandBuf, TRUE, string);
  else {
    if ( ! g_TTYMode) {
      NewScreenLine();
      JotGotoXY(0, g_ScreenHeight-1);
      JotAddString(string); 
      JotGotoXY(g_x, g_y); }
    else 
      printf("%s\n", string); } }

//----------------------------------------------Prompt
int Prompt(char *FormatString, ...)
  { //In curses mode, directs messages to console otherwise to stderr. All output suppressed in QuietMode.
  va_list ap;
  int Length;
  
  if (s_Verbose & NonSilent) {
    char string[StringMaxChr];
    va_start(ap, FormatString);
//    Length = vsprintf(string, FormatString, ap);
    Length = vsnprintf(string, StringMaxChr, FormatString, ap);
    if ( ! g_TTYMode) {
      JotAddString(string); }
    else
      fprintf(stderr, "%s", string);
#if defined(LINUX)
    refresh();
#endif
    va_end(ap);
    return Length; }
  return 0; }

//----------------------------------------------PriorityPrompt
void PriorityPrompt(char *FormatString, ...)
  { //Same as Prompt() except that it ignores s_Verbose.
  va_list ap;
//  int Length;
  char string[StringMaxChr];
  
  va_start(ap, FormatString);
//  Length = vsnprintf(string, StringMaxChr, FormatString, ap);
  vsnprintf(string, StringMaxChr, FormatString, ap);
  if ( ! g_TTYMode)  {
    JotAddString(string); }
  else
    fprintf(stderr, "%s", string);
#if defined(LINUX)
  refresh();
#endif
  va_end(ap);
  return; }

//----------------------------------------------DisplayRecord
void DisplayRecord(struct Buf *TextBuf, int ShowCursor)
  { // Used only by the 'P' command - in interactive mode the record is displayed in the command window,
    // In StreamOut mode the record is output to the stdout stream.
  if (s_StreamOut)
    printf("%s\n", TextBuf->CurrentRec->text);
  else
    DisplayDiag(TextBuf, ShowCursor, ""); }

//----------------------------------------------DisplaySource
void DisplaySource(struct Com *Command, char *Text)
  { //Extracts and checks pointer to source command string for debug/diagnostic display, constructs a dummy command buffer to pass into DisplayDiag().
  struct Buf SourceCopyBuf;
  struct Com * NextCommand;
  struct Rec * ThisRec;
  int NextCommandStart, Key;
  
  if (Command != NULL && (Command->CommandBuf->UnchangedStatus & SameSinceCompiled) != 0) { //Commannd buffer is unchanged.
    NextCommand = Command->NextCommand;
    ThisRec = Command->CommandRec;
    NextCommandStart = (NextCommand == NULL) ? strlen(ThisRec->text) : NextCommand->CommandChrNo;
    Key = Command->CommandKey;
    if (Key == '(' || Key == ')')
      NextCommandStart = (Command->CommandChrNo)+1;
    SourceCopyBuf.FirstRec = ThisRec;
    SourceCopyBuf.CurrentRec = ThisRec;
    SourceCopyBuf.SubstringLength = NextCommandStart-(Command->CommandChrNo);
    SourceCopyBuf.CurrentChr = Command->CommandChrNo; 
    if (strlen(SourceCopyBuf.CurrentRec->text) < SourceCopyBuf.SubstringLength) //Tack - but it seems to fix error with display of new command lines shorter than previous one.
      SourceCopyBuf.SubstringLength = 0;
    if (Command->CommandBuf != g_CommandBuf) { //Add location details.
      char NewText[StringMaxChr];
      sprintf(NewText, "%s (line %d of buffer %c)", Text, Command->CommandLineNo, Command->CommandBuf->BufferKey);
      DisplayDiag(&SourceCopyBuf, TRUE, NewText); }
    else
      DisplayDiag(&SourceCopyBuf, TRUE, Text); }
  else
    Message(NULL, "{%s} <original command string not available for display.>", Text); }

//----------------------------------------------DisplayDiag
void DisplayDiag(struct Buf *TextBuf, int ShowCursor, char * MessageText)
  { // Procedure gives diagnostic data on state of string.
  int RecordLength = strlen(TextBuf->CurrentRec->text);
  char * Record = (char *)malloc(RecordLength+2);  //That's one extra byte for the string terminator and one more in case a trailing '~' is required.
  int CurrentChr = TextBuf->CurrentChr;
  int SubstringLength = TextBuf->SubstringLength;
  int SubstringLeft, SubstringRight;   //The leftmost and rightmost extent of the substring.
  int FirstByte = 0;                   //The leftmost character to appear on the screen.
  int LastByte = RecordLength;         //The last character to be displayed.
  int CurChrNo;
  int MessageLength = strlen(MessageText);
  int ChrLimit =                       //The length of line available to display the record. 
    g_TTYMode ? RecordLength : g_ScreenWidth-(MessageLength ? MessageLength+3 : 0);
  int Chrs, Bytes, Result;
  
  s_NoUnicode = TextBuf->NoUnicode;
  memset(Record, '\0', RecordLength+2);
   
  for (CurChrNo = 0; CurChrNo <= RecordLength; CurChrNo++) {
    char Chr = TextBuf->CurrentRec->text[CurChrNo];
    if (Chr < ' ') {
      if (Chr < 0) {
        if (s_NoUnicode)
          Chr = '~'; }
      else
        Chr = '~'; }
    Record[CurChrNo] = Chr; }
     
  Record[RecordLength] = '\0';
   
  CurChrNo = CurrentChr;
  if (SubstringLength == 0)
    SubstringLeft = SubstringRight = CurrentChr;
  else if (0 <= SubstringLength) {
    SubstringLeft = CurrentChr;
    SubstringRight = SubstringLeft+SubstringLength-1; }
  else {
    SubstringLeft = CurrentChr+SubstringLength;
    SubstringRight = SubstringLeft-SubstringLength; }
#if defined(VC)
    ChrLimit--;
#endif
  if (ChrLimit < RecordLength) { //This record will overflow the available line width.
    if (CurrentChr < ChrLimit)
      LastByte = ChrLimit;
    else {
      FirstByte = (SubstringLength < ChrLimit) ? CurrentChr-(ChrLimit/2) : (SubstringLength < 0) ? CurrentChr+ChrLimit-3 : CurrentChr+3;
      if (FirstByte < 0)
        FirstByte = 0;
      if (RecordLength < FirstByte+ChrLimit)
        FirstByte = RecordLength-ChrLimit+1;
      LastByte = FirstByte+ChrLimit; } }
  NewScreenLine(); 
  if (0 < MessageLength) {
    PriorityPrompt("{");
    PriorityPrompt(MessageText);
    PriorityPrompt("}");
    MessageLength += 2; }
    
  if (ShowCursor) { // Display cursor.
    if ( ! g_TTYMode) { // Display substring and cursor using screen attributes.
      JotSetAttr(Normal_Text);
      JotAddBoundedString(Record+FirstByte, NULL, SubstringLeft-FirstByte, ChrLimit, NULL, &Chrs);
      Bytes = SubstringLeft;
      if (SubstringLeft == CurrentChr) { //Substring begins at current chr or no substring
        JotSetAttr(Current_Chr);
        if (RecordLength-1 < Bytes) {
          JotAddBoundedString("~", NULL, 1, 1, &Result, NULL);
          Bytes += Result-1; }
        else {
          JotAddBoundedString(Record+SubstringLeft, NULL, RecordLength-SubstringLeft, 1, &Result, NULL);
          Bytes += Result; }
        Chrs++;
        JotSetAttr(Selected_Substring);
        if (Bytes < SubstringRight) {
          JotAddBoundedString(Record+Bytes, NULL, SubstringRight-Bytes+1, ChrLimit-Chrs, NULL, &Result);
          Chrs += Result;
          Bytes = SubstringRight+1; } }
      else { // Current character follows end of substring
        JotSetAttr(Selected_Substring);
        JotAddBoundedString(Record+SubstringLeft, NULL, SubstringRight-SubstringLeft, ChrLimit-Chrs, NULL, &Result);
        Chrs += Result;
        Bytes += SubstringRight-SubstringLeft;
        JotSetAttr(Current_Chr);
        JotAddBoundedString(Record+Bytes, NULL, RecordLength-Bytes, 1, &Result, NULL);
        Bytes += Result;
        Chrs++;
        if (RecordLength <= Bytes) {
          JotAddBoundedString("~", NULL, 1, 1,  NULL, &Result);
          Chrs += Result; } }
      JotSetAttr(Normal_Text);
      JotAddBoundedString(Record+Bytes, NULL, RecordLength-Bytes, ChrLimit-Chrs, NULL, NULL);
      JotGotoXY(g_x, g_y); }
    else { //Don't use screen attributes to display.
      int Index;
      fprintf(stdout, "%s\n", TextBuf->CurrentRec->text);
      if (s_Verbose & NonSilent) {
        for (Index = 1; Index <= (SubstringLeft+MessageLength); Index++)
          fputc(' ', stderr);
        if (SubstringLeft == CurrentChr) {
          fputc('^', stderr);
          SubstringLeft += 1; }
        for (Index = SubstringLeft; Index < SubstringRight; Index++)
          fputc('~', stderr);
        if (Index == CurrentChr)
          fputc('^', stderr);
        fputc('\n', stderr); } } }
  else { // Print without showing cursor and substrings.
    int Index;
    if ( ! g_TTYMode) { 
      for (Index = 0 ; Index < LastByte; )
        JotAddChr(Record[Index++]);
      JotGotoXY(g_x, g_y); }
    else { // Write line to stdout/terminal.
      int Index;
      for (Index = 0 ; Index < LastByte; )
        fputc(Record[Index++], stdout); } }
  free(Record); }

//----------------------------------------------JotDebug
void JotDebug(char *Text)
  { //Breakpoint prompt and command handler.
  int TraceModeSave = g_TraceMode;
  struct Com * CurrentCommandSave = g_CurrentCommand;
  struct BacktraceFrame ThisBacktraceFrame;
  char asConsole;
  
  if ( ! g_EditorInput)
    return;
  asConsole = g_EditorInput->asConsole;
  g_EditorInput->asConsole = FALSE;
  s_DebugLevel++;
  JotUpdateWindow();
   
  for ( ; ; ) {
    int Exit = FALSE;
    struct Seq * DebugComSequence = NULL;
    DisplaySource(g_CurrentCommand, Text);
    s_DebugBuf->CurrentRec->text[0] = '\0';
    g_Failure = FALSE;
    s_BombOut = FALSE;
    JotUpdateWindow();
    NewScreenLine();
    g_Failure = FALSE;
    s_BombOut = FALSE;
    Prompt("Debug Command> ");
    s_DebugBuf->CurrentRec->text[0] = '\0';
//    Exit != ReadCommand(s_DebugBuf, g_EditorConsole->FileHandle, "(Debug %d) %d %c> ", s_DebugLevel, g_CurrentBuf->LineNumber , g_CurrentBuf->BufferKey);
    Exit = !ReadCommand(s_DebugBuf, g_EditorConsole->FileHandle, "(Debug %d) %d %c> ", s_DebugLevel, g_CurrentBuf->LineNumber , g_CurrentBuf->BufferKey);
    if (s_BombOut) {
      g_TraceMode &= !Trace_AllCommands;
      break; }
    if (Exit)
      break;
    if (strlen(s_DebugBuf->CurrentRec->text) == 0)
      break;
    DebugComSequence = (struct Seq *)malloc(sizeof(struct Seq));
    memset(DebugComSequence, 0, sizeof(struct Seq));
    JOT_Sequence(s_DebugBuf, DebugComSequence, FALSE);
    g_TraceMode = Trace_Break; 
    ThisBacktraceFrame.prev = s_BacktraceFrame;
    ThisBacktraceFrame.ParentCommand = g_CurrentCommand;
    ThisBacktraceFrame.Type = MacroFrame;
    if ( ! Run_Sequence(DebugComSequence))
      Message(NULL, "That command failed");
    if (g_TraceMode == Trace_Break)
      g_TraceMode = TraceModeSave;
    else if (g_TraceMode == 0)
      break;
    g_CurrentCommand = CurrentCommandSave;
    FreeSequence(&DebugComSequence); }
     
  s_DebugLevel--;
  g_CurrentCommand = CurrentCommandSave;
  s_BacktraceFrame = ThisBacktraceFrame.prev;
  g_EditorInput->asConsole = asConsole; }

//----------------------------------------------Disaster
void Disaster(char *Text, ...)
  { // Flags showstoppers detected at startup time.
  va_list ap;
  char string[StringMaxChr];
  
  va_start(ap, Text);
//  vsprintf(string, Text, ap);
  vsnprintf(string, StringMaxChr, Text, ap);
  va_end(ap);
  JotTidyUp();
  fprintf(stderr, "%s\n", string);
  exit(1); } 

//----------------------------------------------JotTidyUp
int JotTidyUp()
  { // Exit procedure called by TidyUp(). 
  int i = 0;
  struct Buf *ThisBuf = g_BufferChain;
  char BuffersToBeWritten[100] = "";
  
  if (s_HoldScreen) {
    int Chr;
    NewScreenLine();
    Prompt("Hit C to continue, E to exit script or any other key to exit now ");
    Chr = JotGetCh(NULL);
    if (Chr == 'C')
      return 1;
    else if (Chr == 'E')
      RunError("Exiting script"); }
     
  while (ThisBuf != NULL) {
    if ( (ThisBuf->EditLock & WriteIfChanged) && ! (ThisBuf->UnchangedStatus & SameSinceIO) )
      BuffersToBeWritten[i++] = ThisBuf->BufferKey;
    ThisBuf = ThisBuf->NextBuf; }
  BuffersToBeWritten[i] = '\0';
  if (0 < i) {
    RunError("The following buffers are WriteIfChanged and need writing: \"%s\"", BuffersToBeWritten);
    return 1; }
  
  while (EndCommandFile())
    continue;
      
#if defined(VC)
  if ( ! g_TTYMode) { //Restore console.
    JotGotoXY(0, g_ScreenHeight-1);
    SetConsoleTextAttribute(hStdout, wOldColorAttrs); }
#elif defined(LINUX)
  if (mainWin) //Close down curses.
    endwin();
#endif
    
#if defined(VC)
  if (s_JournalHand) { //Close and delete the journal files.
    HANDLE DirHand = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FileData;
    DWORD dwRet;
    struct stat Stat;
    char Path[StringMaxChr];
    
    fclose(s_JournalHand);
    strcpy(Path, s_JournalPath);
    strcat(Path, "*");
    DirHand = FindFirstFile(Path, &FileData);
    if (DirHand == INVALID_HANDLE_VALUE)
      return 1;
    do {
      strcpy(Path, s_JournalPath);
      strcat(Path, FileData.cFileName);
      stat(Path, &Stat);
      if ((Stat.st_mode & S_IFMT) == S_IFDIR)
        continue;
      if(unlink(Path))
        printf("Failed to delete journal/backup file %s\n", Path); }
    while (FindNextFile(DirHand, &FileData) != 0);
    rmdir(s_JournalPath); } 
#else
  if (s_JournalHand) { //Close and delete the journal files.
    DIR * PathElemDIR = opendir(s_JournalPath);
    struct dirent * Entry;
    
    if (PathElemDIR) { //PathElemDIR is NULL if some recoverry session has already deleted the journal files.
      fclose(s_JournalHand);
      while ( (Entry = readdir(PathElemDIR)) ) {
        char temp[1000];
        struct stat Stat;
        strcpy(temp, s_JournalPath);
        strcat(temp, Entry->d_name);
        stat(temp, &Stat);
        if ((Stat.st_mode & S_IFMT) == S_IFDIR)
          continue;
        if(unlink(temp))
          printf("Failed to delete journal/backup file %s\n", temp); }
      if(rmdir(s_JournalPath))
        printf("Failed to delete journal directory %s\n", s_JournalPath); } }
  mousemask(0, NULL);
#endif
  return 0; }

#ifndef ANDROID
////-----------------------------------------------HashInsert
//int HashInsert(char * Key, struct Buf * Buffer, struct HtabEntry * Entry, int Verify)
//  { //Adds item to the specified (or current) buffer's hashtable, if Verify is true then it fails in the event of a collision.
//  char * KeyCopy = (char *)malloc(strlen(Key)+1);
//  ENTRY e, *retval;
//  
//  strcpy(KeyCopy, Key); 
//  e.key = KeyCopy;
//  e.data = (void *)Entry;
//  if (hsearch_r(e, ENTER, &retval, Buffer->htab)) {
//    ENTRY *AnotherRetval;
//    if ( ! Verify || (Entry == retval->data) )
//      return 0;
//    else //Collision detected - restore original and fail.
//      hsearch_r(*retval,  ENTER, &AnotherRetval, Buffer->htab); }
//  return 1; }

//----------------------------------------------DoHash
int DoHash(struct Buf * Query, char NomBufKey)
  { //Maintenance of hashtable entries in buffer nominated by NomBuf, defaults to g_CurrentBuf.
  char * Text = Query->CurrentRec->text;
  struct Buf * NomBuf;
  
  if (strstr(Text, "create") == Text) { //create - initialize the table with specified no. of elements.
    int NoElem, status;
     
    sscanf(Text+7, "%d", &NoElem);
    if (NoElem <= 0) {
      SynError(NULL, "No hash-table size specified");
      return 1; }
    if (NomBufKey != '=') {
      if ( (NomBuf = GetBuffer(NomBufKey, OptionallyNew)) == NULL)
        return 1; }
    else
      NomBuf = g_CurrentBuf;
    if ( ! NomBuf)
      NomBuf = g_CurrentBuf;
    if ( ! NomBuf->FirstRec)
      GetRecord(NomBuf, 1);
    DestroyHtab(NomBuf);
    NomBuf->HashtableMode = 
      strstr(Text, "-destroy") ? DestroyHashtables : 
      (strstr(Text, "-protect") ? ProtectEntries : 
      (strstr(Text, "-adjust") ? AdjustEntries : 
      (strstr(Text, "-delete") ? DeleteEntries :
      AdjustEntries)));
    NomBuf->htab = (struct hsearch_data *)malloc(sizeof(struct hsearch_data));
    NomBuf->htab->table = NULL; 
    NomBuf->htab->size = 0; 
    NomBuf->htab->filled = 0; 
    status = hcreate_r(NoElem, NomBuf->htab);
    NomBuf->FirstEntry = NULL;
    return ( ! status); }
  
  else if (NomBufKey != '=') {
    if ( (NomBuf = GetBuffer(NomBufKey, NeverNew)) == NULL)
      return 1; }
  else
    NomBuf = g_CurrentBuf;

  if ( (strstr(Text, "add") == Text) || (strstr(Text, "new") == Text) ) { //add or new - puts item into table.
    ENTRY * OldVal, NewVal;
    struct HtabEntry * Entry;
    char * KeyCopy;
    struct Tag * ThisTag;
    int NewEntry = TRUE;
    
    if ( ! NomBuf)
      NomBuf = g_CurrentBuf;
    if ( ! NomBuf->htab) {
      Fail("No hashtable associated with this buffer");
      return 1; }
    if ( ! NomBuf->htab) {
      Fail("No hash-table associated with buffer %c", NomBuf->BufferKey);
      return 1; }
      
    NewVal.key = Text+4;
    NewVal.data = NULL;
    if (hsearch_r(NewVal, FIND, &OldVal, NomBuf->htab)) {
      NewEntry = FALSE;
      if (strstr(Text, "new")) { //Collision detected while adding 'new' entry - restore original and fail.
        if (OldVal) {
          Entry = (struct HtabEntry *)OldVal->data;
          if (Entry->TargetRec) //Collision detected while adding 'new' entry - restore original and fail.
            return 1; } } }
      
    if (NewEntry) {
      Entry = (struct HtabEntry *)malloc(sizeof(struct HtabEntry));
      KeyCopy = (char *)malloc(strlen(Text+4)+1);
      strcpy(KeyCopy, Text+4); 
      NewVal.key = KeyCopy;
      NewVal.data = (void *)Entry;
      Entry->HashKey = KeyCopy;
      Entry->next = NomBuf->FirstEntry;
      NomBuf->FirstEntry = Entry; }
    else {
      NewVal = *OldVal;
      Entry = NewVal.data;
      FreeTarget(NomBuf, Entry);
      Entry->TargetRec = NULL; }
      
    Entry->HashBufKey = NomBuf->BufferKey;
    Entry->BufferKey = g_CurrentBuf->BufferKey;
    Entry->TargetRec = g_CurrentBuf->CurrentRec;
    Entry->CurrentChr = g_CurrentBuf->CurrentChr;
    Entry->LineNumber = g_CurrentBuf->LineNumber;
    NomBuf->UnchangedStatus |= SameSinceIndexed;
    
    if (NewEntry)
      hsearch_r(NewVal, ENTER, &OldVal, NomBuf->htab);
        
    //Create TargetPoint tag for target record.
    ThisTag = (struct Tag *)malloc(sizeof(struct Tag));
    ThisTag->next = NULL;
    ThisTag->StartPoint = g_CurrentBuf->CurrentChr;
    ThisTag->Type = TargetType;
    ThisTag->Attr = (void *)Entry;
    AddTag(g_CurrentBuf->CurrentRec, ThisTag);
    
    return 0; }

  else if (strstr(Text, "find") == Text) { //find - looks up item in table.
    struct Buf * TargetBuf;
    if ( ! NomBuf)
      NomBuf = g_CurrentBuf;
    TargetBuf = QueryKey(Text+5, NomBuf);
    if (TargetBuf)
      g_CurrentBuf = TargetBuf;
    return TargetBuf == NULL; }

  else if (strstr(Text, "call") == Text) { //call - compiles and runs specified routine.
    struct Buf * CodeRepositoryBuf;
    struct Seq * DeferredSeq;
    struct BacktraceFrame ThisBacktraceFrame;
    
    if ( ! NomBuf)
      NomBuf = g_CurrentBuf;
    if ( ! (CodeRepositoryBuf = QueryKey(Text+5, NomBuf))) {
      Fail("Function %s not found in code repository \" %c \"", Text+5, NomBuf->BufferKey);
      return 1; }
    //Named routine exists - compile it and run.
   
    if (CodeRepositoryBuf == NULL) {
      Fail("Undefined buffer");
      return 0; }
    CodeRepositoryBuf->CurrentRec = CodeRepositoryBuf->CurrentRec->next;
    CodeRepositoryBuf->SubstringLength = 0;
    CodeRepositoryBuf->CurrentChr = 0;
     
    DeferredSeq = (struct Seq *)malloc(sizeof(struct Seq));
    JOT_Sequence(CodeRepositoryBuf, DeferredSeq, TRUE);
    if (s_BombOut)
      Message(CodeRepositoryBuf, "Exiting macro %c, line no %d", CodeRepositoryBuf->BufferKey, CodeRepositoryBuf->LineNumber);
    if (s_CommandChr == ')') {
      SynError(CodeRepositoryBuf, "Surplus \')\'");
      free(DeferredSeq);
      return 1; }
    ThisBacktraceFrame.prev = s_BacktraceFrame;
    ThisBacktraceFrame.ParentCommand = g_CurrentCommand;
    ThisBacktraceFrame.Type = CallFrame;
    s_BacktraceFrame = &ThisBacktraceFrame;
    g_Failure = ! Run_Sequence(DeferredSeq);
    s_BacktraceFrame = ThisBacktraceFrame.prev;
    FreeSequence(&DeferredSeq);
    g_CurrentCommand = NULL;
    return g_Failure; }

  else if (strstr(Text, "fix") == Text) { //fix - corrects line numbers in hashtable entries.
    struct Rec * Record;
    int LineNumber = 1;
    
    if ( ! NomBuf)
      NomBuf = g_CurrentBuf;
      
    ResetBuffer(NomBuf);
    Record = NomBuf->FirstRec;
    do {
      struct Tag * ThisTag = Record->TagChain;
      while (ThisTag) {
        if (ThisTag->Type == TargetType) //This record is a hash-table target.
          ((struct HtabEntry * )ThisTag->Attr)->LineNumber = LineNumber;
        ThisTag = ThisTag->next; }
      Record = Record->next;
      LineNumber++; }
      while (Record != NomBuf->FirstRec);
    
    return 0; }

  else if (strstr(Text, "delete") == Text) { //delete - removes item from the table.
    char Key[1000];
    ENTRY * Ptr;
    ENTRY Entry;
  
    sscanf(Text+6, "%s", (char *)&Key);
    if ( ! NomBuf)
      NomBuf = g_CurrentBuf;
   
    if ( ! NomBuf->htab) {
      Fail("No hash-table associated with buffer %c", NomBuf->BufferKey);
      return FALSE; }
    Entry.key = Key;
    Entry.data = NULL;
  
    hsearch_r(Entry, FIND, &Ptr, NomBuf->htab);
    if (Ptr) {
      struct HtabEntry * Entry = (struct HtabEntry *)Ptr->data;
      Entry->TargetRec = NULL;
      return 0; }
    else 
      return 1; }

  else if (strstr(Text, "testkey") == Text) { //testkey - verify that there is at least one key matching the given string,
    char String[1000];
    int NoString = (sscanf(Text+7, " %s", (char *)&String) < 1);
    if ( ! NomBuf)
      NomBuf = g_CurrentBuf;
    if (NomBuf->htab) {
      struct HtabEntry * ThisEntry = NomBuf->FirstEntry;
      
      if (NoString)
        return 0;
    
      while (ThisEntry) {
        if (strstr(ThisEntry->HashKey, String))
          return 0;
        ThisEntry = ThisEntry->next; }
      return 1; }
    else
      return 1; }

  else if (strstr(Text, "destroy") == Text) { //destroy - frees the hashtable and unprotects records.
    if (strstr(Text, " -all")) {
      struct Buf * ThisBuf = g_BufferChain;
      while (ThisBuf) {
        DestroyHtab(ThisBuf);
        ThisBuf->HashtableMode = NoHashtable;
        ThisBuf->htab = NULL;
        ThisBuf = ThisBuf->NextBuf; } }
    else {
      if ( ! NomBuf)
        NomBuf = g_CurrentBuf;
        
      DestroyHtab(NomBuf);
      NomBuf->htab = NULL; }
    return 0; }
  
  else {
    RunError("Unrecognized %H qualifier %s", Text); 
    return 1; }
    
  return 1; }

//----------------------------------------------DestroyHtab
void DestroyHtab(struct Buf * TextBuf)
  { //Destroys the specified h-list data object.
  struct HtabEntry * ThisEntry;
  
  if (TextBuf->htab) {
    hdestroy_r(TextBuf->htab);
    free(TextBuf->htab);
    ThisEntry = TextBuf->FirstEntry;
    
    while(ThisEntry) {
      struct HtabEntry * NextEntry = ThisEntry->next;
      FreeTarget(TextBuf, ThisEntry);
      free(ThisEntry->HashKey);
      free(ThisEntry);
      ThisEntry = NextEntry; }
      
    TextBuf->htab = NULL;
    TextBuf->HashtableMode = NoHashtable;
    TextBuf->FirstEntry = NULL; } }

//----------------------------------------------FreeTarget
void FreeTarget(struct Buf * TextBuf, struct HtabEntry * ThisEntry)
  { //Removes links to Hash-table target tag and free's it.
  struct Rec * Record = ThisEntry->TargetRec;
  
  if (Record) {
    struct Tag ** PrevTagPtr = &(Record->TagChain);
    struct Tag * ThisTag = *PrevTagPtr;
    
    while (ThisTag) {
      if ( (ThisTag->Type == TargetType) && (ThisTag->Attr == ThisEntry) ) {
        *PrevTagPtr = ThisTag->next;
        free(ThisTag);
        return; }
      PrevTagPtr = &(ThisTag->next);
      ThisTag = *PrevTagPtr; } } }

//----------------------------------------------FreeTag
void FreeTag(struct Rec * Record, struct Tag * TheTag)
  { //Removes any tag from Record's tag chain.
  struct Tag ** PrevTagPtr = &(Record->TagChain);
  struct Tag * ThisTag = *PrevTagPtr;
  
  while (ThisTag) {
    if (ThisTag == TheTag) {
      *PrevTagPtr = ThisTag->next;
      if (TheTag->Type == TextType)
        free(TheTag->Attr);
      free(TheTag);
      return; }
    PrevTagPtr = &(ThisTag->next);
    ThisTag = *PrevTagPtr; } }

//----------------------------------------------QueryKey
struct Buf *  QueryKey(char * Key, struct Buf * Buffer)
  { //Queries the hashtable in the specified buffer using the specified key. Returns darget buffer or NULL if failure.
  ENTRY * Ptr;
  ENTRY Entry;
  struct Buf * TargetBuf = NULL;
   
  if ( ! Buffer->htab) {
    Fail("No hash-table associated with buffer %c", Buffer->BufferKey);
    return TargetBuf; }
  Entry.key = Key;
  Entry.data = NULL;
  
  hsearch_r(Entry, FIND, &Ptr, Buffer->htab);
  if (Ptr) {
    struct HtabEntry * Entry = (struct HtabEntry *)Ptr->data;
    if ( ! Entry->TargetRec) //A deleted entry.
      return TargetBuf;
    if ((TargetBuf = GetBuffer(Entry->BufferKey, NeverNew))) {
      TargetBuf->CurrentRec = Entry->TargetRec;
      TargetBuf->LineNumber = Entry->LineNumber;
      TargetBuf->CurrentChr = Entry->CurrentChr;
      TargetBuf->SubstringLength = 0; } }
    return TargetBuf; }

#else
int QueryKey(char * Key, struct Buf * Buffer)
  { }
  
int DoHash(struct Buf * Query, BufferKey)
  { }
  
int mblen(const char * String, size_t Max)
  { //Stop-gap for android.
  return strlen(String); }
  
int wctomb(char *String, wchar_t WString)
  { //Stop-gap for android.
  String[0] = (char)String;
  String[1] = '\0';
  return strlen(String); }
#endif

