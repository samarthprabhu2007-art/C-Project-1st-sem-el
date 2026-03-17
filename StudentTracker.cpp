#define _WIN32_IE 0x0400        
#define _WIN32_WINNT 0x0501     
#include <windows.h>
#include <commctrl.h>           
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <fstream> 
#include <stdio.h>              

#ifndef MCS_NOTODAY
#define MCS_NOTODAY 0x0004
#endif
#ifndef MCS_NOTODAYCIRCLE
#define MCS_NOTODAYCIRCLE 0x0008
#endif

#pragma comment(lib, "comctl32.lib")

using namespace std;

// --- 1. DATA STRUCTURES (UNTOUCHED) ---
struct Student {
    int id;
    string name, phone, parentPhone, age, dob, email, status;      
    string lastIn;  
    string lastOut; 
};

vector<Student> students;
bool isLoggedIn = false;
int editingIndex = -1; 

// --- 2. UI HANDLES ---
HWND hLoginLabel, hUserEdit, hPassEdit, hBtnLogin; 
HWND hDashTitle, hList, hCountLabel, hSearchEdit, hSearchLabel; 
HWND hBtnGoAdd, hBtnGoEdit, hBtnGoRemove, hBtnCheckIn, hBtnCheckOut, hBtnReset, hBtnLogout, hBtnBack;
HWND hEditID, hEditName, hEditPhone, hEditParent, hEditAge, hEditDOB, hEditEmail;
HWND hL1, hL2, hL3, hL4, hL5, hL6, hL7, hL8, hL9, hDateLabel; 
HWND hBtnSubmitAdd, hBtnSubmitUpdate, hBtnSaveExit;
HWND hCalendar; 

// --- 3. PREMIUM UI ASSETS ---
HFONT hFontMain, hFontTitle;
HBRUSH hBrushSidebar = CreateSolidBrush(RGB(10, 25, 47));   
HBRUSH hBrushWorkspace = CreateSolidBrush(RGB(30, 41, 59)); 
HBRUSH hBrushField = CreateSolidBrush(RGB(51, 65, 85));     
COLORREF colText = RGB(241, 245, 249);                      
COLORREF colAzure = RGB(14, 165, 233);                     

// --- 4. CORE LOGIC (UNTOUCHED) ---
void SortStudents() { sort(students.begin(), students.end(), [](const Student& a, const Student& b) { return a.id < b.id; }); }

void SaveData() {
    SortStudents();
    ofstream file("student_data.txt");
    if (file.is_open()) {
        file << students.size() << endl; 
        for (const auto& s : students) {
            file << s.id << "\n" << s.name << "\n" << s.phone << "\n" << s.parentPhone << "\n" << s.age << "\n" << s.dob << "\n" << s.email << "\n" << s.status << "\n" << (s.lastIn.empty() ? "N/A" : s.lastIn) << "\n" << (s.lastOut.empty() ? "N/A" : s.lastOut) << endl; 
        }
        file.close();
    }
}

void LoadData() {
    ifstream file("student_data.txt");
    if (file.is_open()) {
        int count; file >> count; if(count > 0) students.clear();
        for (int i = 0; i < count; i++) {
            Student s; file >> s.id; file.ignore(); getline(file, s.name); getline(file, s.phone); getline(file, s.parentPhone); getline(file, s.age); getline(file, s.dob); getline(file, s.email); getline(file, s.status); getline(file, s.lastIn); getline(file, s.lastOut);
            students.push_back(s);
        }
        file.close(); SortStudents();
    }
}

string GetRealTime() { time_t now = time(0); tm *ltm = localtime(&now); char buffer[80]; strftime(buffer, 80, "%I:%M %p", ltm); return string(buffer); }
string GetFullDateTime() { time_t now = time(0); tm *ltm = localtime(&now); char buffer[100]; strftime(buffer, 100, "%d/%m/%Y | %I:%M:%S %p", ltm); return string(buffer); }

void UpdateList() {
    SortStudents();
    char qBuf[50]; GetWindowText(hSearchEdit, qBuf, 50);
    string query = qBuf; transform(query.begin(), query.end(), query.begin(), ::tolower);
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    int pres = 0;
    for (const auto& s : students) {
        string n = s.name; transform(n.begin(), n.end(), n.begin(), ::tolower);
        if (query.empty() || n.find(query) != string::npos || to_string(s.id).find(query) != string::npos) {
            if (s.status == "Present") pres++;
            string row = "ID: " + to_string(s.id) + " | " + s.name + " (" + s.status + ") | IN: " + s.lastIn + " | OUT: " + s.lastOut;
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)row.c_str());
        }
    }
    SetWindowText(hCountLabel, ("TOTAL PRESENT: " + to_string(pres)).c_str());
}

void ShowPage(int page, HWND hwnd) {
    // FIXED: Massive array to ensure everything is hidden properly before showing new content
    HWND all[] = {hLoginLabel, hUserEdit, hPassEdit, hBtnLogin, hDashTitle, hList, hCountLabel, hSearchEdit, hSearchLabel, hBtnGoAdd, hBtnGoEdit, hBtnGoRemove, hBtnLogout, hBtnCheckIn, hBtnCheckOut, hBtnReset, hBtnBack, hEditID, hEditName, hEditPhone, hEditParent, hEditAge, hEditDOB, hEditEmail, hL1, hL2, hL3, hL4, hL5, hL6, hL7, hL8, hL9, hBtnSubmitAdd, hBtnSubmitUpdate, hBtnSaveExit, hDateLabel, hCalendar};
    for(HWND h : all) ShowWindow(h, SW_HIDE);
    
    if (page == -1) { ShowWindow(hLoginLabel, SW_SHOW); ShowWindow(hUserEdit, SW_SHOW); ShowWindow(hPassEdit, SW_SHOW); ShowWindow(hBtnLogin, SW_SHOW); }
    else {
        ShowWindow(hDashTitle, SW_SHOW); ShowWindow(hBtnLogout, SW_SHOW); ShowWindow(hBtnSaveExit, SW_SHOW);
        ShowWindow(hBtnGoAdd, SW_SHOW); ShowWindow(hBtnGoEdit, SW_SHOW); ShowWindow(hDateLabel, SW_SHOW); ShowWindow(hBtnCheckIn, SW_SHOW); ShowWindow(hBtnCheckOut, SW_SHOW); ShowWindow(hBtnGoRemove, SW_SHOW); ShowWindow(hBtnReset, SW_SHOW);
        if (page == 0) { ShowWindow(hSearchLabel, SW_SHOW); ShowWindow(hSearchEdit, SW_SHOW); ShowWindow(hList, SW_SHOW); ShowWindow(hCountLabel, SW_SHOW); UpdateList(); }
        else {
            ShowWindow(hBtnBack, SW_SHOW); ShowWindow(hL1, SW_SHOW); ShowWindow(hEditID, SW_SHOW); ShowWindow(hL2, SW_SHOW); ShowWindow(hEditName, SW_SHOW); ShowWindow(hL3, SW_SHOW); ShowWindow(hEditPhone, SW_SHOW); ShowWindow(hL4, SW_SHOW); ShowWindow(hEditParent, SW_SHOW); ShowWindow(hL5, SW_SHOW); ShowWindow(hEditAge, SW_SHOW); ShowWindow(hL6, SW_SHOW); ShowWindow(hEditDOB, SW_SHOW); ShowWindow(hL7, SW_SHOW); ShowWindow(hEditEmail, SW_SHOW);
            if(page == 1 || page == 2) { 
                HWND inputs[] = {hEditID, hEditName, hEditPhone, hEditParent, hEditAge, hEditEmail}; 
                for(HWND h : inputs) SendMessage(h, EM_SETREADONLY, FALSE, 0); 
                SendMessage(hEditDOB, EM_SETREADONLY, TRUE, 0); 
                if(page == 1) ShowWindow(hBtnSubmitAdd, SW_SHOW); else ShowWindow(hBtnSubmitUpdate, SW_SHOW); 
            }
            else if(page == 4) { ShowWindow(hL8, SW_SHOW); ShowWindow(hL9, SW_SHOW); HWND inputs[] = {hEditID, hEditName, hEditPhone, hEditParent, hEditAge, hEditDOB, hEditEmail}; for(HWND h : inputs) SendMessage(h, EM_SETREADONLY, TRUE, 0); }
        }
    }
    InvalidateRect(hwnd, NULL, TRUE); // Forces the window to repaint and clear old graphics
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam; SetBkMode(hdc, TRANSPARENT);
            if ((HWND)lParam == hEditDOB) { SetTextColor(hdc, RGB(255,255,255)); return (INT_PTR)hBrushField; }
            if ((HWND)lParam == hDateLabel) SetTextColor(hdc, colAzure); 
            else if ((HWND)lParam == hDashTitle) { SetTextColor(hdc, RGB(255,255,255)); SelectObject(hdc, hFontTitle); } 
            else SetTextColor(hdc, colText); 
            return (INT_PTR)hBrushWorkspace; 
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX: { HDC hdc = (HDC)wParam; SetTextColor(hdc, RGB(255,255,255)); SetBkMode(hdc, TRANSPARENT); return (INT_PTR)hBrushField; }
        case WM_CREATE: {
            LoadData(); int w = GetSystemMetrics(SM_CXSCREEN), h = GetSystemMetrics(SM_CYSCREEN), cx = w/2;
            hFontMain = CreateFont(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hFontTitle = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hLoginLabel = CreateWindowEx(0, "STATIC", "ADMIN LOGIN", WS_CHILD | SS_CENTER, cx-150, 200, 300, 40, hwnd, NULL, NULL, NULL);
            hUserEdit = CreateWindowEx(0, "EDIT", "admin", WS_CHILD | WS_BORDER | ES_CENTER, cx-150, 250, 300, 35, hwnd, NULL, NULL, NULL);
            hPassEdit = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_BORDER | ES_PASSWORD | ES_CENTER, cx-150, 300, 300, 35, hwnd, NULL, NULL, NULL);
            hBtnLogin = CreateWindowEx(0, "BUTTON", "LOGIN", WS_CHILD, cx-150, 355, 300, 45, hwnd, (HMENU)1, NULL, NULL);
            hDateLabel = CreateWindowEx(0, "STATIC", "", WS_CHILD | SS_RIGHT, w-400, 20, 380, 30, hwnd, NULL, NULL, NULL);
            SetTimer(hwnd, 99, 1000, NULL);
            int sy = 120, sh = 50, sp = 60;
            hBtnGoAdd = CreateWindowEx(0, "BUTTON", "REGISTER", WS_CHILD, 30, sy, 190, sh, hwnd, (HMENU)101, NULL, NULL);
            hBtnGoEdit = CreateWindowEx(0, "BUTTON", "EDIT", WS_CHILD, 30, sy+sp, 190, sh, hwnd, (HMENU)102, NULL, NULL);
            hBtnCheckIn = CreateWindowEx(0, "BUTTON", "CHECK IN", WS_CHILD, 30, sy+sp*2, 190, sh, hwnd, (HMENU)104, NULL, NULL);
            hBtnCheckOut = CreateWindowEx(0, "BUTTON", "CHECK OUT", WS_CHILD, 30, sy+sp*3, 190, sh, hwnd, (HMENU)105, NULL, NULL);
            hBtnGoRemove = CreateWindowEx(0, "BUTTON", "REMOVE", WS_CHILD, 30, sy+sp*4, 190, sh, hwnd, (HMENU)103, NULL, NULL);
            hBtnReset = CreateWindowEx(0, "BUTTON", "RESET DB", WS_CHILD, 30, sy+sp*5, 190, sh, hwnd, (HMENU)106, NULL, NULL);
            hBtnLogout = CreateWindowEx(0, "BUTTON", "LOGOUT", WS_CHILD, 30, h-160, 190, sh, hwnd, (HMENU)6, NULL, NULL);
            hBtnSaveExit = CreateWindowEx(0, "BUTTON", "SAVE & EXIT", WS_CHILD, 30, h-95, 190, sh, hwnd, (HMENU)7, NULL, NULL);
            hDashTitle = CreateWindowEx(0, "STATIC", "STUDENT MANAGEMENT SYSTEM", WS_CHILD | SS_CENTER, 270, 20, w-290, 50, hwnd, NULL, NULL, NULL);
            hSearchLabel = CreateWindowEx(0, "STATIC", "SEARCH ID:", WS_CHILD, 270, 90, 120, 30, hwnd, NULL, NULL, NULL);
            hSearchEdit = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_BORDER, 395, 85, 300, 35, hwnd, (HMENU)11, NULL, NULL);
            hList = CreateWindowEx(0, "LISTBOX", NULL, WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS, 270, 140, w-320, h-400, hwnd, (HMENU)50, NULL, NULL);
            hCountLabel = CreateWindowEx(0, "STATIC", "PRESENT: 0", WS_CHILD, 270, h-200, 400, 30, hwnd, NULL, NULL, NULL);
            int fx = 320, fy = 150, lw = 160, ew = 450, fh = 35, fs = 50;
            hL1 = CreateWindowEx(0, "STATIC", "ID:", WS_CHILD, fx, fy, lw, fh, hwnd, NULL, NULL, NULL);
            hEditID = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_BORDER, fx+lw, fy, ew, fh, hwnd, NULL, NULL, NULL);
            hL2 = CreateWindowEx(0, "STATIC", "NAME:", WS_CHILD, fx, fy+fs, lw, fh, hwnd, NULL, NULL, NULL);
            hEditName = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_BORDER, fx+lw, fy+fs, ew, fh, hwnd, NULL, NULL, NULL);
            hL3 = CreateWindowEx(0, "STATIC", "PHONE:", WS_CHILD, fx, fy+fs*2, lw, fh, hwnd, NULL, NULL, NULL);
            hEditPhone = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_BORDER, fx+lw, fy+fs*2, ew, fh, hwnd, NULL, NULL, NULL);
            hL4 = CreateWindowEx(0, "STATIC", "PARENT:", WS_CHILD, fx, fy+fs*3, lw, fh, hwnd, NULL, NULL, NULL);
            hEditParent = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_BORDER, fx+lw, fy+fs*3, ew, fh, hwnd, NULL, NULL, NULL);
            hL5 = CreateWindowEx(0, "STATIC", "AGE:", WS_CHILD, fx, fy+fs*4, lw, fh, hwnd, NULL, NULL, NULL);
            hEditAge = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_BORDER, fx+lw, fy+fs*4, ew, fh, hwnd, NULL, NULL, NULL);
            hL6 = CreateWindowEx(0, "STATIC", "DOB (Click Box):", WS_CHILD, fx, fy+fs*5, lw, fh, hwnd, NULL, NULL, NULL);
            hEditDOB = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_BORDER | ES_READONLY, fx+lw, fy+fs*5, ew, fh, hwnd, NULL, NULL, NULL);
            hL7 = CreateWindowEx(0, "STATIC", "GMAIL:", WS_CHILD, fx, fy+fs*6, lw, fh, hwnd, NULL, NULL, NULL);
            hEditEmail = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_BORDER, fx+lw, fy+fs*6, ew, fh, hwnd, NULL, NULL, NULL);
            hCalendar = CreateWindowEx(0, MONTHCAL_CLASS, NULL, WS_CHILD | WS_BORDER | MCS_NOTODAY | MCS_NOTODAYCIRCLE, fx+lw+ew+20, fy+fs*2, 230, 160, hwnd, NULL, NULL, NULL);
            hL8 = CreateWindowEx(0, "STATIC", "LAST IN: N/A", WS_CHILD, fx, fy+fs*7, lw+ew, fh, hwnd, NULL, NULL, NULL);
            hL9 = CreateWindowEx(0, "STATIC", "LAST OUT: N/A", WS_CHILD, fx, fy+fs*8, lw+ew, fh, hwnd, NULL, NULL, NULL);
            hBtnSubmitAdd = CreateWindowEx(0, "BUTTON", "SAVE NEW", WS_CHILD, fx, fy+fs*9, lw+ew, 55, hwnd, (HMENU)201, NULL, NULL);
            hBtnSubmitUpdate = CreateWindowEx(0, "BUTTON", "UPDATE", WS_CHILD, fx, fy+fs*9, lw+ew, 55, hwnd, (HMENU)202, NULL, NULL);
            hBtnBack = CreateWindowEx(0, "BUTTON", "BACK", WS_CHILD, 270, 76, 100, 32, hwnd, (HMENU)100, NULL, NULL);
            HWND hArr[] = {hLoginLabel, hUserEdit, hPassEdit, hBtnLogin, hDashTitle, hList, hCountLabel, hSearchEdit, hSearchLabel, hBtnGoAdd, hBtnGoEdit, hBtnGoRemove, hBtnLogout, hBtnCheckIn, hBtnCheckOut, hBtnReset, hBtnLogout, hBtnSaveExit, hEditID, hEditName, hEditPhone, hEditParent, hEditAge, hEditDOB, hEditEmail, hL1, hL2, hL3, hL4, hL5, hL6, hL7, hL8, hL9, hBtnSubmitAdd, hBtnSubmitUpdate, hBtnBack, hDateLabel};
            for(HWND hnd : hArr) SendMessage(hnd, WM_SETFONT, (WPARAM)hFontMain, TRUE);
            ShowPage(-1, hwnd); break;
        }
        case WM_COMMAND: {
            int a = LOWORD(wParam); int n = HIWORD(wParam);
            if (a == 50 && n == LBN_DBLCLK) {
                int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    char buf[200]; SendMessage(hList, LB_GETTEXT, sel, (LPARAM)buf); string sBuf = buf; int id = stoi(sBuf.substr(4, sBuf.find(" |") - 4));
                    for(auto &s : students) if(s.id == id) { SetWindowText(hEditID, to_string(s.id).c_str()); SetWindowText(hEditName, s.name.c_str()); SetWindowText(hEditPhone, s.phone.c_str()); SetWindowText(hEditParent, s.parentPhone.c_str()); SetWindowText(hEditAge, s.age.c_str()); SetWindowText(hEditDOB, s.dob.c_str()); SetWindowText(hEditEmail, s.email.c_str()); SetWindowText(hL8, ("LAST IN: " + s.lastIn).c_str()); SetWindowText(hL9, ("LAST OUT: " + s.lastOut).c_str()); ShowPage(4, hwnd); break; }
                }
            }
            if (lParam == (LPARAM)hEditDOB && n == EN_SETFOCUS) ShowWindow(hCalendar, SW_SHOW);
            if (a == 1) { char p[50]; GetWindowText(hPassEdit, p, 50); if (string(p) == "admin123") { isLoggedIn = true; ShowPage(0, hwnd); } else MessageBox(hwnd, "Wrong", "Error", MB_OK); }
            if (a == 11 && n == EN_CHANGE) UpdateList();
            if (isLoggedIn) {
                if (a == 100) ShowPage(0, hwnd); if (a == 6) { SaveData(); isLoggedIn = false; ShowPage(-1, hwnd); } if (a == 7) { SaveData(); PostQuitMessage(0); }
                if (a == 101) { SetWindowText(hEditID, ""); SetWindowText(hEditName, ""); SetWindowText(hEditPhone, ""); SetWindowText(hEditParent, ""); SetWindowText(hEditAge, ""); SetWindowText(hEditDOB, ""); SetWindowText(hEditEmail, ""); ShowPage(1, hwnd); ShowWindow(hCalendar, SW_HIDE); }
                if (a == 102) { char idB[20]; GetWindowText(hSearchEdit, idB, 20); editingIndex = -1; for(int i=0; i<students.size(); i++) if(to_string(students[i].id) == idB) editingIndex = i; if(editingIndex != -1) { SetWindowText(hEditID, to_string(students[editingIndex].id).c_str()); SetWindowText(hEditName, students[editingIndex].name.c_str()); SetWindowText(hEditPhone, students[editingIndex].phone.c_str()); SetWindowText(hEditParent, students[editingIndex].parentPhone.c_str()); SetWindowText(hEditAge, students[editingIndex].age.c_str()); SetWindowText(hEditDOB, students[editingIndex].dob.c_str()); SetWindowText(hEditEmail, students[editingIndex].email.c_str()); ShowPage(2, hwnd); ShowWindow(hCalendar, SW_HIDE); } else MessageBox(hwnd, "Locate ID first", "Note", MB_OK); }
                if (a == 103) { char idB[20]; GetWindowText(hSearchEdit, idB, 20); if(MessageBox(hwnd, "Delete?", "Confirm", MB_YESNO) == IDYES) { for(auto it = students.begin(); it != students.end(); ++it) if(to_string(it->id) == idB) { students.erase(it); SaveData(); UpdateList(); break; } } }
                if (a == 104) { char idB[20]; GetWindowText(hSearchEdit, idB, 20); for(auto &s : students) if(to_string(s.id) == idB) { if(s.status == "Present") MessageBox(hwnd, "Already present!", "Error", MB_OK); else { s.status = "Present"; s.lastIn = GetRealTime(); SaveData(); UpdateList(); } } }
                if (a == 105) { char idB[20]; GetWindowText(hSearchEdit, idB, 20); for(auto &s : students) if(to_string(s.id) == idB) { if(s.status == "Checked Out" || s.status == "Registered") MessageBox(hwnd, "Already out!", "Error", MB_OK); else { s.status = "Checked Out"; s.lastOut = GetRealTime(); SaveData(); UpdateList(); } } }
                if (a == 106) { if(MessageBox(hwnd,"Wipe DB?","Warning",MB_YESNO)==IDYES){students.clear();SaveData();UpdateList();}}
                if (a == 201 || a == 202) { char idS[20], nm[50], ph[20], pph[20], ag[10], db[20], em[50]; GetWindowText(hEditID, idS, 20); GetWindowText(hEditName, nm, 50); GetWindowText(hEditPhone, ph, 20); GetWindowText(hEditParent, pph, 20); GetWindowText(hEditAge, ag, 10); GetWindowText(hEditDOB, db, 20); GetWindowText(hEditEmail, em, 50); if(strlen(idS)==0 || strlen(nm)==0 || strlen(ph)==0 || strlen(pph)==0 || strlen(ag)==0 || strlen(db)==0 || strlen(em)==0) MessageBox(hwnd, "Fill ALL boxes!", "Error", MB_OK); else { int nID = stoi(idS); bool dup = false; for (int i = 0; i < (int)students.size(); i++) if (students[i].id == nID && (a == 201 || i != editingIndex)) { dup = true; break; } if (dup) MessageBox(hwnd, "Duplicate ID!", "Error", MB_OK); else { if (a == 201) students.push_back({nID, nm, ph, pph, ag, db, em, "Registered", "N/A", "N/A"}); else { students[editingIndex].id = nID; students[editingIndex].name=nm; students[editingIndex].phone=ph; students[editingIndex].parentPhone=pph; students[editingIndex].age=ag; students[editingIndex].dob=db; students[editingIndex].email=em; } SaveData(); ShowPage(0, hwnd); } } }
            } break; }
        case WM_NOTIFY: {
            LPNMHDR lpnmhdr = (LPNMHDR)lParam;
            if (lpnmhdr->hwndFrom == hCalendar && lpnmhdr->code == MCN_SELECT) {
                SYSTEMTIME st; SendMessage(hCalendar, MCM_GETCURSEL, 0, (LPARAM)&st);
                char dateBuf[20]; sprintf(dateBuf, "%02d/%02d/%d", st.wDay, st.wMonth, st.wYear);
                SetWindowText(hEditDOB, dateBuf); ShowWindow(hCalendar, SW_HIDE);
            }
            break;
        }
        case WM_TIMER: { if (wParam == 99) SetWindowText(hDateLabel, GetFullDateTime().c_str()); break; }
        case WM_DRAWITEM: { LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam; if (pdis->itemID == -1) return 0; char text[256]; SendMessage(pdis->hwndItem, LB_GETTEXT, pdis->itemID, (LPARAM)text); FillRect(pdis->hDC, &pdis->rcItem, (pdis->itemState & ODS_SELECTED) ? hBrushField : hBrushWorkspace); SetTextColor(pdis->hDC, (pdis->itemState & ODS_SELECTED) ? colAzure : colText); TextOut(pdis->hDC, pdis->rcItem.left + 10, pdis->rcItem.top + 5, text, strlen(text)); return TRUE; }
        case WM_MEASUREITEM: { LPMEASUREITEMSTRUCT pmis = (LPMEASUREITEMSTRUCT)lParam; pmis->itemHeight = 35; return TRUE; }
        case WM_PAINT: { PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps); RECT sb = {0, 0, 250, GetSystemMetrics(SM_CYSCREEN)}; FillRect(hdc, &sb, hBrushSidebar); EndPaint(hwnd, &ps); break; }
        case WM_DESTROY: SaveData(); PostQuitMessage(0); return 0;
    } return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hI, HINSTANCE hP, LPSTR lp, int nS) {
    INITCOMMONCONTROLSEX icex; icex.dwSize = sizeof(icex); icex.dwICC = ICC_DATE_CLASSES; InitCommonControlsEx(&icex); 
    WNDCLASS wc = {0}; wc.lpfnWndProc = WindowProc; wc.hInstance = hI; wc.lpszClassName = "FinalV41"; wc.hCursor = LoadCursor(NULL, IDC_ARROW); wc.hbrBackground = hBrushWorkspace; RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, "FinalV41", "Student Tracker Pro v41", WS_POPUP | WS_VISIBLE, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hI, NULL);
    MSG msg; while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    return 0;
}