#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <stdlib.h> 
#include <io.h> // Console
#include <fcntl.h> // Console
#include <iostream> // Console
#include <stdlib.h> 
#include <shlobj.h> //file dialog and getknownfolderpath
#include <wrl\client.h> // COM Smart pointer
#include "MidiFile.h"
#include "Offsets.h"

#pragma comment(lib,"comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <CommCtrl.h>
#include "resource.h"

using Microsoft::WRL::ComPtr;
using namespace smf;

#define OW_180_CONSTANT 27273.0f //moves 180 degrees while using sens 1


class Melody {
public:
	double tick;
	double duration;
	int    pitch;
};


void ClearMaestro();
void OpenFileDialog();
void LaunchConsole();
void ToggleSensitivity();
void PlayMidi();
inline std::wstring GetEditboxString(HWND hEdit);
void SetAngle(float pitch, float yaw, bool bDoClick);
void SetPreviewAngle(ANGLES QueuedLocalPreviewRotation, HWND hwnd);
INT_PTR CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
std::string NarrowString(const std::wstring &s);
void ConvertToMelody(MidiFile& midifile, std::vector<Melody>& melody);
void SortMelody(std::vector<Melody>& melody);
int  CompareNotes(const void* a, const void* b);

HINSTANCE hInst;
const std::wstring AppStatus[] = { L"Idle", L"Playing" };
const std::wstring AppName = L"Maestro";
const std::wstring AppTitle = L"%s \u266A [%s] \u266A %s";
const COMDLG_FILTERSPEC FileTypes[] = { { L"MIDI Sequence (*.mid)", L"*.mid" } };
ANGLES CurrentRotation = { 0.f, 0.f, FALSE};
HWND hMaestro = NULL;
HWND hOverwatch = NULL;
ANGLES QueuedPreviewRotation;
HHOOK MouseHook;
HWND hPreviouslyFocussedControl = NULL;
POINT CursorPosBeforePreview = { 0, 0 };
bool bIsPreviewing = FALSE;
bool bIsPlayingPiano = FALSE;
bool bIsPianoThreadRunning = FALSE;
bool bIsSensitivitySet = FALSE;
std::vector<bool> CurrentTracks;
float GameSensitivy = 30.f;
float CurrentSpeed = 1.f;
MidiFile MidiSequence;
std::vector<Melody> CurrentMelody;
std::string Filepath;
std::wstring Filename;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;
	UNREFERENCED_PARAMETER(nCmdShow);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(hPrevInstance);
	return static_cast<int>(DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DlgProc));
}

// On Windows 10, this won't get passed down the chain, and be essentially useless
LRESULT CALLBACK MaskMouseInjection(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		MSLLHOOKSTRUCT* MouseHookPointer = (MSLLHOOKSTRUCT *)lParam;
		if ((MouseHookPointer->flags & LLMHF_LOWER_IL_INJECTED) || (MouseHookPointer->flags & LLMHF_INJECTED))
		{
			MouseHookPointer->flags &= ~LLMHF_LOWER_IL_INJECTED;
			MouseHookPointer->flags &= ~LLMHF_INJECTED;
		}
	}
	return CallNextHookEx(MouseHook, nCode, wParam, lParam);
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_INITDIALOG:
		{
			INITCOMMONCONTROLSEX iccex;
			iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
			iccex.dwICC = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
			LaunchConsole();

			/*
			if (!(MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MaskMouseInjection, GetModuleHandle(0), NULL)))
			{
				MessageBox(NULL, L"Couldn't install hook. Quitting.", L"SCHEISSE", MB_ICONERROR);
				EndDialog(hwnd, 0);
				break;
			}
			*/

			RegisterHotKey(hwnd, 1, MOD_NOREPEAT, VK_INSERT);
			RegisterHotKey(hwnd, 2, MOD_NOREPEAT, VK_DELETE);
			HICON hIcon = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED | LR_DEFAULTSIZE));
			SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
			SetWindowText(hwnd, reinterpret_cast<LPCWSTR>(AppName.c_str()));

			HWND hOffsetYaw = GetDlgItem(hwnd, IDC_EDIT2);
			HWND hOffsetPitch = GetDlgItem(hwnd, IDC_EDIT3);
			HWND hSpeedMultiplier = GetDlgItem(hwnd, IDC_SPEEDOFFSET);
			HWND hSens = GetDlgItem(hwnd, IDC_SENS);
			SendMessage(hOffsetYaw, EM_SETLIMITTEXT, 12, 0);
			SendMessage(hOffsetYaw, WM_SETTEXT, 0, (LPARAM)L"0.0");
			SendMessage(hOffsetPitch, EM_SETLIMITTEXT, 12, 0);
			SendMessage(hOffsetPitch, WM_SETTEXT, 0, (LPARAM)L"0.0");
			SendMessage(hSpeedMultiplier, EM_SETLIMITTEXT, 12, 0);
			SendMessage(hSpeedMultiplier, WM_SETTEXT, 0, (LPARAM)L"1.0");
			SendMessage(hSens, EM_SETLIMITTEXT, 12, 0);
			SendMessage(hSens, WM_SETTEXT, 0, (LPARAM)L"30.0");
			hMaestro = hwnd;
			hOverwatch = ::FindWindow(NULL, L"Overwatch");
			if (!hOverwatch)
				MessageBox(hwnd, L"Overwatch isn't running!", L"SCHEISSE", MB_OK | MB_ICONERROR);

			break;
		}
		case WM_HOTKEY:
		{
			if (HIWORD(lParam) == VK_INSERT)
			{
				if (!bIsPlayingPiano && !bIsPianoThreadRunning)
				{
					bIsPlayingPiano = true;
					bIsPianoThreadRunning = true;
					std::thread(&PlayMidi).detach();
					std::wstring TitleStr(128, '\0');
					swprintf(&TitleStr[0], 128, AppTitle.c_str(), AppName.c_str(), Filename.c_str(), AppStatus[bIsPianoThreadRunning].c_str());
					SetWindowText(hMaestro, reinterpret_cast<LPCWSTR>(TitleStr.c_str()));
				}
				else
				{
					bIsPlayingPiano = false;
					std::wcout << L"Stopping MIDI \"" << Filename << L"\"" << std::endl;
				}
				break;
			}
			else if (HIWORD(lParam) == VK_DELETE)
			{
				std::thread(&ToggleSensitivity).detach();
				break;
			}
			break;
		}
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED)
			{
				switch (LOWORD(wParam))
				{
				case IDC_PREVIEW:
				{
					if (bIsPreviewing)
						break;
					QueuedPreviewRotation = { static_cast<float>(::strtod((NarrowString(GetEditboxString(GetDlgItem(hwnd, IDC_EDIT2)))).c_str(), NULL)), static_cast<float>(::strtod(NarrowString(GetEditboxString(GetDlgItem(hwnd, IDC_EDIT3))).c_str(), NULL)), true };
					if (hOverwatch)
						::SetForegroundWindow(hOverwatch);
					POINT cp;
					GetCursorPos(&cp);
					CursorPosBeforePreview = cp;
					bIsPreviewing = TRUE;
					break;
				}
				case IDC_OPENMIDI:
					OpenFileDialog();
					break;
				case IDC_SETSPEED:
					CurrentSpeed = static_cast<float>(::strtod((NarrowString(GetEditboxString(GetDlgItem(hwnd, IDC_SPEEDOFFSET)))).c_str(), NULL));
					break;
				case IDC_SETSENS:
					GameSensitivy = static_cast<float>(::strtod((NarrowString(GetEditboxString(GetDlgItem(hwnd, IDC_SENS)))).c_str(), NULL));
					break;
				}
				break;
			}
			else if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				int *SelectedTracks = new int[CurrentTracks.size()]();
				int NumSelectedTracks = static_cast<int>(SendMessage(reinterpret_cast<HWND>(lParam), LB_GETSELITEMS, CurrentTracks.size(), reinterpret_cast<LPARAM>(&SelectedTracks[0])));

				for (auto Selected : CurrentTracks)
					Selected = false;

				for (int i = 0; i < NumSelectedTracks; i++)
					CurrentTracks[SelectedTracks[i]] = true;

				delete[] SelectedTracks;
				CurrentMelody.clear();
				ConvertToMelody(MidiSequence, CurrentMelody);
				SortMelody(CurrentMelody);
				
				break;
			}
			break;
		}
		case WM_ACTIVATE:
		{
			hPreviouslyFocussedControl = GetFocus();

			if (LOWORD(wParam) == WA_INACTIVE && QueuedPreviewRotation.bDirty)
			{
				std::wcout << L"Deactivate" << std::endl;
				std::thread(&SetPreviewAngle, QueuedPreviewRotation, hwnd).detach();
				QueuedPreviewRotation.bDirty = false;
			}
			break;
		}
		case WM_CLOSE:
			UnhookWindowsHookEx(MouseHook);
			UnregisterHotKey(hwnd, 1);
			UnregisterHotKey(hwnd, 2);
			EndDialog(hwnd, 0);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

void ResetPreviewMouse()
{
	Sleep(100);
	SetCursorPos(CursorPosBeforePreview.x, CursorPosBeforePreview.y);
	bIsPreviewing = FALSE;
}

void SetPreviewAngle(ANGLES QueuedLocalPreviewRotation, HWND hwnd)
{
	Sleep(250);
	SetAngle(QueuedLocalPreviewRotation.pitch, QueuedLocalPreviewRotation.yaw, false);
	::SetForegroundWindow(hwnd);
	SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hPreviouslyFocussedControl, TRUE);
	std::thread(&ResetPreviewMouse).detach();
}

void SendKeyInput(int ScanCode)
{
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.time = 0;
	ip.ki.wVk = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.dwFlags = KEYEVENTF_SCANCODE;
	ip.ki.wScan = static_cast<WORD>(ScanCode);
	SendInput(1, &ip, sizeof(INPUT));
	ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));
}

void ToggleSensitivity()
{
	SendKeyInput(0x01);
	Sleep(400);
	SetCursorPos(321, 58);
	SendKeyInput(0x50);
	SendKeyInput(0x50);
	SendKeyInput(0x39);
	Sleep(300);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(400);
	SetCursorPos(1172, 269);
	Sleep(200);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(100);
	if (!bIsSensitivitySet)
		SendKeyInput(0x02);
	else
	{
		SendKeyInput(0x04);
		SendKeyInput(0x0B);
	}
	Sleep(30);
	SendKeyInput(0x1C);
	SendKeyInput(0x01);
	Sleep(100);
	SendKeyInput(0x01);
	bIsSensitivitySet = !bIsSensitivitySet;
}

void PlayMidi()
{
	std::wcout << L"Playing MIDI \"" << Filename << L"\"" << std::endl;
	for (unsigned int i = 0; i < CurrentMelody.size(); i++)
	{
		if (!bIsPlayingPiano)
			break;

		if (CurrentMelody[i].pitch >= 24 && CurrentMelody[i].pitch <= 88)
		{
			ANGLES Angle = PianoKeyAngles[CurrentMelody[i].pitch - 24];
			SetAngle(Angle.pitch, Angle.yaw, true);
		}
		else
			std::wcout << L"Ignoring invalid note with pitch " << CurrentMelody[i].pitch << std::endl;

		double SleepTime = 150;
		if (i + 1 != CurrentMelody.size())
			SleepTime = MidiSequence.getTimeInSeconds(static_cast<int>(CurrentMelody[i + 1].tick)) - MidiSequence.getTimeInSeconds(static_cast<int>(CurrentMelody[i].tick));

		DWORD AdjustedSleepTime = static_cast<DWORD>((SleepTime * 1000 / CurrentSpeed));
		if (AdjustedSleepTime > 20000)
			std::wcout << L"MIDI queued long sleep of " << AdjustedSleepTime << L"ms. Skipping event." << std::endl;
		else
			Sleep(AdjustedSleepTime);
	}
	bIsPlayingPiano = false;
	SetAngle(0.f, 0.f, false);
	CurrentRotation = { 0.f, 0.f, false };
	bIsPianoThreadRunning = false;
	std::wstring TitleStr(128, '\0');
	swprintf(&TitleStr[0], 128, AppTitle.c_str(), AppName.c_str(), Filename.c_str(), AppStatus[bIsPianoThreadRunning].c_str());
	SetWindowText(hMaestro, reinterpret_cast<LPCWSTR>(TitleStr.c_str()));
}

void TestThread1()
{
	for (auto& Angle : PianoKeyAngles)
	{
		if (!bIsPlayingPiano)
			break;
		SetAngle(Angle.pitch, Angle.yaw, true);
		Sleep(100);
	}
	bIsPlayingPiano = false;
	SetAngle(0.f, 0.f, false);
	CurrentRotation = { 0.f, 0.f, false };
}

void SetAngleShootDelay(DWORD Delay)
{
	Sleep(Delay);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void SetAngle(float pitch, float yaw, bool bDoClick)
{
	yaw = yaw * ((OW_180_CONSTANT / 180.0f) / GameSensitivy);
	pitch = pitch * ((OW_180_CONSTANT / 180.0f) / GameSensitivy);
	mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(-CurrentRotation.yaw), 0, 0, 0);
	mouse_event(MOUSEEVENTF_MOVE, 0, static_cast<DWORD>(-CurrentRotation.pitch), 0, 0);
	mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(yaw), 0, 0, 0);
	mouse_event(MOUSEEVENTF_MOVE, 0, static_cast<DWORD>(pitch), 0, 0);
	CurrentRotation = { pitch, yaw, false };

	if (bDoClick)
		std::thread(&SetAngleShootDelay, 5).detach();
}

int CompareNotes(const void* a, const void* b)
{
	const Melody& aa = *static_cast<const Melody*>(a);
	const Melody& bb = *static_cast<const Melody*>(b);

	if (aa.tick < bb.tick)
		return -1;
	else if (aa.tick > bb.tick)
		return 1;
	else
	{
		// highest note comes first
		if (aa.pitch > bb.pitch)
			return 1;
		else if (aa.pitch < bb.pitch)
			return -1;
		else
			return 0;
	}
}

void SortMelody(std::vector<Melody>& melody)
{
	qsort(melody.data(), melody.size(), sizeof(Melody), CompareNotes);
}

void ConvertToMelody(MidiFile& midifile, std::vector<Melody>& melody)
{
	midifile.absoluteTicks();

	int numEvents = midifile.getNumEvents(0);

	std::vector<int> state(128);   // for keeping track of the note states

	int i;
	for (i = 0; i < 128; i++)
		state[i] = -1;

	melody.reserve(numEvents);
	melody.clear();

	Melody mtemp;
	int command;
	int pitch;
	int velocity;

	for (i = 0; i < numEvents; i++)
	{
		if (!CurrentTracks[midifile[0][i].track])
			continue;

		command = midifile[0][i][0] & 0xf0;
		if (command == 0x90)
		{
			pitch = midifile[0][i][1];
			velocity = midifile[0][i][2];
			if (velocity == 0)
				goto noteoff;
			else
				state[pitch] = midifile[0][i].tick; // note on
		}
		else if (command == 0x80)
		{
			// note off
			pitch = midifile[0][i][1];
		noteoff:
			if (state[pitch] == -1)
				continue;

			mtemp.tick = state[pitch];
			mtemp.duration = midifile[0][i].tick - state[pitch];
			mtemp.pitch = pitch;
			melody.push_back(mtemp);
			state[pitch] = -1;
		}
	}
}

inline std::wstring GetEditboxString(HWND hEdit)
{
	uint32_t size = GetWindowTextLength(hEdit);
	std::wstring str(size, '\0');
	GetWindowText(hEdit, (LPWSTR)str.data(), size + 1);
	return str;
}

std::string NarrowString(const std::wstring &s)
{
	std::string wsTmp(s.begin(), s.end());
	return wsTmp;
}

void LaunchConsole()
{
	// Alloc debug console
	AllocConsole();
	std::wstring cTitle = L"Piano Debug Console";
	SetConsoleTitle(cTitle.c_str());

	// Get STDOUT handle
	HANDLE ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	int SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);
	FILE *COutputHandle = _fdopen(SystemOutput, "w");

	// Get STDERR handle
	HANDLE ConsoleError = GetStdHandle(STD_ERROR_HANDLE);
	int SystemError = _open_osfhandle(intptr_t(ConsoleError), _O_TEXT);
	FILE *CErrorHandle = _fdopen(SystemError, "w");

	// Get STDIN handle
	HANDLE ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
	int SystemInput = _open_osfhandle(intptr_t(ConsoleInput), _O_TEXT);
	FILE *CInputHandle = _fdopen(SystemInput, "r");

	//make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
	std::ios::sync_with_stdio(true);

	// Redirect the CRT standard input, output, and error handles to the console
	freopen_s(&CInputHandle, "CONIN$", "r", stdin);
	freopen_s(&COutputHandle, "CONOUT$", "w", stdout);
	freopen_s(&CErrorHandle, "CONOUT$", "w", stderr);

	// Move console over to other screen and maximize, if possible
	HWND hConsole = GetConsoleWindow();
	SetWindowPos(hConsole, NULL, 3000, 0, 200, 200, SWP_NOSIZE);
	ShowWindow(hConsole, SW_MAXIMIZE);

	std::wcout << cTitle << L" successfully allocated." << std::endl;
}

void OpenFileDialog()
{
	std::wstring fpath;
	std::wstring fname;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		ComPtr<IFileOpenDialog> pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(pFileOpen.ReleaseAndGetAddressOf()));

		// Try to set default folder to default savefile path
		if (SUCCEEDED(hr))
		{
			PWSTR pszFilePath;
			if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Downloads, NULL, NULL, &pszFilePath)))
			{
				ComPtr<IShellItem> pDefaultFolder;
				std::wstring defaultpath = pszFilePath;
				if (SUCCEEDED(SHCreateItemFromParsingName((PCWSTR)defaultpath.c_str(), NULL, IID_PPV_ARGS(pDefaultFolder.ReleaseAndGetAddressOf()))))
					pFileOpen->SetDefaultFolder(pDefaultFolder.Get());
				CoTaskMemFree(pszFilePath);
			}
			pFileOpen->SetFileTypes(ARRAYSIZE(FileTypes), FileTypes);
		}

		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				ComPtr<IShellItem> pItem;
				hr = pFileOpen->GetResult(pItem.ReleaseAndGetAddressOf());
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					PWSTR pszFileName;
					pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					pItem->GetDisplayName(SIGDN_NORMALDISPLAY, &pszFileName);
					fpath = pszFilePath;
					fname = pszFileName;
					CoTaskMemFree(pszFilePath);
					CoTaskMemFree(pszFileName);
				}
			}
		}
		if (!fpath.empty() && !fname.empty())
		{
			Filepath = NarrowString(fpath);
			Filename = fname;

			ClearMaestro();
			MidiSequence.read(Filepath);
			for (int i = 0; i < MidiSequence.size(); i++)
			{
				std::wstring str = L"Track " + std::to_wstring(i) + L" (" + std::to_wstring(MidiSequence.getNumEvents(i)) + L" Events)";
				SendMessage(GetDlgItem(hMaestro, IDC_TRACKLIST), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(&str[0]));
				CurrentTracks.push_back(false);
			}
			MidiSequence.joinTracks();

			std::wstring TitleStr(128, '\0');
			swprintf(&TitleStr[0], 128, AppTitle.c_str(), AppName.c_str(), Filename.c_str(), AppStatus[bIsPlayingPiano].c_str());
			SetWindowText(hMaestro, reinterpret_cast<LPCWSTR>(TitleStr.c_str()));
		}
	}
	CoUninitialize();
}

void ClearMaestro()
{
	CurrentTracks.clear();
	CurrentTracks.shrink_to_fit();
	MidiSequence.clear();
	CurrentRotation = { 0.f, 0.f, false };
	CurrentMelody.clear();
	SendMessage(GetDlgItem(hMaestro, IDC_TRACKLIST), LB_RESETCONTENT, 0, 0);
}