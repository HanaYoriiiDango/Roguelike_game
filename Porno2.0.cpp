#include <windows.h>
#include <algorithm>
#include <wingdi.h> 
#include <vector>
#include <iostream>
#include <cmath>

using namespace std;

//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS) - ожидает wWinMain, а не main  ? Alt+251.
//configuration::advanced::character set - not set - могу обращаться к структурам c typedef ? Alt+0215
//linker::input::additional dependensies Msimg32.lib; Winmm.lib ?

struct { 
    HWND hwnd;
    HBITMAP hBack;
    HDC hdc, mem_dc;
    int width, height;

} window;

struct sprite {
    float x, y, speed, rad, dx, dy;
    int widht, height;
    bool active = true;
    HBITMAP hBitmap;

};

struct {
    float x, y;              
    float targetX, targetY;  
    float smooth = 0.30f;    // speed
} camera;

struct {
    bool action = false;

} game;

sprite Hero;
const int ZOOM = 4; 

void InitWindow() {

    RECT r;
    GetClientRect(window.hwnd, &r);

    window.width = r.right - r.left;
    window.height = r.bottom - r.top;

} 

void InitGame() {

    srand(time(nullptr));

    window.hBack = (HBITMAP)LoadImageW(NULL, L"test_map.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    Hero.hBitmap = (HBITMAP)LoadImageW(NULL, L"ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    Hero.widht = 40;
    Hero.height = 40;
    Hero.x = 1024 / 2.0f - Hero.widht / 2.0f;  // center map
    Hero.y = 1024 / 2.0f - Hero.height / 2.0f;
    Hero.speed = 20.0f;

}

int ScreenX(float worldX) { return (int)((worldX - camera.x) * ZOOM); }
int ScreenY(float worldY) { return (int)((worldY - camera.y) * ZOOM); }

void ShowSprite(int x, int y, int w, int h, HBITMAP hBitmap, bool transparent) {

    BITMAP bm;
    HDC memDC = CreateCompatibleDC(window.mem_dc);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

    if (hBitmap) {

        GetObject(hBitmap, sizeof(BITMAP), &bm);

        if (transparent) TransparentBlt(window.mem_dc, x, y, w, h, memDC, 0, 0, w, h, RGB(0, 0, 0));
        else StretchBlt(window.mem_dc, x, y, w, h, memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    }

    SelectObject(memDC, hOldBitmap);

    DeleteObject(hOldBitmap);
    DeleteDC(memDC);

}

void UpdateCamera() {

    const float viewW = window.width / ZOOM;
    const float viewH = window.height / ZOOM;

    camera.targetX = Hero.x + Hero.widht / 2.0f - viewW / 2.0f;
    camera.targetY = Hero.y + Hero.height / 2.0f - viewH / 2.0f;
    
    camera.x += (camera.targetX - camera.x) * camera.smooth;
    camera.y += (camera.targetY - camera.y) * camera.smooth;

    const float MAP_H = 1024, MAP_W = 1024;

    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x > MAP_W - viewW) camera.x = MAP_W - viewW;
    if (camera.y > MAP_H - viewH) camera.y = MAP_H - viewH;



}

void ShowMap() {

    HDC mapDC = CreateCompatibleDC(window.mem_dc);
    HBITMAP hOld = (HBITMAP)SelectObject(mapDC, window.hBack);

    SetStretchBltMode(window.mem_dc, COLORONCOLOR);

    int viewW = window.width / ZOOM;
    int viewH = window.height / ZOOM;

    if (window.hBack) {

        StretchBlt(window.mem_dc, 0, 0, window.width, window.height, mapDC, camera.x, camera.y, viewW, viewH, SRCCOPY);

    }

    SelectObject(mapDC, hOld);
    DeleteDC(mapDC);
}


void ShowObject() {

    ShowMap();
    //ShowSprite(0, 0, window.width, window.height, window.hBack, false);
    ShowSprite(ScreenX(Hero.x), ScreenY(Hero.y), Hero.widht, Hero.height, Hero.hBitmap, true);

}


void ShowGame() {

    window.mem_dc = CreateCompatibleDC(window.hdc);
    HBITMAP hMemBmp = CreateCompatibleBitmap(window.hdc, window.width, window.height);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(window.mem_dc, hMemBmp);

    ShowObject();
    BitBlt(window.hdc, 0, 0, window.width, window.height, window.mem_dc, 0, 0, SRCCOPY);

    SelectObject(window.mem_dc, hOldBmp);
    DeleteObject(hOldBmp);
    DeleteDC(window.mem_dc);
}

void ProcessInput(WPARAM wParam) {

    if (wParam == VK_ESCAPE) DestroyWindow(window.hwnd); // уничтожаем окно
    if (wParam == 'W') Hero.y -= Hero.speed;
    if (wParam == 'A') Hero.x -= Hero.speed;
    if (wParam == 'S') Hero.y += Hero.speed;
    if (wParam == 'D') Hero.x += Hero.speed;
    if (wParam == VK_SPACE) game.action = true;

}

void CheckWals() {

    Hero.x = min(Hero.x, 1024 - Hero.widht);
    Hero.x = max(Hero.x, 0);

    Hero.y = min(Hero.y, 1024 - Hero.height);
    Hero.y = max(Hero.y, 0); 


}

void ProcessGame() {

    UpdateCamera();
    CheckWals();
    //ProcessBall();
    

}

void ClearGame() {

    DeleteObject(Hero.hBitmap);
    DeleteObject(window.hBack);

}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // просто объявление функции
// так правильнее потому что компилятору в main нужно знать какой обработчик использовать для класса окна 

int APIENTRY wWinMain( // Точка входа с поддержкой юникода (WINAPI - для примеров в доках)
	_In_ HINSTANCE hInstance, // "паспорт" приложения 
	_In_opt_ HINSTANCE hPrevInstance, // дескриптор предыдущего экземпляра, в современных всегда NULL
	_In_ PWSTR pCmdLine, // командная строка приложения, LPWSTR - указатель на строку в формате юникода
	_In_ int nCmdShow // параметр с флагами, определяющими начальное состояние окна 

	// _In_ - система аннотаций SAL (Source Annotation Language). Документируют код,
	// явно показывая назначения параметров для компилятора
	// предотвращает краши и ускоряет разработку, т.к в случае чего компилятор сообщит об ошибке

) 
{ // 
    // Создаю класс окна
    const wchar_t* CLASS_NAME = L"Main";  // L префикс для широких символов
    WNDCLASSEX wc = {}; // вся структура для регистрации класса окна инициализирована нулями пппп

    // то что нужно настроить более явно пишу тут:
    wc.cbSize = sizeof(WNDCLASSEX); // размеры класса, так и не понял зачем
    wc.lpfnWndProc = WndProc; // какой обработчик испоьзовать
    wc.hInstance = hInstance; // дескриптор приложения
    wc.hCursor = NULL; // скрываю курсор
    wc.lpszClassName = CLASS_NAME;  // имя используемого класса

    // Регистрирую класс окна
    //RegisterClassEx(&wc); // хз нужна ли здесь проверка на создание класса через мэссэджбокс

    if (!RegisterClassEx(&wc)) { // оказалась нужна 
        MessageBox(NULL, L"Ошибка регистрации класса окна!", L"Ошибка", MB_ICONERROR);
        return 0;
    }

    window.width = GetSystemMetrics(SM_CXSCREEN); // эти переменные укажем в размер окна 
    window.height = GetSystemMetrics(SM_CYSCREEN);

    window.hwnd = CreateWindowEx
    (
        0, 
        CLASS_NAME, 
        L"Roguelike prototype", 
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        window.width, window.height, 
        NULL, NULL, hInstance, NULL
    );

    if (!window.hwnd) { // проверка на создание окна 

        MessageBox(
            NULL, // здесь дескриптор окна,NULL значит что нет родитльеского
            L"Ошибка создания окна!", // текст сообщения 
            L"Ошибка", // заголовк окна 
            MB_ICONERROR // флаг который задает отображение окна с кнопкой ок и иконокой ошибки
        );
        return 0; // возвращаем нольб, заверщшаем программу 

    }

    // эти функции идут всегда в паре и нужны чтобы управлять видимостью окна 
    // и оптимизировать процесс отрисовку
    // однако если использовать стиль WS_VISIBLE, то их применение не обязателно 
    InitWindow();

    ShowWindow(window.hwnd, SW_SHOW);
    UpdateWindow(window.hwnd);

    MSG msg = {}; // структур дял хранения сообщений  

    // GeMassage() извлекает сообщения из очереди 
    // цикл будет работать до тех пор пока в очередь не поступит WM_QUIT

    while (GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg); // обрабатывает ввод клавиш и преобразует его в сообщения 
        DispatchMessage(&msg); // передает сообщения в обработчик

    }
}

// Обработка сообщений
LRESULT CALLBACK WndProc(

    // LRESULT для возврата результата обработки сообщения
    // CALLBACK - соглашение о вызове функций 
    // "Когда придет посылка (событие), распакуй ее вот по этой схеме (функция CALLBACK)".
    // Система работает так же: когда происходит событие она ище инструкцию-обработчик и выполняет ее

    HWND hwnd, // дескриптор (идентификатьор) окна
    UINT msg, // числвой код события 
    WPARAM wParam, // доп инфа о сообщзениях
    LPARAM lParam // WPARAM (обработка клавиш), LPARAM (движение курсора) 

) {

    switch (msg) {

    case WM_CREATE: // здесь загрузка всех ресурсов при поздании окна 

        InitGame();
        ShowCursor(FALSE); // скрыл курсор
        SetTimer(hwnd, 1, 16, NULL);

        break;

    case WM_PAINT:  // здесь будет вся отричовка в окне  
    {   
        PAINTSTRUCT ps;
        window.hdc = BeginPaint(hwnd, &ps);
        ShowGame();
        EndPaint(hwnd, &ps);

    }

        break;

    case WM_TIMER: 
    {
        //GetCursorPos(&p);
        //ball.x = p.x;
        //ball.y = p.y;
        
        ProcessGame();
        InvalidateRect(hwnd, NULL, FALSE);
    }

        break;

    case WM_KEYDOWN: // обработка нажатий клавиш 

        ProcessInput(wParam);
        InvalidateRect(hwnd, NULL, TRUE);

        break;


    case WM_DESTROY: // при уничтожении окна посылаем сообщение WM_QUIT - завершает цикл сообщений. 

        ClearGame();
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        break;


    default: // здесь обработка всех сообщений по умолчанию, если мы не написали как их обрабюатывать
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
