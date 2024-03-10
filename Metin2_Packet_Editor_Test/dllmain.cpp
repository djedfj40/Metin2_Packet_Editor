#include <pch.h>
#include <Windows.h>
#include <string>
#include <thread>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>
#include <Psapi.h>

// Global variables
using namespace std;
HINSTANCE hInstance;
HWND hMainWnd, hBigTextBox, hSmallTextBox, hSendButton, hClearButton;
int sequenceNumber = 1;
HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Calibri"); // Set font for our textboxes
wstring text;  // Text that will be written in the big text box
DWORD esi_address = 0x0;
int gate_keeper_flag = 0;
int packet_length = 0;

// Jmp placement addresses
DWORD HOOK_ADDRESS = 0x0046C7D4; // You must adjust this for your version of metin2 (This is the hook address)
DWORD jmpbackaddress = HOOK_ADDRESS + 5;

string convert(wstring buffer) { // Remove the blanks from the user given buffer data
    string str = string(buffer.begin(), buffer.end());
    string result = "";
    for (char c : str) {
        if (c != ' ') {
             result += c;
        }
    }
    return result;
}

typedef void(__fastcall* example_function_ptr)(int);

void send_packet(wstring buffer) { // This function will call the send function and write buffer data

    string converted_str = convert(buffer);
    //unsigned int data[] = { 0x0 };
    int size = converted_str.length();
    vector<char> data(size, 0);

    int counter = 0;
    for (int i = 0; i <= converted_str.length()/2;i++) {
        std::string hexStr = converted_str.substr(i+counter, 2);
        std::istringstream iss(hexStr);
        int hexValue;
        iss >> std::hex >> hexValue;
        counter++;
        data[i] = hexValue;
   }
    counter = 0;

    int dataSize = data.size(); //packet buffer size
    int prediction = dataSize * 2 + 1;
    dataSize = ((prediction - dataSize) - 2) / 2; // Make sure buffer size matches the buffer data size
  
    example_function_ptr func = (example_function_ptr)(HOOK_ADDRESS-0x4); // packet send function 
    __asm {
        mov esi, esi_address;
        mov eax, esi
        add eax, 0x34 // esi+0x34 packet buffer size 
        mov esi, dataSize
        mov[eax], esi
    }

    DWORD writeThis = esi_address + 0x2c; // esi + 0x2c  packet buffer address
    unsigned char** toWrite = reinterpret_cast<unsigned char**>(writeThis);
    unsigned char* writeAddress = reinterpret_cast<unsigned char*>(*toWrite);

    for (int i = 0; i <= dataSize; i++) { // write packet buffer
        writeAddress[i] = data[i];

    }
 

}


void CapturePacket() { // capturing and writing sent packets to big text box
    DWORD buffer_address = 0x0;
    unsigned char** toWrite;
    unsigned char* writeAddress;
    unsigned char data[1024] = { };
    wstring buffer_data;
    int send_check = 0;
    while (true) {
        if (gate_keeper_flag == 1) {  // When a packet is sent
            buffer_address = esi_address + 0x2c;
            toWrite = reinterpret_cast<unsigned char**>(buffer_address);
            writeAddress = reinterpret_cast<unsigned char*>(*toWrite);
            for (int i = 0; i <= packet_length; i++) { // read packet buffer
                data[i] = writeAddress[i];

            }
            for (int i = 0; i < packet_length; ++i) {
                std::wstringstream ss;
                ss << std::uppercase << std::hex << std::setw(2) << std::setfill(L'0') << static_cast<int>(data[i]);
                ss << L" ";
                buffer_data += ss.str();

                if (i < sizeof(packet_length) - 1) {
                    buffer_data += L" ";
                }
            }

            int len = GetWindowTextLengthW(hSmallTextBox);

            wstring sequenceText = L"send-" + to_wstring(sequenceNumber++) + L": ";

            // Writing the sent data to big text box
            int bigTextBoxLen = GetWindowTextLengthW(hBigTextBox);
            SendMessageW(hBigTextBox, EM_SETSEL, bigTextBoxLen, bigTextBoxLen);
            SendMessageW(hBigTextBox, EM_REPLACESEL, FALSE, (LPARAM)sequenceText.c_str());
            SendMessageW(hBigTextBox, EM_SETSEL, bigTextBoxLen + sequenceText.length(), bigTextBoxLen + sequenceText.length());
            SendMessageW(hBigTextBox, EM_REPLACESEL, FALSE, (LPARAM)buffer_data.c_str());
            SendMessageW(hBigTextBox, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n\r\n");

            gate_keeper_flag = 0;
            buffer_data = L""; // resetting the buffer data after writing


        }

        void send_func(); {
            if (send_check == 1) {
                unsigned char data1[] = { 0x0D, 0x01, 0x03, 0x00, 0x01, 0x04, 0x00, 0x01, 0x00 }; // packet buffer
                int dataSize = sizeof(data1); //packet buffer size
                //int esi_address = 0x072BE940; // esi address

                example_function_ptr func = (example_function_ptr)0x006DC7D0; // packet send function 
                __asm {
                    mov esi, esi_address;
                    mov eax, esi
                        add eax, 0x34 // esi+0x34 packet buffer size 
                        mov esi, dataSize
                        mov[eax], esi
                }

                DWORD writeThis1 = esi_address + 0x2c; // esi + 0x2c  packet buffer address
                unsigned char** toWrite1 = reinterpret_cast<unsigned char**>(writeThis1);
                unsigned char* writeAddress1 = reinterpret_cast<unsigned char*>(*toWrite1);

                for (int i = 0; i <= dataSize; i++) { // write packet buffer
                    writeAddress1[i] = data1[i];

                }

            }//if
            send_check = 0;
        }


    }
}//


// Window Defining Procedure
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:

        // Create big text box
        hBigTextBox = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
            10, 10, 870, 400, hWnd, NULL, hInstance, NULL);

        // Create small text box
        hSmallTextBox = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            10, 420, 740, 25, hWnd, NULL, hInstance, NULL);

        // Create Send button
        hSendButton = CreateWindow(L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            760, 420, 60, 25, hWnd, (HMENU)1, hInstance, NULL);

        // Create Clear button
        hClearButton = CreateWindow(L"BUTTON", L"Clear", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            830, 420, 70, 25, hWnd, (HMENU)2, hInstance, NULL);

        SendMessage(hBigTextBox, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hSmallTextBox, WM_SETFONT, (WPARAM)hFont, TRUE);
        break;
    
    case WM_COMMAND:

        if (HIWORD(wParam) == BN_CLICKED) {  // When send button is clicked
            if ((HWND)lParam == hSendButton) {
                // Get the length of the text in the small text box
                int len = GetWindowTextLengthW(hSmallTextBox);
                if (len > 0) { // Allocate memory for the text
                   
                    text.resize(len + 1);
                    
                    // Get the text from the small text box

                    GetWindowTextW(hSmallTextBox, &text[0], len + 1);

                    send_packet(text); // Call send packet function
                }
            }
            else if ((HWND)lParam == hClearButton) { // Clear the big text box

                SetWindowTextW(hBigTextBox, L"");
                sequenceNumber = 1;
            }
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


// Code detour
typedef void (*TargetFunction)(void);

// New assembly code after jmp operation
__declspec(naked) void MyNewCode() {
    __asm {
        mov eax, [ecx + 0x34]
        sub eax, [ecx + 0x38]
        mov esi_address, esi
        mov gate_keeper_flag, 0x1  // Flag for writing the buffer data once
        mov packet_length, eax
        jmp jmpbackaddress // Turn back so program will execute normally
        
    }


}


// Code detour
void ApplyDetour() {

    //HOOK_ADDRESS = 0x006DC7D4;
    // Target code block
    LPBYTE targetAddress = (LPBYTE)HOOK_ADDRESS;

    // Backup variables
    DWORD oldProtect;
    DWORD relativeJump;

    // Space for saving the old code (Not necessary if you don't want to restore to original)
    BYTE originalBytes[5];

    // Copy first 5 bytes of the target code block
    memcpy(originalBytes, targetAddress, 5);

    // Get permission
    VirtualProtect(targetAddress, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

    // Place the jmp instruction
    relativeJump = (DWORD)((DWORD_PTR)MyNewCode - (DWORD_PTR)targetAddress - 5);
    targetAddress[0] = 0xE9;
    memcpy(targetAddress + 1, &relativeJump, sizeof(DWORD));

    // Restore the permissions after jmp
    VirtualProtect(targetAddress, 5, oldProtect, &oldProtect);
}

// Entry point for the DLL
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {

    
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        ApplyDetour();  // apply the detouring process when dll is injected
        // Save the instance handle
        hInstance = hinstDLL;

        // Register window class
        {
            WNDCLASS wc = { 0 };
            wc.lpfnWndProc = WindowProc;
            wc.hInstance = hInstance;
            wc.lpszClassName = L"MyDLLWindowClass";
            RegisterClass(&wc);
        }

        // Create the main window
        hMainWnd = (CreateWindowEx(0, L"MyDLLWindowClass", L"Packet Editor", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 940, 500, NULL, NULL, hInstance, NULL));

        // Show the window
        ShowWindow(hMainWnd, SW_SHOW);
        UpdateWindow(hMainWnd);

        thread(CapturePacket).detach(); // Create a new thread apart from the main thread
        break;
    case DLL_PROCESS_DETACH:
        // Clean up resources
        DestroyWindow(hMainWnd);
        UnregisterClass(L"MyDLLWindowClass", hInstance);
        break;
    }
    return TRUE;
}
