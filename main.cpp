#include <string>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
//#include <iostream>
#include <string.h>
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>

#pragma comment(lib, "comctl32.lib")

std::string GetListViewItemText(HWND hwndListView, int iItem, int iSubItem, HANDLE hProcess) {
  LVITEMA lvItem;
  char buffer[4096];
  ZeroMemory(&lvItem, sizeof(LVITEMA));
  lvItem.iSubItem = iSubItem;
  lvItem.cchTextMax = sizeof(buffer);
  lvItem.pszText = buffer;
  lvItem.mask = LVIF_TEXT;

  // Allocate memory in the target process
  LPVOID lvItemRemote = VirtualAllocEx(hProcess, nullptr, sizeof(LVITEMA), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  LPVOID bufferRemote = VirtualAllocEx(hProcess, nullptr, sizeof(buffer), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

  lvItem.pszText = (LPSTR)bufferRemote;

  // Write the LVITEM structure to the target process memory
  WriteProcessMemory(hProcess, lvItemRemote, &lvItem, sizeof(LVITEM), nullptr);

  // Send the message to the list view
  SendMessageA(hwndListView, LVM_GETITEMTEXT, (WPARAM)iItem, (LPARAM)lvItemRemote);
  
  // Read the data back from the target process memory
  ReadProcessMemory(hProcess, bufferRemote, buffer, sizeof(buffer), nullptr);

  // Free allocated memory in the target process
  VirtualFreeEx(hProcess, lvItemRemote, 0, MEM_RELEASE);
  VirtualFreeEx(hProcess, bufferRemote, 0, MEM_RELEASE);

  CharLowerA(buffer);

  return std::string(buffer);
}


void CheckListViewItem(HWND hwndListView, int index, BOOL check, HANDLE hProcess) {
  UINT state = check ? INDEXTOSTATEIMAGEMASK(2) : INDEXTOSTATEIMAGEMASK(1);
  
  // Set item state
  LVITEMA lvItem = { 0 };
  lvItem.stateMask = LVIS_STATEIMAGEMASK;
  lvItem.state = state;

  LPVOID lvItemRemote = VirtualAllocEx(hProcess, nullptr, sizeof(LVITEMA), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

  WriteProcessMemory(hProcess, lvItemRemote, &lvItem, sizeof(LVITEM), nullptr);

  SendMessageA(hwndListView, LVM_SETITEMSTATE, index, (LPARAM)lvItemRemote);

  VirtualFreeEx(hProcess, lvItemRemote, 0, MEM_RELEASE);
}

void SetCheckboxState(HWND hwndCheckbox, bool check) {
  SendMessageA(hwndCheckbox, BM_SETCHECK, check ? BST_CHECKED : BST_UNCHECKED, 0);
}

void SetText(HWND hwndEdit, LPCSTR text) {
  SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)text);
}

void ClickDefault(HWND hwndWindow) {
  SendMessageA(hwndWindow, WM_COMMAND, 1, 0);
}

void ClickButton(HWND hwndWindow, HWND hwndButton, INT idButton) {
  //SendMessage(hwndWindow, WM_COMMAND, MAKEWPARAM(idButton, BN_CLICKED), (LPARAM)hwndButton);
  SendMessageA(hwndWindow, WM_COMMAND, 1, 0);
  //SendMessage(hwndButton, WM_COMMAND, 1, NULL);
}

/*
void ClickButton(HWND hwndButton) {
  SendMessage(hwndButton, BM_CLICK, 0, 0);
  SendMessage(hwndButton, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
  SendMessage(hwndButton, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
}*/

bool IsButtonEnabled(HWND hwndButton) {
  return IsWindowEnabled(hwndButton) != FALSE;
}

#define ISARG(arg, test, next) (strncmp(arg, test, sizeof(test) - 1) == 0) && (next = arg + sizeof(test) - 1)

std::vector<std::vector<std::string>> GetAllListViewItems(HWND hwndListView, HANDLE hProcess) {
  std::vector<std::vector<std::string>> items;
  int itemCount = SendMessageA(hwndListView, LVM_GETITEMCOUNT, 0, 0);

  if (itemCount == -1) {
      return items;
  }

  LVCOLUMNA lvColumn = { 0 };
  int columnCount = 0;
  char buffer[4096];

  lvColumn.mask = LVCF_TEXT;
  lvColumn.pszText = buffer;
  lvColumn.cchTextMax = sizeof(buffer);

  LPVOID lvColumnRemote = VirtualAllocEx(hProcess, nullptr, sizeof(LVCOLUMNA), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  LPVOID bufferRemote = VirtualAllocEx(hProcess, nullptr, sizeof(buffer), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

  lvColumn.pszText = (LPSTR)bufferRemote;
  WriteProcessMemory(hProcess, lvColumnRemote, &lvColumn, sizeof(LVCOLUMNA), nullptr);
  // Determine number of columns
  
  while (SendMessageA(hwndListView, LVM_GETCOLUMN, columnCount, (LPARAM)lvColumnRemote)) {
      columnCount++;
  }

  VirtualFreeEx(hProcess, lvColumnRemote, 0, MEM_RELEASE);
  VirtualFreeEx(hProcess, bufferRemote, 0, MEM_RELEASE);

  for (int i = 0; i < itemCount; ++i) {
      std::vector<std::string> row;
      for (int j = 0; j < columnCount; ++j) {
          std::string text = GetListViewItemText(hwndListView, i, j, hProcess);
          row.push_back(text);
      }
      items.push_back(row);
  }

  //CloseHandle(hProcess);
  return items;
}

void AcceptEula() {
  const char* registryPath = "SOFTWARE\\Sysinternals\\Disk2Vhd";
    const char* valueName = "EulaAccepted";
    DWORD valueData = 1; // The DWORD value to be written

    HKEY hKey;
    LONG openRes = RegCreateKeyExA(
        HKEY_CURRENT_USER,  // hKey
        registryPath,       // lpSubKey
        0,                  // Reserved
        NULL,               // lpClass
        REG_OPTION_NON_VOLATILE, // dwOptions
        KEY_WRITE,          // samDesired
        NULL,               // lpSecurityAttributes
        &hKey,              // phkResult
        NULL                // lpdwDisposition
    );

    if (openRes == ERROR_SUCCESS) {
        // Set the value
        LONG setRes = RegSetValueExA(
            hKey,          // hKey
            valueName,     // lpValueName
            0,             // Reserved
            REG_DWORD,     // dwType
            (const BYTE*)&valueData, // lpData
            sizeof(valueData)        // cbData
        );

        if (setRes == ERROR_SUCCESS) {
            //std::cout << "Registry value set successfully." << std::endl;
        } else {
        }

        // Close the registry key
        RegCloseKey(hKey);
    } else {
        //error
    }
}

bool MatchSimpleRegEx(LPCSTR Text, LPCSTR RegEx)
{
	if (RegEx == nullptr)
	{
		return true;
	}
	if (Text == nullptr)
	{
		return (strcmp(RegEx, "*") == 0);
	}
	if (RegEx[0] == 0 && Text[0] != 0)
	{
		return false;
	}
	
	std::string CRegEx = RegEx;
	std::vector<std::string> Strings;
	intptr_t LastPos = 0;
	intptr_t TextLen = strlen(Text);
	//int RegExLen = CRegEx.GetLength();
	
	while(true)
	{
		LPCSTR Pos = strchr(RegEx + LastPos, '*');
		intptr_t iPos = Pos - RegEx;
		if (Pos == NULL)
		{
			Strings.push_back(RegEx + LastPos);
			break;
		}
		
		Strings.push_back(CRegEx.substr((int)LastPos, (int)(iPos - LastPos)));
		LastPos = iPos + 1;
	}
	
	LastPos = 0;
	intptr_t Max = Strings.size();
	for (intptr_t i = 0; i < Max; i++)
	{
		intptr_t SLen = Strings[i].size();
		if (SLen != 0)
		{
			LPCSTR Pos = strstr(Text + LastPos, Strings[i].c_str());
			intptr_t iPos = Pos - Text;
			if (Pos == NULL)
			{
				return false;
			}
			if (i == 0 && iPos != 0)
			{
				return false;
			}
			else if (i == Max - 1 && iPos != TextLen - SLen)
			{
				return false;
			}
			
			LastPos = iPos + SLen;
		}
	}
	return true;
}

PROCESS_INFORMATION m_prcInfo {};
HWND mainHwnd = NULL;
int main(int argc, char* argv[]) {
  STARTUPINFOA si = {};
	si.cb = sizeof(si);

  if (argc == 1) {
    printf(
      "Usage disk2vhdex.exe arg1=value arg2=value ...\n"
      "\n"
      "Arguments:\n"
      "\toutput=FILENAME - output file\n"
      "\tvhdx=(1/0) - use VhdX format\n"
      "\tshadowcopy=(1/0) - enable shadow volume copy\n"
      "\tvirtualpc=(1/0) - virtual pc mode\n"
      "\tvolume=VOLUME_NAME include volume with this name (can use * wildcard, case ignored). This option can be declared many times. volume=c:\\ volume=d:\\ ...\n"
      "\tlabel=VOLUME_LABEL - include volume with this labels (can use * wildcard, case ignored). This option can be declared many times. label=MY_DRIVE1 label=MY_DRIVE2 ...\n"
      "\rvolumelabel=VOLUME_NAME=VOLUME_LABEL- include volume with this name and label (can use * wildcard, case ignored). This option can be declared many times. volumelabel=*volume*|[no label]\n"
      "\tshowlist - show drives list (just 'showlist' without '=' and values)\n"
      "\trun - run vhd creating (just 'run' without '=' and values)\n"
      "\n"
      "Example:\n"
      "\tdisk2vhdex.exe output=D:\\my_backup.vhdx vhdx=1 shadowcopy=1 virtualpc=0 volume=c:\\ volume=e:\\ label=*DATA run\n"
      "This option will create 'D:\\my_backup.vhdx' and try to include volumes 'c:\\' 'e:\\' and volume(s) with label that ends with 'data' \n"
    );
    return 0;
  }

  AcceptEula();
	BOOL bRet = CreateProcessA(NULL, "disk2vhd64.exe", NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &m_prcInfo);
  DWORD dwRet = WaitForInputIdle(m_prcInfo.hProcess, 60000);
  if (WAIT_OBJECT_0 == dwRet)
  {
    EnumWindows( [](HWND hwnd, LPARAM lParam) -> int {
      DWORD dwProcessId;
      GetWindowThreadProcessId(hwnd, &dwProcessId);
      if (dwProcessId == m_prcInfo.dwProcessId)
      {
        mainHwnd = hwnd;
        return 0;
      }
      return 1;
    }, (LPARAM)0);

    if (mainHwnd != NULL)
    {
      printf("Disk2vhd window found\n");
      HWND editHwnd = GetDlgItem(mainHwnd, 1001);//Edit
      HWND vPCHwnd = GetDlgItem(mainHwnd, 1012);//VirtualPc
      HWND vscHwnd = GetDlgItem(mainHwnd, 1013);//VolumeShadowCopy
      HWND vhdxHwnd = GetDlgItem(mainHwnd, 1014);//VHDX
      HWND createHwnd = GetDlgItem(mainHwnd, 1001);//CreateButton
      HWND listHwnd = GetDlgItem(mainHwnd, 1006);//List View

      std::vector<std::vector<std::string>> items = GetAllListViewItems(listHwnd, m_prcInfo.hProcess);

      bool enableVhdx = true;
      bool enableVirtualPc = false;
      bool enableVolumeShadowCopy = true;
      char* next = nullptr;
      bool run = false;
      std::vector<std::string> volumeslabels;
      std::vector<std::string> volumes;
      std::vector<std::string> labels;
      for (int i = 1; i < argc; i++) {
        char *arg = CharLowerA(argv[i]);
        //printf("arg: %s %i\n", arg, sizeof("output"));
        if (ISARG(arg, "output=", next)) {
          printf("OUTPUT!\n");
          SetText(editHwnd, next);
        } else if (ISARG(arg, "vhdx=", next)) {
          enableVhdx = (atoi(next) != 0);
        } else if (ISARG(arg, "shadowcopy=", next)) {
          enableVolumeShadowCopy = (atoi(next) != 0);
        } else if (ISARG(arg, "virtualpc=", next)) {
          enableVirtualPc = (atoi(next) != 0);
        } else if (ISARG(arg, "volume=", next)) {
          volumes.push_back(next);
        } else if (ISARG(arg, "label=", next)) {
          labels.push_back(next);
        } else if (ISARG(arg, "volumelabel=", next)) {
          volumeslabels.push_back(next);
        } else if (strcmp(arg, "showlist") == 0) {
          for (int i = 0; i < items.size(); i++)
          {
            printf("Volume: %s, Label: %s\n", items[i][0].c_str(), items[i][1].c_str());
          }
        } else if (strcmp(arg, "run") == 0) {
          run = true;
        }
      }

      printf("Total volumes: %zu\n", items.size());
      intptr_t FoundVolumes = 0;
      for (int i = 0; i < items.size(); i++)
      {
        bool value = false;
        for (auto s : volumes) {
          //printf("Match: %s = %s\n", items[i][0].c_str(), s.c_str());
          if (MatchSimpleRegEx(items[i][0].c_str(), s.c_str())) {
            printf("\tFound volume \"%s\" matched \"%s\"\n", items[i][0].c_str(), s.c_str());
            value = true;
            break;
          }
        }

        if (!value)
        {
          for (auto s : labels) {
            if (MatchSimpleRegEx(items[i][1].c_str(), s.c_str())) {
              printf("\tFound volume with label \"%s\" matched \"%s\"\n", items[i][1].c_str(), s.c_str());
              value = true;
              break;
            }
          }
        }

        if (!value)
        {
          std::string vl = items[i][0] + std::string("=") + items[i][1];
          for (auto s : volumeslabels) {
            if (MatchSimpleRegEx(vl.c_str(), s.c_str())) {
              printf("\tFound volume with name and label \"%s\" matched \"%s\"\n", vl.c_str(), s.c_str());
              value = true;
              break;
            }
          }
        }

        FoundVolumes += (value ? 1 : 0);
        CheckListViewItem(listHwnd, i, value ? 1 : 0, m_prcInfo.hProcess);
      }

      printf("Found volumes: %zi\n", FoundVolumes);

      SetCheckboxState(vPCHwnd, enableVirtualPc);
      SetCheckboxState(vscHwnd, enableVolumeShadowCopy);
      SetCheckboxState(vhdxHwnd, enableVhdx);

      if (run) {
        if (FoundVolumes == 0) {
          printf("Error: Can't run. There is no volumes for backup\n");
          return 1;
        }
        //printf("Wait...\n");
        printf("Virtual Disk Creation Started\n");
        ClickDefault(mainHwnd);
        Sleep(500);
        while (!IsButtonEnabled(createHwnd)) {
          Sleep(500);
        }
        TerminateProcess(m_prcInfo.hProcess, 0);
        printf("Virtual Disk Creation Finished\n");
      }


      return 0;
    }
    else
    {
      printf("Error: disk2vhd64.exe Window not found!\n");
    }
  }
  else
  {
    printf("Error: Waiting for main window timeouted!\n");
  }

  return 1;
}
